// A 3d model
// (C)+(W) by Thorsten Jordan. See LICENSE

#ifndef MODEL_H
#define MODEL_H

#include "vector3.h"
#include "texture.h"
#include "color.h"
#include <vector>
#include <fstream>
using namespace std;



class model {
	class material {
		material(const material& );
		material& operator= (const material& );
	public:
		string name;
		string filename;
		color col;
		float angle;	// uv rotation angle
		texture* mytexture;
		material() : angle(0), mytexture(0) {}
		void init(void);
		~material() { delete mytexture; }
		void set_gl_values(void) const;
	};
	
	struct mesh {
		struct vertex {
			vector2f uv;
			vector3f normal;
			vector3f pos;
			vertex() {}
			~vertex() {}
			vertex(const vector2f& t, const vector3f& n, const vector3f& p) : uv(t), normal(n), pos(p) {}
			vertex(const vertex& o) : uv(o.uv), normal(o.normal), pos(o.pos) {}
			vertex& operator= (const vertex& o) { uv = o.uv; normal = o.normal; pos = o.pos; return *this; }
		};
		struct face {
			unsigned v[3];
			face(unsigned a, unsigned b, unsigned c) { v[0] = a; v[1] = b; v[2] = c; }
		};
	
		vector<vertex> vertices;
		vector<face> faces;
		float xformmat[4][3];	// rotation and translation
		material* mymaterial;
		void display(void) const;

		mesh(const mesh& m) : vertices(m.vertices), faces(m.faces), mymaterial(m.mymaterial) {}
		mesh& operator= (const mesh& m) { vertices = m.vertices; faces = m.faces; mymaterial = m.mymaterial; return *this; }
		mesh() : mymaterial(0) {}
		~mesh() {}
	};
	
	vector<material*> materials;
	vector<mesh> meshes;

	vector3f min, max;

	void compute_bounds(void);	
	void compute_normals(void);
	
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
	void m3ds_process_materialmap_chunks(istream& in, m3ds_chunk& parent, material* m);
	void m3ds_read_color_chunk(istream& in, m3ds_chunk& ch, material* m);
	void m3ds_read_faces(istream& in, m3ds_chunk& ch, mesh& m);
	void m3ds_read_uv_coords(istream& in, m3ds_chunk& ch, mesh& m);
	void m3ds_read_vertices(istream& in, m3ds_chunk& ch, mesh& m);
	void m3ds_read_material(istream& in, m3ds_chunk& ch, mesh& m);
	// ------------ end of 3ds loading functions ------------------
	
	model();
	model(const model& );
	model& operator= (const model& );

public:
	static unsigned mapping;	// 0 nearest, 1 bilinear, 2 bilinear mipmap, 3 trilinear
	model(const string& filename);
	~model();
	void display(void) const;
	vector3f get_min(void) const { return min; }
	vector3f get_max(void) const { return max; }
	double get_length(void) const { return (max - min).y; }
	double get_width(void) const { return (max - min).x; }
	double get_height(void) const { return (max - min).z; }
	vector3f get_boundbox_size(void) const { return max-min; }
};	

#endif
