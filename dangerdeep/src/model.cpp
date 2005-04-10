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

// wether the dftd model file format should contain normals or not
//#define STORE_NORMALS

int model::mapping = GL_LINEAR_MIPMAP_LINEAR;//GL_NEAREST;

model::model(const string& filename, bool usematerial_, bool makedisplaylist) :
	display_list(0), usematerial(usematerial_)
{
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
	
	compute_bounds();
	compute_normals();

	if (makedisplaylist) {
		// fixme: determine automatically if we can make one (bump mapping
		// without vertex shaders means that we can't compile a list!)
		// create display list
		unsigned dl = glGenLists(1);
		sys().myassert(dl != 0, "no more display list indices available");
		glNewList(dl, GL_COMPILE);
		display();
		glEndList();
		display_list = dl;
	}
}

model::~model()
{
	glDeleteLists(display_list, 1);
	for (vector<model::material*>::iterator it = materials.begin(); it != materials.end(); ++it)
		delete *it;
}

void model::compute_bounds(void)
{
	if (meshes.size() == 0) return;
	meshes[0].compute_bounds();
	min = meshes[0].min;
	max = meshes[0].max;

	for (vector<model::mesh>::iterator it = ++meshes.begin(); it != meshes.end(); ++it) {
		it->compute_bounds();
		min = it->min.min(min);
		max = it->max.max(max);
	}
}

void model::compute_normals(void)
{
	for (vector<model::mesh>::iterator it = meshes.begin(); it != meshes.end(); ++it) {
		it->compute_normals();
	}
}

void model::mesh::compute_bounds(void)
{
	if (vertices.size() == 0) return;
	min = max = vertices[0];

	for (vector<vector3f>::iterator it2 = ++vertices.begin(); it2 != vertices.end(); ++it2) {
		min = it2->min(min);
		max = it2->max(max);
	}
}

void model::mesh::compute_normals(void)
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

	//fixme: 3ds may have stored some normals already
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
	
	// if we use bump mapping for this mesh, we need tangent values, too!
	// tangentsy get computed at runtime from normals and tangentsx
	// tangentsx are computed that way:
	// from each vertex we find a vector in positive u direction
	// and project it onto the plane given by the normal -> tangentx
	// because bump maps use stored texture coordinates (x = positive u!)
	if (mymaterial && mymaterial->bump) {
		tangentsx.clear();
		tangentsx.resize(vertices.size(), vector3f(0, 0, 1));
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
	if (fabsf(det) < 0.0001f) {
		//find sane solution for this situation!
		//if delta_u is zero for d_uv0 and d_uv1, but delta_v is not, we could
		//compute tangentsy from v and tangentsx with the cross product
		//or we just don't store a tangentsx value and hope that the vertex
		//can be computed via another triangle
		//just hope and wait seems to work, at least one face adjacent to the vertex
		//should give sane tangent values.
		return false;
	} else {
		float a = d_uv1.y/det;
		float b = -d_uv0.y/det;
		vector3f rv = (vertices[i1] - vertices[i0]) * a + (vertices[i2] - vertices[i0]) * b;
		rv = rv - (rv * n) * n;
		tangentsx[i0] = rv;
		return true;
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



pair<model::mesh, model::mesh> model::mesh::split(const vector3f& abc, float d) const
{
	model::mesh part0, part1;
	part0.name = name + "_part0";
	part1.name = name + "_part1";
	part0.transformation = part1.transformation = transformation;
	part0.mymaterial = part1.mymaterial = mymaterial;
	part0.vertices.reserve(vertices.size()/2);
	part1.vertices.reserve(vertices.size()/2);
	part0.texcoords.reserve(texcoords.size()/2);
	part1.texcoords.reserve(texcoords.size()/2);
	part0.normals.reserve(normals.size()/2);
	part1.normals.reserve(normals.size()/2);
	part0.tangentsx.reserve(tangentsx.size()/2);
	part1.tangentsx.reserve(tangentsx.size()/2);
	part0.indices.reserve(indices.size()/2);
	part1.indices.reserve(indices.size()/2);

	// determine on which side the vertices are
	vector<float> dists(vertices.size());
	vector<unsigned> ixtrans(vertices.size());
	for (unsigned i = 0; i < vertices.size(); ++i) {
		dists[i] = vertices[i] * abc + d;
		if (dists[i] >= 0) {
			ixtrans[i] = part0.vertices.size();
			part0.vertices.push_back(vertices[i]);
			if (texcoords.size() > 0) part0.texcoords.push_back(texcoords[i]);
			if (normals.size() > 0) part0.normals.push_back(normals[i]);
			if (tangentsx.size() > 0) part0.tangentsx.push_back(tangentsx[i]);
		} else {
			ixtrans[i] = part1.vertices.size();
			part1.vertices.push_back(vertices[i]);
			if (texcoords.size() > 0) part1.texcoords.push_back(texcoords[i]);
			if (normals.size() > 0) part1.normals.push_back(normals[i]);
			if (tangentsx.size() > 0) part1.tangentsx.push_back(tangentsx[i]);
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
			part0.indices.push_back(ixtrans[ix[0]]);
			part0.indices.push_back(ixtrans[ix[1]]);
			part0.indices.push_back(ixtrans[ix[2]]);
			continue;
		}
		if (ds[0] < 0 && ds[1] < 0 && ds[2] < 0) {
			part1.indices.push_back(ixtrans[ix[0]]);
			part1.indices.push_back(ixtrans[ix[1]]);
			part1.indices.push_back(ixtrans[ix[2]]);
			continue;
		}

		// face needs to get splitted
		unsigned p0v = part0.vertices.size();
		unsigned p1v = part1.vertices.size();
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
			part0.vertices.push_back(newv);
			part1.vertices.push_back(newv);
			if (texcoords.size() > 0) {
				vector2f newtexc = texcoords[ix[j]] * (1 - fac) + texcoords[ix[next[j]]] * fac;
				part0.texcoords.push_back(newtexc);
				part1.texcoords.push_back(newtexc);
			}
			if (normals.size() > 0) {
				vector3f newnorm = (normals[ix[j]] * (1 - fac) + normals[ix[next[j]]] * fac).normal();
				part0.normals.push_back(newnorm);
				part1.normals.push_back(newnorm);
			}
			if (tangentsx.size() > 0) {
				vector3f newtanx = (tangentsx[ix[j]] * (1 - fac) + tangentsx[ix[next[j]]] * fac).normal();
				part0.tangentsx.push_back(newtanx);
				part1.tangentsx.push_back(newtanx);
			}
			++splitptr;
		}
		sys().myassert(splitptr == 2, "splitptr != 2 ?!");
		// add indices to parts.
		part0.indices.push_back(newindi0[0]);
		part0.indices.push_back(newindi0[1]);
		part0.indices.push_back(newindi0[2]);
		if (newindi0ptr == 4) {
			part0.indices.push_back(newindi0[0]);
			part0.indices.push_back(newindi0[2]);
			part0.indices.push_back(newindi0[3]);
		}
		part1.indices.push_back(newindi1[0]);
		part1.indices.push_back(newindi1[1]);
		part1.indices.push_back(newindi1[2]);
		if (newindi1ptr == 4) {
			part1.indices.push_back(newindi1[0]);
			part1.indices.push_back(newindi1[2]);
			part1.indices.push_back(newindi1[3]);
		}
		sys().myassert((newindi0ptr == 3 || newindi1ptr == 3) && (newindi0ptr + newindi1ptr == 7), "newindi ptr corrupt!");
	}

	return make_pair(part0, part1);
}



void model::material::map::init(int mapping, bool makenormalmap, float detailh)
{
	delete mytexture;
	mytexture = 0;
	if (filename.length() > 0) {
		mytexture = new texture(get_texture_dir() + filename, mapping,
					makenormalmap, detailh);
	}
}

void model::material::init(void)
{
	if (tex1) tex1->init(model::mapping);
	//fixme: what is best mapping for bump maps?
	// compute normalmap if not given
	if (bump) bump->init(GL_LINEAR /*_MIPMAP_LINEAR*/, true, 128.0f /*16.0f*/ /*fixme, read from model file*/);
}


void model::material::set_gl_values(void) const
{
	diffuse.set_gl_color();
	glActiveTexture(GL_TEXTURE0);
	if (tex1 && tex1->mytexture) {
		if (bump && bump->mytexture) {
			glActiveTexture(GL_TEXTURE0);
			glMatrixMode(GL_TEXTURE);
			glLoadIdentity();
			glScalef(1, -1, 1); // y flip texture
			glTranslatef(bump->uoffset, bump->voffset, 0);
			glRotatef(-bump->angle, 0, 0, 1);
			glScalef(bump->uscal, bump->vscal, 1);
			bump->mytexture->set_gl_texture();
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
			glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_DOT3_RGBA); 
			glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_PRIMARY_COLOR);
			glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
			glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_TEXTURE);
			glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
			glActiveTexture(GL_TEXTURE1);
			glEnable(GL_TEXTURE_2D);
			//tex1->mytexture->set_gl_texture();
			glBindTexture(GL_TEXTURE_2D, 0);
			glMatrixMode(GL_TEXTURE);
			glLoadIdentity();
			glScalef(1, -1, 1); // y flip texture
			glTranslatef(tex1->uoffset, tex1->voffset, 0);
			glRotatef(-tex1->angle, 0, 0, 1);
			glScalef(tex1->uscal, tex1->vscal, 1);
			// primary color alpha seems to be ONE...
			float alphac[4] = { 1,1,1, 0.6f };//make ambient value (=alpha) variable
			// bump map function with ambient:
			// color * (bump_brightness * (1-ambient) + ambient)
			// tex1 color is just an replace, we could use it for some effect
			// like modulating it with environment color (which is free)
			// e.g. colored ambient light etc.
			glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, alphac);
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
			glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE); 
			glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE);
			glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
			glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_INTERPOLATE);
			glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_PREVIOUS);
			glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
			// we need one here, so we take primary color alpha, which is one.
			glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, GL_PRIMARY_COLOR);
			glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, GL_SRC_ALPHA);
			glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE2_ALPHA, GL_CONSTANT);
			glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND2_ALPHA, GL_SRC_ALPHA);
			glBlendFunc(GL_SRC_ALPHA, GL_ZERO);
		} else {
			glColor3f(1, 1, 1);
			glActiveTexture(GL_TEXTURE0);
			tex1->mytexture->set_gl_texture();
			glMatrixMode(GL_TEXTURE);
			glLoadIdentity();
			glScalef(1, -1, 1); // y flip texture
			glTranslatef(tex1->uoffset, tex1->voffset, 0);
			glRotatef(-tex1->angle, 0, 0, 1);
//			cout << "uv off " << tex1->uoffset << "," << tex1->voffset << " ang " << tex1->angle << " scal " << tex1->uscal << "," << tex1->vscal << "\n";
			glScalef(tex1->uscal, tex1->vscal, 1);
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, 0);
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		}
		glMatrixMode(GL_MODELVIEW);
	} else {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

void model::mesh::display(bool usematerial) const
{
	bool has_texture_u0 = false, has_texture_u1 = false;
	bool bumpmapping = false;
	if (usematerial) {
		if (mymaterial != 0) {
			if (mymaterial->tex1 && mymaterial->tex1->mytexture)
				has_texture_u0 = true;
			if (mymaterial->bump && mymaterial->bump->mytexture)
				has_texture_u1 = true;
			bumpmapping = has_texture_u1;	// maybe more options here...
			mymaterial->set_gl_values();
		} else {
			glBindTexture(GL_TEXTURE_2D, 0);
			glColor3f(0.5,0.5,0.5);
		}
	}

	glVertexPointer(3, GL_FLOAT, sizeof(vector3f), &vertices[0]);
	glEnableClientState(GL_VERTEX_ARRAY);

	glNormalPointer(GL_FLOAT, sizeof(vector3f), &normals[0]);	
	glEnableClientState(GL_NORMAL_ARRAY);

	glClientActiveTexture(GL_TEXTURE0);
	if (has_texture_u0 && texcoords.size() == vertices.size()) {
		glTexCoordPointer(2, GL_FLOAT, sizeof(vector2f), &texcoords[0]);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	} else {
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	}

	glClientActiveTexture(GL_TEXTURE1);
	if (has_texture_u1 && texcoords.size() == vertices.size()) {
		// maybe offer second texture coords. how are they stored in .3ds files?!
		glTexCoordPointer(2, GL_FLOAT, sizeof(vector2f), &texcoords[0]);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	} else {
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	}

	//with bump mapping, we need colors.
	vector<Uint8> colors;
	if (bumpmapping) {
		float lighttmp[4];
		glGetLightfv(GL_LIGHT0, GL_POSITION, lighttmp);
		vector3f lighttmpv(lighttmp[0], lighttmp[1], lighttmp[2]);
		cout << lighttmpv << "\n";
		//fixme: why inverse modelview matrix???
		//fixme: what about directional light? must get transformed somehow different?!
		matrix4f invmodelview = (matrix4f::get_gl(GL_MODELVIEW_MATRIX)).inverse();
		vector3f lightpos = invmodelview * lighttmpv;
		cout << lightpos << "\n";
		colors.resize(3*vertices.size());
		for (unsigned i = 0; i < vertices.size(); ++i) {
			const vector3f& nx = tangentsx[i];
			const vector3f& nz = normals[i];
			vector3f ny = nz.cross(nx);
			vector3f lp = lightpos - vertices[i];
			vector3f nl = vector3f(nx * lp, ny * lp, nz * lp).normal();
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
	
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, &indices[0]);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);	// disable tex1
	glClientActiveTexture(GL_TEXTURE0);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);	// disable tex0
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void model::display(void) const
{
	if (display_list) {
		glCallList(display_list);
	} else {
		for (vector<model::mesh>::const_iterator it = meshes.begin(); it != meshes.end(); ++it)
			it->display(usematerial);
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
	return meshes.at(nr);
}

const model::mesh& model::get_mesh(unsigned nr) const
{
	return meshes.at(nr);
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
	for (vector<model::mesh>::iterator it = meshes.begin(); it != meshes.end(); ++it) {
		it->transform(m);
	}
}


// -------------------------------- dftd model file writing --------------------------------------
// write our own model file format.
void model::write_to_dftd_model_file(const std::string& filename) const
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
		write_color_to_dftd_model_file(mat, m->ambient, "ambient");
		write_color_to_dftd_model_file(mat, m->diffuse, "diffuse");
		write_color_to_dftd_model_file(mat, m->specular, "specular");

		// maps.
		if (m->tex1) {
			m->tex1->write_to_dftd_model_file(mat, "diffuse");
		}
		if (m->bump) {
			m->bump->write_to_dftd_model_file(mat, "normal");
		}
		/*
		if (m->specularmap) {
			m->specularmap->write_to_dftd_model_file(mat, "specular");
		}
		*/
	}

	// save meshes.
	nr = 0;
	for (vector<mesh>::const_iterator it = meshes.begin(); it != meshes.end(); ++it, ++nr) {
		TiXmlElement* msh = new TiXmlElement("mesh");
		root->LinkEndChild(msh);
		msh->SetAttribute("name", it->name);
		msh->SetAttribute("id", nr);

		// material.
		if (it->mymaterial) {
			unsigned matid = 0;
			for ( ; matid < materials.size(); ++matid) {
				if (materials[matid] == it->mymaterial)
					break;
			}
			msh->SetAttribute("material", matid);
		}

		// vertices.
		TiXmlElement* verts = new TiXmlElement("vertices");
		msh->LinkEndChild(verts);
		verts->SetAttribute("nr", it->vertices.size());
		ostringstream ossv;
		for (vector<vector3f>::const_iterator vit = it->vertices.begin(); vit != it->vertices.end(); ++vit) {
			ossv << vit->x << " " << vit->y << " " << vit->z << " ";
		}
		TiXmlText* vertt = new TiXmlText(ossv.str());
		verts->LinkEndChild(vertt);

		// indices.
		TiXmlElement* indis = new TiXmlElement("indices");
		msh->LinkEndChild(indis);
		indis->SetAttribute("nr", it->indices.size());
		ostringstream ossi;
		for (vector<unsigned>::const_iterator iit = it->indices.begin(); iit != it->indices.end(); ++iit) {
			ossi << *iit << " ";
		}
		TiXmlText* indit = new TiXmlText(ossi.str());
		indis->LinkEndChild(indit);

		// texcoords.
		if (it->mymaterial) {
			TiXmlElement* texcs = new TiXmlElement("texcoords");
			msh->LinkEndChild(texcs);
			ostringstream osst;
			for (vector<vector2f>::const_iterator tit = it->texcoords.begin(); tit != it->texcoords.end(); ++tit) {
				osst << tit->x << " " << tit->y << " ";
			}
			TiXmlText* texct = new TiXmlText(osst.str());
			texcs->LinkEndChild(texct);
		}

#ifdef STORE_NORMALS
		// normals.
		TiXmlElement* nrmls = new TiXmlElement("normals");
		msh->LinkEndChild(nrmls);
		ostringstream ossn;
		for (vector<vector3f>::const_iterator nit = it->normals.begin(); nit != it->normals.end(); ++nit) {
			ossn << nit->x << " " << nit->y << " " << nit->z << " ";
		}
		TiXmlText* nrmlt = new TiXmlText(ossn.str());
		nrmls->LinkEndChild(nrmlt);
#endif

		// transformation.
		if (true /*it->transformation != matrix4f::one()*/) {
			TiXmlElement* trans = new TiXmlElement("transformation");
			msh->LinkEndChild(trans);
			ostringstream osst;
			for (unsigned y = 0; y < 4; ++y) {
				for (unsigned x = 0; x < 4; ++x) {
					osst << it->transformation.elem(x, y) << " ";
				}
			}
			TiXmlText* transt = new TiXmlText(osst.str());
			trans->LinkEndChild(transt);
		}
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
						    const std::string& type) const
{
	TiXmlElement* mmap = new TiXmlElement("map");
	parent->LinkEndChild(mmap);
	mmap->SetAttribute("type", type);
	mmap->SetAttribute("filename", filename);
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

model::material::map::map(TiXmlElement* parent) : uscal(1.0f), vscal(1.0f),
						  uoffset(0.0f), voffset(0.0f), angle(0.0f),
						  mytexture(0)
{
	const char* tmp = parent->Attribute("filename");
	sys().myassert(tmp != 0, "no filename given for materialmap!");
	filename = tmp;
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
#define 			M3DS_MATTRANSPARENCY	0xA050//not used yet, fixme
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
		vector<char> x(n);
		in.read(&x[0], n);
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
	meshes.push_back(mesh());
	meshes.back().name = objname;
	while (!parent.fully_read()) {
		m3ds_chunk ch = m3ds_read_chunk(in);
//	cout<<"found trimesh chunk"<<(void*)ch.id<<"\n";
		switch (ch.id) {
			case M3DS_TRI_VERTEXL:
				m3ds_read_vertices(in, ch, meshes.back());
				break;
			case M3DS_TRI_FACEL1:
				m3ds_read_faces(in, ch, meshes.back());
				m3ds_process_face_chunks(in, ch, meshes.back());
				break;
			case M3DS_TRI_MAPPINGCOORDS:
				m3ds_read_uv_coords(in, ch, meshes.back());
				break;

//fixme: this matrix seems to describe the model rotation and translation that IS ALREADY computed for the vertices
//but why are some models so much off the origin? (corvette, largefreighter)
//is there another chunk i missed while reading?
			case M3DS_TRI_MESHMATRIX:
				for (int j = 0; j < 4; ++j) {
					for (int i = 0; i < 3; ++i) {
						meshes.back().transformation.elem(j,i) = read_float(in);
					}
				}
/*
				{ mesh& m = meshes.back();
				for (vector<model::mesh::vertex>::iterator it2 = m.vertices.begin(); it2 != m.vertices.end(); ++it2)
					it2->pos.x -= 2*m.xformmat[3][0];
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
			case M3DS_MATAMBIENT:
				m3ds_read_color_chunk(in, ch, m->ambient);
				break;
			case M3DS_MATDIFFUSE:
				m3ds_read_color_chunk(in, ch, m->diffuse);
				break;
			case M3DS_MATSPECULAR:
				m3ds_read_color_chunk(in, ch, m->specular);
				break;
/*
			case M3DS_MATTRANSPARENCY:
				m3ds_read_color_chunk(in, ch, m->transparency);
				break;
*/
			case M3DS_MATMAPTEX1:
				if (!m->tex1) {
					m->tex1 = new material::map();
					m3ds_process_materialmap_chunks(in, ch, m->tex1);
				}
				break;
			case M3DS_MATMAPBUMP:
				if (!m->bump) {
					m->bump = new material::map();
					m3ds_process_materialmap_chunks(in, ch, m->bump);
				}
				break;
//			default: cout << "skipped chunk with id " << (void*)ch.id << "\n";
		}
		ch.skip(in);
		parent.bytes_read += ch.length;
	}

	// 3d studio uses diffuse color just for editor display
	if (m->tex1)
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
				m->vscal = 1.0f/read_float(in);
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
				m->angle = read_float(in);
				ch.bytes_read += 4;
//			printf("angle %f\n",m->angle);
				break;
//			case M3DS_MATMAPAMOUNTBUMP://see above
		}
		ch.skip(in);
		parent.bytes_read += ch.length;
	}
//	cout << "map: angle " << m->angle << " uscal,vscal " << m->uscal << "," << m->vscal << " uoff,voff " << m->uoffset << "," << m->voffset << "\n";
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
		float v = read_float(in);
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
	mesh m;
	m.name = basename;
	m.vertices.resize(nr_vertices);
	m.indices.resize(3*nr_faces);
	
	for (i = 0; i < nr_vertices; i++) {
		float a, b, c;
		fscanf(f, "%f %f %f\n", &a, &b, &c);
		m.vertices[i].x = a;
		m.vertices[i].y = b;
		m.vertices[i].z = c;
	}
	for (i = 0; i < nr_faces; i++) {
		unsigned j, v0, v1, v2;
		fscanf(f, "%u %u %u %u\n", &j, &v0, &v1, &v2);
		if (j != 3) return;
		m.indices[i*3] = v0;
		m.indices[i*3+1] = v1;
		m.indices[i*3+2] = v2;
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

			mat->ambient = read_color_from_dftd_model_file(eattr, "ambient");
			mat->diffuse = read_color_from_dftd_model_file(eattr, "diffuse");
			mat->specular = read_color_from_dftd_model_file(eattr, "specular");

			for (TiXmlElement* emap = eattr->FirstChildElement("map") ; emap != 0; emap = emap->NextSiblingElement("map")) {
				string type = XmlAttrib(emap, "type");
				if (type == "diffuse") {
					mat->tex1 = new material::map(emap);
				} else if (type == "normal") {
					mat->bump = new material::map(emap);
/*
				} else if (type == "specular") {
					mat->specularmap = new material::map(emap);
*/
				} else {
					sys().myassert(false, string("unknown material map type '")+type+string("' in ")+filename);
				}
			}

			mat->init();
			materials.push_back(mat);
		} else if (attrtype == "mesh") {
			// meshes.
			meshes.push_back(mesh());
			mesh& msh = meshes.back();
			msh.name = XmlAttrib(eattr, "name");
			// material
			string matids = XmlAttrib(eattr, "material");
			if (matids != "") {
				unsigned matid = atoi(matids.c_str());
				map<unsigned, material* >::iterator it = mat_id_mapping.find(matid);
				if (it != mat_id_mapping.end()) {
					msh.mymaterial = it->second;
				} else {
					// print message or abort
					sys().myassert(false, string("referenced unknown material id ")+filename);
				}
			}
			// vertices
			TiXmlElement* verts = eattr->FirstChildElement("vertices");
			sys().myassert(verts != 0, string("no vertices tag in ")+filename);
			unsigned nrverts = XmlAttribu(verts, "nr");
			msh.vertices.reserve(nrverts);
			TiXmlText* verttext = verts->FirstChild()->ToText();
			sys().myassert(verttext != 0, string("no vertex data in ")+filename);
			istringstream issv(verttext->Value());
			for (unsigned i = 0; i < nrverts; ++i) {
				float x, y, z;
				issv >> x >> y >> z;
				msh.vertices.push_back(vector3f(x, y, z));
			}
			// indices
			TiXmlElement* indis = eattr->FirstChildElement("indices");
			sys().myassert(indis != 0, string("no indices tag in ")+filename);
			unsigned nrindis = XmlAttribu(indis, "nr");
			msh.indices.reserve(nrindis);
			TiXmlText* inditext = indis->FirstChild()->ToText();
			sys().myassert(inditext != 0, string("no index data in ")+filename);
			istringstream issi(inditext->Value());
			for (unsigned i = 0; i < nrindis; ++i) {
				unsigned idx;
				issi >> idx;
				sys().myassert(idx < nrverts, string("vertex index out of range ")+filename);
				msh.indices.push_back(idx);
			}
			// tex coords
			if (msh.mymaterial) {
				TiXmlElement* texcs = eattr->FirstChildElement("texcoords");
				sys().myassert(texcs != 0, string("no texcoords tag in ")+filename);
				msh.texcoords.reserve(nrverts);
				TiXmlText* texctext = texcs->FirstChild()->ToText();
				sys().myassert(texctext != 0, string("no texcoord data in ")+filename);
				istringstream isst(texctext->Value());
				for (unsigned i = 0; i < nrverts; ++i) {
					float x, y;
					isst >> x >> y;
					msh.texcoords.push_back(vector2f(x, y));
				}
			}
#ifdef STORE_NORMALS
			// normals
			TiXmlElement* nrmls = eattr->FirstChildElement("normals");
			if (nrmls != 0) {
				msh.normals.reserve(nrverts);
				TiXmlText* nrmltext = nrmls->FirstChild()->ToText();
				sys().myassert(nrmltext != 0, string("no normal data in ")+filename);
				istringstream issn(nrmltext->Value());
				for (unsigned i = 0; i < nrverts; ++i) {
					float x, y, z;
					issn >> x >> y >> z;
					msh.normals.push_back(vector3f(x, y, z));
				}
			}
#endif
			// transformation	
			TiXmlElement* trans = eattr->FirstChildElement("transformation");
			if (trans != 0) {
				TiXmlText* transtext = trans->FirstChild()->ToText();
				sys().myassert(transtext != 0, string("no transformation data in ")+filename);
				istringstream isst(transtext->Value());
				for (unsigned y = 0; y < 4; ++y) {
					for (unsigned x = 0; x < 4; ++x) {
						isst >> msh.transformation.elem(x, y);
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
		} else {
			sys().myassert(false, string("unknown type ")+attrtype+string(" in ")+filename);
		}
	}
}

// -------------------------------- end of dftd model file reading ------------------------------
