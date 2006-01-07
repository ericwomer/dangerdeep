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

#ifndef MODEL_H
#define MODEL_H

#include "vector3.h"
#include "matrix4.h"
#include "texture.h"
#include "color.h"
#include <vector>
#include <fstream>
#include <memory>

class xml_elem;

class model {
public:
	typedef std::auto_ptr<model> ptr;

	class material {
		material(const material& );
		material& operator= (const material& );
	public:
		class map {
			map(const map& );
			map& operator= (const map& );
		public:
			std::string filename;	// also in mytexture, fixme
			float uscal, vscal, uoffset, voffset;
			float angle;	// uv rotation angle;
			std::auto_ptr<texture> mytexture;
			map();
			void init(texture::mapping_mode mapping, bool makenormalmap = false, float detailh = 1.0f, bool rgb2grey = false);
			void write_to_dftd_model_file(xml_elem& parent, const std::string& type, bool withtrans = true) const;
			// read and construct from dftd model file
			map(const xml_elem& parent, bool withtrans = true);
			// set up opengl texture matrix with map transformation values
			void setup_glmatrix() const;
		};
	
		std::string name;
		color diffuse;	// only used when colormap is 0.
		color specular;	// material specular color, used with and without texture mapping.
		float shininess; // shininess (Halfangle dot Normal exponent)
		auto_ptr<map> colormap;	// replaces diffuse color if not defined.
		auto_ptr<map> normalmap;	// should be of type RGB to work properly.
		auto_ptr<map> specularmap; // should be of type LUMINANCE to work properly.
		
		material(const std::string& nm = "Unnamed material");
		void init();
		void set_gl_values() const;
		void set_gl_values_mirror_clip() const;
	};
	
	class mesh {
		mesh(const mesh& );
		mesh& operator= (const mesh& );
	public:
		std::string name;
		std::vector<vector3f> vertices;
		std::vector<vector3f> normals;
		std::vector<vector3f> tangentsx;
		std::vector<vector2f> texcoords;
		// we can have a right handed or a left handed coordinate system for each vertex,
		// dependent on the direction of the u,v coordinates. We do not store the third
		// vector per vertex but a flag, which saves space.
		std::vector<Uint8> righthanded;	// a vector of bools. takes more space than a bitvector, but faster access.
		std::vector<unsigned> indices;	// 3 indices per face
		matrix4f transformation;	// rot., transl., scaling
		material* mymaterial;
		vector3f min, max;
		GLuint display_list;		// OpenGL display list for the mesh

		void display(bool use_display_list = true) const; // used only when list is available.
		void display_mirror_clip() const;
		void compute_bounds();	
		void compute_normals();
		bool compute_tangentx(unsigned i0, unsigned i1, unsigned i2);

		mesh(const string& nm = "Unnamed mesh");
		~mesh();

		// make display list if possible
		void compile();

		// transform vertices by matrix
		void transform(const matrix4f& m);
		void write_off_file(const std::string& fn) const;

		// give plane equation (abc must have length 1)
		pair<mesh*, mesh*> split(const vector3f& abc, float d) const;
	};

	struct light {
		std::string name;
		vector3f pos;
		float colr, colg, colb;
		float ambient;
		void set_gl(unsigned nr_of_light) const;
		light() : colr(1.0f), colg(1.0f), colb(1.0f), ambient(0.1f) {}
	};

protected:	
	// a 3d object, references meshes
	struct object {
		unsigned id;
		std::string name;
		const mesh* mymesh;
		vector3f translation;
		vector3f rotat_axis;
		float rotat_angle;	// in degrees
		float rotat_angle_min;	// in degrees
		float rotat_angle_max;	// in degrees
		std::vector<object> children;
		object(unsigned id_ = 0, const std::string& nm = "???", const mesh* m = 0)
			: id(id_), name(nm), mymesh(m), rotat_angle(0), rotat_angle_min(0),
			  rotat_angle_max(0) { rotat_axis.z = 1; }
		bool set_angle(float ang);
		object* find(unsigned id);
		object* find(const std::string& name);
		void display() const;
	};

	std::vector<material*> materials;
	std::vector<mesh*> meshes;
	std::vector<light> lights;

	object scene;
	
	std::string basename;	// base name of the scene/model, computed from filename

	vector3f min, max;

	// class-wide variables: shaders supported and enabled, shader number and init count
	static unsigned init_count;

	// Booleans for supported OpenGL extensions
	static bool vertex_program_supported;
	static bool fragment_program_supported;
	static bool compiled_vertex_arrays_supported;

	// Config options (only used when supported and enabled)
	static bool use_shaders;

	// Shader programs
	static std::vector<GLuint> default_vertex_programs;
	static std::vector<GLuint> default_fragment_programs;

	enum shader_programs {
		VFP_COLOR_NORMAL_SPECULAR,
		VFP_COLOR_NORMAL,
		VFP_MIRROR_CLIP,
		NR_VFP,
	}; // more variants later.

	// init / deinit
	static void render_init();
	static void render_deinit();

	void compute_bounds();
	void compute_normals();
	
	std::vector<float> cross_sections;	// array over angles
	
	void read_cs_file(const std::string& filename);
	
	// ------------ 3ds loading functions ------------------
	struct m3ds_chunk {
		unsigned short id;
		unsigned bytes_read;
		unsigned length;
		bool fully_read() const { return bytes_read >= length; }
		void skip(istream& in);
	};
	void m3ds_load(const std::string& fn);
	std::string m3ds_read_string(istream& in, m3ds_chunk& ch);
	m3ds_chunk m3ds_read_chunk(istream& in);
	std::string m3ds_read_string_from_rest_of_chunk(istream& in, m3ds_chunk& ch);
	void m3ds_process_toplevel_chunks(istream& in, m3ds_chunk& parent);
	void m3ds_process_model_chunks(istream& in, m3ds_chunk& parent);
	void m3ds_process_object_chunks(istream& in, m3ds_chunk& parent, const std::string& objname);
	void m3ds_process_trimesh_chunks(istream& in, m3ds_chunk& parent, const std::string& objname);
	void m3ds_process_light_chunks(istream& in, m3ds_chunk& parent, const std::string& objname);
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

	void read_off_file(const std::string& fn);

	void read_dftd_model_file(const std::string& filename);
	void write_color_to_dftd_model_file(xml_elem& parent, const color& c,
					    const std::string& type) const;
	color read_color_from_dftd_model_file(const xml_elem& parent, const std::string& type);

	// store shared lookup functions for pow function (specular lighting)
	// fixme: check if exponent is integer or float.
	// map<double, texture*> powlookup_functions;

	void read_objects(const xml_elem& parent, object& parentobj);

public:
	model();

	static texture::mapping_mode mapping;	// GL_* mapping constants (default GL_LINEAR_MIPMAP_LINEAR)
	static bool enable_shaders;	// en-/disable use of FP/VP (default true)

	model(const std::string& filename, bool use_material = true);
	~model();
	void display() const;
	// display model but clip away coords with z < 0 in world space.
	// set up modelview matrix with world->eye transformation before calling this function
	// and give additional object->world transformation as matrix of texture unit #1 (2nd. unit).
	void display_mirror_clip() const;
	mesh& get_mesh(unsigned nr);
	const mesh& get_mesh(unsigned nr) const;
	material& get_material(unsigned nr);
	const material& get_material(unsigned nr) const;
	light& get_light(unsigned nr);
	const light& get_light(unsigned nr) const;
	unsigned get_nr_of_meshes() const { return meshes.size(); }
	unsigned get_nr_of_materials() const { return materials.size(); }
	unsigned get_nr_of_lights() const { return lights.size(); }
	vector3f get_min() const { return min; }
	vector3f get_max() const { return max; }
	float get_length() const { return (max - min).y; }
	float get_width() const { return (max - min).x; }
	float get_height() const { return (max - min).z; }
	vector3f get_boundbox_size() const { return max-min; }
	float get_cross_section(float angle) const;	// give angle in degrees.
	static std::string tolower(const std::string& s);
	void add_mesh(mesh* m) { meshes.push_back(m); }//fixme: maybe recompute bounds
	void add_material(material* m) { materials.push_back(m); }
	// transform meshes by matrix (attention: scaling destroys normals)
	void transform(const matrix4f& m);
	// compile display lists
	void compile();

	// write our own model file format.
	void write_to_dftd_model_file(const std::string& filename, bool store_normals = true) const;

	// manipulate object angle(s), returns false on error (wrong id or angle out of bounds)
	bool set_object_angle(unsigned objid, double ang);
	bool set_object_angle(const std::string& objname, double ang);
};	

#endif
