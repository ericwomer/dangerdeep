// A 3d model (C) 2001 Thorsten Jordan

#include "model.h"
#include "system.h"
#include "global_data.h"
#include <GL/gl.h>

float model::read_packed_float(FILE* f)
{
	short s = 0;
	fread(&s, 2, 1, f);
	return float(s)/16384.0;
}

void model::write_packed_float(FILE* f, float t)
{
	if (t < -2.0 || t >= 2.0) t = 0.0;
	short s = short(t*16384.0);
	fwrite(&s, 2, 1, f);
}

float model::read_quantified_float(FILE* f, double min, double max)
{
	unsigned short u;
	fread(&u, 2, 1, f);
	return double(u)*(max - min)/65535.0 + min;
}

void model::write_quantified_float(FILE* f, float t, double min, double max)
{
	unsigned short u = (unsigned short)((t - min)*65535.0/(max - min));
	fwrite(&u, 2, 1, f);
}

model::model(const string& filename) : tex(0)
{
	read(filename);
}

void model::read(const string& filename)
{
	vertices.clear();
	faces.clear();
	FILE* f = fopen(filename.c_str(), "rb");
	system::sys()->myassert(f, string("model: failed to open")+filename);
	char tmp[16];
	fread(tmp, 16, 1, f);
	system::sys()->myassert(memcmp("TJ___MODELFILE00", tmp, 16) == 0,
		string("model: file has no legal model header")+filename);
	
	unsigned tns = 0;
	fread(&tns, 2, 1, f);
	string texname;
	if (tns > 0) {
		char* tmps = new char [tns+1];
		fread(tmps, tns, 1, f);
		tmps[tns] = 0;
		texname = string(tmps);
		delete [] tmps;
	}
	texmapping = 0;
	fread(&texmapping, 1, 1, f);
	clamp = ((texmapping & 0x80) == 0);
	texmapping = texmapping & 0x7f;
	if (texname != "")
		tex = new texture(get_data_dir() + "textures/" + texname, texmapping, clamp);
	else
		tex = 0;

	unsigned nrv = 0, nrf = 0;
	fread(&nrv, 4, 1, f);
	fread(&nrf, 4, 1, f);
	vertices.reserve(nrv);
	faces.reserve(nrf);
	float tmpf[3];
	fread(tmpf, sizeof(float), 3, f);
	min.x = tmpf[0];
	min.y = tmpf[1];
	min.z = tmpf[2];
	fread(tmpf, sizeof(float), 3, f);
	max.x = tmpf[0];
	max.y = tmpf[1];
	max.z = tmpf[2];
	int indexsize = (nrf < 256) ? 1 : ((nrf < 65536) ? 2 : ((nrf < 16777216) ? 3 : 4 ));
	for ( ; nrv > 0; --nrv) {
		vertex v;
		v.pos.x = read_quantified_float(f, min.x, max.x);
		v.pos.y = read_quantified_float(f, min.y, max.y);
		v.pos.z = read_quantified_float(f, min.z, max.z);
		v.normal.x = read_packed_float(f);
		v.normal.y = read_packed_float(f);
		v.normal.z = read_packed_float(f);
		v.u = read_packed_float(f);
		v.v = read_packed_float(f);
		vertices.push_back(v);
	}
	for ( ; nrf > 0; --nrf) {
		face fc;
		unsigned v0 = 0, v1 = 0, v2 = 0;
		fread(&v0, indexsize, 1, f);
		fread(&v1, indexsize, 1, f);
		fread(&v2, indexsize, 1, f);
		fc.v[0] = v0;
		fc.v[1] = v1;
		fc.v[2] = v2;
		faces.push_back(fc);
	}
	fclose(f);
	system::sys()->add_console(string("read model file ")+filename);
}

void model::read_from_OFF(const string& filename, const string& texture_name, unsigned mapping,
	unsigned tilesx, unsigned tilesy)
{
	double TILESX = tilesx;
	double TILESY = tilesy;
	texmapping = mapping;
	clamp = (tilesx == 1 && tilesy == 1);
	max = vector3(-1e10, -1e10, -1e10);
	min = vector3(1e10, 1e10, 1e10);
	int nr_vertices = 0, nr_faces = 0;
	FILE *f = fopen(filename.c_str(), "rb");
	system::sys()->myassert(f, string("model: failed to open")+filename);
	int i, j;
	char header[5];
	fread(header, 1, 5, f);
	system::sys()->myassert(header[0]=='O'&&header[1]=='F'&&header[2]=='F', string("model: failed to open OFF")+filename);
	bool withuv=false;
	if(header[3]=='U'&&header[4]=='V')
		withuv=true;
	else
		system::sys()->myassert(header[3]=='\n', string("model: no OFF header")+filename);
	rewind(f);
	if (withuv)
		fscanf(f, "OFFUV\n%i %i %i\n", &nr_vertices, &nr_faces, &i);
	else
		fscanf(f, "OFF\n%i %i %i\n", &nr_vertices, &nr_faces, &i);
	vertices.resize(nr_vertices);
	faces.resize(nr_faces);
	for (i = 0; i < nr_vertices; i++) {
		float a, b, c, d = 0, e = 0;
		if (withuv)
			fscanf(f, "%f %f %f %f %f\n", &a, &b, &c, &d, &e);
		else
			fscanf(f, "%f %f %f\n", &a, &b, &c);
		vertices[i].pos.x = a;
		vertices[i].pos.y = b;
		vertices[i].pos.z = c;
		vertices[i].normal = vector3(0, 0, 0);
		vertices[i].u = d;
		vertices[i].v = e;
		min = vertices[i].pos.min(min);
		max = vertices[i].pos.max(max);
	}
	vector3 deltamaxmin = max - min;
	vector3 center = max * 0.5 + min * 0.5;
	if (!withuv) {
		for (i = 0; i < nr_vertices; i++) {
			vertices[i].u = TILESX*(vertices[i].pos.y - min.y)/deltamaxmin.y;
			vertices[i].v = TILESY*(1.0 - (vertices[i].pos.z - min.z)/deltamaxmin.z);
		}
	}
	for (i = 0; i < nr_faces; i++) {
		fscanf(f, "%i %i %i %i\n", &j, &faces[i].v[0], &faces[i].v[1], &faces[i].v[2]);
		system::sys()->myassert(j==3, string("model: face has != 3 edges")+filename);
		// calculate face normal.
		const vector3& v0 = vertices[faces[i].v[0]].pos;
		const vector3& v1 = vertices[faces[i].v[1]].pos;
		const vector3& v2 = vertices[faces[i].v[2]].pos;
		vector3 face_normal = (v1-v0).orthogonal(v2-v0).normal();
		vertices[faces[i].v[0]].normal += face_normal;
		vertices[faces[i].v[1]].normal += face_normal;
		vertices[faces[i].v[2]].normal += face_normal;
	}
	// calculate vertex normals
	for (i = 0; i < nr_vertices; i++) {
		vertices[i].normal.normalize();
	}
	fclose(f);
	
	if (texture_name == "")
		tex = 0;
	else
		tex = new texture(texture_name, mapping, clamp);
}
		
void model::write(const string& filename) const
{
	FILE* f = fopen(filename.c_str(), "wb");
	system::sys()->myassert(f, string("model: failed to open")+filename);
	fwrite("TJ___MODELFILE00", 16, 1, f);
	if (tex) {
		string texname = tex->get_name();
		unsigned tns = texname.size();
		fwrite(&tns, 2, 1, f);
		fwrite(&(texname[0]), tns, 1, f);
	} else {
		unsigned tns = 0;
		fwrite(&tns, 2, 1, f);
	}
	unsigned txm = texmapping;
	if (!clamp) txm |= 0x80;
	fwrite(&txm, 1, 1, f);
	unsigned nrv = vertices.size(), nrf = faces.size();
	fwrite(&nrv, 4, 1, f);
	fwrite(&nrf, 4, 1, f);
	float tmpf[3];
	tmpf[0] = min.x;
	tmpf[1] = min.y;
	tmpf[2] = min.z;
	fwrite(&tmpf, sizeof(float), 3, f);
	tmpf[0] = max.x;
	tmpf[1] = max.y;
	tmpf[2] = max.z;
	fwrite(&tmpf, sizeof(float), 3, f);
	int indexsize = (nrf < 256) ? 1 : ((nrf < 65536) ? 2 : ((nrf < 16777216) ? 3 : 4 ));
	for (vector<vertex>::const_iterator it = vertices.begin(); it != vertices.end(); ++it) {
		const vertex& v = *it;
		write_quantified_float(f, v.pos.x, min.x, max.x);
		write_quantified_float(f, v.pos.y, min.y, max.y);
		write_quantified_float(f, v.pos.z, min.z, max.z);
		write_packed_float(f, v.normal.x);
		write_packed_float(f, v.normal.y);
		write_packed_float(f, v.normal.z);
		write_packed_float(f, v.u);
		write_packed_float(f, v.v);
	}
	for (vector<face>::const_iterator it = faces.begin(); it != faces.end(); ++it) {
		const face& fc = *it;
		fwrite(&(fc.v[0]), indexsize, 1, f);
		fwrite(&(fc.v[1]), indexsize, 1, f);
		fwrite(&(fc.v[2]), indexsize, 1, f);
	}
	fclose(f);
}
		
void model::display(bool with_texture, color* col1, color* col2) const
{
	// fixme use Vertex Arrays (or display lists, or both)
	if (with_texture && tex != 0)
		glBindTexture(GL_TEXTURE_2D, tex->get_opengl_name());
	else
		glBindTexture(GL_TEXTURE_2D, 0);
	if (col1 != 0 && col2 == 0)
		col1->set_gl_color();
	glBegin(GL_TRIANGLES);
	double zdiff = max.z - min.z;
	for (vector<face>::const_iterator it = faces.begin(); it != faces.end(); ++it) {
		for (int j = 0; j < 3; j++) {
			const vertex& v = vertices[it->v[j]];
			const vector3& vp = v.pos;
			const vector3& vn = v.normal;
			glNormal3f(vn.x, vn.y, vn.z);
			glTexCoord2f(v.u, v.v);
			if (col1 != 0 && col2 != 0) {
				color(*col1, *col2, (vp.z - min.z)/zdiff).set_gl_color();
			}
			glVertex3f(vp.x, vp.y, vp.z);
		}
	}
	glEnd();
	if (col1 != 0 || col2 != 0)
		glColor3f(1,1,1);
}

void model::scale(double x, double y, double z)
{
	for (vector<vertex>::iterator it = vertices.begin(); it != vertices.end(); ++it) {
		it->pos.x *= x;
		it->pos.y *= y;
		it->pos.z *= z;
	}
}
