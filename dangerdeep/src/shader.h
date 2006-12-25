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

#include <string>
#include <list>

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
		FRAGMENT
	};

	/// create a shader
	glsl_shader(const std::string& filename, type stype);

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

	/// check if GLSL is supported
	static bool supported();

 protected:
	unsigned id;
	bool linked;
	static int glsl_supported;
	std::list<glsl_shader*> attached_shaders;

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
			  const std::string& filename_fshader);

	/// use this setup
	void use() const;

	/// use fixed function pipeline instead of particular setup
	static void use_fixed();

 protected:
	glsl_shader vs, fs;
	glsl_program prog;
};

#endif
