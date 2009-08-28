/*
Danger from the Deep - Open source submarine simulation
Copyright (C) 2003-2006  Thorsten Jordan, Luis Barrancos and others.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

// A 3d model
// (C)+(W) by Thorsten Jordan. See LICENSE

#include "model.h"

#ifdef WIN32
#undef min
#undef max
#endif

#include <cmath>
#include "dmath.h"
#include "plane.h"
#include "caustics.h"
#include "system.h"
#include "datadirs.h"
#include "oglext/OglExt.h"
#include "matrix4.h"
#include "binstream.h"
#include "xml.h"
#include "log.h"
#include <sstream>
#include <map>

using namespace std;

/* bounding sphere for a triangle: diameter is longest edge, chose center accordingly */

texture::mapping_mode model::mapping = texture::LINEAR_MIPMAP_LINEAR;//texture::NEAREST;

unsigned model::init_count = 0;

/*
fixme: possible cleanup/simplification of rendering EVERYWHERE:
0) maybe introduce a camera class that generates projection and camera modelview matrices.
1) reduce use of glActiveTexture, is rarely needed, now only for setting the
   right texture matrix, which can be surpassed by using uniforms and setting
   a matrix directly in the shader, not using the default texture matrices
   one intermediate step would be to make modelviewmatrix transform to
   world space, not eye space, and multiply the world space to eye space
   transform to the projection matrix. This would made clipping easier,
   as that is always done in world space. But we would need to use a special
   form of world space, one that makes viewer world origin, because of
   the high magnitues of coefficients we need to render.
   The downside of that method is that some gl entities would not expect
   this, e.g. light source positions would be wrong or would be needed to
   set by hand. But if all objects would use this modelviewmatrix, it wouldn't
   matter, so we can do it that way.
2) replace Matrix use (glPushMatrix/glPopMatrix, texture matrix switch etc.)
   at least replace texture matrix use. At the moment only matrix for unit #1
   is used to implement a clipping plane for non-hqsfx modes, this can be done
   in different other ways (arbitrary plane clipping as best)
3) maybe replace use of default attributes (would gain full OpenGl3.0 compatibility).
   But this would mean to use VBO only.
*/

auto_ptr<glsl_shader_setup> model::glsl_plastic;
auto_ptr<glsl_shader_setup> model::glsl_color;
auto_ptr<glsl_shader_setup> model::glsl_color_normal;
auto_ptr<glsl_shader_setup> model::glsl_color_normal_specular;
auto_ptr<glsl_shader_setup> model::glsl_color_normal_caustic;
auto_ptr<glsl_shader_setup> model::glsl_color_normal_specular_caustic;
auto_ptr<glsl_shader_setup> model::glsl_mirror_clip;
unsigned model::loc_c_tex_color;
unsigned model::loc_cn_tex_normal;
unsigned model::loc_cn_tex_color;
unsigned model::loc_cnc_tex_normal;
unsigned model::loc_cnc_tex_color;
unsigned model::loc_cnc_tex_caustic;
unsigned model::loc_cns_tex_normal;
unsigned model::loc_cns_tex_color;
unsigned model::loc_cns_tex_specular;
unsigned model::loc_cnsc_tex_normal;
unsigned model::loc_cnsc_tex_color;
unsigned model::loc_cnsc_tex_specular;
unsigned model::loc_cnsc_tex_caustic;
unsigned model::loc_mc_tex_color;


const std::string model::default_layout = "*default*";

bool model::object::set_angle(float ang)
{
	if (ang < rotat_angle_min) return false;
	if (ang > rotat_angle_max) return false;
	rotat_angle = ang;
	return true;
}



bool model::object::set_translation(float value)
{
	if (value < trans_val_min) return false;
	if (value > trans_val_max) return false;
	if (translation_constraint_axis == 0)
		translation.x = value;
	else if (translation_constraint_axis == 1)
		translation.y = value;
	else
		translation.z = value;
	return true;
}



model::object* model::object::find(unsigned id_)
{
	if (id == id_) return this;
	for (vector<object>::iterator it = children.begin(); it != children.end(); ++it) {
		object* obj = it->find(id_);
		if (obj) return obj;
	}
	return 0;
}



model::object* model::object::find(const std::string& name_)
{
	if (name == name_) return this;
	for (vector<object>::iterator it = children.begin(); it != children.end(); ++it) {
		object* obj = it->find(name_);
		if (obj) return obj;
	}
	return 0;
}



const model::object* model::object::find(unsigned id_) const
{
	if (id == id_) return this;
	for (vector<object>::const_iterator it = children.begin(); it != children.end(); ++it) {
		const object* obj = it->find(id_);
		if (obj) return obj;
	}
	return 0;
}



const model::object* model::object::find(const std::string& name_) const
{
	if (name == name_) return this;
	for (vector<object>::const_iterator it = children.begin(); it != children.end(); ++it) {
		const object* obj = it->find(name_);
		if (obj) return obj;
	}
	return 0;
}



void model::object::display(const texture *caustic_map) const
{
	glPushMatrix();
	glTranslated(translation.x, translation.y, translation.z);
	glRotated(rotat_angle, rotat_axis.x, rotat_axis.y, rotat_axis.z);
	if (mymesh) mymesh->display(caustic_map);
	for (vector<object>::const_iterator it = children.begin(); it != children.end(); ++it) {
		it->display(caustic_map);
	}
	glPopMatrix();
}



void model::object::display_mirror_clip() const
{
	// matrix mode is GL_MODELVIEW and active texture is GL_TEXTURE1 here
	glPushMatrix();
	glTranslated(translation.x, translation.y, translation.z);
	glRotated(rotat_angle, rotat_axis.x, rotat_axis.y, rotat_axis.z);

	if (mymesh) mymesh->display_mirror_clip();
	for (vector<object>::const_iterator it = children.begin(); it != children.end(); ++it) {
		it->display_mirror_clip();
	}

	glPopMatrix();
}



void model::object::compute_bounds(vector3f& min, vector3f& max, const matrix4f& transmat) const
{
	matrix4f mytransmat = transmat * get_transformation();
	// handle vertices of mymesh if present
	if (mymesh) {
		mymesh->compute_bounds(min, max, mytransmat);
	}
	// handle children
	for (vector<object>::const_iterator it = children.begin(); it != children.end(); ++it) {
		it->compute_bounds(min, max, mytransmat);
	}
}



matrix4f model::object::get_transformation() const
{
	return matrix4f::trans(translation) * quaternionf::rot(rotat_angle, rotat_axis).rotmat4();
}



void model::render_init()
{
	// initialize shaders
	//log_info("Using OpenGL GLSL shaders...");

	glsl_shader::defines_list dl;
	glsl_plastic.reset(new glsl_shader_setup(get_shader_dir() + "modelrender.vshader",
						 get_shader_dir() + "modelrender.fshader"));
	dl.push_back("USE_COLORMAP");
	glsl_color.reset(new glsl_shader_setup(get_shader_dir() + "modelrender.vshader",
					       get_shader_dir() + "modelrender.fshader"));
	dl.push_back("USE_NORMALMAP");
	glsl_color_normal.reset(new glsl_shader_setup(get_shader_dir() + "modelrender.vshader",
						      get_shader_dir() + "modelrender.fshader"));
	glsl_shader::defines_list dl2 = dl;
	dl.push_back("USE_SPECULARMAP");
	glsl_color_normal_specular.reset(new glsl_shader_setup(get_shader_dir() + "modelrender.vshader",
							       get_shader_dir() + "modelrender.fshader", dl));
	dl = dl2;
	dl.push_back("USE_CAUSTIC");
	glsl_color_normal_caustic.reset(new glsl_shader_setup(get_shader_dir() + "modelrender.vshader",
							      get_shader_dir() + "modelrender.fshader", dl));
	dl.push_back("USE_SPECULARMAP");
	glsl_color_normal_specular_caustic.reset(new glsl_shader_setup(get_shader_dir() + "modelrender.vshader",
								       get_shader_dir() + "modelrender.fshader", dl));
	dl = dl2;
	glsl_mirror_clip.reset(new glsl_shader_setup(get_shader_dir() + "modelrender_mirrorclip.vshader",
						     get_shader_dir() + "modelrender_mirrorclip.fshader", dl));
	// request uniform locations
	glsl_color->use();
	loc_c_tex_color = glsl_color->get_uniform_location("tex_color");
	glsl_color_normal->use();
	loc_cn_tex_normal = glsl_color_normal->get_uniform_location("tex_normal");
	loc_cn_tex_color = glsl_color_normal->get_uniform_location("tex_color");
	glsl_color_normal_caustic->use();
	loc_cnc_tex_normal = glsl_color_normal_caustic->get_uniform_location("tex_normal");
	loc_cnc_tex_color = glsl_color_normal_caustic->get_uniform_location("tex_color");
	loc_cnc_tex_caustic = glsl_color_normal_caustic->get_uniform_location("tex_caustic");
	glsl_color_normal_specular->use();
	loc_cns_tex_normal = glsl_color_normal_specular->get_uniform_location("tex_normal");
	loc_cns_tex_color = glsl_color_normal_specular->get_uniform_location("tex_color");
	loc_cns_tex_specular = glsl_color_normal_specular->get_uniform_location("tex_specular");
	glsl_color_normal_specular_caustic->use();
	loc_cnsc_tex_normal = glsl_color_normal_specular_caustic->get_uniform_location("tex_normal");
	loc_cnsc_tex_color = glsl_color_normal_specular_caustic->get_uniform_location("tex_color");
	loc_cnsc_tex_specular = glsl_color_normal_specular_caustic->get_uniform_location("tex_specular");
	loc_cnsc_tex_caustic = glsl_color_normal_specular_caustic->get_uniform_location("tex_caustic");
	glsl_mirror_clip->use();
	loc_mc_tex_color = glsl_mirror_clip->get_uniform_location("tex_color");
}



void model::render_deinit()
{
	glsl_color_normal.reset();
	glsl_color_normal_caustic.reset();
	glsl_color_normal_specular.reset();
	glsl_color_normal_specular_caustic.reset();
	glsl_mirror_clip.reset();
}



model::model()
{
	if (init_count == 0) render_init();
	++init_count;
}


model::model(const string& filename_, bool use_material)
	: filename(filename_),
	  scene(0xffffffff, "<scene>", 0)
{
	if (init_count == 0) render_init();
	++init_count;

	string::size_type st = filename.rfind(".");
	string extension = (st == string::npos) ? "" : filename.substr(st);
	for (unsigned e = 0; e < extension.length(); ++e)
		extension[e] = ::tolower(extension[e]);
	st = filename.rfind("/");  // we use the slash as path separator on ALL systems. C/C++ want it so.
	basepath = (st == string::npos) ? "" : filename.substr(0, st+1);
	basename = filename.substr(basepath.length(),
				   filename.length()-basepath.length()-extension.length());

	string filename2 = filename;
	FILE* ftest = fopen(filename2.c_str(), "rb");
	if (!ftest) {
		// try to load model from model dir
		filename2 = get_model_dir() + filename.substr(basepath.length());
		ftest = fopen(filename2.c_str(), "rb");
		if (!ftest) {
			throw error(string("could not open model file ") + filename2);
		}
	}
	fclose(ftest);

	// determine loader by extension here.
	if (extension == ".off") {
		read_off_file(filename2);
	} else if (extension == ".xml" || extension == ".ddxml") {
		read_dftd_model_file(filename2);
	} else {
		throw error(string("model: unknown extension or file format: ") + filename2);
	}

	// clear material info if requested
	if (!use_material) {
		for (vector<mesh*>::iterator it = meshes.begin(); it != meshes.end(); ++it)
			(*it)->mymaterial = 0;
		for (vector<material*>::iterator it = materials.begin(); it != materials.end(); ++it)
			delete *it;
		materials.clear();
	}

	compute_bounds();
	compute_normals();
	compile();

	// try to read physical data file, needs min/max data etc., so call it after
	// compute_bounds().
	read_phys_file(filename2);
}


model::~model()
{
	for (vector<model::mesh*>::iterator it = meshes.begin(); it != meshes.end(); ++it)
		delete *it;
	for (vector<model::material*>::iterator it = materials.begin(); it != materials.end(); ++it)
		delete *it;
	--init_count;
	if (init_count == 0) render_deinit();
}


void model::compute_bounds()
{
	//fixme: repace with bounding sphere hierarchy
	if (meshes.size() == 0) return;

	// could be done once... not here
	for (vector<model::mesh*>::iterator it = meshes.begin(); it != meshes.end(); ++it)
		(*it)->compute_vertex_bounds();

	// with an objectree, we need to iterate the meshes along the tree and handle
	// per-object translations, without object tree just handle all meshes
	min = vector3f(1e30, 1e30, 1e30);
	max = -min;
	if (!scene.children.empty()) {
		scene.compute_bounds(min, max, matrix4f::one());
	} else {
		for (vector<model::mesh*>::iterator it = meshes.begin(); it != meshes.end(); ++it)
			(*it)->compute_bounds(min, max, matrix4f::one());
	}

	boundsphere_radius = max.max(-min).length();
}



void model::compute_normals()
{
	for (vector<model::mesh*>::iterator it = meshes.begin(); it != meshes.end(); ++it) {
		(*it)->compute_normals();
	}
}



int model::mesh::gl_primitive_type() const
{
	switch (indices_type) {
	case pt_triangles:
		return GL_TRIANGLES;
	case pt_triangle_strip:
		return GL_TRIANGLE_STRIP;
	default:
		throw std::runtime_error("invalid primitive type for mesh!");
	}
}



const char* model::mesh::name_primitive_type() const
{
	switch (indices_type) {
	case pt_triangles:
		return "triangles";
	case pt_triangle_strip:
		return "triangle_strip";
	default:
		throw std::runtime_error("invalid primitive type for mesh!");
	}
}



std::auto_ptr<model::mesh::triangle_iterator> model::mesh::get_tri_iterator() const
{
	switch (indices_type) {
	case pt_triangles:
		return std::auto_ptr<triangle_iterator>(new triangle_iterator(indices));
	case pt_triangle_strip:
		return std::auto_ptr<triangle_iterator>(new triangle_strip_iterator(indices));
	default:
		throw std::runtime_error("invalid primitive type for mesh!");
	}
}



model::mesh::triangle_iterator::triangle_iterator(const std::vector<Uint32>& indices)
	: _i0(0), _i1(0), _i2(0), idx(indices), ptr(0)
{
	if (idx.size() < 3)
		throw std::invalid_argument("triangle_iterator: must have at least one triangle");
	_i0 = idx[0];
	_i1 = idx[1];
	_i2 = idx[2];
	ptr = 3;
}



bool model::mesh::triangle_iterator::next()
{
	if (ptr + 3 > idx.size()) return false;
	_i0 = idx[ptr];
	_i1 = idx[ptr+1];
	_i2 = idx[ptr+2];
	ptr += 3; // points on first index of next triangle
	return true;
}



model::mesh::triangle_strip_iterator::triangle_strip_iterator(const std::vector<Uint32>& indices)
	: triangle_iterator(indices)
{
	if (idx.size() < 3)
		throw std::invalid_argument("triangle_iterator: must have at least one triangle");
	_i0 = idx[0];
	_i1 = idx[1];
	_i2 = idx[2];
	ptr = 2; // points on index for next triangle
}



bool model::mesh::triangle_strip_iterator::next()
{
	if (ptr + 1 > idx.size()) return false;
	// scheme depends on ptr value
	// either n-2,n-1,n for even n or n-1,n-2,n for odd n.
	unsigned x = ptr & 1;
	_i0 = idx[ptr-2+x];
	_i1 = idx[ptr-1-x];
	_i2 = idx[ptr];
	++ptr;
	return true;
}



void model::mesh::compute_vertex_bounds()
{
	if (vertices.size() == 0) return;
	min = max = vertices[0];

	for (vector<vector3f>::iterator it2 = ++vertices.begin(); it2 != vertices.end(); ++it2) {
		min = it2->min(min);
		max = it2->max(max);
	}
}



void model::mesh::compute_bounds(vector3f& totmin, vector3f& totmax, const matrix4f& transmat)
{
	if (vertices.size() == 0) return;
	matrix4f mytransmat = transmat * transformation;
	for (vector<vector3f>::iterator it2 = vertices.begin(); it2 != vertices.end(); ++it2) {
		vector3f tmp = mytransmat * *it2;
		totmin = tmp.min(totmin);
		totmax = tmp.max(totmax);
	}
}



void model::mesh::compute_normals()
{
	// auto-detection of hard edges (creases) would be cool:
	// if the angle between faces at an edge is above a certain value,
	// the corners of the edge are duplicated and each instance gets their
	// own normals (like a mesh border), the same for vertices (cusps).
	// How to detect this: compute normals per face and adjacency information.
	// (For vertex cusps also vertex normals need to get computed).
	// If angle between normals (face to face or face to vertex) is higher than
	// treshold (e.g. 30 degrees) make a new instance of this vertex/edge for each
	// neighbour. Mark edges and vertexes if they are creases/cusps.
	// The corner vertices of an crease edge are also cusp vertices.
	// Adjacency information needed: face -> face, vertex -> faces

	// do not recompute normals if there are already some
	if (normals.size() != vertices.size()) {
		normals.clear();
		normals.resize(vertices.size());
		std::auto_ptr<triangle_iterator> tit(get_tri_iterator());
		do {
			const vector3f& v0 = vertices[tit->i0()];
			const vector3f& v1 = vertices[tit->i1()];
			const vector3f& v2 = vertices[tit->i2()];
			vector3f ortho = (v1-v0).orthogonal(v2-v0);
			// avoid degenerated triangles
			float lf = 1.0/ortho.length();
			// icc fix, doesn't like the other method, gcc does something wierd with isfinite()
			if ((fpclassify(lf) != FP_NAN && fpclassify(lf) != FP_INFINITE)) {
				vector3f face_normal = ortho * lf;
				//normals could be weighted by face area, that gives better results.
				normals[tit->i0()] += face_normal;
				normals[tit->i1()] += face_normal;
				normals[tit->i2()] += face_normal;
			}
		} while (tit->next());
		for (vector<vector3f>::iterator it = normals.begin(); it != normals.end(); ++it) {
			// this can lead to NAN values in vertex normals.
			// but only for degenerated vertices, so we don't care.
			it->normalize();
		}
	}
	
	// if we use normal mapping for this mesh, we need tangent values, too!
	// tangentsy get computed at runtime from normals and tangentsx
	// tangentsx are computed that way:
	// from each vertex we find a vector in positive u direction
	// and project it onto the plane given by the normal -> tangentx
	// because normal maps use stored texture coordinates (x = positive u!)
	if (mymaterial && mymaterial->normalmap.get()) {
		tangentsx.clear();
		tangentsx.resize(vertices.size(), vector3f(0, 0, 1));
		righthanded.clear();
		righthanded.resize(vertices.size(), 0);
		vector<bool> vertexok(vertices.size());
		std::auto_ptr<triangle_iterator> tit(get_tri_iterator());
		do {
			unsigned i0 = tit->i0();
			unsigned i1 = tit->i1();
			unsigned i2 = tit->i2();
			if (!vertexok[i0])
				vertexok[i0] = compute_tangentx(i0, i1, i2);
			if (!vertexok[i1])
				vertexok[i1] = compute_tangentx(i1, i2, i0);
			if (!vertexok[i2])
				vertexok[i2] = compute_tangentx(i2, i0, i1);
		} while (tit->next());
	}
}



bool model::mesh::compute_tangentx(unsigned i0, unsigned i1, unsigned i2)
{
	const vector2f& uv0 = texcoords[i0];
	const vector2f& uv1 = texcoords[i1];
	const vector2f& uv2 = texcoords[i2];
	const vector3f& n = normals[i0];
	vector2f d_uv0 = uv1 - uv0;
	vector2f d_uv1 = uv2 - uv0;
	float det = d_uv0.x * d_uv1.y - d_uv1.x * d_uv0.y;
	// dynamic limit for test against "zero"
	float med = (fabs(d_uv0.x) + fabs(d_uv0.y) + fabs(d_uv1.x) + fabs(d_uv1.y)) * 0.25;
	float eps = med * med * 0.01;
	//cout << "test " << d_uv0 << ", " << d_uv1 << ", med " << med << ", eps " << eps << "\n";
	if (fabsf(det) <= eps) {
		//find sane solution for this situation!
		//if delta_u is zero for d_uv0 and d_uv1, but delta_v is not, we could
		//compute tangentsy from v and tangentsx with the cross product
		//or we just don't store a tangentsx value and hope that the vertex
		//can be computed via another triangle
		//just hope and wait seems to work, at least one face adjacent to the vertex
		//should give sane tangent values.

		//cout << "tangent comp failed for i0 " << i0 << ", uv0 " << d_uv0 << ", uv1 " << d_uv1 << ", det " << det << "\n";
		return false;
	} else {
		vector3f v01 = vertices[i1] - vertices[i0];
		vector3f v02 = vertices[i2] - vertices[i0];
		// compute tangentx
		float a = d_uv1.y/det;
		float b = -d_uv0.y/det;
		vector3f rx = v01 * a + v02 * b;
		tangentsx[i0] = (rx - (rx * n) * n).normal();

//		cout << "tangent * n " << i0 << ", " << tangentsx[i0] * n << "\n";

		// compute tangent y
		float c = -d_uv1.x/det;
		float d = d_uv0.x/det;
		vector3f ry = v01 * c + v02 * d;
		vector3f tangentsy = (ry - (ry * n) * n).normal();
		float g = tangentsx[i0].cross(tangentsy) * n;
		// fixme: untersuche, wie righthanded-info in den shadern gebraucht wird. kann man denn stattdessen nicht einfach
		// die normale umdrehen? n?ann geht der Rest nicht mehr. Wie viele lefthanded-Koordinaten gibt es denn so?
		// was sagt Luis dazu?
		// das wird sich aber wohl nicht vermeiden lassen?
		// tangenty wird nicht uebertragen, aber wenn man tangentx umdreht, geht das auch?!
		// righthanded aber erst versuchen zu entfernen, wenn der rest geht!!!
		// der mix aus zwei Koordinatensystemen kann eigentlich nicht gut gehen.
		// wird aber vermutlich gebraucht um nur eine Seitenansicht fuer beide Rumpfseiten zu nehmen, aber das macht
		// Luis ja gar nicht...
		righthanded[i0] = !(g > 0); // fixme: negation seems needed???
		return true;
	}
}



model::mesh::mesh(const string& nm)
	: name(nm),
	  transformation(matrix4f::one()),
	  mymaterial(0),
	  vbo_positions(false),
	  vbo_normals(false),
	  vbo_texcoords(false),
	  vbo_tangents_righthanded(false),
	  vbo_colors(false),
	  index_data(true),
	  vertex_attrib_index(0),
	  inertia_tensor(matrix3::one())
{
	set_indices_type(pt_triangles);
}



model::mesh::mesh(unsigned w, unsigned h, const std::vector<float>& heights, const vector3f& scales,
		  const vector3f& trans,
		  const std::string& nm)
	: name(nm),
	  transformation(matrix4f::one()),
	  mymaterial(0),
	  vbo_positions(false),
	  vbo_normals(false),
	  vbo_texcoords(false),
	  vbo_tangents_righthanded(false),
	  vbo_colors(false),
	  index_data(true),
	  vertex_attrib_index(0)
{
	set_indices_type(pt_triangle_strip);
	if (w < 2 || h < 2 || heights.size() != w * h)
		throw std::invalid_argument("height field size invalid");

	// fill in vertices, texcoords
	vertices.reserve(heights.size());
	texcoords.reserve(heights.size());
	const float rw = w;
	const float rh = h;
	for (unsigned y = 0; y < h; ++y) {
		for (unsigned x = 0; x < w; ++x) {
			vertices.push_back(vector3f(float(x)-rw*0.5f, float(y)-rh*0.5f, heights[y*w+x]).coeff_mul(scales) + trans);
			texcoords.push_back(vector2f(float(x)/(w-1), float(y)/(h-1)));
		}
	}

	// generate indices.
	// it is better to build columns of 16 or 32 quads each, to make use of
	// the 16- or 32-sized vertex cache of GPUs.
	// it helps a bit.
	const unsigned column_width = 32;
	const unsigned columns = (w < column_width) ? 1 : w / column_width;
	const unsigned w_total = w;
	unsigned w_off = 0;
	w = w_total / columns + 1;
	// per line w quads, so *2 tri's, plus 2 degenerated.
	// remove 2 last degenerated of last line.
	indices.reserve((h-1) * ((w_total + columns - 1) * 2 + 2) - 2);
	for (unsigned c = 0; c < columns; ++c) {
		bool last_column = (c + 1 == columns);
		unsigned w_off_next = last_column ? w_total - 1 : w_off + w - 1;
		w = w_off_next + 1 - w_off;
		bool left_to_right = true;
		for (unsigned y = 0; y + 1 < h; ++y) {
			if (left_to_right) {
				for (unsigned x = 0; x < w; ++x) {
					indices.push_back(w_off + x + (y+1)*w_total);
					indices.push_back(w_off + x +  y   *w_total);
				}
				// append degenerated
				if (y + 2 < h) {
					indices.push_back(w_off + w-1 +  y   *w_total);
					indices.push_back(w_off + w-1 + (y+1)*w_total);
				} else if (!last_column) {
					indices.push_back(w_off + w-1 +  y   *w_total);
					indices.push_back(w_off_next + w_total);
				}
			} else {
				for (unsigned x = 0; x < w; ++x) {
					indices.push_back(w_off + w-1-x +  y   *w_total);
					indices.push_back(w_off + w-1-x + (y+1)*w_total);
				}
				// append degenerated
				if (y + 2 < h) {
					indices.push_back(w_off + (y+1)*w_total);
					indices.push_back(w_off + (y+2)*w_total);
				} else if (!last_column) {
					indices.push_back(w_off + (y+1)*w_total);
					indices.push_back(w_off_next + w_total);
				}
			}
			left_to_right = !left_to_right;
		}
		w_off = w_off_next;
	}

	// finish mesh
	compute_normals();
	compile();
}



unsigned model::mesh::get_nr_of_triangles() const
{
	switch (indices_type) {
	case pt_triangles:
		return indices.size() / 3;
	case pt_triangle_strip:
		return std::max(2U, (unsigned int)indices.size()) - 2;
	default:
		return 0;
	}
}



void model::mesh::get_plain_triangle(unsigned triangle, Uint32 idx[3]) const
{
	unsigned t = triangle * 3;
	idx[0] = indices[t];
	idx[1] = indices[t+1];
	idx[2] = indices[t+2];
}



void model::mesh::get_strip_triangle(unsigned triangle, Uint32 idx[3]) const
{
	unsigned x = triangle & 1;
	idx[0] = indices[triangle  +x];
	idx[1] = indices[triangle+1-x];
	idx[2] = indices[triangle+2];
}



void model::mesh::set_indices_type(primitive_type pt)
{
	switch (pt) {
	case pt_triangles:
		get_triangle_ptr = &model::mesh::get_plain_triangle;
		break;
	case pt_triangle_strip:
		get_triangle_ptr = &model::mesh::get_strip_triangle;
		break;
	default:
		return;
	}
	indices_type = pt;
}



void model::mesh::compile()
{
	bool has_texture_u0 = false, has_texture_u1 = false;
	if (mymaterial != 0) {
		has_texture_u0 = mymaterial->needs_texcoords();
		if (mymaterial->normalmap.get())
			has_texture_u1 = true;
	}
	const unsigned vs = vertices.size();

	// vertices
	vbo_positions.init_data(sizeof(vector3f) * vs, &vertices[0].x, GL_STATIC_DRAW);
	// normals
	vbo_normals.init_data(sizeof(vector3f) * vs, &normals[0].x, GL_STATIC_DRAW);
	// texcoords
	if (has_texture_u0 && texcoords.size() == vs) {
		vbo_texcoords.init_data(sizeof(vector2f) * vs, &texcoords[0].x, GL_STATIC_DRAW);
	}
	// auxiliary data
	// give tangents as texture coordinates for unit 1.
	if (has_texture_u0 && tangentsx.size() == vs) {
		if (mymaterial->use_default_shader()) {
			vbo_tangents_righthanded.init_data(4 * sizeof(float) * vs, 0, GL_STATIC_DRAW);
			float* xdata = (float*) vbo_tangents_righthanded.map(GL_WRITE_ONLY);
			for (unsigned i = 0; i < vs; ++i) {
				xdata[4*i+0] = tangentsx[i].x;
				xdata[4*i+1] = tangentsx[i].y;
				xdata[4*i+2] = tangentsx[i].z;
				xdata[4*i+3] = (righthanded[i]) ? 1.0f : -1.0f;
			}
			vbo_tangents_righthanded.unmap();
			vbo_tangents_righthanded.unbind();
			glsl_shader_setup& gss = mymaterial->specularmap.get() ? *glsl_color_normal_specular : *glsl_color_normal;
			gss.use();
			vertex_attrib_index = gss.get_vertex_attrib_index("tangentx_righthanded");
		} else {
			vbo_tangents_righthanded.init_data(3 * sizeof(float) * vs, 0, GL_STATIC_DRAW);
			float* xdata = (float*) vbo_tangents_righthanded.map(GL_WRITE_ONLY);
			for (unsigned i = 0; i < vs; ++i) {
				xdata[3*i+0] = tangentsx[i].x;
				xdata[3*i+1] = tangentsx[i].y;
				xdata[3*i+2] = tangentsx[i].z;
			}
			vbo_tangents_righthanded.unmap();
			vbo_tangents_righthanded.unbind();
			glsl_shader_setup& gss = static_cast<material_glsl*>(mymaterial)->get_shadersetup();
			gss.use();
			vertex_attrib_index = gss.get_vertex_attrib_index("tangentx");
		}
	}

	// indices - Note: for models with less than 65536 vertices we could
	// use uint16 as data type for indices, but it doesn't bring more
	// performance. OpenGL can do it for use, when we use glDrawRangeElements()
	// later.
	index_data.init_data(indices.size() * 4 /* index type is Uint32! */, &indices[0], GL_STATIC_DRAW);
}



void model::mesh::transform(const matrix4f& m)
{
	for (unsigned i = 0; i < vertices.size(); ++i)
		vertices[i] = m * vertices[i];
	// transform normals: only apply rotation
	matrix4f m2 = m;
	m2.elem(3,0) = m2.elem(3,1) = m2.elem(3,2) = 0;
	for (unsigned j = 0; j < normals.size(); ++j)
		normals[j] = m2 * normals[j];
}



void model::mesh::write_off_file(const string& fn) const
{
	std::ofstream out(fn.c_str());
	out << "OFF\n" << vertices.size() << " " << get_nr_of_triangles() << " 0\n";
	for (unsigned i = 0; i < vertices.size(); ++i) {
		out << vertices[i].x << " " << vertices[i].y << " " << vertices[i].z << "\n";
	}
	for (unsigned j = 0; j < get_nr_of_triangles(); ++j) {
		Uint32 idx[3];
		get_triangle(j, idx);
		out << "3 " << idx[0] << " " << idx[1] << " " << idx[2] << "\n";
	}
}



pair<model::mesh*, model::mesh*> model::mesh::split(const vector3f& abc, float d) const
{
	if (indices_type != pt_triangles) throw std::runtime_error("split: can't handle primitives other than triangles!");

	model::mesh* part0 = new model::mesh();
	model::mesh* part1 = new model::mesh();
	part0->name = name + "_part0";
	part1->name = name + "_part1";
	part0->transformation = part1->transformation = transformation;
	part0->mymaterial = part1->mymaterial = mymaterial;
	part0->vertices.reserve(vertices.size()/2);
	part1->vertices.reserve(vertices.size()/2);
	part0->texcoords.reserve(texcoords.size()/2);
	part1->texcoords.reserve(texcoords.size()/2);
	part0->normals.reserve(normals.size()/2);
	part1->normals.reserve(normals.size()/2);
	part0->tangentsx.reserve(tangentsx.size()/2);
	part1->tangentsx.reserve(tangentsx.size()/2);
	part0->righthanded.reserve(righthanded.size()/2);
	part1->righthanded.reserve(righthanded.size()/2);
	part0->indices.reserve(indices.size()/2);
	part1->indices.reserve(indices.size()/2);

	// determine on which side the vertices are
	vector<float> dists(vertices.size());
	vector<unsigned> ixtrans(vertices.size());
	for (unsigned i = 0; i < vertices.size(); ++i) {
		dists[i] = vertices[i] * abc + d;
		if (dists[i] >= 0) {
			ixtrans[i] = part0->vertices.size();
			part0->vertices.push_back(vertices[i]);
			if (texcoords.size() > 0) part0->texcoords.push_back(texcoords[i]);
			if (normals.size() > 0) part0->normals.push_back(normals[i]);
			if (tangentsx.size() > 0) part0->tangentsx.push_back(tangentsx[i]);
			if (righthanded.size() > 0) part0->righthanded.push_back(righthanded[i]);
		} else {
			ixtrans[i] = part1->vertices.size();
			part1->vertices.push_back(vertices[i]);
			if (texcoords.size() > 0) part1->texcoords.push_back(texcoords[i]);
			if (normals.size() > 0) part1->normals.push_back(normals[i]);
			if (tangentsx.size() > 0) part1->tangentsx.push_back(tangentsx[i]);
			if (righthanded.size() > 0) part0->righthanded.push_back(righthanded[i]);
		}
	}

	// now loop over all faces and split them
	for (unsigned i = 0; i < indices.size(); i += 3) {
		unsigned ix[3];
		float ds[3];
		for (unsigned j = 0; j < 3; ++j) {
			ix[j] = indices[i+j];
			ds[j] = dists[ix[j]];
		}

		// check for faces completly on one side
		if (ds[0] >= 0 && ds[1] >= 0 && ds[2] >= 0) {
			part0->indices.push_back(ixtrans[ix[0]]);
			part0->indices.push_back(ixtrans[ix[1]]);
			part0->indices.push_back(ixtrans[ix[2]]);
			continue;
		}
		if (ds[0] < 0 && ds[1] < 0 && ds[2] < 0) {
			part1->indices.push_back(ixtrans[ix[0]]);
			part1->indices.push_back(ixtrans[ix[1]]);
			part1->indices.push_back(ixtrans[ix[2]]);
			continue;
		}

		// face needs to get splitted
		unsigned p0v = part0->vertices.size();
		unsigned p1v = part1->vertices.size();
		unsigned splitptr = 0;
		unsigned newindi0[4];	// at most 4 indices
		unsigned newindi0ptr = 0;
		unsigned newindi1[4];	// at most 4 indices
		unsigned newindi1ptr = 0;
		unsigned next[3] = { 1, 2, 0 };
		for (unsigned j = 0; j < 3; ++j) {
			float d0 = ds[j], d1 = ds[next[j]];
			if (d0 >= 0)
				newindi0[newindi0ptr++] = ixtrans[ix[j]];
			else
				newindi1[newindi1ptr++] = ixtrans[ix[j]];
			if (d0 * d1 >= 0) continue;
			newindi0[newindi0ptr++] = p0v+splitptr;
			newindi1[newindi1ptr++] = p1v+splitptr;
			float fac = fabs(d0) / (fabs(d0) + fabs(d1));
			vector3f newv = vertices[ix[j]] * (1 - fac) + vertices[ix[next[j]]] * fac;
			part0->vertices.push_back(newv);
			part1->vertices.push_back(newv);
			if (texcoords.size() > 0) {
				vector2f newtexc = texcoords[ix[j]] * (1 - fac) + texcoords[ix[next[j]]] * fac;
				part0->texcoords.push_back(newtexc);
				part1->texcoords.push_back(newtexc);
			}
			if (normals.size() > 0) {
				vector3f newnorm = (normals[ix[j]] * (1 - fac) + normals[ix[next[j]]] * fac).normal();
				part0->normals.push_back(newnorm);
				part1->normals.push_back(newnorm);
			}
			if (tangentsx.size() > 0) {
				vector3f newtanx = (tangentsx[ix[j]] * (1 - fac) + tangentsx[ix[next[j]]] * fac).normal();
				part0->tangentsx.push_back(newtanx);
				part1->tangentsx.push_back(newtanx);
			}
			if (righthanded.size() > 0) {
				//fixme: check if this is correct
				part0->righthanded.push_back(righthanded[ix[j]]);
				part1->righthanded.push_back(righthanded[ix[j]]);
			}
			++splitptr;
		}
		if (splitptr != 2) throw error("splitptr != 2 ?!");
		// add indices to parts.
		part0->indices.push_back(newindi0[0]);
		part0->indices.push_back(newindi0[1]);
		part0->indices.push_back(newindi0[2]);
		if (newindi0ptr == 4) {
			part0->indices.push_back(newindi0[0]);
			part0->indices.push_back(newindi0[2]);
			part0->indices.push_back(newindi0[3]);
		}
		part1->indices.push_back(newindi1[0]);
		part1->indices.push_back(newindi1[1]);
		part1->indices.push_back(newindi1[2]);
		if (newindi1ptr == 4) {
			part1->indices.push_back(newindi1[0]);
			part1->indices.push_back(newindi1[2]);
			part1->indices.push_back(newindi1[3]);
		}
		if (!((newindi0ptr == 3 || newindi1ptr == 3) && (newindi0ptr + newindi1ptr == 7)))
			throw error("newindi ptr corrupt!");
	}

	return make_pair(part0, part1);
}



bool model::mesh::is_inside(const vector3f& p) const
{
	/* algorithm:
	   for every triangle of the mesh, build a tetrahedron of the three
	   points of the triangle and the center of the mesh (e.g. center
	   of gravity). For all tetrahedrons that p is in, count the
	   tetrahedrons with "positive" volume and "negative" volume.
	   The former are all tetrahedrons where the triangle is facing
	   away from the center point, the latter are all tetrahedrons,
	   where the triangle is facing the center point.
	   A point p is inside the tetrahedron consisting of A, B, C, D
	   when: b = B-A, c = C-A, d = D-A, and p = A+r*b+s*c+t*d
	   and r,s,t >= 0 and r+s+t <= 1.
	   We can compute if the triangle is facing the center point D,
	   by computing the sign of the dot product of the normal of
	   triangle A,B,C and the vector D-A=d
	   if (b cross c) * d >= 0 then A,B,C is facing D.
	*/
	int in_out_count = 0;
	std::auto_ptr<triangle_iterator> tit(get_tri_iterator());
	do {
		unsigned i0 = tit->i0();
		unsigned i1 = tit->i1();
		unsigned i2 = tit->i2();
		const vector3f& A = vertices[i0];
		const vector3f& B = vertices[i1];
		const vector3f& C = vertices[i2];
		const vector3f D; // we use the center of mesh space for D.
		vector3f b = B - A;
		vector3f c = C - A;
		vector3f d = D - A;
		float s, r, t;
		if ((p - A).solve(b, c, d, s, r, t)) {
			if (r >= 0.0f && s >= 0.0f && t >= 0.0f && r+s+t <= 1.0f) {
				// p is inside the tetrahedron
				bool facing_to_D = b.cross(c) * d >= 0;
				in_out_count += facing_to_D ? -1 : 1;
			}
		}
	} while (tit->next());
	// for tests:
	//std::cout << "is_inside p=" << p << " p=" << p << " ioc=" << in_out_count << "\n";
	return in_out_count > 0;
}



/* computing Volume
*/
double model::mesh::compute_volume() const
{
	double vsum = 0;
	std::auto_ptr<triangle_iterator> tit(get_tri_iterator());
	do {
		unsigned i0 = tit->i0();
		unsigned i1 = tit->i1();
		unsigned i2 = tit->i2();
		const vector3f& A = vertices[i0];
		const vector3f& B = vertices[i1];
		const vector3f& C = vertices[i2];
		const vector3f D; // we use the center of mesh space for D.
		vector3 a = A - D;
		vector3 b = B - D;
		vector3 c = C - D;
		vector3 abcd = A + B + C + D;
		double V_i = (1.0/6.0) * (b.cross(c) * a);
		vsum += V_i;
	} while (tit->next());
	// result is always matching vertex data, NOT treating the transformation!
	return vsum;
}



/* computing center of gravity:
   Divide sum over tetrahedrons with V_i * c_i each by sum over tetrahedrons
   with V_i each. Where V_i and c_i are volume and center of mass for each
   tetrahedron, given by c = 1/4 * (A+B+C+D) and V = 1/6 * (A-D)*(B-D)x(C-D)
*/
vector3 model::mesh::compute_center_of_gravity() const
{
	vector3 vsum;
	double vdiv = 0;
	std::auto_ptr<triangle_iterator> tit(get_tri_iterator());
	do {
		unsigned i0 = tit->i0();
		unsigned i1 = tit->i1();
		unsigned i2 = tit->i2();
		const vector3f& A = vertices[i0];
		const vector3f& B = vertices[i1];
		const vector3f& C = vertices[i2];
		const vector3f D; // we use the center of mesh space for D.
		vector3 a = A - D;
		vector3 b = B - D;
		vector3 c = C - D;
		vector3 abcd = A + B + C + D;
		double V_i = (1.0/6.0) * (b.cross(c) * a);
		vector3 c_i = (1.0/4.0) * abcd;
		vsum += V_i * c_i;
		vdiv += V_i;
	} while (tit->next());
	//std::cout << "center of gravity is " << vsum << "/" << vdiv << " = " << ((1.0/vdiv) * vsum) << "\n";
	// result is always matching vertex data, NOT treating the transformation!
	return (1.0/vdiv) * vsum;
}



bool model::mesh::has_adjacency_info() const
{
	return triangle_adjacency.size()*3 == get_nr_of_triangles();
}



// Auxiliary structures - store list of edges per vertex
// sorted by vertex with smaller index
struct adjacency_edge_aux_data
{
	unsigned triangle, edge;
	unsigned v0, v1;
	adjacency_edge_aux_data(unsigned t, unsigned e, unsigned v0_, unsigned v1_)
		: triangle(t), edge(e), v0(v0_), v1(v1_) {}
	bool operator< (const adjacency_edge_aux_data& other) const {
		return v0 == other.v0 ? v1 < other.v1 : v0 < other.v0;
	}
};

void model::mesh::compute_adjacency()
{
	unsigned nr_tri = get_nr_of_triangles();
	triangle_adjacency.clear();
	vertex_triangle_adjacency.clear();
	triangle_adjacency.resize(nr_tri, no_adjacency);
	vertex_triangle_adjacency.resize(vertices.size(), no_adjacency);

	// build/use auxiliary data while building adjacency data
	std::vector<std::set<adjacency_edge_aux_data> > tri_of_vertex(vertices.size());
	for (unsigned i = 0; i < nr_tri; ++i) {
		Uint32 idx[3];
		get_triangle(i, idx);
		unsigned degenerated_edges = 0;
		degenerated_edges += (idx[0] == idx[1]) ? 1 : 0;
		degenerated_edges += (idx[0] == idx[2]) ? 1 : 0;
		degenerated_edges += (idx[1] == idx[2]) ? 1 : 0;
		if (degenerated_edges > 0)
			continue;
		for (unsigned j = 0; j < 3; ++j) {
			unsigned v0 = idx[j];
			unsigned v1 = idx[(j+1)%3];
			unsigned va = std::min(v0, v1), vb = std::max(v0, v1);
			adjacency_edge_aux_data aa(i, j, va, vb);
			std::pair<std::set<adjacency_edge_aux_data>::iterator, bool> pib = 
				tri_of_vertex[va].insert(aa);
			vertex_triangle_adjacency[va] = aa.triangle;
			if (!pib.second) {
				// edge already existing
				const adjacency_edge_aux_data& a2 = *pib.first;
				if (triangle_adjacency[a2.triangle*3 + a2.edge] != no_adjacency)
					throw std::runtime_error("inconsistent mesh");
				if (triangle_adjacency[aa.triangle*3 + aa.edge] != no_adjacency)
					throw std::runtime_error("inconsistent mesh");
				triangle_adjacency[a2.triangle*3 + a2.edge] = aa.triangle;
				triangle_adjacency[aa.triangle*3 + aa.edge] = a2.triangle;
			}
		}
	}
}



bool model::mesh::check_adjacency() const
{
	// fixme: for all triangles check if adjacent triangle has vertex pair matching edge
	// and that edge has same pair as current edge
	return false;
}

/* computing the inertia tensor for a mesh,
   from the RigidBodySimulation paper.
   The inertia tensor is:

   (M / Sum_over_i V_i) * Sum_over_i Integral over volume ...
   where M is total mass, i iterates over the tetrahedrons (hence triangles).
   and the integral (a matrix) can be written as:
   (1/120) * ((A-D)*(B-D)x(C-D))((A+B+C+D)(A+B+C+D)^T + AA^T + BB^T + CC^T + DD^T)
   where A,B,C form the base triangle and together with D the tetrahedron.
   D should be at center of gravity, for simplicities sake this should be (0,0,0),
   hence we can adjust the transformation matrix of the mesh or better the vertices.

   The formula decomposes to a scalar (1/120 and first brace) and a matrix
   (second brace).

   Problem: we can't manipulate the vertices or transformation matrices
   to shift center of gravity to (0,0,0) as we need both ways...

   however this routine should give inertia tensor matching the current
   object - but this can give problems for simulation later,
   if the c.o.g is not at 0,0,0 ...
*/
matrix3 model::mesh::compute_inertia_tensor(const matrix4f& transmat) const
{
	matrix3 msum;
	const double mass = 1.0; // is just a scalar to the matrix
	const vector3 center_of_gravity = transmat.mul4vec3xlat(compute_center_of_gravity());
	double vdiv = 0;
	std::auto_ptr<triangle_iterator> tit(get_tri_iterator());
	do {
		unsigned i0 = tit->i0();
		unsigned i1 = tit->i1();
		unsigned i2 = tit->i2();
		vector3 A = transmat * vertices[i0];
		vector3 B = transmat * vertices[i1];
		vector3 C = transmat * vertices[i2];
		const vector3& D = center_of_gravity;
		vector3 abcd = A + B + C + D;
		double V_i = (1.0/6.0) * ((A - D) * (B - D).cross(C - D));
		double fac0 = V_i / 20.0; // 6*20=120
		matrix3 abcd2 = matrix3::vec_sqr(abcd);
		matrix3 A2 = matrix3::vec_sqr(A);
		matrix3 B2 = matrix3::vec_sqr(B);
		matrix3 C2 = matrix3::vec_sqr(C);
		matrix3 D2 = matrix3::vec_sqr(D);
		matrix3 h = (abcd2 + A2 + B2 + C2 + D2) * fac0;
		// we have to build the matrix with the integral
		// to compute out of sums / products of coefficients
		// of the helper matrix h.
		matrix3 im(h.elem(1,1) + h.elem(2,2), // y^2+z^2
			   -h.elem(1,0), // -xy
			   -h.elem(2,0), // -xz
			   -h.elem(1,0), // -xy
			   h.elem(0,0) + h.elem(2,2), // x^2+z^2
			   -h.elem(2,1), // -yz
			   -h.elem(2,0), // -xz
			   -h.elem(2,1), // -yz
			   h.elem(0,0) + h.elem(1,1)); // x^2+y^2
		msum = msum + im;
		vdiv += V_i;
	} while (tit->next());
	// result is in model-space, not mesh-space
	return msum * (mass/vdiv);
}



void model::mesh::compute_bv_tree()
{
	// build leaf nodes for every triangle of m
	std::list<bv_tree::leaf_data> leaf_nodes;
	std::auto_ptr<triangle_iterator> tit(get_tri_iterator());
	unsigned tri_index = 0;
	do {
		bv_tree::leaf_data ld;
		ld.tri_idx[0] = tit->i0();
		ld.tri_idx[1] = tit->i1();
		ld.tri_idx[2] = tit->i2();
		ld.triangle = tri_index;
		leaf_nodes.push_back(ld);
		++tri_index;
	} while (tit->next());
	// clear memory first
	bounding_volume_tree.reset();
	bounding_volume_tree = bv_tree::create(vertices, leaf_nodes);
}



model::material::map::map()
	: tex(0), ref_count(0)
{
}



model::material::map::~map()
{
	for (std::map<string, skin>::iterator it = skins.begin(); it != skins.end(); ++it)
		delete it->second.mytexture;
}



void model::material::map::register_layout(const std::string& name,
					   const string& basepath,
					   texture::mapping_mode mapping,
					   bool makenormalmap,
					   float detailh,
					   bool rgb2grey)
{
	std::map<string, skin>::iterator it = skins.find(name);
	if (it != skins.end()) {
		// skin texture
		if (it->second.ref_count == 0) {
			// load texture. Skins are expected in the same path as the model
			// itself.
			it->second.mytexture = new texture(basepath + it->second.filename, mapping, texture::CLAMP,
							   makenormalmap, detailh, rgb2grey);
		}
		++(it->second.ref_count);
	} else {
		if (ref_count == 0) {
			// load texture
			try {
				mytexture.reset(new texture(basepath + filename, mapping, texture::CLAMP,
							    makenormalmap, detailh, rgb2grey));
			}
			catch (std::exception& e) {
				mytexture.reset(new texture(get_texture_dir() + filename, mapping, texture::CLAMP,
							    makenormalmap, detailh, rgb2grey));
			}
		}
		++ref_count;
	}
}



void model::material::map::unregister_layout(const std::string& name)
{
	std::map<string, skin>::iterator it = skins.find(name);
	if (it != skins.end()) {
		if (it->second.ref_count == 0)
			throw error("unregistered texture, but skin ref_count already zero");
		--(it->second.ref_count);
		if (it->second.ref_count == 0) {
			delete it->second.mytexture;
			it->second.mytexture = 0;
		}
	} else {
		if (ref_count == 0)
			throw error("unregistered texture, but ref_count already zero");
		--ref_count;
		if (ref_count == 0) {
			mytexture.reset();
		}
	}
}



void model::material::map::set_layout(const std::string& layout)
{
	std::map<string, skin>::const_iterator it = skins.find(layout);
	if (it != skins.end()) {
		tex = it->second.mytexture;
	} else {
		tex = mytexture.get();
	}
}



int model::get_object_id_by_name(const std::string& name) const
{
	const object* obj = scene.find(name);
	if (!obj) return -1;
	return int(obj->id);
}



void model::material::map::get_all_layout_names(std::set<std::string>& result) const
{
	for (std::map<string, skin>::const_iterator it = skins.begin(); it != skins.end(); ++it)
		result.insert(it->first);
}



model::material::material(const std::string& nm) : name(nm), shininess(50.0f), two_sided(false)
{
}



void model::material::map::set_gl_texture() const
{
	if (tex)
		tex->set_gl_texture();
	else
		throw error("set_gl_texture with empty texture");
}



void model::material::map::set_gl_texture(const glsl_program& prog, unsigned loc, unsigned texunitnr) const
{
	if (!tex)
		throw error("set_gl_texture(shader) with empty texture");
	prog.set_gl_texture(*tex, loc, texunitnr);
}



void model::material::map::set_gl_texture(const glsl_shader_setup& gss, unsigned loc, unsigned texunitnr) const
{
	if (!tex)
		throw error("set_gl_texture(shader) with empty texture");
	gss.set_gl_texture(*tex, loc, texunitnr);
}



void model::material::map::set_texture(texture* t)
{
	mytexture.reset(t);
	tex = t;
}



void model::material::set_gl_values(const texture *caustic_map) const
{
	// set some values to be used in shader - should better be done with uniforms
	// to leave out this fixed-pipeline stuff, fixme
	GLfloat coltmp[4];
	specular.store_rgba(coltmp);
	glMaterialfv(GL_FRONT, GL_SPECULAR, coltmp);
	glMaterialf(GL_FRONT, GL_SHININESS, shininess);

	if (colormap.get()) {
		if (normalmap.get()) {
			// texture units / coordinates:
			// tex0: color map / matching texcoords
			// tex1: normal map / texcoords show vector to light
			// tex2: specular map / texcoords show vector to viewer, if available
			if (specularmap.get() && !caustic_map) {
				glsl_color_normal_specular->use();
				specularmap->set_gl_texture(*glsl_color_normal_specular, loc_cns_tex_specular, 2);
				normalmap->set_gl_texture(*glsl_color_normal_specular, loc_cns_tex_normal, 1);
				colormap->set_gl_texture(*glsl_color_normal_specular, loc_cns_tex_color, 0);
			} else if (specularmap.get() && caustic_map) {
				glsl_color_normal_specular_caustic->use();
				glsl_color_normal_specular_caustic->set_gl_texture(*const_cast<texture *>(caustic_map), loc_cnsc_tex_caustic, 3);
				specularmap->set_gl_texture(*glsl_color_normal_specular_caustic, loc_cnsc_tex_specular, 2);
				normalmap->set_gl_texture(*glsl_color_normal_specular_caustic, loc_cnsc_tex_normal, 1);
				colormap->set_gl_texture(*glsl_color_normal_specular_caustic, loc_cnsc_tex_color, 0);
			} else if (!specularmap.get() && !caustic_map) {
				glsl_color_normal->use();
				normalmap->set_gl_texture(*glsl_color_normal, loc_cn_tex_normal, 1);
				colormap->set_gl_texture(*glsl_color_normal, loc_cn_tex_color, 0);
			} else if (!specularmap.get() && caustic_map) {
				glsl_color_normal_caustic->use();
				glsl_color_normal_caustic->set_gl_texture(*const_cast<texture *>(caustic_map), loc_cnc_tex_caustic, 2);
				normalmap->set_gl_texture(*glsl_color_normal_caustic, loc_cnc_tex_normal, 1);
				colormap->set_gl_texture(*glsl_color_normal_caustic, loc_cnc_tex_color, 0);
			}
		} else {
			// just texture map and per-pixel lighting with vertex normals
			glsl_color->use();
			colormap->set_gl_texture(*glsl_color, loc_c_tex_color, 0);
		}
	} else {
		// just base color and per-pixel lighting with vertex normals
		glsl_plastic->use();
		// fixme: better give color as uniform to shader!
		glColor4ub(diffuse.r, diffuse.g, diffuse.b, diffuse.a);
	}
}



void model::material::set_gl_values_mirror_clip() const
{
	glsl_mirror_clip->use();

	if (colormap.get()) {
		// plain texture mapping with diffuse lighting only, but with shaders
		colormap->set_gl_texture(*glsl_mirror_clip, loc_mc_tex_color, 0);
	}
}



void model::material::register_layout(const std::string& name, const std::string& basepath)
{
	if (colormap.get())
		colormap->register_layout(name, basepath, model::mapping);
	//fixme: what is best mapping for normal maps?
	// compute normalmap if not given
	// fixme: segfaults when enabled. see texture.cpp
	// fixme: without shaders it seems we need to multiply this with ~16 or even more.
	// maybe because direction vectors are no longer normalized over faces...
	// with shaders a value of 1.0 is enough.
	// fixme: read value from model file... and multiply with this value...
	float normalmapheight = 4.0f;
	if (normalmap.get())
		normalmap->register_layout(name, basepath, texture::LINEAR/*_MIPMAP_LINEAR*/, true, normalmapheight, true);
	if (specularmap.get())
		specularmap->register_layout(name, basepath, texture::LINEAR_MIPMAP_LINEAR, false, 0.0f, true);
}



void model::material::unregister_layout(const std::string& name)
{
	if (colormap.get())
		colormap->unregister_layout(name);
	if (normalmap.get())
		normalmap->unregister_layout(name);
	if (specularmap.get())
		specularmap->unregister_layout(name);
}



void model::material::set_layout(const std::string& layout)
{
	if (colormap.get())
		colormap->set_layout(layout);
	if (normalmap.get())
		normalmap->set_layout(layout);
	if (specularmap.get())
		specularmap->set_layout(layout);
}



void model::material::get_all_layout_names(std::set<std::string>& result) const
{
	if (colormap.get())
		colormap->get_all_layout_names(result);
	if (normalmap.get())
		normalmap->get_all_layout_names(result);
	if (specularmap.get())
		specularmap->get_all_layout_names(result);
}



model::material_glsl::material_glsl(const std::string& nm, const std::string& vsfn, const std::string& fsfn)
	: material(nm),
	  vertexshaderfn(vsfn),
	  fragmentshaderfn(fsfn),
	  shadersetup(get_shader_dir() + vsfn, get_shader_dir() + fsfn),
	  nrtex(0)
{
	for (unsigned i = 0; i < DFTD_MAX_TEXTURE_UNITS; ++i)
		loc_texunit[i] = 0;
}



void model::material_glsl::compute_texloc()
{
	shadersetup.use();

	for (unsigned i = 0; i < nrtex; ++i) {
		loc_texunit[i] = shadersetup.get_uniform_location(texnames[i]);
		if (loc_texunit[i] == unsigned(-1))
			throw error(std::string("unable to lookup uniform location of shader for material_glsl, texname=") + texnames[i] + ", NOTE: shader needs to _USE_ the uniform (defining the symbol is not enough, use means it has to contribute to the output) to be linked into the shader program!");
	}
}



void model::material_glsl::set_gl_values(const texture * /*caustic_map*/) const
{
	shadersetup.use();
	// set up up to four tex units
	for (unsigned i = 0; i < nrtex; ++i) {
		if (texmaps[i].get()) {
			glActiveTexture(GL_TEXTURE0 + i);
			texmaps[i]->set_gl_texture(shadersetup, loc_texunit[i], i);
		}
	}
}



void model::material_glsl::set_gl_values_mirror_clip() const
{
	// no special handling possible
	return set_gl_values();
}



void model::material_glsl::register_layout(const std::string& name, const std::string& basepath)
{
	for (unsigned i = 0; i < nrtex; ++i) {
		if (texmaps[i].get()) {
			// fixme: any specific mapping here?? see material::register_layout
			texmaps[i]->register_layout(name, basepath, model::mapping);
		}
	}
}



void model::material_glsl::unregister_layout(const std::string& name)
{
	for (unsigned i = 0; i < nrtex; ++i) {
		if (texmaps[i].get()) {
			texmaps[i]->unregister_layout(name);
		}
	}
}



void model::material_glsl::set_layout(const std::string& layout)
{
	for (unsigned i = 0; i < nrtex; ++i) {
		if (texmaps[i].get()) {
			texmaps[i]->set_layout(name);
		}
	}
}



void model::material_glsl::get_all_layout_names(std::set<std::string>& result) const
{
	for (unsigned i = 0; i < nrtex; ++i) {
		if (texmaps[i].get()) {
			texmaps[i]->get_all_layout_names(result);
		}
	}
}



void model::mesh::display(const texture *caustic_map) const
{
	// set up material
	if (mymaterial != 0) {
		mymaterial->set_gl_values(caustic_map);
		if (mymaterial->two_sided) {
			glDisable(GL_CULL_FACE);
		}
	} else {
		glsl_shader_setup::default_opaque->use();
		glsl_shader_setup::default_opaque->set_uniform(glsl_shader_setup::loc_o_color,
							       colorf(1,1,1,1));
	}

	// local transformation matrix.
	glPushMatrix();
	transformation.multiply_glf();

	bool has_texture_u0 = false, has_texture_u1 = false;
	if (mymaterial != 0) {
		has_texture_u0 = mymaterial->needs_texcoords();
		if (mymaterial->normalmap.get())
			has_texture_u1 = true;
	}

	// set up vertex data.
	vbo_positions.bind();
	glVertexPointer(3, GL_FLOAT, sizeof(vector3f), 0);

	// set up normals (only used with shaders or for plain rendering without normal maps).
	vbo_normals.bind();
	glNormalPointer(GL_FLOAT, sizeof(vector3f), 0);
	glEnableClientState(GL_NORMAL_ARRAY);

	// without pixel shaders texture coordinates must be set for both texture units and are the same.
	if (has_texture_u0 && texcoords.size() == vertices.size()) {
		vbo_texcoords.bind();
		glTexCoordPointer(2, GL_FLOAT, sizeof(vector2f), 0);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	}

	// Using vertex and fragment programs.
	// give tangents/righthanded info as vertex attribute.
	if (has_texture_u0 && tangentsx.size() == vertices.size()) {
		if (mymaterial->use_default_shader()) {
			vbo_tangents_righthanded.bind();
			glVertexAttribPointer(vertex_attrib_index, 4, GL_FLOAT, GL_FALSE,
					      0, 0);
			glEnableVertexAttribArray(vertex_attrib_index);
		} else {
			vbo_tangents_righthanded.bind();
			glVertexAttribPointer(vertex_attrib_index, 3, GL_FLOAT, GL_FALSE,
					      0, 0);
			glEnableVertexAttribArray(vertex_attrib_index);
		}
	}

	// unbind VBOs (can't be static or we would need to define type of VBO vert/index)
	vbo_positions.unbind();

	// render geometry, glDrawRangeElements is faster than glDrawElements.
	index_data.bind();
	glDrawRangeElements(gl_primitive_type(), 0, vertices.size()-1, indices.size(), GL_UNSIGNED_INT, 0);
	index_data.unbind();

	// maybe: add code to show normals as Lines

	// cleanup
	glDisableVertexAttribArray(vertex_attrib_index);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);	// disable tex0
	if (mymaterial != 0 && mymaterial->two_sided) {
		glEnable(GL_CULL_FACE);
	}
	
	// local transformation matrix.
	glPopMatrix();

	// clean up for material
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}



void model::mesh::display_mirror_clip() const
{
	// matrix mode is GL_MODELVIEW and active texture is GL_TEXTURE1 here
	glPushMatrix();
	transformation.multiply_glf();

	bool has_texture_u0 = false;
	if (mymaterial != 0) {
		has_texture_u0 = mymaterial->needs_texcoords();
		mymaterial->set_gl_values_mirror_clip();
	} else {
		glsl_mirror_clip->use();
	}

	vbo_positions.bind();
	glVertexPointer(3, GL_FLOAT, sizeof(vector3f), 0);

	// basic lighting, we need normals
	vbo_normals.bind();
	glNormalPointer(GL_FLOAT, sizeof(vector3f), 0);
	glEnableClientState(GL_NORMAL_ARRAY);

	// and texture coordinates
	if (has_texture_u0 && texcoords.size() == vertices.size()) {
		vbo_texcoords.bind();
		glTexCoordPointer(2, GL_FLOAT, sizeof(vector2f), 0);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	}

	// unbind VBOs
	vbo_positions.unbind();

	// render geometry
	index_data.bind();
	glDrawRangeElements(gl_primitive_type(), 0, vertices.size()-1, indices.size(), GL_UNSIGNED_INT, 0);
	index_data.unbind();

	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);	// disable tex0
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glPopMatrix();
}



void model::set_layout(const std::string& layout)
{
//	cout << "set layout '" << layout << "' for model '" << filename << "'\n";
	if (current_layout == layout)
		return;
	for (vector<material*>::iterator it = materials.begin(); it != materials.end(); ++it)
		(*it)->set_layout(layout);
	current_layout = layout;
}



void model::display(const texture *caustic_map) const
{
	if (current_layout.length() == 0) {
		throw error(filename + ": trying to render model, but no layout was set yet");
	}

	// default scene: no objects, just draw all meshes.
	if (scene.children.size() == 0) {
		for (vector<model::mesh*>::const_iterator it = meshes.begin(); it != meshes.end(); ++it) {
			(*it)->display(caustic_map);
		}
	} else {
		scene.display(caustic_map);
	}
}



void model::display_mirror_clip() const
{
	// set up a object->worldspace transformation matrix in tex unit#1 matrix.
	if (scene.children.size() == 0) {
		// default scene: no objects, just draw all meshes.
		for (vector<model::mesh*>::const_iterator it = meshes.begin(); it != meshes.end(); ++it) {
			(*it)->display_mirror_clip();
		}
	} else {
		scene.display_mirror_clip();
	}
}



model::mesh& model::get_mesh(unsigned nr)
{
	return *meshes.at(nr);
}

const model::mesh& model::get_mesh(unsigned nr) const
{
	return *meshes.at(nr);
}

model::mesh& model::get_base_mesh()
{
	if (scene.children.empty())
		return get_mesh(0);
	mesh* m = scene.children.front().mymesh;
	if (!m)
		throw error("can't compute base mesh, mymesh=0");
	return *m;
}

const model::mesh& model::get_base_mesh() const
{
	if (scene.children.empty())
		return get_mesh(0);
	mesh* m = scene.children.front().mymesh;
	if (!m)
		throw error("can't compute base mesh, mymesh=0");
	return *m;
}

model::material& model::get_material(unsigned nr)
{
	return *(materials.at(nr));
}

const model::material& model::get_material(unsigned nr) const
{
	return *(materials.at(nr));
}

static inline unsigned char2hex(char c)
{
	if (c >= '0' && c <= '9')
		return unsigned(c - '0');
	else if (c >= 'A' && c <= 'F')
		return unsigned(c - 'A' + 10);
	else if (c >= 'a' && c <= 'f')
		return unsigned(c - 'a' + 10);
	else
		return 0;
}

static inline float hex2float(const std::string& s, size_t off)
{
	unsigned n0 = char2hex(s[off]);
	unsigned n1 = char2hex(s[off+1]);
	return float(n0*16+n1)/255;
}

void model::read_phys_file(const string& filename)
{
	string fn = filename.substr(0, filename.rfind(".")) + ".phys";
	xml_doc physdat(fn);
	try {
		physdat.load();
	}
	catch (...) {
		return;
	}
	xml_elem physroot = physdat.child("dftd-physical-data");
	xml_elem physcs = physroot.child("cross-section");
	cross_sections.resize(physcs.attru("angles"));
	std::istringstream iss(physcs.child_text());
	for (unsigned i = 0; i < cross_sections.size(); ++i) {
		iss >> cross_sections[i];
	}

	// set inertia tensor of mesh #0
	mesh& m = get_base_mesh();
	istringstream iss2(physroot.child("inertia-tensor").child_text());
	m.inertia_tensor = matrix3(iss2);

	// set volume of mesh #0
	m.volume = physroot.child("volume").attrf();

	// set voxel data
	xml_elem ve = physroot.child("voxels");
	voxel_resolution = vector3i(ve.attri("x"), ve.attri("y"), ve.attri("z"));
	unsigned nrvoxels = voxel_resolution.x*voxel_resolution.y*voxel_resolution.z;
	voxel_data.reserve(ve.attru("innr"));
	vector<float> insidevol(nrvoxels);
	istringstream iss3(ve.child_text());
	for (unsigned k = 0; k < nrvoxels; ++k) {
		iss3 >> insidevol[k];
	}
	if (iss3.fail())
		throw error(filename + ", error reading inside volume data");

	vector<float> massdistri;
	if (ve.has_child("mass-distribution")) {
		istringstream iss4(ve.child("mass-distribution").child_text());
		massdistri.resize(nrvoxels);
		for (unsigned k = 0; k < nrvoxels; ++k) {
			iss4 >> massdistri[k];
		}
		if (iss4.fail())
			throw error("error reading mass distribution data");
	}

	const vector3f& bmax = m.max;
	const vector3f& bmin = m.min;
	const vector3f bsize = bmax - bmin;
	voxel_size = vector3f(bsize.x / voxel_resolution.x,
			      bsize.y / voxel_resolution.y,
			      bsize.z / voxel_resolution.z);
	double voxel_volume = voxel_size.x * voxel_size.y * voxel_size.z;
	total_volume_by_voxels = ve.attrf("invol") * voxel_volume;
	voxel_radius = pow(voxel_volume * 3.0 / (4.0 * M_PI), 1.0/3); // sphere of same volume
	unsigned ptr = 0;
	float mass_part_sum = 0;
	double volume_rcp = 1.0/m.volume;
	voxel_index_by_pos.resize(voxel_resolution.x*voxel_resolution.y*voxel_resolution.z, -1);
	for (int izz = 0; izz < voxel_resolution.z; ++izz) {
		// quick test hack, linear distribution top->down 0->1
		float mass_part = (voxel_resolution.z - izz)/float(voxel_resolution.z);
		for (int iyy = 0; iyy < voxel_resolution.y; ++iyy) {
			for (int ixx = 0; ixx < voxel_resolution.x; ++ixx) {
				float f = insidevol[ptr];
				if (f >= 1.0f/255.0f) {
					voxel_index_by_pos[ptr] = int(voxel_data.size());
					float m = f * mass_part;
					if (!massdistri.empty())
						m = massdistri[ptr];
					voxel_data.push_back(voxel(vector3f(ixx + 0.5 + bmin.x/voxel_size.x,
									    iyy + 0.5 + bmin.y/voxel_size.y,
									    izz + 0.5 + bmin.z/voxel_size.z),
								   f, m, f * voxel_volume * volume_rcp));
					mass_part_sum += m;
				}
				++ptr;
			}
		}
	}
	// renormalize mass parts
	if (massdistri.empty()) {
		for (unsigned i = 0; i < voxel_data.size(); ++i)
			voxel_data[i].relative_mass /= mass_part_sum;
	}
	// compute neighbouring information
	ptr = 0;
	int dx[6] = {  0, -1,  0,  1,  0,  0 };
	int dy[6] = {  0,  0,  1,  0, -1,  0 };
	int dz[6] = {  1,  0,  0,  0,  0, -1 };
	for (int izz = 0; izz < voxel_resolution.z; ++izz) {
		for (int iyy = 0; iyy < voxel_resolution.y; ++iyy) {
			for (int ixx = 0; ixx < voxel_resolution.x; ++ixx) {
				int revvi = voxel_index_by_pos[ptr];
				if (revvi >= 0) {
					// there is a voxel at that position
					for (int k = 0; k < 6; ++k) {
						int nx = ixx + dx[k];
						int ny = iyy + dy[k];
						int nz = izz + dz[k];
						if (nx >= 0 && ny >= 0 && nz >= 0 &&
						    nx < voxel_resolution.x &&
						    ny < voxel_resolution.y &&
						    nz < voxel_resolution.z) {
							int ng = voxel_index_by_pos[(nz * voxel_resolution.y + ny) * voxel_resolution.x + nx];
							if (ng >= 0) {
								voxel_data[revvi].neighbour_idx[k] = ng;
								//DBGOUT8(ixx,iyy,izz,k,ng,nx,ny,nz);
							}
						}
					}
				}
				++ptr;
			}
		}
	}
}

float model::get_cross_section(float angle) const
{
	unsigned cs = cross_sections.size();
	if (cs == 0) return 0.0f;
	float fcs = angle * cs / 360.0;
	float fac = fcs - floor(fcs);
	unsigned id0 = unsigned(floor(fcs)) % cs;
	unsigned id1 = (id0 + 1) % cs;
	return cross_sections[id0] * (1.0f - fac) + cross_sections[id1] * fac;
}



string model::tolower(const string& s)
{
	string s2 = s;
	for (unsigned i = 0; i < s.length(); ++i)
		s2[i] = ::tolower(s[i]);
	return s2;
}



void model::transform(const matrix4f& m)
{
	for (vector<model::mesh*>::iterator it = meshes.begin(); it != meshes.end(); ++it) {
		(*it)->transform(m);
	}
}



void model::compile()
{
	for (vector<model::mesh*>::iterator it = meshes.begin(); it != meshes.end(); ++it) {
		(*it)->compile();
	}
}



// -------------------------------- dftd model file writing --------------------------------------
// write our own model file format.
void model::write_to_dftd_model_file(const std::string& filename, bool store_normals) const
{
	xml_doc doc(filename);
	xml_elem root = doc.add_child("dftd-model");
	root.set_attr(1.1f, "version");//fixme: write relations too and increase to 1.2

	// save materials.
	unsigned nr = 0;
	for (vector<material*>::const_iterator it = materials.begin(); it != materials.end(); ++it, ++nr) {
		const material* m = *it;
		xml_elem mat = root.add_child("material");
		mat.set_attr(m->name, "name");
		mat.set_attr(nr, "id");

		const material_glsl* matglsl = dynamic_cast<const material_glsl*>(m);
		if (matglsl) {
			// fixme: save code
			xml_elem es = mat.add_child("shader");
			es.set_attr(matglsl->get_vertexshaderfn(), "vertex");
			es.set_attr(matglsl->get_fragmentshaderfn(), "fragment");
			for (unsigned i = 0; i < 4; ++i) {
				if (matglsl->texmaps[i].get()) {
					matglsl->texmaps[i]->write_to_dftd_model_file(mat, matglsl->texnames[i]);
				}
			}
		} else {
			// colors.
			write_color_to_dftd_model_file(mat, m->diffuse, "diffuse");
			write_color_to_dftd_model_file(mat, m->specular, "specular");

			// shininess
			xml_elem sh = mat.add_child("shininess");
			sh.set_attr(m->shininess, "exponent");

			// maps.
			if (m->colormap.get()) {
				m->colormap->write_to_dftd_model_file(mat, "diffuse");
			}
			if (m->normalmap.get()) {
				m->normalmap->write_to_dftd_model_file(mat, "normal");
			}
			if (m->specularmap.get()) {
				m->specularmap->write_to_dftd_model_file(mat, "specular");
			}
		}
	}

	// save meshes.
	nr = 0;
	for (vector<mesh*>::const_iterator it = meshes.begin(); it != meshes.end(); ++it, ++nr) {
		mesh* mp = *it;
		xml_elem msh = root.add_child("mesh");
		msh.set_attr(mp->name, "name");
		msh.set_attr(nr, "id");

		// material.
		if (mp->mymaterial) {
			unsigned matid = 0;
			for ( ; matid < materials.size(); ++matid) {
				if (materials[matid] == mp->mymaterial)
					break;
			}
			msh.set_attr(matid, "material");
		}

		// vertices.
		xml_elem verts = msh.add_child("vertices");
		verts.set_attr(unsigned(mp->vertices.size()), "nr");
		ostringstream ossv;
		//unsigned nrcrd = 0;
		for (vector<vector3f>::const_iterator vit = mp->vertices.begin(); vit != mp->vertices.end(); ++vit) {
			// add return after each 8th coordinate - doesn't work with tinyxml this way!
			//if (nrcrd++ % 8 == 0)
			//ossv << "\n";
			ossv << vit->x << " " << vit->y << " " << vit->z << " ";
		}
		verts.add_child_text(ossv.str());

		// indices.
		xml_elem indis = msh.add_child("indices");
		indis.set_attr(unsigned(mp->indices.size()), "nr");
		// need to convert to string for proper type selection of overloaded function
		indis.set_attr(std::string(mp->name_primitive_type()), "type");
		ostringstream ossi;
		//unsigned nrind = 0;
		for (vector<Uint32>::const_iterator iit = mp->indices.begin(); iit != mp->indices.end(); ++iit) {
			// add return after each 32th index - doesn't work with tinyxml this way!
			//if (nrind++ % 32 == 0)
			//ossi << "\n";
			ossi << *iit << " ";
		}
		indis.add_child_text(ossi.str());

		// texcoords.
		if (mp->mymaterial) {
			xml_elem texcs = msh.add_child("texcoords");
			ostringstream osst;
			//unsigned nrcrd = 0;
			for (vector<vector2f>::const_iterator tit = mp->texcoords.begin();
			     tit != mp->texcoords.end(); ++tit) {
				// add return after each 8th coordinate - doesn't work with tinyxml this way!
				//if (nrcrd++ % 8 == 0)
				//osst << "\n";
				osst << tit->x << " " << tit->y << " ";
			}
			texcs.add_child_text(osst.str());
		}

		if (store_normals) {
			// normals.
			xml_elem nrmls = msh.add_child("normals");
			ostringstream ossn;
			//unsigned nrcrd = 0;
			for (vector<vector3f>::const_iterator nit = mp->normals.begin(); nit != mp->normals.end(); ++nit) {
				// add return after each 8th coordinate - doesn't work with tinyxml this way!
				//if (nrcrd++ % 8 == 0)
				//ossn << "\n";
				ossn << nit->x << " " << nit->y << " " << nit->z << " ";
			}
			nrmls.add_child_text(ossn.str());
		}

		// transformation.
		xml_elem trans = msh.add_child("transformation");
		ostringstream osst;
		for (unsigned y = 0; y < 4; ++y) {
			for (unsigned x = 0; x < 4; ++x) {
				osst << mp->transformation.elem(x, y) << " ";
			}
		}
		trans.add_child_text(osst.str());
	}

	// finally save file.
	doc.save();
}


void model::write_color_to_dftd_model_file(xml_elem& parent, const color& c, const string& type) const
{
	xml_elem cl = parent.add_child(type);
	ostringstream osscl;
	osscl << float(c.r)/255 << " " << float(c.g)/255 << " " << float(c.b)/255;
	cl.set_attr(osscl.str(), "color");
}


color model::read_color_from_dftd_model_file(const xml_elem& parent, const std::string& type)
{
	xml_elem ecol = parent.child(type);
	if (!ecol.has_attr("color"))
		throw xml_error("no color information given", parent.doc_name());
	string tmp = ecol.attr("color");
	istringstream iss(tmp);
	float r, g, b;
	iss >> r >> g >> b;
	return color(Uint8(r*255), Uint8(g*255), Uint8(b*255));
}


void model::material::map::write_to_dftd_model_file(xml_elem& parent,
						    const std::string& type) const
{
	xml_elem mmap = parent.add_child("map");
	// write here possible skin children, fixme
	mmap.set_attr(type, "type");
	mmap.set_attr(filename, "filename");
	// skins
	for (std::map<string, skin>::const_iterator it = skins.begin(); it != skins.end(); ++it) {
		xml_elem s = mmap.add_child("skin");
		s.set_attr(it->second.filename, "filename");
		s.set_attr(it->first, "layout");
	}
}



model::material::map::map(const xml_elem& parent)
	: tex(0), ref_count(0)
{
	if (!parent.has_attr("filename"))
		throw xml_error("no filename given for materialmap!", parent.doc_name());
	filename = parent.attr("filename");

	// skins
	for (xml_elem::iterator it = parent.iterate("skin"); !it.end(); it.next()) {
		string layoutname = it.elem().attr("layout");
		pair<std::map<string, skin>::iterator, bool> insok =
			skins.insert(make_pair(layoutname, skin()));
		if (!insok.second)
			throw xml_error("layout names not unique", it.elem().doc_name());
		skin& s = insok.first->second;
		s.filename = it.elem().attr("filename");
		// load textures in init() function.
// 		new texture(basepath + filename, mapping, texture::CLAMP,
// 			    makenormalmap, detailh, rgb2grey));
	}
}

// -------------------------------- end of dftd model file writing ------------------------------



// -------------------------------- off file loading --------------------------------------
void model::read_off_file(const string& fn)
{
	FILE *f = fopen(fn.c_str(), "rb");
	if (!f) return;
	unsigned i;
	unsigned nr_vertices, nr_faces;
	fscanf(f, "OFF\n%u %u %u\n", &nr_vertices, &nr_faces, &i);
	mesh* m = new mesh();
	m->name = basename;
	m->vertices.resize(nr_vertices);
	m->indices.resize(3*nr_faces);
	
	for (i = 0; i < nr_vertices; i++) {
		float a, b, c;
		fscanf(f, "%f %f %f\n", &a, &b, &c);
		m->vertices[i].x = a;
		m->vertices[i].y = b;
		m->vertices[i].z = c;
	}
	for (i = 0; i < nr_faces; i++) {
		unsigned j, v0, v1, v2;
		fscanf(f, "%u %u %u %u\n", &j, &v0, &v1, &v2);
		if (j != 3) return;
		m->indices[i*3] = v0;
		m->indices[i*3+1] = v1;
		m->indices[i*3+2] = v2;
	}
	fclose(f);
	meshes.push_back(m);

}

// -------------------------------- end of off file loading -------------------------------------

// -------------------------------- dftd model file reading -------------------------------------

static string next_part_of_string(const string& s, string::size_type& fromwhere)
{
	string::size_type st = s.find(" ", fromwhere);
	if (st == string::npos) {
		string tmp = s.substr(fromwhere);
		fromwhere = st;
		return tmp;
	} else {
		string tmp = s.substr(fromwhere, st - fromwhere);
		fromwhere = st + 1;
		if (fromwhere == s.length())
			fromwhere = string::npos;
		return tmp;
	}
}



void model::read_dftd_model_file(const std::string& filename)
{
	xml_doc doc(filename);
	doc.load();
	xml_elem root = doc.child("dftd-model");
	float version = root.attrf("version");
	//fixme: float is a bad idea for a version string, because of accuracy
	if (version > 1.21)//fixme with relations 1.2
		throw xml_error("model file format version unknown ", root.doc_name());

	// read elements.
	map<unsigned, material* > mat_id_mapping;
	unsigned nr_of_objecttrees = 0;
	for (xml_elem::iterator it = root.iterate(); !it.end(); it.next()) {
		xml_elem e = it.elem();
		string etype = e.get_name();
		if (etype == "material") {
			// materials.
			bool is_shader_material = e.has_child("shader");
			std::auto_ptr<material> mat;
			material_glsl* matglsl = 0;
			if (is_shader_material) {
				xml_elem es = e.child("shader");
				matglsl = new material_glsl(e.attr("name"),
							    es.attr("vertex"), es.attr("fragment"));
				mat.reset(matglsl);
			} else {
				mat.reset(new material(e.attr("name")));
			}
			unsigned id = e.attru("id");
			mat_id_mapping[id] = mat.get();

			if (!is_shader_material) {
				mat->diffuse = read_color_from_dftd_model_file(e, "diffuse");
				mat->specular = read_color_from_dftd_model_file(e, "specular");
			}

			for (xml_elem::iterator it2 = e.iterate("map"); !it2.end(); it2.next()) {
				xml_elem emap = it2.elem();
				// check here for possible children of type "skin", fixme
				string type = emap.attr("type");
				if (is_shader_material) {
					if (matglsl->nrtex >= DFTD_MAX_TEXTURE_UNITS)
						throw xml_error(string("too many material maps for glsl material ") + type, emap.doc_name());
					matglsl->texmaps[matglsl->nrtex].reset(new material::map(emap));
					matglsl->texnames[matglsl->nrtex] = type;
					matglsl->nrtex++;
				} else if (type == "diffuse") {
					mat->colormap.reset(new material::map(emap));
				} else if (type == "normal") {
					mat->normalmap.reset(new material::map(emap));
				} else if (type == "specular") {
					mat->specularmap.reset(new material::map(emap));
				} else {
					throw xml_error(string("unknown material map type ") + type, emap.doc_name());
				}
			}

			if (!is_shader_material && e.has_child("shininess")) {
				xml_elem eshin = e.child("shininess");
				if (!eshin.has_attr("exponent"))
					throw xml_error("shininess defined but no exponent given!", e.doc_name());
				mat->shininess = eshin.attrf("exponent");
			}

			if (is_shader_material)
				matglsl->compute_texloc();

			materials.push_back(0); // exception safe
			materials.back() = mat.release();
		} else if (etype == "mesh") {
			// meshes.
			mesh* msh = new mesh();
			meshes.push_back(msh);
			msh->name = e.attr("name");
			// material
			if (e.has_attr("material")) {
				unsigned matid = e.attru("material");
				map<unsigned, material* >::iterator it = mat_id_mapping.find(matid);
				if (it != mat_id_mapping.end()) {
					msh->mymaterial = it->second;
				} else {
					throw xml_error(string("referenced unknown material id, mesh ") + msh->name, e.doc_name());
				}
			}
			// vertices
			xml_elem verts = e.child("vertices");
			unsigned nrverts = verts.attru("nr");
			string values = verts.child_text();
			msh->vertices.reserve(nrverts);
			string::size_type valuepos = 0;
			for (unsigned i = 0; i < nrverts; ++i) {
				float x, y, z;
				// no stream here because of NaN strings that would break the stream
				string value = next_part_of_string(values, valuepos);
				x = atof(value.c_str());
				value = next_part_of_string(values, valuepos);
				y = atof(value.c_str());
				value = next_part_of_string(values, valuepos);
				z = atof(value.c_str());
				msh->vertices.push_back(vector3f(x, y, z));
			}
			// indices
			xml_elem indis = e.child("indices");
			unsigned nrindis = indis.attru("nr");
			if (indis.has_attr("type")) {
				string idxtype = indis.attr("type");
				if (idxtype == "triangles")
					msh->set_indices_type(mesh::pt_triangles);
				else if (idxtype == "triangle_strip")
					msh->set_indices_type(mesh::pt_triangle_strip);
				else
					throw xml_error(string("invalid indices type, mesh ") + msh->name, filename);
			}
			values = indis.child_text();
			msh->indices.reserve(nrindis);
			istringstream issi(values);
			for (unsigned i = 0; i < nrindis; ++i) {
				unsigned idx;
				issi >> idx;
				if (idx >= nrverts)
					throw xml_error(string("vertex index out of range, mesh ") + msh->name, filename);
				msh->indices.push_back(idx);
			}
			// tex coords
			if (msh->mymaterial) {
				xml_elem texcs = e.child("texcoords");
				msh->texcoords.reserve(nrverts);
				values = texcs.child_text();
				string::size_type valuepos = 0;
				for (unsigned i = 0; i < nrverts; ++i) {
					float x, y;
					// no stream here because of NaN strings that would break the stream
					string value = next_part_of_string(values, valuepos);
					x = atof(value.c_str());
					value = next_part_of_string(values, valuepos);
					y = atof(value.c_str());
					msh->texcoords.push_back(vector2f(x, y));
				}
			}

			// normals
			if (e.has_child("normals")) {
				msh->normals.reserve(nrverts);
				values = e.child("normals").child_text();
				string::size_type valuepos = 0;
				for (unsigned i = 0; i < nrverts; ++i) {
					float x, y, z;
					// no stream here because of NaN strings that would break the stream
					string value = next_part_of_string(values, valuepos);
					x = atof(value.c_str());
					value = next_part_of_string(values, valuepos);
					y = atof(value.c_str());
					value = next_part_of_string(values, valuepos);
					z = atof(value.c_str());
					msh->normals.push_back(vector3f(x, y, z));
				}
			}

			// transformation
			if (e.has_child("transformation")) {
				values = e.child("transformation").child_text();
				istringstream isst(values);
				for (unsigned y = 0; y < 4; ++y) {
					for (unsigned x = 0; x < 4; ++x) {
						isst >> msh->transformation.elem(x, y);
					}
				}
			}
		} else if (etype == "objecttree") {
			++nr_of_objecttrees;
			if (nr_of_objecttrees > 1)
				throw xml_error("more than one object tree defined!", e.doc_name());
		} else {
			throw xml_error(string("unknown tag type ") + etype, e.doc_name());
		}
	}

	// check if we have an objecttree and read it
	if (root.has_child("objecttree")) {
		read_objects(root.child("objecttree"), scene);
	} else {
		// create default objects for all objects in the scene
		// fixme
	}
}



void model::read_objects(const xml_elem& parent, object& parentobj)
{
	for (xml_elem::iterator it = parent.iterate("object"); !it.end(); it.next()) {
		xml_elem e = it.elem();
		mesh* msh = 0;
		if (e.has_attr("mesh")) {
			unsigned meshid = e.attru("mesh");
			if (meshid >= meshes.size())
				throw xml_error("illegal mesh id in object node", e.doc_name());
			msh = meshes[meshid];
		}
		object obj(e.attru("id"), e.attr("name"), msh);
		if (e.has_child("translation")) {
			xml_elem t = e.child("translation");
			string vec = t.attr("vector");
			istringstream iss(vec);
			iss >> obj.translation.x >> obj.translation.y >> obj.translation.z;
			if (t.has_attr("constraint")) {
				string c = t.attr("constraint");
				istringstream iss2(c);
				string tmp;
				iss2 >> tmp >> obj.trans_val_min >> obj.trans_val_max;
				if (tmp == "x")
					obj.translation_constraint_axis = 0;
				else if (tmp == "y")
					obj.translation_constraint_axis = 1;
				else
					obj.translation_constraint_axis = 2;
			}
		}
		if (e.has_child("rotation")) {
			xml_elem r = e.child("rotation");
			string axis = r.attr("axis");
			istringstream iss(axis);
			iss >> obj.rotat_axis.x >> obj.rotat_axis.y >> obj.rotat_axis.z;
			obj.rotat_angle = r.attrf("angle");
			obj.rotat_angle_min = r.attrf("minangle");
			obj.rotat_angle_max = r.attrf("maxangle");
		}
		read_objects(e, obj);
		parentobj.children.push_back(obj);
	}
}

// -------------------------------- end of dftd model file reading ------------------------------

bool model::set_object_angle(unsigned objid, double ang)
{
	object* obj = scene.find(objid);
	if (!obj) return false;
	return obj->set_angle(ang);
}



bool model::set_object_angle(const std::string& objname, double ang)
{
	object* obj = scene.find(objname);
	if (!obj) return false;
	return obj->set_angle(ang);
}



vector2f model::get_object_angle_constraints(unsigned objid)
{
	object* obj = scene.find(objid);
	if (!obj) return vector2f();
	return vector2f(obj->rotat_angle_min, obj->rotat_angle_max);
}



vector2f model::get_object_angle_constraints(const std::string& objname)
{
	object* obj = scene.find(objname);
	if (!obj) return vector2f();
	return vector2f(obj->rotat_angle_min, obj->rotat_angle_max);
}



bool model::set_object_translation(unsigned objid, double value)
{
	object* obj = scene.find(objid);
	if (!obj) return false;
	return obj->set_translation(value);
}



bool model::set_object_translation(const std::string& objname, double value)
{
	object* obj = scene.find(objname);
	if (!obj) return false;
	return obj->set_translation(value);
}



vector2f model::get_object_translation_constraints(unsigned objid)
{
	object* obj = scene.find(objid);
	if (!obj) return vector2f();
	return vector2f(obj->trans_val_min, obj->trans_val_max);
}



vector2f model::get_object_translation_constraints(const std::string& objname)
{
	object* obj = scene.find(objname);
	if (!obj) return vector2f();
	return vector2f(obj->trans_val_min, obj->trans_val_max);
}



void model::register_layout(const std::string& name)
{
//	cout << "register layout '" << name << "' for model '" << filename << "'\n";
	if (name.length() == 0) {
		throw error(filename + ": trying to register empty layout!");
	}
	for (vector<material*>::iterator it = materials.begin(); it != materials.end(); ++it)
		(*it)->register_layout(name, basepath);
}



void model::unregister_layout(const std::string& name)
{
	if (name.length() == 0) {
		throw error(filename + ": trying to unregister empty layout!");
	}
	for (vector<material*>::iterator it = materials.begin(); it != materials.end(); ++it)
		(*it)->unregister_layout(name);
}



void model::get_all_layout_names(std::set<std::string>& result) const
{
	for (vector<material*>::const_iterator it = materials.begin(); it != materials.end(); ++it)
		(*it)->get_all_layout_names(result);
	result.insert(default_layout);
}



/* must handle object-tree, later...
bool model::is_inside(const vector3f& p) const
{
	for (vector<mesh*>::const_iterator it = meshes.begin(); it != meshes.end(); ++it) {
		if ((*it)->is_inside(p))
			return true;
	}
	return false;
}
*/



matrix4f model::get_base_mesh_transformation() const
{
	if (scene.children.empty())
		return matrix4f::one();
	return scene.children.front().get_transformation() * scene.children.front().mymesh->transformation;
}



unsigned model::get_voxel_closest_to(const vector3f& pos)
{
	matrix4f transmat = get_base_mesh_transformation()
		* matrix4f::diagonal(voxel_size);
	unsigned closestvoxel = 0;
	double dist = 1e30;
	for (unsigned i = 0; i < voxel_data.size(); ++i) {
		vector3f p = transmat.mul4vec3xlat(voxel_data[i].relative_position);
		double d = p.square_distance(pos);
		if (d < dist) {
			dist = d;
			closestvoxel = i+1;
		}
	}
	if (!closestvoxel)
		throw error("no voxel data available");
	return closestvoxel-1;
}



// maybe add a function that returns all voxels within a sphere
// that way damage can be computed better, as damage is not applied to
// a point but a full subspace of 3-space.
// with the usual 5x7x7 partition a voxel for a 20m*100m wide*long ship is ca. 
// 4m*14m in size, so a torpedo can damage several voxels...
// here it is:
std::vector<unsigned> model::get_voxels_within_sphere(const vector3f& pos, double radius)
{
	matrix4f transmat = get_base_mesh_transformation()
		* matrix4f::diagonal(voxel_size);
	double rad2 = radius*radius;
	std::vector<unsigned> result;
	result.reserve(8); // should be typically enough
	for (unsigned i = 0; i < voxel_data.size(); ++i) {
		vector3f p = transmat.mul4vec3xlat(voxel_data[i].relative_position);
		double d = p.square_distance(pos);
		if (d <= rad2)
			result.push_back(i);
	}
	return result;
}
