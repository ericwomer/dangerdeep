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

// OpenGL GLSL shaders
// (C)+(W) by Thorsten Jordan. See LICENSE

#ifndef SHADER_H
#define SHADER_H

#include "vector4.h"
#include "matrix4.h"
#include "color.h"
#include <string>
#include <list>
#include <vector>
#include <memory>

class texture;

/// this class handles an OpenGL GLSL Shader
///@note needs OpenGL 2.0.
///@note reference counting is done by OpenGL.
///@note shaders can be deleted after they have been attached to a glsl_program.
class glsl_shader
{
 public:
	/// type of shader (vertex or fragment, later maybe geometry shader with GF8800+)
	enum type {
		VERTEX,
		FRAGMENT,
		VERTEX_IMMEDIATE,	// source instead of filename
		FRAGMENT_IMMEDIATE	// source instead of filename
	};

	/// a list of strings with shader preprocessor defines
	typedef std::list<std::string> defines_list;

	/// use a default define HQSFX everywhere if enabled
	static bool enable_hqsfx;

	/// use for some specific shader optimizations
	static bool is_nvidia_card;

	/// create a shader
	glsl_shader(const std::string& filename, type stype, const defines_list& dl = defines_list());

	/// destroy a shader
	~glsl_shader();

 protected:
	friend class glsl_program;	// for id access

	unsigned id;

 private:
	glsl_shader();
	glsl_shader(const glsl_shader& );
	glsl_shader& operator= (const glsl_shader& );
};



/// this class handles an OpenGL GLSL Program, that is a link unit of shaders.
///@note needs OpenGL 2.0
class glsl_program
{
 public:
	/// create program
	glsl_program();

	/// destroy program
	~glsl_program();

	/// attach a shader
	void attach(glsl_shader& s);

	/// attach a shader
	void detach(glsl_shader& s);

	/// link program after all shaders are attached
	void link();

	/// use this program
	///@note link program before using it!
	void use() const;

	/// use fixed function pipeline instead of particular program
	static void use_fixed();

	/// check if a program is bound (more for debugging)
	static bool is_fixed_in_use();

	/// get location (number) of uniform value
	unsigned get_uniform_location(const std::string& name) const;

	/// set up texture for a particular shader name
	void set_gl_texture(const texture& tex, unsigned loc, unsigned texunit) const;

	/// set uniform variable (float)
	void set_uniform(unsigned loc, float value) const;

	/// set uniform variable (doubles)
	void set_uniform(unsigned loc, const vector3& value) const;
		
	/// set uniform variable (vec2)
	void set_uniform(unsigned loc, const vector2f& value) const;
		
	/// set uniform variable (vec3)
	void set_uniform(unsigned loc, const vector3f& value) const;
		
	/// set uniform variable (vec4)
	void set_uniform(unsigned loc, const vector4f& value) const;
		
	/// set uniform variable (vec4 array)
	void set_uniform(unsigned loc, const std::vector<vector4f>& values) const;

	/// set uniform variable (matrix4)
	void set_uniform(unsigned loc, const matrix4& value) const;

	/// set uniform variable (vec4)
	void set_uniform(unsigned loc, const colorf& value) const;

	/// get vertex attribute index
	unsigned get_vertex_attrib_index(const std::string& name) const;

 protected:
	unsigned id;
	bool linked;
	std::list<glsl_shader*> attached_shaders;
	static const glsl_program* used_program;

 private:
	glsl_program(const glsl_program& );
	glsl_program& operator= (const glsl_program& );
};



/// this class combines two shaders and one program to a shader setup
class glsl_shader_setup
{
 public:
	/// create shader setup of two shaders
	glsl_shader_setup(const std::string& filename_vshader,
			  const std::string& filename_fshader,
			  const glsl_shader::defines_list& dl = glsl_shader::defines_list(),
			  bool immediate = false);

	/// use this setup
	void use() const;

	/// use fixed function pipeline instead of particular setup
	static void use_fixed();

	/// get location (number) of uniform value
	unsigned get_uniform_location(const std::string& name) const {
		return prog.get_uniform_location(name);
	}

	/// set up texture for a particular shader name
	void set_gl_texture(const texture& tex, unsigned loc, unsigned texunitnr) const {
		prog.set_gl_texture(tex, loc, texunitnr);
	}

	/// set uniform variable
	void set_uniform(unsigned loc, const vector3f& value) const {
		prog.set_uniform(loc, value);
	}

	/// set uniform variable
	void set_uniform(unsigned loc, const vector2f& value) const {
		prog.set_uniform(loc, value);
	}

	/// set uniform variable
	void set_uniform(unsigned loc, float value) const {
		prog.set_uniform(loc, value);
	}

	/// set uniform variable (doubles)
	void set_uniform(unsigned loc, const vector3& value) const {
		prog.set_uniform(loc, value);
	}

	/// set uniform variable (matrix4)
	void set_uniform(unsigned loc, const matrix4& value) const {
		prog.set_uniform(loc, value);
	}

	/// set uniform variable (vec4)
	void set_uniform(unsigned loc, const vector4f& value) const {
		prog.set_uniform(loc, value);
	}

	/// set uniform variable (vec4)
	void set_uniform(unsigned loc, const std::vector<vector4f>& value) const {
		prog.set_uniform(loc, value);
	}
		
	/// set uniform variable (vec4)
	void set_uniform(unsigned loc, const colorf& value) const {
		prog.set_uniform(loc, value);
	}

	/// get vertex attribute index
	unsigned get_vertex_attrib_index(const std::string& name) const {
		return prog.get_vertex_attrib_index(name);
	}

	static std::auto_ptr<glsl_shader_setup> default_opaque;
	static std::auto_ptr<glsl_shader_setup> default_col;
	static std::auto_ptr<glsl_shader_setup> default_tex;
	static std::auto_ptr<glsl_shader_setup> default_coltex;
	static unsigned loc_o_color;
	static unsigned loc_t_tex;
	static unsigned loc_t_color;
	static unsigned loc_ct_tex;
	static void default_init();
	static void default_deinit();

 protected:
	glsl_shader vs, fs;
	glsl_program prog;
};

#endif
