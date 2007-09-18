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
#include "matrix3.h"
#include "matrix4.h"
#include "texture.h"
#include "color.h"
#include "shader.h"
#include "vertexbufferobject.h"
#include <vector>
#include <fstream>
#include <memory>
#include <map>
#include <set>

class xml_elem;

///\brief Handles a 3D model, it's animation and OpenGL based rendering and display.
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
			std::string filename;	// also in mytexture, a bit redundant
			float uscal, vscal, uoffset, voffset;
			float angle;	// uv rotation angle;

		protected:
			texture* tex;	// set by set_layout

			//maybe unite list of skins and default-texture, both
			//have texture ptr, filename and ref_count.
			std::auto_ptr<texture> mytexture;	// default "skin", MUST BE SET!
			unsigned ref_count;

			struct skin {
				texture* mytexture;
				unsigned ref_count;
				std::string filename;
				skin() : mytexture(0), ref_count(0) {}
			};

			// layout-name to skin mapping
			std::map<std::string, skin> skins;
		public:
			map();
			~map();
			void write_to_dftd_model_file(xml_elem& parent, const std::string& type, bool withtrans = true) const;
			// read and construct from dftd model file
			map(const xml_elem& parent, bool withtrans = true);
			// set up opengl texture matrix with map transformation values
			void setup_glmatrix() const;
			void set_gl_texture() const;
 			void set_gl_texture(glsl_program& prog, const std::string& texname, unsigned texunitnr) const;
 			void set_gl_texture(glsl_shader_setup& gss, const std::string& texname, unsigned texunitnr) const;
			void set_texture(texture* t);
			void register_layout(const std::string& name, const std::string& basepath,
					     texture::mapping_mode mapping,
					     bool makenormalmap = false,
					     float detailh = 1.0f,
					     bool rgb2grey = false);
			void unregister_layout(const std::string& name);
			void set_layout(const std::string& layout);
			void get_all_layout_names(std::set<std::string>& result) const;
		};
	
		std::string name;
		color diffuse;	// only used when colormap is 0.
		color specular;	// material specular color, used with and without texture mapping.
		float shininess; // shininess (Halfangle dot Normal exponent)
		std::auto_ptr<map> colormap;	// replaces diffuse color if not defined.
		std::auto_ptr<map> normalmap;	// should be of type RGB to work properly.
		std::auto_ptr<map> specularmap; // should be of type LUMINANCE to work properly.
		
		material(const std::string& nm = "Unnamed material");
		virtual ~material() {}
		virtual void set_gl_values(const texture *caustic_map) const;
		virtual void set_gl_values_mirror_clip() const;
		virtual void register_layout(const std::string& name, const std::string& basepath);
		virtual void unregister_layout(const std::string& name);
		virtual void set_layout(const std::string& layout);
		virtual void get_all_layout_names(std::set<std::string>& result) const;
	};
	
	class mesh {
		mesh(const mesh& );
		mesh& operator= (const mesh& );

	public:
		// translate primitive_type to GL enum
		int gl_primitive_type() const;
		const char* name_primitive_type() const;

		// meshes can be made of triangles (default) or other common primitives.
		enum primitive_type {
			pt_triangles,
			pt_triangle_strip
		};

		// classes to iterate over triangles for all primitive types
		class triangle_iterator
		{
		protected:
			unsigned _i0, _i1, _i2;
			const std::vector<Uint32>& idx;
			unsigned ptr;
		public:
			triangle_iterator(const std::vector<Uint32>& indices);
			virtual ~triangle_iterator() {}
			unsigned i0() const { return _i0; }
			unsigned i1() const { return _i1; }
			unsigned i2() const { return _i2; }
			virtual bool next();
		};

		class triangle_strip_iterator : public triangle_iterator
		{
		public:
			triangle_strip_iterator(const std::vector<Uint32>& indices);
			virtual bool next();
		};

		std::auto_ptr<triangle_iterator> get_tri_iterator() const;

		std::string name;
		// This data is NOT needed for rendering, as it is stored also in VBOs,
		// except for the old pipeline where we need the data (or part of it),
		// to compute lighting values per frame.
		std::vector<vector3f> vertices;
		std::vector<vector3f> normals;
		std::vector<vector3f> tangentsx;
		std::vector<vector2f> texcoords;
		// we can have a right handed or a left handed coordinate system for each vertex,
		// dependent on the direction of the u,v coordinates. We do not store the third
		// vector per vertex but a flag, which saves space.
		// fixme: research if we can have a right-handed system always. This would
		// save fetching the colors to the vertex shader and thus spare memory bandwidth.
		std::vector<Uint8> righthanded;	// a vector of bools. takes more space than a bitvector, but faster access.
		std::vector<Uint32> indices;	// 3 indices per face
		primitive_type indices_type;
		matrix4f transformation;	// rot., transl., scaling
		material* mymaterial;
		vector3f min, max;
		// OpenGL VBOs for the data
		vertexbufferobject vbo_positions;
		vertexbufferobject vbo_normals;
		vertexbufferobject vbo_texcoords;
		vertexbufferobject vbo_tangents_righthanded;
		mutable vertexbufferobject vbo_colors;	// mutable because non-shader pipeline writes to it
		vertexbufferobject index_data;
		unsigned vertex_attrib_index;
		matrix3 inertia_tensor;
		double volume;

		void display(const texture *caustic_map=NULL) const;
		void display_mirror_clip() const;
		void compute_bounds();	
		void compute_normals();
		bool compute_tangentx(unsigned i0, unsigned i1, unsigned i2);

		mesh(const std::string& nm = "Unnamed mesh");

		/// create mesh from height map - around world origin
		///@param w - width of 2d field's data values
		///@param h - height of 2d field's data values
		///@param heights - height values
		///@param scales - scalars for x,y,z values
		///@param trans - translation for each vertex
		///@nm - name
		mesh(unsigned w, unsigned h, const std::vector<float>& heights, const vector3f& scales,
		     const vector3f& trans = vector3f(),
		     const std::string& nm = "Heightfield");

		// make display list if possible
		void compile();

		// transform vertices by matrix
		void transform(const matrix4f& m);
		void write_off_file(const std::string& fn) const;

		// give plane equation (abc must have length 1)
		std::pair<mesh*, mesh*> split(const vector3f& abc, float d) const;

		bool is_inside(const vector3f& p) const;
		//fixme: add function to compute total volume
		vector3 compute_center_of_gravity() const;
		matrix3 compute_inertia_tensor() const;
	};

	struct light {
		std::string name;
		vector3f pos;
		float colr, colg, colb;
		float ambient;
		void set_gl(unsigned nr_of_light) const;
		light() : colr(1.0f), colg(1.0f), colb(1.0f), ambient(0.1f) {}
	};

	/// voxel, the space of a model is partitioned in subspaces
	class voxel
	{
	public:
		/// position of center of voxel relative to the model
		vector3f relative_position;
		/// part of voxel that is filled with model volume (0...1)
		float part_of_volume;
		/// relative mass of the voxel of total mass (0...1)
		float relative_mass;
		/// construct a voxel
		voxel(const vector3f& rp, float pv, float m)
			: relative_position(rp), part_of_volume(pv), relative_mass(m) {}
	};

protected:	
	// a 3d object, references meshes
	struct object {
		unsigned id;
		std::string name;
		const mesh* mymesh;
		vector3f translation;
		int translation_constraint_axis;	// can be 0/1/2 for x/y/z
		float trans_val_min;	// minimum value for translation along axis
		float trans_val_max;	// maximum value for translation along axis
		vector3f rotat_axis;
		float rotat_angle;	// in degrees
		float rotat_angle_min;	// in degrees
		float rotat_angle_max;	// in degrees
		std::vector<object> children;
		object(unsigned id_ = 0, const std::string& nm = "???", const mesh* m = 0)
			: id(id_), name(nm), mymesh(m), rotat_angle(0), rotat_angle_min(0),
			  rotat_angle_max(0) { rotat_axis.z = 1; }
		bool set_angle(float ang);
		bool set_translation(float value);
		object* find(unsigned id);
		object* find(const std::string& name);
		void display(const texture *caustic_map=NULL) const;
		void compute_bounds(vector3f& min, vector3f& max) const;
	};

	// store that for debugging purposes.
	std::string filename;

	std::vector<material*> materials;
	std::vector<mesh*> meshes;
	std::vector<light> lights;

	object scene;
	
	std::string basename;	// base name of the scene/model, computed from filename
	std::string basepath;	// base path name of the scene/model, computed from filename

	vector3f min, max;

	std::string current_layout;

	// class-wide variables: shaders supported and enabled, shader number and init count
	static unsigned init_count;

	// Config options (only used when supported and enabled)
	static bool use_shaders;

	// Shader programs
	static std::auto_ptr<glsl_shader_setup> glsl_color_normal;
	static std::auto_ptr<glsl_shader_setup> glsl_color_normal_caustic;
	static std::auto_ptr<glsl_shader_setup> glsl_color_normal_specular;
	static std::auto_ptr<glsl_shader_setup> glsl_color_normal_specular_caustic;
	static std::auto_ptr<glsl_shader_setup> glsl_mirror_clip;

	// init / deinit
	static void render_init();
	static void render_deinit();

	void compute_bounds();
	void compute_normals();
	
	std::vector<float> cross_sections;	// array over angles

	/// nr of voxels in every dimension
	vector3i voxel_resolution;
	/// size of a voxel in 3-space
	vector3f voxel_size;
	/// "radius" of a voxel in 3-space
	float voxel_radius;
	/// per voxel: relative 3d position and part of volume that is inside (0...1)
	std::vector<voxel> voxel_data;
	
	void read_phys_file(const std::string& filename);
	
	// ------------ 3ds loading functions ------------------
	struct m3ds_chunk {
		unsigned short id;
		unsigned bytes_read;
		unsigned length;
		bool fully_read() const { return bytes_read >= length; }
		void skip(std::istream& in);
	};
	void m3ds_load(const std::string& fn);
	std::string m3ds_read_string(std::istream& in, m3ds_chunk& ch);
	m3ds_chunk m3ds_read_chunk(std::istream& in);
	std::string m3ds_read_string_from_rest_of_chunk(std::istream& in, m3ds_chunk& ch);
	void m3ds_process_toplevel_chunks(std::istream& in, m3ds_chunk& parent);
	void m3ds_process_model_chunks(std::istream& in, m3ds_chunk& parent);
	void m3ds_process_object_chunks(std::istream& in, m3ds_chunk& parent, const std::string& objname);
	void m3ds_process_trimesh_chunks(std::istream& in, m3ds_chunk& parent, const std::string& objname);
	void m3ds_process_light_chunks(std::istream& in, m3ds_chunk& parent, const std::string& objname);
	void m3ds_process_face_chunks(std::istream& in, m3ds_chunk& parent, mesh& m);
	void m3ds_process_material_chunks(std::istream& in, m3ds_chunk& parent);
	void m3ds_process_materialmap_chunks(std::istream& in, m3ds_chunk& parent, material::map* m);
	void m3ds_read_color_chunk(std::istream& in, m3ds_chunk& ch, color& col);
	void m3ds_read_faces(std::istream& in, m3ds_chunk& ch, mesh& m);
	void m3ds_read_uv_coords(std::istream& in, m3ds_chunk& ch, mesh& m);
	void m3ds_read_vertices(std::istream& in, m3ds_chunk& ch, mesh& m);
	void m3ds_read_material(std::istream& in, m3ds_chunk& ch, mesh& m);
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
	static const std::string default_layout;
	void set_layout(const std::string& layout = default_layout);
	void display(const texture *caustic_map=NULL) const;
	// display model but clip away coords with z < 0 in world space.
	// set up modelview matrix with world->eye transformation before calling this function
	// and give additional object->world transformation as matrix of texture unit #1 (2nd. unit).
	void display_mirror_clip() const;
	mesh& get_mesh(unsigned nr);
	const mesh& get_mesh(unsigned nr) const;
	/// get mesh at root of object tree or first mesh if no tree defined
	mesh& get_base_mesh();
	const mesh& get_base_mesh() const;
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
	// get min/max angles of an object. returns 0/0 if object does not exist
	vector2f get_object_angle_constraints(unsigned objid);
	vector2f get_object_angle_constraints(const std::string& objname);

	// manipulate object translation, returns false on error (wrong id or value out of bounds)
	bool set_object_translation(unsigned objid, double value);
	bool set_object_translation(const std::string& objname, double value);
	// get min/max translation values of an object. returns 0/0 if object does not exist
	vector2f get_object_translation_constraints(unsigned objid);
	vector2f get_object_translation_constraints(const std::string& objname);

	void register_layout(const std::string& name = default_layout);
	void unregister_layout(const std::string& name = default_layout);

	// collect all possible layout names from all materials/maps and insert them in "result"
	void get_all_layout_names(std::set<std::string>& result) const;

	std::string get_filename() const { return filename; }

	/// check if a given point is inside the model
	///@param p - a point in world space
	bool is_inside(const vector3f& p) const;

	/// request voxel data resolution
	const vector3i& get_voxel_resolution() const { return voxel_resolution; }
	/// request voxel size
	const vector3f& get_voxel_size() const { return voxel_size; }
	/// request voxel "radius"
	float get_voxel_radius() const { return voxel_radius; }
	/// request voxel data
	const std::vector<voxel>& get_voxel_data() const { return voxel_data; }
};	

#endif
