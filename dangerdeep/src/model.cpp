// A 3d model
// (C)+(W) by Thorsten Jordan. See LICENSE

#include "model.h"
#include "system.h"
#include "global_data.h"

#ifdef WIN32
#undef min
#undef max
#endif

int model::mapping = GL_NEAREST;

model::model(const string& filename, bool usematerial_) : display_list(0), usematerial(usematerial_)
{
	// fixme: determine loader by extension here. currently only 3ds supported
	m3ds_load(filename);

	read_cs_file(filename);	// try to read cross section file
	
	compute_bounds();
	compute_normals();

	// create display list
	unsigned dl = glGenLists(1);
	system::sys().myassert(dl != 0, "no more display list indices available");
	glNewList(dl, GL_COMPILE);
	display();
	glEndList();
	display_list = dl;
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
	min = max = meshes[0].vertices[0].pos;

	for (vector<model::mesh>::iterator it = meshes.begin(); it != meshes.end(); ++it) {
		for (vector<model::mesh::vertex>::iterator it2 = it->vertices.begin(); it2 != it->vertices.end(); ++it2) {
			min = it2->pos.min(min);
			max = it2->pos.max(max);
		}
	}
}

void model::compute_normals(void)
{
	//fixme: 3ds may have stored some normals already
	for (vector<model::mesh>::iterator it = meshes.begin(); it != meshes.end(); ++it) {
		for (vector<model::mesh::vertex>::iterator it2 = it->vertices.begin(); it2 != it->vertices.end(); ++it2)
			it2->normal = vector3f();
		for (vector<model::mesh::face>::iterator it2 = it->faces.begin(); it2 != it->faces.end(); ++it2) {
			const vector3f& v0 = it->vertices[it2->v[0]].pos;
			const vector3f& v1 = it->vertices[it2->v[1]].pos;
			const vector3f& v2 = it->vertices[it2->v[2]].pos;
			vector3f ortho = (v1-v0).orthogonal(v2-v0);
			// avoid degenerated triangles
			float lf = 1.0/ortho.length();
			if (isfinite(lf)) {
				vector3f face_normal = ortho * lf;
				it->vertices[it2->v[0]].normal += face_normal;
				it->vertices[it2->v[1]].normal += face_normal;
				it->vertices[it2->v[2]].normal += face_normal;
			}
		}
		for (vector<model::mesh::vertex>::iterator it2 = it->vertices.begin(); it2 != it->vertices.end(); ++it2) {
			// this can lead to NAN values in vertex normals.
			// but only for degenerated vertices, so we don't care.
			it2->normal.normalize();
		}
	}
}

void model::material::init(void)
{
	delete mytexture;
	mytexture = 0;
	if (filename.length() > 0)
		mytexture = new texture(get_model_dir() + filename, model::mapping);
}

void model::material::set_gl_values(void) const
{
	if (mytexture != 0) {
		mytexture->set_gl_texture();
		glColor3f(1,1,1);
	} else {
		glBindTexture(GL_TEXTURE_2D, 0);
		col.set_gl_color();
	}
}

void model::mesh::display(bool usematerial) const
{
	bool has_texture = false;
	if (usematerial) {
		if (mymaterial != 0) {
			has_texture = (mymaterial->mytexture != 0);
			mymaterial->set_gl_values();
		} else {
			glBindTexture(GL_TEXTURE_2D, 0);
			glColor3f(0,0,0);
		}
	}

	if (has_texture)
		glInterleavedArrays(GL_T2F_N3F_V3F, sizeof(mesh::vertex), &(vertices[0].uv));
	else
		glInterleavedArrays(GL_N3F_V3F, sizeof(mesh::vertex), &(vertices[0].normal));
	glDrawElements(GL_TRIANGLES, 3*faces.size(), GL_UNSIGNED_INT, &faces[0].v[0]);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

void model::display(void) const
{
	if (display_list) {
		glCallList(display_list);
	} else {
		for (vector<model::mesh>::const_iterator it = meshes.begin(); it != meshes.end(); ++it)
			it->display(usematerial);
	}
}

model::mesh model::get_mesh(unsigned nr) const
{
	if (nr < meshes.size())
		return meshes[nr];
	else
		return mesh();
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

// ------------------------------------------ 3ds loading functions -------------------------- 
// ----------------------------- 3ds file reading data ---------------------------
// taken from a NeHe tutorial (nehe.gamedev.net)
// intendations describe the tree structure
#define M3DS_MAIN3DS	   	0x4D4D
	#define M3DS_EDIT3DS		0x3D3D
		#define M3DS_EDIT_MATERIAL	0xAFFF
			#define M3DS_MATNAME	   	0xA000
			#define M3DS_MATDIFFUSE		0xA020
			#define M3DS_MATMAP		0xA200
				#define M3DS_MATMAPFILE		0xA300
				#define M3DS_MATMAPANG		0xA35C
		#define M3DS_EDIT_OBJECT	0x4000
			#define M3DS_OBJ_TRIMESH   	0x4100
				#define M3DS_TRI_VERTEXL	0x4110
				#define M3DS_TRI_FACEL1		0x4120
					#define M3DS_TRI_MATERIAL	0x4130
				#define M3DS_TRI_MAPPINGCOORDS	0x4140
				#define M3DS_TRI_MESHMATRIX  0x4160
	#define M3DS_VERSION		0x0002
	#define M3DS_KEYF3DS		0xB000
// ----------------------------- end of 3ds file reading data ----------------------

void model::m3ds_load(const string& fn)
{
	ifstream in(fn.c_str(), ios::in | ios::binary);
	m3ds_chunk head = m3ds_read_chunk(in);
	if (head.id != M3DS_MAIN3DS)
		system::sys().myassert(false, string("[model::load_m3ds] Unable to load PRIMARY chuck from file \"")+fn+string("\""));
	m3ds_process_toplevel_chunks(in, head);
	head.skip(in);
}

void model::m3ds_chunk::skip(istream& in)
{
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
		switch (ch.id) {
			case M3DS_EDIT_MATERIAL:
				m3ds_process_material_chunks(in, ch);
				break;
			case M3DS_EDIT_OBJECT:
				m3ds_read_string(in, ch);	// read (and ignore) name
				m3ds_process_object_chunks(in, ch);
				break;
		}
		ch.skip(in);
		parent.bytes_read += ch.length;
	}
}

void model::m3ds_process_object_chunks(istream& in, m3ds_chunk& parent)
{
	while (!parent.fully_read()) {
		m3ds_chunk ch = m3ds_read_chunk(in);
		switch (ch.id) {
			case M3DS_OBJ_TRIMESH:
				m3ds_process_trimesh_chunks(in, ch);
				break;
		}
		ch.skip(in);
		parent.bytes_read += ch.length;
	}
}

void model::m3ds_process_trimesh_chunks(istream& in, m3ds_chunk& parent)
{
	meshes.push_back(mesh());
	while (!parent.fully_read()) {
		m3ds_chunk ch = m3ds_read_chunk(in);
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
						meshes.back().xformmat[j][i] = read_float(in);
//						cout << "j="<<j<<", i="<<i<<": "<<meshes.back().xformmat[j][i]<<"\n";
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
			case M3DS_MATDIFFUSE:
				m3ds_read_color_chunk(in, ch, m);
				break;
			case M3DS_MATMAP:
				m3ds_process_materialmap_chunks(in, ch, m);
				break;
		}
		ch.skip(in);
		parent.bytes_read += ch.length;
	}
	m->init();
	materials.push_back(m);
}

void model::m3ds_process_materialmap_chunks(istream& in, m3ds_chunk& parent, model::material* m)
{
	while (!parent.fully_read()) {
		m3ds_chunk ch = m3ds_read_chunk(in);
		switch (ch.id) {
			case M3DS_MATMAPFILE:
				m->filename = m3ds_read_string(in, ch);
				break;
			case M3DS_MATMAPANG:
				m->angle = read_float(in);
				ch.bytes_read += 4;
//			printf("angle %f\n",m->angle);
				break;
		}
		ch.skip(in);
		parent.bytes_read += ch.length;
	}
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
		c = tolower(c);
		++ch.bytes_read;
		if (c == 0) break;
		s += c;
	}
	return s;
}

void model::m3ds_read_color_chunk(istream& in, m3ds_chunk& parent, model::material* m)
{
	m3ds_chunk ch = m3ds_read_chunk(in);
	m->col.r = read_u8(in);
	m->col.g = read_u8(in);
	m->col.b = read_u8(in);
	ch.bytes_read += 3;
	ch.skip(in);
	parent.bytes_read += ch.length;
}

void model::m3ds_read_faces(istream& in, m3ds_chunk& ch, model::mesh& m)
{
	unsigned nr_faces = read_u16(in);
	ch.bytes_read += 2;

	m.faces.clear();	
	m.faces.reserve(nr_faces);
	for (unsigned n = 0; n < nr_faces; ++n) {
		unsigned a = read_u16(in);
		unsigned b = read_u16(in);
		unsigned c = read_u16(in);
		m.faces.push_back(model::mesh::face(a, b, c));
		read_u16(in);	// ignore 4th value
	}
	ch.bytes_read += nr_faces * 4 * 2;
}

void model::m3ds_read_uv_coords(istream& in, m3ds_chunk& ch, model::mesh& m)
{
	unsigned nr_uv_coords = read_u16(in);
	ch.bytes_read += 2;

	system::sys().myassert(nr_uv_coords == m.vertices.size(), "number of texture coordinates doesn't match number of vertices");
		
	for (unsigned n = 0; n < nr_uv_coords; ++n) {
		m.vertices[n].uv.x = read_float(in);
		m.vertices[n].uv.y = read_float(in);
	}
	ch.bytes_read += nr_uv_coords * 2 * 4;
}

void model::m3ds_read_vertices(istream& in, m3ds_chunk& ch, model::mesh& m)
{
	unsigned nr_verts = read_u16(in);
	ch.bytes_read += 2;
	
	m.vertices.resize(nr_verts);
	for (unsigned n = 0; n < nr_verts; ++n) {
		m.vertices[n].pos.x = read_float(in);
		m.vertices[n].pos.y = read_float(in);
		m.vertices[n].pos.z = read_float(in);
	}
	ch.bytes_read += nr_verts * 3 * 4;
}

void model::m3ds_read_material(istream& in, m3ds_chunk& ch, model::mesh& m)
{
	string matname = m3ds_read_string(in, ch);

	for (vector<model::material*>::iterator it = materials.begin(); it != materials.end(); ++it) {
		if ((*it)->name == matname) {
			m.mymaterial = *it;
			
			// rotate texture coords (and negate v also)
			float ca = cos(m.mymaterial->angle * M_PI / 180.0);
			float sa = sin(m.mymaterial->angle * M_PI / 180.0);
			for (vector<model::mesh::vertex>::iterator it2 = m.vertices.begin(); it2 != m.vertices.end(); ++it2) {
				float u = it2->uv.x;
				float v = it2->uv.y;
				it2->uv.x = ca * u - sa * v;
				it2->uv.y = sa * u - ca * v;
			}

			return;
		}
	}
	system::sys().myassert(false, "object has unknown material");
}		   

// -------------------------------- end of 3ds loading functions -----------------------------------
