// A 3d model
// (C)+(W) by Thorsten Jordan. See LICENSE

#ifndef MODEL_H
#define MODEL_H

#include "vector3.h"
#include "matrix4.h"
#include "texture.h"
#include "color.h"
#include <vector>
#include <fstream>
using namespace std;



class model {
public:
	class material {
		material(const material& );
		material& operator= (const material& );
	public:
		class map {
			map(const map& );
			map& operator= (const map& );
		public:
			string filename;	// also in mytexture, fixme
			float uscal, vscal, uoffset, voffset;
			float angle;	// uv rotation angle;
			texture* mytexture;
			map() : uscal(1.0f), vscal(1.0f), uoffset(0.0f), voffset(0.0f), angle(0.0f), mytexture(0) {}
			~map() { delete mytexture; }
			void init(int mapping);
		};
	
		string name;
		color ambient;
		color diffuse;
		color specular;
		color transparency;//????
		map* tex1;
		map* bump;
		
		material() : tex1(0), bump(0) {}
		void init(void);
		~material() { delete tex1; delete bump; }
#ifndef DONT_USE_OPENGL
		void set_gl_values(void) const;
#endif
	};
	
	struct mesh {
		string name;
		vector<vector3f> vertices;
		vector<vector3f> normals;
		vector<vector3f> tangentsx;
		vector<vector2f> texcoords;
		vector<unsigned> indices;	// 3 indices per face
		matrix4f transformation;	// rot., transl., scaling
		material* mymaterial;
#ifndef DONT_USE_OPENGL
		void display(bool usematerial) const;
#endif
		void compute_normals(void);
		bool compute_tangentx(unsigned i0, unsigned i1, unsigned i2);

		mesh(const mesh& m) : name(m.name), vertices(m.vertices), normals(m.normals),
				      tangentsx(m.tangentsx), texcoords(m.texcoords),
				      indices(m.indices), transformation(m.transformation),
				      mymaterial(m.mymaterial) {}
		mesh& operator= (const mesh& m) { name = m.name; vertices = m.vertices;
			normals = m.normals; tangentsx = m.tangentsx; texcoords = m.texcoords;
			indices = m.indices; transformation = m.transformation;
			mymaterial = m.mymaterial; return *this; }
		mesh() : transformation(matrix4f::one()), mymaterial(0) {}
		~mesh() {}

		// transform vertices by matrix
		void transform(const matrix4f& m);
		void write_off_file(const string& fn) const;
	};

	struct light {
		string name;
		vector3f pos;
		float colr, colg, colb;
#ifndef DONT_USE_OPENGL
		void set_gl(unsigned nr_of_light) const;
#endif
		light(const light& l) : name(l.name), pos(l.pos),
					colr(l.colr), colg(l.colg), colb(l.colb) {}
		light& operator= (const light& l) { name = l.name; pos = l.pos;
			colr = l.colr; colg = l.colg; colb = l.colb; return *this; }
		light() : colr(1.0f), colg(1.0f), colb(1.0f) {}
		~light() {}
	};

protected:	
	vector<material*> materials;
	vector<mesh> meshes;
	vector<light> lights;
	
	unsigned display_list;	// OpenGL display list for the model
	bool usematerial;

	string basename;	// base name of the scene/model, computed from filename

	vector3f min, max;

	void compute_bounds(void);	
	void compute_normals(void);
	
	vector<float> cross_sections;	// array over angles
	
	void read_cs_file(const string& filename);
	
	// ------------ 3ds loading functions ------------------
	struct m3ds_chunk {
		unsigned short id;
		unsigned bytes_read;
		unsigned length;
		bool fully_read(void) const { return bytes_read >= length; }
		void skip(istream& in);
	};
	void m3ds_load(const string& fn);
	string m3ds_read_string(istream& in, m3ds_chunk& ch);
	m3ds_chunk m3ds_read_chunk(istream& in);
	string m3ds_read_string_from_rest_of_chunk(istream& in, m3ds_chunk& ch);
	void m3ds_process_toplevel_chunks(istream& in, m3ds_chunk& parent);
	void m3ds_process_model_chunks(istream& in, m3ds_chunk& parent);
	void m3ds_process_object_chunks(istream& in, m3ds_chunk& parent, const string& objname);
	void m3ds_process_trimesh_chunks(istream& in, m3ds_chunk& parent, const string& objname);
	void m3ds_process_light_chunks(istream& in, m3ds_chunk& parent, const string& objname);
	void m3ds_process_face_chunks(istream& in, m3ds_chunk& parent, mesh& m);
	void m3ds_process_material_chunks(istream& in, m3ds_chunk& parent);
	void m3ds_process_materialmap_chunks(istream& in, m3ds_chunk& parent, material::map* m);
	void m3ds_read_color_chunk(istream& in, m3ds_chunk& ch, color& col);
	void m3ds_read_faces(istream& in, m3ds_chunk& ch, mesh& m);
	void m3ds_read_uv_coords(istream& in, m3ds_chunk& ch, mesh& m);
	void m3ds_read_vertices(istream& in, m3ds_chunk& ch, mesh& m);
	void m3ds_read_material(istream& in, m3ds_chunk& ch, mesh& m);
	// ------------ end of 3ds loading functions ------------------
	
	model(const model& );
	model& operator= (const model& );

	void read_off_file(const string& fn);

public:
	model() : display_list(0), usematerial(true) {}

	static int mapping;	// GL_* mapping constants

	model(const string& filename, bool usematerial = true, bool makedisplaylist = true);
	~model();
#ifndef DONT_USE_OPENGL
	void display(void) const;
#endif
	mesh& get_mesh(unsigned nr);
	const mesh& get_mesh(unsigned nr) const;
	material& get_material(unsigned nr);
	const material& get_material(unsigned nr) const;
	light& get_light(unsigned nr);
	const light& get_light(unsigned nr) const;
	unsigned get_nr_of_meshes(void) const { return meshes.size(); }
	unsigned get_nr_of_materials(void) const { return materials.size(); }
	unsigned get_nr_of_lights(void) const { return lights.size(); }
	vector3f get_min(void) const { return min; }
	vector3f get_max(void) const { return max; }
	float get_length(void) const { return (max - min).y; }
	float get_width(void) const { return (max - min).x; }
	float get_height(void) const { return (max - min).z; }
	vector3f get_boundbox_size(void) const { return max-min; }
	float get_cross_section(float angle) const;	// give angle in degrees.
	static string tolower(const string& s);
	void add_mesh(const mesh& m) { meshes.push_back(m); }//fixme: maybe recompute bounds
	void add_material(material* m) { materials.push_back(m); }
	// transform meshes by matrix (attention: scaling destroys normals)
	void transform(const matrix4f& m);
};	

#endif
