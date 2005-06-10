// A 3d model
// (C)+(W) by Thorsten Jordan. See LICENSE

#include "model.h"

#ifdef WIN32
#define PATH_SEPARATOR "\\"
#undef min
#undef max
#include <float.h>
#ifndef isfinite
#define isfinite(x) _finite(x)
#endif
#else
#define PATH_SEPARATOR "/"
#endif

#if (defined(__APPLE__) && defined(__GNUC__)) || defined(__MACOSX__)
#define isfinite(x) finite(x)
#endif

#include "system.h"
#include "global_data.h"
#include "oglext/OglExt.h"
#include "matrix4.h"
#include "tinyxml/tinyxml.h"
#include <sstream>

string modelpath;

texture::mapping_mode model::mapping = texture::LINEAR_MIPMAP_LINEAR;//texture::NEAREST;
bool model::enable_shaders = true;

unsigned model::init_count = 0;
bool model::vertex_program_supported = false;
bool model::fragment_program_supported = false;
bool model::compiled_vertex_arrays_supported = false;
bool model::use_shaders = false;
vector<GLuint> model::default_vertex_programs;
vector<GLuint> model::default_fragment_programs;


void model::render_init()
{
	vertex_program_supported = sys().extension_supported("GL_ARB_vertex_program");
	fragment_program_supported = sys().extension_supported("GL_ARB_fragment_program");
	compiled_vertex_arrays_supported = sys().extension_supported("GL_EXT_compiled_vertex_array");

	use_shaders = enable_shaders && vertex_program_supported && fragment_program_supported;

	// initialize shaders if wanted
	if (use_shaders) {
		default_fragment_programs.resize(NR_VFP);
		default_vertex_programs.resize(NR_VFP);
		list<string> defines;
		// fixme: create shaders for use with/without specularmap and/or normal map
		default_fragment_programs[VFP_COLOR_NORMAL] =
			texture::create_shader(GL_FRAGMENT_PROGRAM_ARB, get_shader_dir() + "modelrender_fp.shader" , defines);
		default_vertex_programs[VFP_COLOR_NORMAL] =
			texture::create_shader(GL_VERTEX_PROGRAM_ARB, get_shader_dir() + "modelrender_vp.shader" , defines);
		defines.push_back("USE_SPECULARMAP");
		default_fragment_programs[VFP_COLOR_NORMAL_SPECULAR] =
			texture::create_shader(GL_FRAGMENT_PROGRAM_ARB, get_shader_dir() + "modelrender_fp.shader" , defines);
		default_vertex_programs[VFP_COLOR_NORMAL_SPECULAR] =
			texture::create_shader(GL_VERTEX_PROGRAM_ARB, get_shader_dir() + "modelrender_vp.shader" , defines);
		sys().add_console("Using OpenGL vertex and fragment programs...");
	}
}



void model::render_deinit()
{
	if (use_shaders) {
		for (unsigned i = 0; i < NR_VFP; ++i) {
			texture::delete_shader(default_fragment_programs[i]);
			texture::delete_shader(default_vertex_programs[i]);
		}
		default_fragment_programs.clear();
		default_vertex_programs.clear();
	}
}



model::model()
{
	if (init_count == 0) render_init();
	++init_count;
}


model::model(const string& filename, bool use_material)
{
	if (init_count == 0) render_init();
	++init_count;

	string::size_type st = filename.rfind(".");
	string extension = (st == string::npos) ? "" : filename.substr(st);
	for (unsigned e = 0; e < extension.length(); ++e)
		extension[e] = ::tolower(extension[e]);
	st = filename.rfind(PATH_SEPARATOR);
	string path = (st == string::npos) ? "" : filename.substr(0, st+1);
	modelpath = path;
	basename = filename.substr(path.length(),
				   filename.length()-path.length()-extension.length());

	// determine loader by extension here.
	if (extension == ".3ds") {
		m3ds_load(filename);
	} else if (extension == ".off") {
		read_off_file(filename);
	} else if (extension == ".xml") {
		read_dftd_model_file(filename);
	} else {
		sys().myassert(false, "model: unknown extension or file format");
	}

	read_cs_file(filename);	// try to read cross section file

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
		(*it)->compute_normals();
	}
}

void model::mesh::compute_bounds()
{
	if (vertices.size() == 0) return;
	min = max = vertices[0];

	for (vector<vector3f>::iterator it2 = ++vertices.begin(); it2 != vertices.end(); ++it2) {
		min = it2->min(min);
		max = it2->max(max);
	}
}

void model::mesh::compute_normals()
{
	// auto-detecion of hard edges (creases) would be cool:
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
		for (unsigned i = 0; i < indices.size(); i += 3) {
			const vector3f& v0 = vertices[indices[i+0]];
			const vector3f& v1 = vertices[indices[i+1]];
			const vector3f& v2 = vertices[indices[i+2]];
			vector3f ortho = (v1-v0).orthogonal(v2-v0);
			// avoid degenerated triangles
			float lf = 1.0/ortho.length();
			if (isfinite(lf)) {
				vector3f face_normal = ortho * lf;
				//normals could be weighted by face area, that gives better results.
				normals[indices[i+0]] += face_normal;
				normals[indices[i+1]] += face_normal;
				normals[indices[i+2]] += face_normal;
			}
		}
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
	if (mymaterial && mymaterial->normalmap.get() && mymaterial->normalmap->mytexture.get()) {
		tangentsx.clear();
		tangentsx.resize(vertices.size(), vector3f(0, 0, 1));
		righthanded.clear();
		righthanded.resize(vertices.size(), 0);
		vector<bool> vertexok(vertices.size());
		for (unsigned i = 0; i < indices.size(); i += 3) {
			unsigned i0 = indices[i+0];
			unsigned i1 = indices[i+1];
			unsigned i2 = indices[i+2];
			if (!vertexok[i0])
				vertexok[i0] = compute_tangentx(i0, i1, i2);
			if (!vertexok[i1])
				vertexok[i1] = compute_tangentx(i1, i2, i0);
			if (!vertexok[i2])
				vertexok[i2] = compute_tangentx(i2, i0, i1);
		}
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

		//fixme: check with luis' freighter, it seems to have 2271 tangents only but > 3000 verts.
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
		righthanded[i0] = !(g > 0); // fixme: negation seems needed???
		return true;
	}
}



model::mesh::mesh(const string& nm) : name(nm), transformation(matrix4f::one()), mymaterial(0), display_list(0)
{
}



model::mesh::~mesh()
{
	if (display_list != 0) {
		glDeleteLists(display_list, 1);
	}
}



void model::mesh::compile()
{
	// check if display list can be compiled.
	if (use_shaders || !(mymaterial && mymaterial->normalmap.get())) {
		if (display_list == 0) {
			display_list = glGenLists(1);
			sys().myassert(display_list != 0, "no more display list indices available");
		}
		glNewList(display_list, GL_COMPILE);
		display(false);
		glEndList();
	} else {
		// delete unused list.
		glDeleteLists(display_list, 1);
		display_list = 0;
	}
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
		sys().myassert(splitptr == 2, "splitptr != 2 ?!");
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
		sys().myassert((newindi0ptr == 3 || newindi1ptr == 3) && (newindi0ptr + newindi1ptr == 7), "newindi ptr corrupt!");
	}

	return make_pair(part0, part1);
}



model::material::map::map() : uscal(1.0f), vscal(1.0f), uoffset(0.0f), voffset(0.0f), angle(0.0f)
{
}



void model::material::map::init(texture::mapping_mode mapping, bool makenormalmap, float detailh,
				bool rgb2grey)
{
	mytexture.reset();
	if (filename.length() > 0) {
		// fixme: which clamp mode should we use for models? REPEAT doesn't harm normally...
		mytexture.reset(new texture(get_texture_dir() + filename, mapping, texture::REPEAT,
					    makenormalmap, detailh, rgb2grey));
	}
}



model::material::material(const std::string& nm) : name(nm), shininess(50.0f)
{
}



void model::material::init()
{
	if (colormap.get()) colormap->init(model::mapping);
	//fixme: what is best mapping for normal maps?
	// compute normalmap if not given
	// fixme: segfaults when enabled. see texture.cpp
	// fixme: without shaders it seems we need to multiply this with ~16 or even more.
	// maybe because direction vectors are no longer normalized over faces...
	// with shaders a value of 1.0 is enough.
	// fixme: read value from model file... and multiply with this value...
	float normalmapheight = use_shaders ? 4.0f : 16.0f;
	if (normalmap.get()) normalmap->init(texture::LINEAR/*_MIPMAP_LINEAR*/, true, normalmapheight, true);
	if (specularmap.get()) specularmap->init(texture::LINEAR_MIPMAP_LINEAR, false, 0.0f, true);
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



void model::material::set_gl_values() const
{
	if (use_shaders) {
		glBindProgramARB(GL_VERTEX_PROGRAM_ARB, 0);
		glDisable(GL_VERTEX_PROGRAM_ARB);

		glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, 0);
		glDisable(GL_FRAGMENT_PROGRAM_ARB);
	}

	glActiveTexture(GL_TEXTURE0);
	if (colormap.get() && colormap->mytexture.get()) {
		if (normalmap.get() && normalmap->mytexture.get()) {
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

				if (specularmap.get() && specularmap->mytexture.get()) {
					glActiveTexture(GL_TEXTURE2);
					glEnable(GL_TEXTURE_2D);
					specularmap->mytexture->set_gl_texture();
					glBindProgramARB(GL_VERTEX_PROGRAM_ARB, default_vertex_programs[VFP_COLOR_NORMAL_SPECULAR]);
					glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, default_fragment_programs[VFP_COLOR_NORMAL_SPECULAR]);
				} else {
					glBindProgramARB(GL_VERTEX_PROGRAM_ARB, default_vertex_programs[VFP_COLOR_NORMAL]);
					glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, default_fragment_programs[VFP_COLOR_NORMAL]);
				}

				glEnable(GL_VERTEX_PROGRAM_ARB);
				glEnable(GL_FRAGMENT_PROGRAM_ARB);

				glActiveTexture(GL_TEXTURE1);
				normalmap->setup_glmatrix();
				normalmap->mytexture->set_gl_texture();

				glActiveTexture(GL_TEXTURE0);
				glEnable(GL_TEXTURE_2D);
				colormap->mytexture->set_gl_texture();
				colormap->setup_glmatrix();

			} else {
				// standard OpenGL texturing with special tricks
				glActiveTexture(GL_TEXTURE0);
				normalmap->setup_glmatrix();
				normalmap->mytexture->set_gl_texture();
				glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
				glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_DOT3_RGBA); 
				glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_PRIMARY_COLOR);
				glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
				glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_TEXTURE);
				glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
				glActiveTexture(GL_TEXTURE1);
				glEnable(GL_TEXTURE_2D);
				colormap->setup_glmatrix();
				colormap->mytexture->set_gl_texture();

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
			colormap->mytexture->set_gl_texture();
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
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

void model::mesh::display(bool use_display_list) const
{
	glPushMatrix();
	transformation.multiply_glf();

	if (display_list != 0 && use_display_list) {
		glCallList(display_list);
		glPopMatrix();
		return;
	}

	// colors may be needed for normal mapping.
	vector<Uint8> colors;

	bool has_texture_u0 = false, has_texture_u1 = false;
	bool normalmapping = false;
	if (mymaterial != 0) {
		if (mymaterial->colormap.get() && mymaterial->colormap->mytexture.get())
			has_texture_u0 = true;
		if (mymaterial->normalmap.get() && mymaterial->normalmap->mytexture.get())
			has_texture_u1 = true;
		normalmapping = has_texture_u1;	// maybe more options here...
		mymaterial->set_gl_values();
	} else {
		glBindTexture(GL_TEXTURE_2D, 0);
		glColor3f(0.5,0.5,0.5);
	}

	glVertexPointer(3, GL_FLOAT, sizeof(vector3f), &vertices[0]);
	glEnableClientState(GL_VERTEX_ARRAY);

	// fixme: with non-fragment-program normal mapping, we don't need normals!
	if (!normalmapping || use_shaders) {
		glNormalPointer(GL_FLOAT, sizeof(vector3f), &normals[0]);	
		glEnableClientState(GL_NORMAL_ARRAY);
	}

	// without pixel shaders texture coordinates must be set for both texture units and are the same.
	glClientActiveTexture(GL_TEXTURE0);
	if (has_texture_u0 && texcoords.size() == vertices.size()) {
		glTexCoordPointer(2, GL_FLOAT, sizeof(vector2f), &texcoords[0]);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	} else {
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	}

	glClientActiveTexture(GL_TEXTURE1);
	if (use_shaders) {
		// give tangents as texture coordinates for unit 1.
		if (has_texture_u0 && tangentsx.size() == vertices.size()) {
			glTexCoordPointer(3, GL_FLOAT, sizeof(vector3f), &tangentsx[0]);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		}

		// give righthanded info as colors (or default: always right handed)
		if (righthanded.size() == vertices.size()) {
			colors.resize(righthanded.size()*3);
			for (unsigned i = 0; i < righthanded.size(); ++i)
				colors[3*i] = (righthanded[i]) ? 255 : 0;
			glColorPointer(3, GL_UNSIGNED_BYTE, 0, &colors[0]);
			glEnableClientState(GL_COLOR_ARRAY);
		} else {
			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		}
	} else {
		if (has_texture_u1 && texcoords.size() == vertices.size()) {
			// maybe offer second texture coords. how are they stored in .3ds files?!
			glTexCoordPointer(2, GL_FLOAT, sizeof(vector2f), &texcoords[0]);
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
			colors.resize(3*vertices.size());
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
			glColorPointer(3, GL_UNSIGNED_BYTE, 0, &colors[0]);
			glEnableClientState(GL_COLOR_ARRAY);
		} else {
			glDisableClientState(GL_COLOR_ARRAY);
		}
	}

	// and finally draw the mesh.
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, &indices[0]);

#if 0
	//fixme: add code to show normals as Lines
#endif

	// cleanup
	if (use_shaders) {
		glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, 0);
		glDisable(GL_FRAGMENT_PROGRAM_ARB);
		glBindProgramARB(GL_VERTEX_PROGRAM_ARB, 0);
		glDisable(GL_VERTEX_PROGRAM_ARB);
		glActiveTexture(GL_TEXTURE2);
		glDisable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE0);
	}
	glEnable(GL_LIGHTING);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);	// disable tex1
	glClientActiveTexture(GL_TEXTURE0);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);	// disable tex0
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glPopMatrix();
}

void model::display() const
{
	for (vector<model::mesh*>::const_iterator it = meshes.begin(); it != meshes.end(); ++it) {
		(*it)->display();
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

void model::read_cs_file(const string& filename)
{
	string fn = filename.substr(0, filename.rfind(".")) + ".cs";
//cout << "read cs '" << fn << "'\n";
	ifstream in(fn.c_str(), ios::in | ios::binary);
	if (!in.good())
		return;
	unsigned cs;
	in >> cs;
//cout << "read cross sectons: " << cs << "\n";
	cross_sections.reserve(cs);
	for (unsigned i = 0; i < cs; ++i) {
		float tmp;
		in >> tmp;
//cout << tmp << ",";		
		cross_sections.push_back(tmp);
	}
//cout << "\n";
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
	TiXmlDocument doc(filename);
	TiXmlElement* root = new TiXmlElement("dftd-model");
	doc.LinkEndChild(root);
	root->SetAttribute("version", "1.0");

	// save materials.
	unsigned nr = 0;
	for (vector<material*>::const_iterator it = materials.begin(); it != materials.end(); ++it, ++nr) {
		TiXmlElement* mat = new TiXmlElement("material");
		root->LinkEndChild(mat);
		mat->SetAttribute("name", (*it)->name);
		mat->SetAttribute("id", nr);
		const material* m = *it;

		// colors.
		write_color_to_dftd_model_file(mat, m->diffuse, "diffuse");
		write_color_to_dftd_model_file(mat, m->specular, "specular");

		// shininess
		TiXmlElement* sh = new TiXmlElement("shininess");
		ostringstream oss; oss << m->shininess;
		sh->SetAttribute("exponent", oss.str());
		mat->LinkEndChild(sh);

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
		TiXmlElement* msh = new TiXmlElement("mesh");
		root->LinkEndChild(msh);
		msh->SetAttribute("name", mp->name);
		msh->SetAttribute("id", nr);

		// material.
		if (mp->mymaterial) {
			unsigned matid = 0;
			for ( ; matid < materials.size(); ++matid) {
				if (materials[matid] == mp->mymaterial)
					break;
			}
			msh->SetAttribute("material", matid);
		}

		// vertices.
		TiXmlElement* verts = new TiXmlElement("vertices");
		msh->LinkEndChild(verts);
		verts->SetAttribute("nr", mp->vertices.size());
		ostringstream ossv;
		for (vector<vector3f>::const_iterator vit = mp->vertices.begin(); vit != mp->vertices.end(); ++vit) {
			ossv << vit->x << " " << vit->y << " " << vit->z << " ";
		}
		TiXmlText* vertt = new TiXmlText(ossv.str());
		verts->LinkEndChild(vertt);

		// indices.
		TiXmlElement* indis = new TiXmlElement("indices");
		msh->LinkEndChild(indis);
		indis->SetAttribute("nr", mp->indices.size());
		ostringstream ossi;
		for (vector<unsigned>::const_iterator iit = mp->indices.begin(); iit != mp->indices.end(); ++iit) {
			ossi << *iit << " ";
		}
		TiXmlText* indit = new TiXmlText(ossi.str());
		indis->LinkEndChild(indit);

		// texcoords.
		if (mp->mymaterial) {
			TiXmlElement* texcs = new TiXmlElement("texcoords");
			msh->LinkEndChild(texcs);
			ostringstream osst;
			for (vector<vector2f>::const_iterator tit = mp->texcoords.begin(); tit != mp->texcoords.end(); ++tit) {
				osst << tit->x << " " << tit->y << " ";
			}
			TiXmlText* texct = new TiXmlText(osst.str());
			texcs->LinkEndChild(texct);
		}

		if (store_normals) {
			// normals.
			TiXmlElement* nrmls = new TiXmlElement("normals");
			msh->LinkEndChild(nrmls);
			ostringstream ossn;
			for (vector<vector3f>::const_iterator nit = mp->normals.begin(); nit != mp->normals.end(); ++nit) {
				ossn << nit->x << " " << nit->y << " " << nit->z << " ";
			}
			TiXmlText* nrmlt = new TiXmlText(ossn.str());
			nrmls->LinkEndChild(nrmlt);
		}

		// transformation.
		TiXmlElement* trans = new TiXmlElement("transformation");
		msh->LinkEndChild(trans);
		ostringstream osst;
		for (unsigned y = 0; y < 4; ++y) {
			for (unsigned x = 0; x < 4; ++x) {
				osst << mp->transformation.elem(x, y) << " ";
			}
		}
		TiXmlText* transt = new TiXmlText(osst.str());
		trans->LinkEndChild(transt);
	}

	// save lights.
	for (vector<light>::const_iterator it = lights.begin(); it != lights.end(); ++it) {
		TiXmlElement* lgt = new TiXmlElement("light");
		root->LinkEndChild(lgt);
		lgt->SetAttribute("name", it->name);
		ostringstream lpos; lpos << it->pos.x << " " << it->pos.y << " " << it->pos.z;
		lgt->SetAttribute("pos", lpos.str());
		ostringstream lcol; lcol << it->colr << " " << it->colg << " " << it->colb;
		lgt->SetAttribute("color", lcol.str());
		ostringstream lamb; lamb << it->ambient;
		lgt->SetAttribute("ambient", lamb.str());
	}

	// finally save file.
	doc.SaveFile();
}


void model::write_color_to_dftd_model_file(TiXmlElement* parent, const color& c,
					   const string& type) const
{
	TiXmlElement* cl = new TiXmlElement(type);
	parent->LinkEndChild(cl);
	ostringstream osscl;
	osscl << float(c.r)/255 << " " << float(c.g)/255 << " " << float(c.b)/255;
	cl->SetAttribute("color", osscl.str());
}


color model::read_color_from_dftd_model_file(TiXmlElement* parent, const std::string& type)
{
	TiXmlElement* ecol = parent->FirstChildElement(type);
	sys().myassert(ecol != 0, string("no color element for type ")+type+string(" in ")+basename);
	const char* tmp = ecol->Attribute("color");
	sys().myassert(tmp != 0, string("no color attribute for type ")+type+string(" in ")+basename);
	istringstream iss(tmp);
	float r, g, b;
	iss >> r >> g >> b;
	return color(Uint8(r*255), Uint8(g*255), Uint8(b*255));
}


void model::material::map::write_to_dftd_model_file(TiXmlElement* parent,
						    const std::string& type, bool withtrans) const
{
	TiXmlElement* mmap = new TiXmlElement("map");
	parent->LinkEndChild(mmap);
	mmap->SetAttribute("type", type);
	mmap->SetAttribute("filename", filename);
	if (withtrans) {
		ostringstream ossus; ossus << uscal;
		mmap->SetAttribute("uscal", ossus.str());
		ostringstream ossvs; ossvs << vscal;
		mmap->SetAttribute("vscal", ossvs.str());
		ostringstream ossuo; ossuo << uoffset;
		mmap->SetAttribute("uoffset", ossuo.str());
		ostringstream ossvo; ossvo << voffset;
		mmap->SetAttribute("voffset", ossvo.str());
		ostringstream ossan; ossan << angle;
		mmap->SetAttribute("angle", ossan.str());
	}
}

model::material::map::map(TiXmlElement* parent, bool withtrans)
	: uscal(1.0f), vscal(1.0f),
	  uoffset(0.0f), voffset(0.0f), angle(0.0f),
	  mytexture(0)
{
	const char* tmp = parent->Attribute("filename");
	sys().myassert(tmp != 0, "no filename given for materialmap!");
	filename = tmp;
	if (withtrans) {
		tmp = parent->Attribute("uscal");
		if (tmp) uscal = atof(tmp);
		tmp = parent->Attribute("vscal");
		if (tmp) vscal = atof(tmp);
		tmp = parent->Attribute("uoffset");
		if (tmp) uoffset = atof(tmp);
		tmp = parent->Attribute("voffset");
		if (tmp) voffset = atof(tmp);
		tmp = parent->Attribute("angle");
		if (tmp) angle = atof(tmp);
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
		sys().myassert(false, string("[model::load_m3ds] Unable to load PRIMARY chuck from file \"")+fn+string("\""));
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

	m->init();
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

//	sys().myassert(nr_uv_coords == m.vertices.size(), "number of texture coordinates doesn't match number of vertices");

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
	sys().myassert(false, "object has unknown material");
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

void model::read_dftd_model_file(const std::string& filename)
{
	TiXmlDocument doc(filename);
	doc.LoadFile();
	TiXmlElement* root = doc.FirstChildElement("dftd-model");
	sys().myassert(root != 0, string("model: load(), no root element found in ") + filename);
	float version = XmlAttribf(root, "version");
	sys().myassert(version <= 1.0, string("model file format version unknown ") + filename);

	// read elements.
	map<unsigned, material* > mat_id_mapping;
	for (TiXmlElement* eattr = root->FirstChildElement() ; eattr != 0; eattr = eattr->NextSiblingElement()) {
		string attrtype = string(eattr->Value());
		if (attrtype == "material") {
			// materials.
			material* mat = new material();
			mat->name = XmlAttrib(eattr, "name");
			unsigned id = XmlAttribu(eattr, "id");
			mat_id_mapping[id] = mat;

			mat->diffuse = read_color_from_dftd_model_file(eattr, "diffuse");
			mat->specular = read_color_from_dftd_model_file(eattr, "specular");

			for (TiXmlElement* emap = eattr->FirstChildElement("map") ; emap != 0; emap = emap->NextSiblingElement("map")) {
				string type = XmlAttrib(emap, "type");
				if (type == "diffuse") {
					mat->colormap.reset(new material::map(emap));
				} else if (type == "normal") {
					mat->normalmap.reset(new material::map(emap));
				} else if (type == "specular") {
					mat->specularmap.reset(new material::map(emap, false));
				} else {
					sys().myassert(false, string("unknown material map type '")+type+string("' in ")+filename);
				}
			}

			TiXmlElement* eshin = eattr->FirstChildElement("shininess");
			if (eshin) {
				const char* shexp = eshin->Attribute("exponent");
				sys().myassert(shexp != 0, string("shininess defined but no exponent given in ")+filename);
				istringstream iss(shexp);
				iss >> mat->shininess;
			}

			mat->init();
			materials.push_back(mat);
		} else if (attrtype == "mesh") {
			// meshes.
			mesh* msh = new mesh();
			meshes.push_back(msh);
			msh->name = XmlAttrib(eattr, "name");
			// material
			string matids = XmlAttrib(eattr, "material");
			if (matids != "") {
				unsigned matid = atoi(matids.c_str());
				map<unsigned, material* >::iterator it = mat_id_mapping.find(matid);
				if (it != mat_id_mapping.end()) {
					msh->mymaterial = it->second;
				} else {
					// print message or abort
					sys().myassert(false, string("referenced unknown material id ")+filename);
				}
			}
			// vertices
			TiXmlElement* verts = eattr->FirstChildElement("vertices");
			sys().myassert(verts != 0, string("no vertices tag in ")+filename);
			unsigned nrverts = XmlAttribu(verts, "nr");
			msh->vertices.reserve(nrverts);
			TiXmlText* verttext = verts->FirstChild()->ToText();
			sys().myassert(verttext != 0, string("no vertex data in ")+filename);
			istringstream issv(verttext->Value());
			for (unsigned i = 0; i < nrverts; ++i) {
				float x, y, z;
				issv >> x >> y >> z;
				msh->vertices.push_back(vector3f(x, y, z));
			}
			// indices
			TiXmlElement* indis = eattr->FirstChildElement("indices");
			sys().myassert(indis != 0, string("no indices tag in ")+filename);
			unsigned nrindis = XmlAttribu(indis, "nr");
			msh->indices.reserve(nrindis);
			TiXmlText* inditext = indis->FirstChild()->ToText();
			sys().myassert(inditext != 0, string("no index data in ")+filename);
			istringstream issi(inditext->Value());
			for (unsigned i = 0; i < nrindis; ++i) {
				unsigned idx;
				issi >> idx;
				sys().myassert(idx < nrverts, string("vertex index out of range ")+filename);
				msh->indices.push_back(idx);
			}
			// tex coords
			if (msh->mymaterial) {
				TiXmlElement* texcs = eattr->FirstChildElement("texcoords");
				sys().myassert(texcs != 0, string("no texcoords tag in ")+filename);
				msh->texcoords.reserve(nrverts);
				TiXmlText* texctext = texcs->FirstChild()->ToText();
				sys().myassert(texctext != 0, string("no texcoord data in ")+filename);
				istringstream isst(texctext->Value());
				for (unsigned i = 0; i < nrverts; ++i) {
					float x, y;
					isst >> x >> y;
					msh->texcoords.push_back(vector2f(x, y));
				}
			}

			// normals
			TiXmlElement* nrmls = eattr->FirstChildElement("normals");
			if (nrmls != 0) {
				msh->normals.reserve(nrverts);
				TiXmlText* nrmltext = nrmls->FirstChild()->ToText();
				sys().myassert(nrmltext != 0, string("no normal data in ")+filename);
				istringstream issn(nrmltext->Value());
				for (unsigned i = 0; i < nrverts; ++i) {
					float x, y, z;
					issn >> x >> y >> z;
					msh->normals.push_back(vector3f(x, y, z));
				}
			}

			// transformation	
			TiXmlElement* trans = eattr->FirstChildElement("transformation");
			if (trans != 0) {
				TiXmlText* transtext = trans->FirstChild()->ToText();
				sys().myassert(transtext != 0, string("no transformation data in ")+filename);
				istringstream isst(transtext->Value());
				for (unsigned y = 0; y < 4; ++y) {
					for (unsigned x = 0; x < 4; ++x) {
						isst >> msh->transformation.elem(x, y);
					}
				}
			}
		} else if (attrtype == "light") {
			// lights.
			light l;
			l.name = XmlAttrib(eattr, "name");
			const char* tmp = eattr->Attribute("pos");
			sys().myassert(tmp != 0, string("no pos for light given in ")+filename);
			istringstream issp(tmp);
			issp >> l.pos.x >> l.pos.y >> l.pos.z;
			tmp = eattr->Attribute("color");
			if (tmp) {
				istringstream issc(tmp);
				issp >> l.colr >> l.colg >> l.colb;
			}
			tmp = eattr->Attribute("ambient");
			if (tmp) {
				istringstream issa(tmp);
				issa >> l.ambient;
			}
		} else {
			sys().myassert(false, string("unknown type ")+attrtype+string(" in ")+filename);
		}
	}
}

// -------------------------------- end of dftd model file reading ------------------------------
