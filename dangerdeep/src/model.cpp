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
	vcoords.clear();
	vtexcoords.clear();
	vnormals.clear();
	facevalues.clear();
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
	vcoords.reserve(nrv);
	vnormals.reserve(nrv);
	vtexcoords.reserve(nrv);
	facevalues.reserve(3*nrf);
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
		vcoords.push_back(vector3(
			read_quantified_float(f, min.x, max.x),
			read_quantified_float(f, min.y, max.y),
			read_quantified_float(f, min.z, max.z) ));
		vnormals.push_back(vector3(
			read_packed_float(f),
			read_packed_float(f),
			read_packed_float(f) ));
		vtexcoords.push_back(vector2(
			read_packed_float(f),
			read_packed_float(f) ));
	}
	for ( ; nrf > 0; --nrf) {
		unsigned v0 = 0, v1 = 0, v2 = 0;
		fread(&v0, indexsize, 1, f);
		fread(&v1, indexsize, 1, f);
		fread(&v2, indexsize, 1, f);
		facevalues.push_back(v0);
		facevalues.push_back(v1);
		facevalues.push_back(v2);
	}
	fclose(f);
	class system* s = system::sys();	// needed for double use via off2mdl
	if (s) s->add_console(string("read model file ")+filename);
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
	vcoords.reserve(nr_vertices);
	vnormals.resize(nr_vertices);
	vtexcoords.reserve(nr_vertices);
	facevalues.reserve(3*nr_faces);
	for (i = 0; i < nr_vertices; i++) {
		float a, b, c, d = 0, e = 0;
		if (withuv)
			fscanf(f, "%f %f %f %f %f\n", &a, &b, &c, &d, &e);
		else
			fscanf(f, "%f %f %f\n", &a, &b, &c);
		vector3 abc(a,b,c);
		vcoords.push_back(abc);
		vtexcoords.push_back(vector2(d, e));
		min = abc.min(min);
		max = abc.max(max);
	}
	vector3 deltamaxmin = max - min;
	// vector3 center = max * 0.5 + min * 0.5; Unused variable
	if (!withuv) {
		for (i = 0; i < nr_vertices; i++) {
			vtexcoords[i].x = TILESX*(vcoords[i].y - min.y)/deltamaxmin.y;
			vtexcoords[i].y = TILESY*(1.0 - (vcoords[i].z - min.z)/deltamaxmin.z);
		}
	}
	for (i = 0; i < nr_faces; i++) {
		unsigned fv0, fv1, fv2;
		fscanf(f, "%i %i %i %i\n", &j, &fv0, &fv1, &fv2);
		facevalues.push_back(fv0);
		facevalues.push_back(fv1);
		facevalues.push_back(fv2);
		system::sys()->myassert(j==3, string("model: face has != 3 edges")+filename);
		// calculate face normal.
		const vector3& v0 = vcoords[facevalues[3*i+0]];
		const vector3& v1 = vcoords[facevalues[3*i+1]];
		const vector3& v2 = vcoords[facevalues[3*i+2]];
		vector3 face_normal = (v1-v0).orthogonal(v2-v0).normal();
		vnormals[facevalues[3*i+0]] += face_normal;
		vnormals[facevalues[3*i+1]] += face_normal;
		vnormals[facevalues[3*i+2]] += face_normal;
	}
	// calculate vertex normals
	for (i = 0; i < nr_vertices; i++) {
		vnormals[i].normalize();
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
	unsigned nrv = vcoords.size(), nrf = facevalues.size()/3;
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
	for (unsigned i = 0; i < nrv; ++i) {
		write_quantified_float(f, vcoords[i].x, min.x, max.x);
		write_quantified_float(f, vcoords[i].y, min.y, max.y);
		write_quantified_float(f, vcoords[i].z, min.z, max.z);
		write_packed_float(f, vnormals[i].x);
		write_packed_float(f, vnormals[i].y);
		write_packed_float(f, vnormals[i].z);
		write_packed_float(f, vtexcoords[i].x);
		write_packed_float(f, vtexcoords[i].y);
	}
	for (unsigned i = 0; i < nrf; ++i) {
		fwrite(&(facevalues[3*i+0]), indexsize, 1, f);
		fwrite(&(facevalues[3*i+1]), indexsize, 1, f);
		fwrite(&(facevalues[3*i+2]), indexsize, 1, f);
	}
	fclose(f);
}
		
void model::display(bool with_texture) const
{
	// fixme use Vertex Arrays (or display lists, or both)
	if (with_texture && tex != 0)
		glBindTexture(GL_TEXTURE_2D, tex->get_opengl_name());
	else
		glBindTexture(GL_TEXTURE_2D, 0);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glVertexPointer(3, GL_DOUBLE, 0, &(vcoords[0]));
	glNormalPointer(GL_DOUBLE, 0, &(vnormals[0]));
	glTexCoordPointer(2, GL_DOUBLE, 0, &(vtexcoords[0]));

	glDrawElements(GL_TRIANGLES, facevalues.size(), GL_UNSIGNED_INT, &facevalues[0]);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
}
