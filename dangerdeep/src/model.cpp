// A 3d model
// (C)+(W) by Thorsten Jordan. See LICENSE

#include "model.h"
#ifndef MODEL_JUST_LOAD
#include "system.h"
#include "global_data.h"
#endif
#include "matrix4.h"

#ifdef WIN32
#undef min
#undef max
#endif

int model::mapping = GL_LINEAR_MIPMAP_LINEAR;//GL_NEAREST;

model::model(const string& filename, bool usematerial_, bool makedisplaylist) : display_list(0), usematerial(usematerial_)
{
	// fixme: determine loader by extension here. currently only 3ds supported
	m3ds_load(filename);

	read_cs_file(filename);	// try to read cross section file
	
	compute_bounds();
	compute_normals();

#ifndef MODEL_JUST_LOAD
	if (makedisplaylist) {
		// fixme: determine automatically if we can make one (bump mapping
		// without vertex shaders means that we can't compile a list!)
		// create display list
		unsigned dl = glGenLists(1);
		system::sys().myassert(dl != 0, "no more display list indices available");
		glNewList(dl, GL_COMPILE);
		display();
		glEndList();
		display_list = dl;
	}
#endif
}

model::~model()
{
#ifndef MODEL_JUST_LOAD
	glDeleteLists(display_list, 1);
#endif
	for (vector<model::material*>::iterator it = materials.begin(); it != materials.end(); ++it)
		delete *it;
}

void model::compute_bounds(void)
{
	if (meshes.size() == 0) return;
	min = max = meshes[0].vertices[0];

	for (vector<model::mesh>::iterator it = meshes.begin(); it != meshes.end(); ++it) {
		for (vector<vector3f>::iterator it2 = it->vertices.begin(); it2 != it->vertices.end(); ++it2) {
			min = it2->min(min);
			max = it2->max(max);
		}
	}
}

void model::compute_normals(void)
{
	for (vector<model::mesh>::iterator it = meshes.begin(); it != meshes.end(); ++it) {
		it->compute_normals();
	}
}

void model::mesh::compute_normals(void)
{
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

void model::material::map::init(int mapping)
{
	delete mytexture;
	mytexture = 0;
	if (filename.length() > 0) {
//cout << "object has texture '" << filename << "' mapping " << model::mapping << "\n";
#ifdef MODEL_JUST_LOAD
		mytexture = new texture(filename, GL_LINEAR, GL_REPEAT, true);
#else
		mytexture = new texture(get_texture_dir() + filename, mapping);
#endif
	}
}

void model::material::init(void)
{
	if (tex1) tex1->init(model::mapping);
	if (bump) bump->init(GL_LINEAR_MIPMAP_LINEAR);//fixme: what is best mapping for bump maps?
}

#ifndef MODEL_JUST_LOAD
void model::material::set_gl_values(void) const
{
	diffuse.set_gl_color();
	glActiveTexture(GL_TEXTURE0);
	if (tex1 && tex1->mytexture) {
		if (bump && bump->mytexture) {
			glActiveTexture(GL_TEXTURE0);
			glMatrixMode(GL_TEXTURE);
			glLoadIdentity();
			glTranslatef(bump->uoffset, bump->voffset, 0);//fixme order of operations is unclear
			glRotatef(bump->angle, 0, 0, 1);
			glScalef(bump->uscal, bump->vscal, 1);
			bump->mytexture->set_gl_texture();
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
			glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_DOT3_RGB); 
			glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_PRIMARY_COLOR);
			glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
			glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_TEXTURE);
			glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
			glActiveTexture(GL_TEXTURE1);
			glEnable(GL_TEXTURE_2D);
			tex1->mytexture->set_gl_texture();
			glMatrixMode(GL_TEXTURE);
			glLoadIdentity();
			glTranslatef(tex1->uoffset, tex1->voffset, 0);//fixme order of operations is unclear
			glRotatef(tex1->angle, 0, 0, 1);
			glScalef(tex1->uscal, tex1->vscal, 1);
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
			glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE); 
			glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_PREVIOUS);
			glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
			glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_TEXTURE);
			glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
			/*
			fixme: add some ambient color to the bump map with alpha blending:
			store ambient as primary color alpha
			add it to tex0 dot3 result (with tex1alpha op)
			set blend mode to src_alpha,GL_ZERO
			add ambient to N*L or use ambient to blend between N*L and 1.
			the latter is better ;-) use interpolate or add_saturated or something
			the like.
			Tex1 could compute something with the color, like modulating the
			texture with environment color etc. (colored ambient?)
			*/
		} else {
			glActiveTexture(GL_TEXTURE0);
			tex1->mytexture->set_gl_texture();
			glMatrixMode(GL_TEXTURE);
			glLoadIdentity();
			glRotatef(tex1->angle, 0, 0, 1);
			glTranslatef(tex1->uoffset, tex1->voffset, 0);
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
		matrix4f invmodelview = (matrix4f::get_gl(GL_MODELVIEW_MATRIX)).inverse();
		vector3f lightpos = invmodelview * vector3f(lighttmp[0], lighttmp[1], lighttmp[2]);
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
#endif

const model::mesh& model::get_mesh(unsigned nr) const
{
	return meshes.at(nr);
}

const model::material& model::get_material(unsigned nr) const
{
	return *(materials.at(nr));
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
// indentations describe the tree structure
#define M3DS_MAIN3DS	   	0x4D4D
	#define M3DS_EDIT3DS		0x3D3D
		#define M3DS_EDIT_MATERIAL	0xAFFF
			#define M3DS_MATNAME	   	0xA000
			#define M3DS_MATAMBIENT		0xA010
			#define M3DS_MATDIFFUSE		0xA020
			#define M3DS_MATSPECULAR	0xA030
			#define M3DS_MATTRANSPARENCY	0xA050//not used yet, fixme
			#define M3DS_MATMAPTEX1		0xA200
				#define M3DS_MATMAPFILE		0xA300
				#define M3DS_MATMAP1OVERU_SCAL	0xA354
				#define M3DS_MATMAP1OVERV_SCAL	0xA356
				#define M3DS_MATMAPUOFFSET	0xA358
				#define M3DS_MATMAPVOFFSET	0xA35A
				#define M3DS_MATMAPANG		0xA35C
				#define M3DS_MATMAPAMOUNTBUMP	0xA252//double byte chunk, displayed amount of bump, not yet used
			#define M3DS_MATMAPBUMP		0xA230
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
	if (head.id != M3DS_MAIN3DS) {
#ifdef MODEL_JUST_LOAD
		cerr << "[model::load_m3ds] Unable to load PRIMARY chuck from file \"" << fn << "\"\n";
		exit(-1);
#else
		system::sys().myassert(false, string("[model::load_m3ds] Unable to load PRIMARY chuck from file \"")+fn+string("\""));
#endif
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
						meshes.back().transformation.elem(i,j) = read_float(in);
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
				m->filename = m3ds_read_string(in, ch);
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
		c = tolower(c);
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

#ifdef MODEL_JUST_LOAD
	if (nr_uv_coords != m.vertices.size()) {
		cerr << "number of texture coordinates doesn't match number of vertices\n";
		exit(-1);
	}
#else
	system::sys().myassert(nr_uv_coords == m.vertices.size(), "number of texture coordinates doesn't match number of vertices");
#endif

	m.texcoords.clear();
	m.texcoords.reserve(nr_uv_coords);		
	for (unsigned n = 0; n < nr_uv_coords; ++n) {
		float u = read_float(in);
		float v = read_float(in);
		m.texcoords.push_back(vector2f(u, v));
	}
	ch.bytes_read += nr_uv_coords * 2 * 4;
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
			
			for (vector<vector2f>::iterator it2 = m.texcoords.begin(); it2 != m.texcoords.end(); ++it2)
				it2->y = -it2->y;

/* fixme: do that with texture matrix.
			// rotate texture coords (and negate v also)
			float ca = cos(m.mymaterial->angle * M_PI / 180.0);
			float sa = sin(m.mymaterial->angle * M_PI / 180.0);
			for (vector<vector2f>::iterator it2 = m.texcoords.begin(); it2 != m.texcoords.end(); ++it2) {
				float u = it2->x;
				float v = it2->y;
				it2->x = ca * u - sa * v;
				it2->y = sa * u - ca * v;
			}
*/
			return;
		}
	}
#ifdef MODEL_JUST_LOAD
	cerr << "object has unknown material\n";
	exit(-1);
#else
	system::sys().myassert(false, "object has unknown material");
#endif
}		   

// -------------------------------- end of 3ds loading functions -----------------------------------
