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
#if (defined(__APPLE__) && defined(__GNUC__)) || defined(__MACOSX__)
#include <complex.h>
#ifndef isfinite
#define isfinite(x) finite(x)
#endif
#elif defined(WIN32)
#include <float.h>
#ifndef isfinite
#define isfinite(x) _finite(x)
#endif
#else
using std::isfinite;
#endif

#include "system.h"
#include "datadirs.h"
#include "oglext/OglExt.h"
#include "matrix4.h"
#include "binstream.h"
#include "xml.h"
#include <sstream>
#include <map>

using namespace std;

texture::mapping_mode model::mapping = texture::LINEAR_MIPMAP_LINEAR;//texture::NEAREST;
bool model::enable_shaders = true;

unsigned model::init_count = 0;
bool model::use_shaders = false;

auto_ptr<glsl_shader_setup> model::glsl_color_normal;
auto_ptr<glsl_shader_setup> model::glsl_color_normal_specular;
auto_ptr<glsl_shader_setup> model::glsl_mirror_clip;

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



void model::object::display() const
{
	glPushMatrix();
	glTranslated(translation.x, translation.y, translation.z);
	glRotated(rotat_angle, rotat_axis.x, rotat_axis.y, rotat_axis.z);
	if (mymesh) mymesh->display();
	for (vector<object>::const_iterator it = children.begin(); it != children.end(); ++it) {
		it->display();
	}
	glPopMatrix();
}



void model::render_init()
{
	use_shaders = enable_shaders && glsl_program::supported();

	// initialize shaders if wanted
	if (use_shaders) {
		sys().add_console("Using OpenGL GLSL shaders...");

		glsl_shader::defines_list dl;
		dl.push_back("USE_SPECULARMAP");
		glsl_color_normal.reset(new glsl_shader_setup(get_shader_dir() + "modelrender.vshader",
							      get_shader_dir() + "modelrender.fshader"));
		glsl_color_normal_specular.reset(new glsl_shader_setup(get_shader_dir() + "modelrender.vshader",
								       get_shader_dir() + "modelrender.fshader", dl));
 		glsl_mirror_clip.reset(new glsl_shader_setup(get_shader_dir() + "modelrender_mirrorclip.vshader",
							     get_shader_dir() + "modelrender_mirrorclip.fshader"));
	}
}



void model::render_deinit()
{
	if (use_shaders) {
		glsl_program::use_fixed();
		glsl_color_normal.reset();
		glsl_color_normal_specular.reset();
		glsl_mirror_clip.reset();
	}
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
	if (extension == ".3ds") {
		m3ds_load(filename2);
	} else if (extension == ".off") {
		read_off_file(filename2);
	} else if (extension == ".xml" || extension == ".ddxml") {
		read_dftd_model_file(filename2);
	} else {
		throw error(string("model: unknown extension or file format: ") + filename2);
	}

	read_phys_file(filename2);	// try to read cross section file

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
	//fixme: with the relations and the objecttree this is not right...
	if (meshes.size() == 0) return;
	meshes[0]->compute_bounds();
	min = meshes[0]->min;
	max = meshes[0]->max;

	for (vector<model::mesh*>::iterator it = ++meshes.begin(); it != meshes.end(); ++it) {
		(*it)->compute_bounds();
		min = (*it)->min.min(min);
		max = (*it)->max.max(max);
	}
}



void model::compute_normals()
{
	for (vector<model::mesh*>::iterator it = meshes.begin(); it != meshes.end(); ++it) {
		try {
			(*it)->compute_normals();
		}
		catch (std::exception& e) {
			// if normal generation fails (happens for some broken 3ds models),
			// ignore that, do not generate normals for such a mesh
		}
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



void model::mesh::compute_bounds()
{
	if (vertices.size() == 0) return;
	min = max = vertices[0];

	for (vector<vector3f>::iterator it2 = ++vertices.begin(); it2 != vertices.end(); ++it2) {
		min = it2->min(min);
		max = it2->max(max);
	}
	
	//fixme: we have to modify min/max according to the transformation matrix! at least the translation...
	min += transformation.column3(3);
	max += transformation.column3(3);
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
			if (isfinite(lf)) {
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
		// die normale umdrehen? nö, dann geht der Rest nicht mehr. Wie viele lefthanded-Koordinaten gibt es denn so?
		// was sagt Luis dazu?
		// das wird sich aber wohl nicht vermeiden lassen?
		// tangenty wird nicht übertragen, aber wenn man tangentx umdreht, geht das auch?!
		// righthanded aber erst versuchen zu entfernen, wenn der rest geht!!!
		// der mix aus zwei Koordinatensystemen kann eigentlich nicht gut gehen.
		// wird aber vermutlich gebraucht um nur eine Seitenansicht für beide Rumpfseiten zu nehmen, aber das macht
		// Luis ja gar nicht...
		righthanded[i0] = !(g > 0); // fixme: negation seems needed???
		return true;
	}
}



model::mesh::mesh(const string& nm)
	: name(nm),
	  indices_type(pt_triangles),
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
}



model::mesh::mesh(unsigned w, unsigned h, const std::vector<float>& heights, const vector3f& scales,
		  const vector3f& trans,
		  const std::string& nm)
	: name(nm),
	  indices_type(pt_triangle_strip),
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



void model::mesh::compile()
{
	bool has_texture_u0 = false, has_texture_u1 = false;
	bool normalmapping = false;
	if (mymaterial != 0) {
		if (mymaterial->colormap.get())
			has_texture_u0 = true;
		if (mymaterial->normalmap.get())
			has_texture_u1 = true;
		normalmapping = has_texture_u1;	// maybe more options here...
	}
	const unsigned vs = vertices.size();

	// vertices
	vbo_positions.init_data(sizeof(vector3f) * vs, &vertices[0].x, GL_STATIC_DRAW);
	// normals
	if (!normalmapping || use_shaders) {
		vbo_normals.init_data(sizeof(vector3f) * vs, &normals[0].x, GL_STATIC_DRAW);
	}
	// texcoords
	if (has_texture_u0 && texcoords.size() == vs) {
		vbo_texcoords.init_data(sizeof(vector2f) * vs, &texcoords[0].x, GL_STATIC_DRAW);
	}
	// auxiliary data
	if (use_shaders) {
		// give tangents as texture coordinates for unit 1.
		if (has_texture_u0 && tangentsx.size() == vs) {
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
			gss.use_fixed();
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
	if (indices_type != pt_triangles) throw std::runtime_error("write_off_file: can't handle primitives other than triangles!");

	FILE *f = fopen(fn.c_str(), "wb");
	if (!f) return;
	fprintf(f, "OFF\n%u %u %u\n", vertices.size(), indices.size()/3, 0);
	
	for (unsigned i = 0; i < vertices.size(); i++) {
		fprintf(f, "%f %f %f\n", vertices[i].x, vertices[i].y, vertices[i].z);
	}
	for (unsigned j = 0; j < indices.size(); j += 3) {
		fprintf(f, "3 %u %u %u\n", indices[j], indices[j+1], indices[j+2]);
	}
	fclose(f);
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
	// transform p to mesh space
	matrix4f invtrans = transformation.inverse();
	vector3f pp = invtrans * p;

	/* algorithm:
	   for every triangle of the mesh, build a tetrahedron of the three
	   points of the triangle and the center of the mesh (e.g. center
	   of gravity). For all tetrahedrons that pp is in, count the
	   tetrahedrons with "positive" volume and "negative" volume.
	   The former are all tetrahedrons where the triangle is facing
	   away from the center point, the latter are all tetrahedrons,
	   where the triangle is facing the center point.
	   A point pp is inside the tetrahedron consisting of A, B, C, D
	   when: b = B-A, c = C-A, d = D-A, and pp = A+r*b+s*c+t*d
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
		if ((pp - A).solve(b, c, d, s, r, t)) {
			if (r >= 0.0f && s >= 0.0f && t >= 0.0f && r+s+t <= 1.0f) {
				// pp is inside the tetrahedron
				bool facing_to_D = b.cross(c) * d >= 0;
				in_out_count += facing_to_D ? -1 : 1;
			}
		}
	} while (tit->next());
	// for tests:
	//std::cout << "is_inside p=" << p << " pp=" << pp << " ioc=" << in_out_count << "\n";
	return in_out_count > 0;
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
	//fixme: transform result by transformation matrix?
	return (1.0/vdiv) * vsum;
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
matrix3 model::mesh::compute_inertia_tensor() const
{
	matrix3 msum;
	const double mass = 1.0; // fixme
	const vector3 center_of_gravity = compute_center_of_gravity(); // fixme, set later
	double vdiv = 0;
	std::auto_ptr<triangle_iterator> tit(get_tri_iterator());
	do {
		unsigned i0 = tit->i0();
		unsigned i1 = tit->i1();
		unsigned i2 = tit->i2();
		vector3 A = vertices[i0];
		vector3 B = vertices[i1];
		vector3 C = vertices[i2];
		const vector3& D = center_of_gravity;
		vector3 abcd = A + B + C + D;
		double V_i = (1.0/6.0) * ((A - D) * (B - D).cross(C - D));
		double fac0 = V_i / 20.0; // 6*20=120
		matrix3 abcd2 = matrix_of_vec_sqr(abcd);
		matrix3 A2 = matrix_of_vec_sqr(A);
		matrix3 B2 = matrix_of_vec_sqr(B);
		matrix3 C2 = matrix_of_vec_sqr(C);
		matrix3 D2 = matrix_of_vec_sqr(D);
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
	//fixme: transform result by transformation matrix?
	return msum * (mass/vdiv);
}



model::material::map::map()
	: uscal(1.0f), vscal(1.0f), uoffset(0.0f), voffset(0.0f), angle(0.0f),
	  tex(0), ref_count(0)
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
			it->second.mytexture = new texture(basepath + it->second.filename, mapping, texture::CLAMP_TO_EDGE,
							   makenormalmap, detailh, rgb2grey);
		}
		++(it->second.ref_count);
	} else {
		if (ref_count == 0) {
			// load texture
			try {
				mytexture.reset(new texture(basepath + filename, mapping, texture::CLAMP_TO_EDGE,
							    makenormalmap, detailh, rgb2grey));
			}
			catch (std::exception& e) {
				mytexture.reset(new texture(get_texture_dir() + filename, mapping, texture::CLAMP_TO_EDGE,
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



void model::material::map::get_all_layout_names(std::set<std::string>& result) const
{
	for (std::map<string, skin>::const_iterator it = skins.begin(); it != skins.end(); ++it)
		result.insert(it->first);
}



model::material::material(const std::string& nm) : name(nm), shininess(50.0f)
{
}



void model::material::map::setup_glmatrix() const
{
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	glTranslatef(this->uoffset, this->voffset, 0);
	glRotatef(this->angle, 0, 0, 1);
	glScalef(this->uscal, this->vscal, 1);
	glMatrixMode(GL_MODELVIEW);
}



void model::material::map::set_gl_texture() const
{
	if (tex)
		tex->set_gl_texture();
	else
		throw error("set_gl_texture with empty texture");
}



void model::material::map::set_gl_texture(glsl_program& prog, const std::string& texname, unsigned texunitnr) const
{
	if (!tex)
		throw error("set_gl_texture(shader) with empty texture");
	prog.set_gl_texture(*tex, texname, texunitnr);
}



void model::material::map::set_gl_texture(glsl_shader_setup& gss, const std::string& texname, unsigned texunitnr) const
{
	if (!tex)
		throw error("set_gl_texture(shader) with empty texture");
	gss.set_gl_texture(*tex, texname, texunitnr);
}



void model::material::map::set_texture(texture* t)
{
	mytexture.reset(t);
	tex = t;
}



void model::material::set_gl_values() const
{
	if (use_shaders) {
		glsl_program::use_fixed();
	}

	glActiveTexture(GL_TEXTURE0);
	if (colormap.get()) {
		if (normalmap.get()) {
			// no opengl lighting in normal map mode.
			glDisable(GL_LIGHTING);
			// set primary color alpha to one.
			glColor4f(1, 1, 1, 1);

			if (use_shaders) {

				// texture units / coordinates:
				// tex0: color map / matching texcoords
				// tex1: normal map / texcoords show vector to light
				// tex2: specular map / texcoords show vector to viewer, if available

				GLfloat coltmp[4];
				specular.store_rgba(coltmp);
				glMaterialfv(GL_FRONT, GL_SPECULAR, coltmp);
				glMaterialf(GL_FRONT, GL_SHININESS, shininess);

				if (specularmap.get()) {
					glsl_color_normal_specular->use();
					specularmap->set_gl_texture(*glsl_color_normal_specular, "tex_specular", 2);
					normalmap->set_gl_texture(*glsl_color_normal_specular, "tex_normal", 1);
					colormap->set_gl_texture(*glsl_color_normal_specular, "tex_color", 0);
				} else {
					glsl_color_normal->use();
					normalmap->set_gl_texture(*glsl_color_normal, "tex_normal", 1);
					colormap->set_gl_texture(*glsl_color_normal, "tex_color", 0);
				}

			} else {
				// standard OpenGL texturing with special tricks
				glActiveTexture(GL_TEXTURE0);
				normalmap->setup_glmatrix();
				normalmap->set_gl_texture();
				glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
				glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_DOT3_RGBA); 
				glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_PRIMARY_COLOR);
				glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
				glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_TEXTURE);
				glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
				glActiveTexture(GL_TEXTURE1);
				glEnable(GL_TEXTURE_2D);
				colormap->setup_glmatrix();
				colormap->set_gl_texture();

				vector4t<GLfloat> ldifcol, lambcol;
				glGetLightfv(GL_LIGHT0, GL_DIFFUSE, &ldifcol.x);
				glGetLightfv(GL_LIGHT0, GL_AMBIENT, &lambcol.x);
				ldifcol.w = lambcol.y;	// use ambient's y/green value as brightness value.

				// normal map function with ambient:
				// light_color * color * (normal_brightness * (1-ambient) + ambient)
				glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, &ldifcol.x);
				glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
				glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
				glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_CONSTANT);
				glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
				glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_TEXTURE);
				glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);

				glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_INTERPOLATE);
				// we need one here, so we take primary color alpha, which is one.
				// couldn't we just use GL_ONE as operand?
				glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_PRIMARY_COLOR);
				glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
				glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, GL_PREVIOUS);
				glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, GL_SRC_ALPHA);
				glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE2_ALPHA, GL_CONSTANT);
				glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND2_ALPHA, GL_SRC_ALPHA);
				glBlendFunc(GL_SRC_ALPHA, GL_ZERO);
			}
		} else {
			// just standard lighting, no need for shaders.
			// Wrong! Shaders are needed for specular lighting.
			// So give a texture with a normal of (0,0,1) to unit 1.
			// (Color 128,128,255).
			// fixme: stupid! we would need tangentsx/y too etc.
			// better use another shader for this case.
			// we can give a white texture as specular map or use a shader without specmap reading etc.
			// same for texmap. Only shader with or without normalmap is much different!
			glColor4f(1, 1, 1, 1);
			glActiveTexture(GL_TEXTURE0);
			colormap->set_gl_texture();
			colormap->setup_glmatrix();
//			cout << "uv off " << colormap->uoffset << "," << colormap->voffset << " ang " << colormap->angle << " scal " << colormap->uscal << "," << colormap->vscal << "\n";
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, 0);
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		}
		glMatrixMode(GL_MODELVIEW);
	} else {
		diffuse.set_gl_color();
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}



void model::material::set_gl_values_mirror_clip() const
{
	if (!use_shaders) return;

	glsl_mirror_clip->use();

	if (colormap.get()) {
		// plain texture mapping with diffuse lighting only, but with shaders
		glColor4f(1, 1, 1, 1);
		colormap->set_gl_texture(*glsl_mirror_clip, "tex_color", 0);
		colormap->setup_glmatrix();
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, 0);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	}
	glMatrixMode(GL_MODELVIEW);
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
	float normalmapheight = use_shaders ? 4.0f : 16.0f;
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



void model::mesh::display() const
{
	// set up material
	if (mymaterial != 0) {
		mymaterial->set_gl_values();
	} else {
		glBindTexture(GL_TEXTURE_2D, 0);
		glColor3f(0.5,0.5,0.5);
	}

	// local transformation matrix.
	glPushMatrix();
	transformation.multiply_glf();

	bool has_texture_u0 = false, has_texture_u1 = false;
	bool normalmapping = false;
	if (mymaterial != 0) {
		if (mymaterial->colormap.get())
			has_texture_u0 = true;
		if (mymaterial->normalmap.get())
			has_texture_u1 = true;
		normalmapping = has_texture_u1;	// maybe more options here...
	}

	// set up vertex data.
	vbo_positions.bind();
	glVertexPointer(3, GL_FLOAT, sizeof(vector3f), 0);
	glEnableClientState(GL_VERTEX_ARRAY);

	// set up normals (only used with shaders or for plain rendering without normal maps).
	if (!normalmapping || use_shaders) {
		vbo_normals.bind();
		glNormalPointer(GL_FLOAT, sizeof(vector3f), 0);
		glEnableClientState(GL_NORMAL_ARRAY);
	}

	// without pixel shaders texture coordinates must be set for both texture units and are the same.
	glClientActiveTexture(GL_TEXTURE0);
	if (has_texture_u0 && texcoords.size() == vertices.size()) {
		vbo_texcoords.bind();
		glTexCoordPointer(2, GL_FLOAT, sizeof(vector2f), 0);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	} else {
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	}

	if (use_shaders) {
		// Using vertex and fragment programs.
		// give tangents/righthanded info as vertex attribute.
		if (has_texture_u0 && tangentsx.size() == vertices.size()) {
			vbo_tangents_righthanded.bind();
			glVertexAttribPointer(vertex_attrib_index, 4, GL_FLOAT, GL_FALSE,
					      0, 0);
			glEnableVertexAttribArray(vertex_attrib_index);
		}
	} else {
		// No shaders, basic old OpenGL 1.5 techniques.
		glClientActiveTexture(GL_TEXTURE1);
		if (has_texture_u1 && texcoords.size() == vertices.size()) {
			vbo_texcoords.bind();
			// maybe offer second texture coords. how are they stored in .3ds files?!
			glTexCoordPointer(2, GL_FLOAT, sizeof(vector2f), 0);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		} else {
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		}

		//with normal mapping, we need colors.
		if (normalmapping) {
			vector4f lightpos_abs;
			glGetLightfv(GL_LIGHT0, GL_POSITION, &lightpos_abs.x);
			//lightpos_abs = vector4f(200,0,-117,0);//test hack
			//fixme: with directional light we have darker results... are some vectors not
			//of unit length then?
			matrix4f invmodelview = (matrix4f::get_gl(GL_MODELVIEW_MATRIX)).inverse();
			vector4f lightpos_rel = invmodelview * lightpos_abs;
			//cout << "lightpos abs " << lightpos_abs << "\n";
			//cout << "lightpos rel " << lightpos_rel << "\n";
			vector3f lightpos3 = lightpos_rel.to_real();
			//cout << "lightpos3 " << lightpos3 << "," << lightpos_rel.xyz() << "," << ((lightpos_rel.w != 0.0f)) << "\n";
			vector<Uint8> colors(3*vertices.size());
			// this loop could be speed up with mmx/sse instructions.
			// however, with shaders it isn't used anyway and more and more computers
			// have cards with shaders. at least newer computers that can do sse should
			// also have shaders, no shaders but sse is rather rare...
			for (unsigned i = 0; i < vertices.size(); ++i) {
				const vector3f& nx = tangentsx[i];
				const vector3f& nz = normals[i];
				vector3f ny = nz.cross(nx);

				//fixme: ny length is not always 1, which can only happen when nx and nz are not othogonal
				//but they should be constructed that way!!!
				//but maybe this is because of unset tangentsx!!! yes, see above
				//cout << nx.length() << "," << ny.length() << "," << nz.length() << " vert " << i << " <-------------\n";

				vector3f lp = (lightpos_rel.w != 0.0f) ? (lightpos3 - vertices[i]) : lightpos_rel.xyz();
				vector3f nl = vector3f(nx * lp, ny * lp, nz * lp).normal();

				// swap light Y direction if in left handed coordinate system
				//test hack: do it the other way round. seems to be necessary?! maybe texture normals are computed wrongly?! no, now it is ok. but tex-y-coordinates are reversed... why it works then?!
				if (!righthanded[i]) nl.y = -nl.y;

				const float s = 127.5f;
				colors[3*i+0] = Uint8(nl.x*s + s);
				colors[3*i+1] = Uint8(nl.y*s + s);
				colors[3*i+2] = Uint8(nl.z*s + s);
			}
			vbo_colors.init_data(3 * vertices.size(), &colors[0], GL_STREAM_DRAW);
			vbo_colors.bind();	// do not forget to bind again
			glColorPointer(3, GL_UNSIGNED_BYTE, 0, 0);
			glEnableClientState(GL_COLOR_ARRAY);
			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		} else {
			glDisableClientState(GL_COLOR_ARRAY);
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
	if (use_shaders) {
		glsl_shader_setup::use_fixed();
		glActiveTexture(GL_TEXTURE2);
		glDisable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE0);
		glDisableVertexAttribArray(vertex_attrib_index);
	}
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glClientActiveTexture(GL_TEXTURE1);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);	// disable tex1
	glClientActiveTexture(GL_TEXTURE0);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);	// disable tex0

	// local transformation matrix.
	glPopMatrix();

	// clean up for material
	glEnable(GL_LIGHTING);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}



void model::mesh::display_mirror_clip() const
{
	if (!use_shaders) return;

	// this should display the mesh with an additional world space z=0 clip plane.
	// the model needs no fancy effects like specular lighting or bump mapping.
	// so plain diffuse texture mapping is enough. We just need texture
	// coordinates and per vertex normals. This can be displayed with
	// plain OpenGL. But we need shaders for clipping.
	// set simple shaders here, no matter which material we have...

	//fixme: what is with local transformation(s) ?

	bool has_texture_u0 = false;
	if (mymaterial != 0) {
		if (mymaterial->colormap.get())
			has_texture_u0 = true;
		// fixme: check for mirror
		mymaterial->set_gl_values_mirror_clip();
	} else {
		glBindTexture(GL_TEXTURE_2D, 0);
		glColor3f(0.5,0.5,0.5);
		glsl_mirror_clip->use();
	}

	// multiply extra transformation to texture[1] matrix
	glActiveTexture(GL_TEXTURE1);
	glMatrixMode(GL_TEXTURE);
	transformation.multiply_glf();
	glMatrixMode(GL_MODELVIEW);
	glActiveTexture(GL_TEXTURE0);

	vbo_positions.bind();
	glVertexPointer(3, GL_FLOAT, sizeof(vector3f), 0);
	glEnableClientState(GL_VERTEX_ARRAY);

	// basic lighting, we need normals
	vbo_normals.bind();
	glNormalPointer(GL_FLOAT, sizeof(vector3f), 0);
	glEnableClientState(GL_NORMAL_ARRAY);

	// and texture coordinates
	glClientActiveTexture(GL_TEXTURE0);
	if (has_texture_u0 && texcoords.size() == vertices.size()) {
		vbo_texcoords.bind();
		glTexCoordPointer(2, GL_FLOAT, sizeof(vector2f), 0);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	} else {
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	}

	// unbind VBOs
	vbo_positions.unbind();

	// render geometry
	index_data.bind();
	glDrawRangeElements(gl_primitive_type(), 0, vertices.size()-1, indices.size(), GL_UNSIGNED_INT, 0);
	index_data.unbind();

	if (use_shaders) {
		glsl_shader_setup::use_fixed();
	}

	glEnable(GL_LIGHTING);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glClientActiveTexture(GL_TEXTURE1);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);	// disable tex1
	glClientActiveTexture(GL_TEXTURE0);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);	// disable tex0
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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



void model::display() const
{
	if (current_layout.length() == 0) {
		throw error(filename + ": trying to render model, but no layout was set yet");
	}

	// default scene: no objects, just draw all meshes.
	if (scene.children.size() == 0) {
		for (vector<model::mesh*>::const_iterator it = meshes.begin(); it != meshes.end(); ++it) {
			(*it)->display();
		}
	} else {
		scene.display();
	}

	// reset texture units
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	glColor4f(1,1,1,1);
	glMatrixMode(GL_MODELVIEW);
}



void model::display_mirror_clip() const
{
	// fixme: add object tree drawing here!
	for (vector<model::mesh*>::const_iterator it = meshes.begin(); it != meshes.end(); ++it) {
		(*it)->display_mirror_clip();
	}

	// reset texture units
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	glColor4f(1,1,1,1);
	glMatrixMode(GL_MODELVIEW);
}



model::mesh& model::get_mesh(unsigned nr)
{
	return *meshes.at(nr);
}

const model::mesh& model::get_mesh(unsigned nr) const
{
	return *meshes.at(nr);
}

model::material& model::get_material(unsigned nr)
{
	return *(materials.at(nr));
}

const model::material& model::get_material(unsigned nr) const
{
	return *(materials.at(nr));
}

model::light& model::get_light(unsigned nr)
{
	return lights.at(nr);
}

const model::light& model::get_light(unsigned nr) const
{
	return lights.at(nr);
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
	mesh& m = get_mesh(0);
	istringstream iss2(physroot.child("inertia-tensor").child_text());
	m.inertia_tensor = matrix3(iss2);

	// set volume of mesh #0
	m.volume = physroot.child("volume").attrf();
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



void model::light::set_gl(unsigned nr_of_light) const
{
	GLfloat tmp[4] = { pos.x, pos.y, pos.z, 1.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, tmp);
	tmp[0] = tmp[1] = tmp[2] = ambient;
	glLightfv(GL_LIGHT0, GL_AMBIENT, tmp);
	tmp[0] = colr; tmp[1] = colg; tmp[2] = colb;
	glLightfv(GL_LIGHT0, GL_DIFFUSE, tmp);
	glLightfv(GL_LIGHT0, GL_SPECULAR, tmp);
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
			m->specularmap->write_to_dftd_model_file(mat, "specular", false);
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

	// save lights.
	for (vector<light>::const_iterator it = lights.begin(); it != lights.end(); ++it) {
		xml_elem lgt = root.add_child("light");
		lgt.set_attr(it->name, "name");
		ostringstream lpos; lpos << it->pos.x << " " << it->pos.y << " " << it->pos.z;
		lgt.set_attr(lpos.str(), "pos");
		ostringstream lcol; lcol << it->colr << " " << it->colg << " " << it->colb;
		lgt.set_attr(lpos.str(), "color");
		lgt.set_attr(it->ambient, "ambient");
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
						    const std::string& type, bool withtrans) const
{
	xml_elem mmap = parent.add_child("map");
	// write here possible skin children, fixme
	mmap.set_attr(type, "type");
	mmap.set_attr(filename, "filename");
	if (withtrans) {
		mmap.set_attr(uscal, "uscal");
		mmap.set_attr(vscal, "vscal");
		mmap.set_attr(uoffset, "uoffset");
		mmap.set_attr(voffset, "voffset");
		mmap.set_attr(angle, "angle");
	}
	// skins
	for (std::map<string, skin>::const_iterator it = skins.begin(); it != skins.end(); ++it) {
		xml_elem s = mmap.add_child("skin");
		s.set_attr(it->second.filename, "filename");
		s.set_attr(it->first, "layout");
	}
}



model::material::map::map(const xml_elem& parent, bool withtrans)
	: uscal(1.0f), vscal(1.0f),
	  uoffset(0.0f), voffset(0.0f), angle(0.0f),
	  tex(0), ref_count(0)
{
	if (!parent.has_attr("filename"))
		throw xml_error("no filename given for materialmap!", parent.doc_name());
	filename = parent.attr("filename");
	if (withtrans) {
		if (parent.has_attr("uscal"))
			uscal = parent.attrf("uscal");
		if (parent.has_attr("vscal"))
			vscal = parent.attrf("vscal");
		if (parent.has_attr("uoffset"))
			uoffset = parent.attrf("uoffset");
		if (parent.has_attr("voffset"))
			voffset = parent.attrf("voffset");
		if (parent.has_attr("angle"))
			angle = parent.attrf("angle");
	}

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
// 		new texture(basepath + filename, mapping, texture::CLAMP_TO_EDGE,
// 			    makenormalmap, detailh, rgb2grey));
	}
}

// -------------------------------- end of dftd model file writing ------------------------------



// ------------------------------------------ 3ds loading functions -------------------------- 
// ----------------------------- 3ds file reading data ---------------------------
// taken from a NeHe tutorial (nehe.gamedev.net)
// indentations describe the tree structure
#define M3DS_COLOR	0x0010	// three floats
#define M3DS_COLOR24	0x0011	// 24bit truecolor
#define	M3DS_MAIN3DS	0x4D4D
#define 	M3DS_EDIT3DS	0x3D3D
#define 		M3DS_EDIT_MATERIAL	0xAFFF
#define 			M3DS_MATNAME	   	0xA000
#define 			M3DS_MATAMBIENT		0xA010
#define 			M3DS_MATDIFFUSE		0xA020
#define 			M3DS_MATSPECULAR	0xA030
#define 			M3DS_MATTRANSPARENCY	0xA050//not used yet, not needed!
#define 			M3DS_MATMAPTEX1		0xA200
#define 				M3DS_MATMAPFILE		0xA300
#define 				M3DS_MATMAP1OVERU_SCAL	0xA354
#define 				M3DS_MATMAP1OVERV_SCAL	0xA356
#define 				M3DS_MATMAPUOFFSET	0xA358
#define 				M3DS_MATMAPVOFFSET	0xA35A
#define 				M3DS_MATMAPANG		0xA35C
#define		 			M3DS_MATMAPAMOUNTBUMP	0xA252//double byte chunk, displayed amount of bump, not yet used
#define 			M3DS_MATMAPBUMP		0xA230
#define 		M3DS_EDIT_OBJECT	0x4000
#define 			M3DS_OBJ_TRIMESH   	0x4100
#define 				M3DS_TRI_VERTEXL	0x4110
#define 				M3DS_TRI_FACEL1		0x4120
#define 					M3DS_TRI_MATERIAL	0x4130
#define 				M3DS_TRI_MAPPINGCOORDS	0x4140
#define 				M3DS_TRI_MESHMATRIX  0x4160
#define				M3DS_OBJ_LIGHT	0x4600
#define 	M3DS_VERSION		0x0002
#define 	M3DS_KEYF3DS		0xB000
// ----------------------------- end of 3ds file reading data ----------------------

void model::m3ds_load(const string& fn)
{
	ifstream in(fn.c_str(), ios::in | ios::binary);
	m3ds_chunk head = m3ds_read_chunk(in);
	if (head.id != M3DS_MAIN3DS) {
		throw error(string("[model::load_m3ds] Unable to load PRIMARY chuck from file \"")+fn+string("\""));
	}
	m3ds_process_toplevel_chunks(in, head);
	head.skip(in);
}

void model::m3ds_chunk::skip(istream& in)
{
//cout << "skipped id " << id << " (hex " << (void*)id << ") while reading chunk.\n";
	if (length > bytes_read) {
		unsigned n = length - bytes_read;
		in.seekg(n, ios::cur);
	}
}

void model::m3ds_process_toplevel_chunks(istream& in, m3ds_chunk& parent)
{
	unsigned version;
	
	while (!parent.fully_read()) {
		m3ds_chunk ch = m3ds_read_chunk(in);
		switch (ch.id) {
			case M3DS_VERSION:
				version = read_u16(in);
				ch.bytes_read += 2;
				// version should be <= 0x03 or else give a warning
				// If the file version is over 3, give a warning that there could be a problem
				if (version > 0x03)
					cout << "warning: 3ds file version is > 0x03\n";
				break;
			case M3DS_EDIT3DS:
				m3ds_process_model_chunks(in, ch);
				break;
			case M3DS_KEYF3DS:	// just ignore
			default:
				break;
		}
		ch.skip(in);
		parent.bytes_read += ch.length;
	}
}

void model::m3ds_process_model_chunks(istream& in, m3ds_chunk& parent)
{
	while (!parent.fully_read()) {
		m3ds_chunk ch = m3ds_read_chunk(in);
		string objname;
		switch (ch.id) {
			case M3DS_EDIT_MATERIAL:
				m3ds_process_material_chunks(in, ch);
				break;
			case M3DS_EDIT_OBJECT:
				objname = m3ds_read_string(in, ch);	// read name
				m3ds_process_object_chunks(in, ch, objname);
				break;
		}
		ch.skip(in);
		parent.bytes_read += ch.length;
	}
}

void model::m3ds_process_object_chunks(istream& in, m3ds_chunk& parent, const string& objname)
{
	while (!parent.fully_read()) {
		m3ds_chunk ch = m3ds_read_chunk(in);
		switch (ch.id) {
		case M3DS_OBJ_TRIMESH:
			m3ds_process_trimesh_chunks(in, ch, objname);
			break;
		case M3DS_OBJ_LIGHT:
			m3ds_process_light_chunks(in, ch, objname);
			break;
		}
		ch.skip(in);
		parent.bytes_read += ch.length;
	}
}

void model::m3ds_process_trimesh_chunks(istream& in, m3ds_chunk& parent, const string& objname)
{
	mesh* msh = new mesh();
	meshes.push_back(msh);
	msh->name = objname;
	while (!parent.fully_read()) {
		m3ds_chunk ch = m3ds_read_chunk(in);
//	cout<<"found trimesh chunk"<<(void*)ch.id<<"\n";
		switch (ch.id) {
			case M3DS_TRI_VERTEXL:
				m3ds_read_vertices(in, ch, *msh);
				break;
			case M3DS_TRI_FACEL1:
				m3ds_read_faces(in, ch, *msh);
				m3ds_process_face_chunks(in, ch, *msh);
				break;
			case M3DS_TRI_MAPPINGCOORDS:
				m3ds_read_uv_coords(in, ch, *msh);
				break;

//fixme: this matrix seems to describe the model rotation and translation that IS ALREADY computed for the vertices
//but why are some models so much off the origin? (corvette, largefreighter)
//is there another chunk i missed while reading?
			case M3DS_TRI_MESHMATRIX:
				msh->transformation = matrix4f::one();
				// read floats and skip them.
				for (unsigned k = 0; k < 4*3; ++k)
					read_float(in);
				/*
				for (int j = 0; j < 4; ++j) {
					for (int i = 0; i < 3; ++i) {
						msh->transformation.elem(j,i) = read_float(in);
					}
				}
				*/
				ch.bytes_read += 4 * 3 * 4;
				break;
		}
		ch.skip(in);
		parent.bytes_read += ch.length;
	}
}

void model::m3ds_process_light_chunks(istream& in, m3ds_chunk& parent, const string& objname)
{
	light lg;
	lg.name = objname;
	lg.pos.x = read_float(in);
	lg.pos.y = read_float(in);
	lg.pos.z = read_float(in);
	parent.bytes_read += 4 * 3;
	while (!parent.fully_read()) {
		m3ds_chunk ch = m3ds_read_chunk(in);
		switch (ch.id) {
		case M3DS_COLOR:
			lg.colr = read_float(in);
			lg.colg = read_float(in);
			lg.colb = read_float(in);
			ch.bytes_read += 3 * 4;
			ch.skip(in);
			parent.bytes_read += ch.length;
			break;
		default:
			ch.skip(in);
			parent.bytes_read += ch.length;
		}
	}
	lights.push_back(lg);
}

void model::m3ds_process_face_chunks(istream& in, m3ds_chunk& parent, model::mesh& m)
{
	while (!parent.fully_read()) {
		m3ds_chunk ch = m3ds_read_chunk(in);
		switch (ch.id) {
			case M3DS_TRI_MATERIAL:
				m3ds_read_material(in, ch, m);
				break;
		}
		ch.skip(in);
		parent.bytes_read += ch.length;
	}
}

void model::m3ds_process_material_chunks(istream& in, m3ds_chunk& parent)
{
	material* m = new material();
	while (!parent.fully_read()) {
		m3ds_chunk ch = m3ds_read_chunk(in);
		switch (ch.id) {
			case M3DS_MATNAME:
				m->name = m3ds_read_string(in, ch);
				break;
/* unused.
			case M3DS_MATAMBIENT:
				m3ds_read_color_chunk(in, ch, m->ambient);
				break;
*/
			case M3DS_MATDIFFUSE:
				m3ds_read_color_chunk(in, ch, m->diffuse);
				break;
			case M3DS_MATSPECULAR:
				m3ds_read_color_chunk(in, ch, m->specular);
				break;
/* unused.
			case M3DS_MATTRANSPARENCY:
				m3ds_read_color_chunk(in, ch, m->transparency);
				break;
*/
			case M3DS_MATMAPTEX1:
				if (!m->colormap.get()) {
					m->colormap.reset(new material::map());
					m3ds_process_materialmap_chunks(in, ch, m->colormap.get());
				}
				break;
			case M3DS_MATMAPBUMP:
				if (!m->normalmap.get()) {
					m->normalmap.reset(new material::map());
					m3ds_process_materialmap_chunks(in, ch, m->normalmap.get());
				}
				break;
//			default: cout << "skipped chunk with id " << (void*)ch.id << "\n";
		}
		ch.skip(in);
		parent.bytes_read += ch.length;
	}

	// 3d studio uses diffuse color just for editor display
	if (m->colormap.get())
		m->diffuse = color::white();

	materials.push_back(m);
}

void model::m3ds_process_materialmap_chunks(istream& in, m3ds_chunk& parent, model::material::map* m)
{
	while (!parent.fully_read()) {
		m3ds_chunk ch = m3ds_read_chunk(in);
		switch (ch.id) {
			case M3DS_MATMAPFILE:
				m->filename = tolower(m3ds_read_string(in, ch));
				break;
			case M3DS_MATMAP1OVERU_SCAL:
				m->uscal = 1.0f/read_float(in);
				ch.bytes_read += 4;
				break;
			case M3DS_MATMAP1OVERV_SCAL:
				m->vscal = -1.0f/read_float(in);
				ch.bytes_read += 4;
				break;
			case M3DS_MATMAPUOFFSET:
				m->uoffset = read_float(in);
				ch.bytes_read += 4;
				break;
			case M3DS_MATMAPVOFFSET:
				m->voffset = read_float(in);
				ch.bytes_read += 4;
				break;
			case M3DS_MATMAPANG:
				m->angle = -read_float(in);
				ch.bytes_read += 4;
//			printf("angle %f\n",m->angle);
				break;
//			case M3DS_MATMAPAMOUNTBUMP://see above
		}
		ch.skip(in);
		parent.bytes_read += ch.length;
	}
	//cout << "[3ds] map: angle " << m->angle << " uscal,vscal " << m->uscal << "," << m->vscal << " uoff,voff " << m->uoffset << "," << m->voffset << "\n";
}

model::m3ds_chunk model::m3ds_read_chunk(istream& in)
{
	m3ds_chunk c;
	c.id = read_u16(in);
//printf("read chunk id %x\n", c.id);	
	c.length = read_u32(in);
	c.bytes_read = 6;
	return c;
}

string model::m3ds_read_string(istream& in, m3ds_chunk& ch)
{
	Uint8 c;
	string s;
	while (true) {
		c = read_u8(in);
		++ch.bytes_read;
		if (c == 0) break;
		s += c;
	}
	return s;
}

void model::m3ds_read_color_chunk(istream& in, m3ds_chunk& parent, color& col)
{
	m3ds_chunk ch = m3ds_read_chunk(in);
	col.r = read_u8(in);
	col.g = read_u8(in);
	col.b = read_u8(in);
	ch.bytes_read += 3;
	ch.skip(in);
	parent.bytes_read += ch.length;
}

void model::m3ds_read_faces(istream& in, m3ds_chunk& ch, model::mesh& m)
{
	unsigned nr_faces = read_u16(in);
	ch.bytes_read += 2;

	m.indices.clear();	
	m.indices.reserve(nr_faces*3);
	for (unsigned n = 0; n < nr_faces; ++n) {
		m.indices.push_back(read_u16(in));
		m.indices.push_back(read_u16(in));
		m.indices.push_back(read_u16(in));
		read_u16(in);	// ignore 4th value
	}
	ch.bytes_read += nr_faces * 4 * 2;
}

//float umin=1e10,umax=-1e10,vmin=1e10,vmax=-1e10;
void model::m3ds_read_uv_coords(istream& in, m3ds_chunk& ch, model::mesh& m)
{
	unsigned nr_uv_coords = read_u16(in);
	ch.bytes_read += 2;

	if (nr_uv_coords == 0) {
		m.texcoords.clear();
		return;
	}

//	use exception here:  sys().myassert(nr_uv_coords == m.vertices.size(), "number of texture coordinates doesn't match number of vertices");

	m.texcoords.clear();
	m.texcoords.reserve(nr_uv_coords);		
	for (unsigned n = 0; n < nr_uv_coords; ++n) {
		float u = read_float(in);
		float v = 1.0f - read_float(in); // for some reason we need to flip 3dstudio's y coords.
//		if(u<umin)umin=u;if(u>umax)umax=u;if(v<vmin)vmin=v;if(v>vmax)vmax=v;cout<<"u in "<<umin<<","<<umax<<" v in "<<vmin<<","<<vmax<<"\n";
		m.texcoords.push_back(vector2f(u, v));
	}
	ch.bytes_read += nr_uv_coords * 2 * 4;

	// make sure that number of uv coords matches number of vertices
	m.texcoords.resize(m.vertices.size());
}

void model::m3ds_read_vertices(istream& in, m3ds_chunk& ch, model::mesh& m)
{
	unsigned nr_verts = read_u16(in);
	ch.bytes_read += 2;
	
	m.vertices.clear();
	m.vertices.reserve(nr_verts);
	for (unsigned n = 0; n < nr_verts; ++n) {
		float x = read_float(in);
		float y = read_float(in);
		float z = read_float(in);
		m.vertices.push_back(vector3f(x, y, z));
	}
	ch.bytes_read += nr_verts * 3 * 4;
}

void model::m3ds_read_material(istream& in, m3ds_chunk& ch, model::mesh& m)
{
	string matname = m3ds_read_string(in, ch);

	for (vector<model::material*>::iterator it = materials.begin(); it != materials.end(); ++it) {
		if ((*it)->name == matname) {
			m.mymaterial = *it;
			return;
		}
	}
	throw error(filename + ": object has unknown material");
}		   

// -------------------------------- end of 3ds loading functions -----------------------------------


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

	// testing...
	light lg;
	lg.name = "testlight";
	lg.pos = vector3f(1000, 1000, 1000);
	lights.push_back(lg);

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
			material* mat = new material();
			mat->name = e.attr("name");
			unsigned id = e.attru("id");
			mat_id_mapping[id] = mat;

			mat->diffuse = read_color_from_dftd_model_file(e, "diffuse");
			mat->specular = read_color_from_dftd_model_file(e, "specular");

			for (xml_elem::iterator it2 = e.iterate("map"); !it2.end(); it2.next()) {
				xml_elem emap = it2.elem();
				// check here for possible children of type "skin", fixme
				string type = emap.attr("type");
				if (type == "diffuse") {
					mat->colormap.reset(new material::map(emap));
				} else if (type == "normal") {
					mat->normalmap.reset(new material::map(emap));
				} else if (type == "specular") {
					mat->specularmap.reset(new material::map(emap, false));
				} else {
					throw xml_error(string("unknown material map type ") + type, emap.doc_name());
				}
			}

			if (e.has_child("shininess")) {
				xml_elem eshin = e.child("shininess");
				if (!eshin.has_attr("exponent"))
					throw xml_error("shininess defined but no exponent given!", e.doc_name());
				mat->shininess = eshin.attrf("exponent");
			}

			materials.push_back(mat);
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
					msh->indices_type = mesh::pt_triangles;
				else if (idxtype == "triangle_strip")
					msh->indices_type = mesh::pt_triangle_strip;
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
		} else if (etype == "light") {
			// lights.
			light l;
			l.name = e.attr("name");
			if (!e.has_attr("pos"))
				throw xml_error("no pos for light given!", e.doc_name());
			istringstream issp(e.attr("pos"));
			issp >> l.pos.x >> l.pos.y >> l.pos.z;
			if (e.has_attr("color")) {
				istringstream issc(e.attr("color"));
				issp >> l.colr >> l.colg >> l.colb;
			}
			if (e.has_attr("ambient")) {
				l.ambient = e.attrf("ambient");
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
		//fixme: this would have to be done for 3ds and off reading too
		//and we would have to add new objects when new meshes are pushed back?! no necessarily...
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



bool model::is_inside(const vector3f& p) const
{
	for (vector<mesh*>::const_iterator it = meshes.begin(); it != meshes.end(); ++it) {
		if ((*it)->is_inside(p))
			return true;
	}
	return false;
}
