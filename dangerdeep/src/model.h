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
		void set_gl_values(void) const;
	};
	
	struct mesh {
		vector<vector3f> vertices;
		vector<vector3f> normals;
		vector<vector3f> tangentsx;
		vector<vector2f> texcoords;
		vector<unsigned> indices;	// 3 indices per face
		matrix4f transformation;	// rot., transl., scaling
		material* mymaterial;
		void display(bool usematerial) const;
		
		void compute_normals(void);
		bool compute_tangentx(unsigned i0, unsigned i1, unsigned i2);

		mesh(const mesh& m) : vertices(m.vertices), normals(m.normals), tangentsx(m.tangentsx), texcoords(m.texcoords), indices(m.indices), mymaterial(m.mymaterial) {}
		mesh& operator= (const mesh& m) { vertices = m.vertices; normals = m.normals; tangentsx = m.tangentsx; texcoords = m.texcoords; indices = m.indices; mymaterial = m.mymaterial; return *this; }
		mesh() : mymaterial(0) {}
		~mesh() {}
	};

protected:	
	vector<material*> materials;
	vector<mesh> meshes;
	
	unsigned display_list;	// OpenGL display list for the model
	bool usematerial;

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
	void m3ds_process_object_chunks(istream& in, m3ds_chunk& parent);
	void m3ds_process_trimesh_chunks(istream& in, m3ds_chunk& parent);
	void m3ds_process_face_chunks(istream& in, m3ds_chunk& parent, mesh& m);
	void m3ds_process_material_chunks(istream& in, m3ds_chunk& parent);
	void m3ds_process_materialmap_chunks(istream& in, m3ds_chunk& parent, material::map* m);
	void m3ds_read_color_chunk(istream& in, m3ds_chunk& ch, color& col);
	void m3ds_read_faces(istream& in, m3ds_chunk& ch, mesh& m);
	void m3ds_read_uv_coords(istream& in, m3ds_chunk& ch, mesh& m);
	void m3ds_read_vertices(istream& in, m3ds_chunk& ch, mesh& m);
	void m3ds_read_material(istream& in, m3ds_chunk& ch, mesh& m);
	// ------------ end of 3ds loading functions ------------------
	
	model();
	model(const model& );
	model& operator= (const model& );

public:
	static int mapping;	// GL_* mapping constants
	model(const string& filename, bool usematerial = true, bool makedisplaylist = true);
	~model();
	void display(void) const;
	mesh get_mesh(unsigned nr) const;
	vector3f get_min(void) const { return min; }
	vector3f get_max(void) const { return max; }
	float get_length(void) const { return (max - min).y; }
	float get_width(void) const { return (max - min).x; }
	float get_height(void) const { return (max - min).z; }
	vector3f get_boundbox_size(void) const { return max-min; }
	float get_cross_section(float angle) const;	// give angle in degrees.
};	

#endif
