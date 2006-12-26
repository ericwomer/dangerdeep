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

#include "shader.h"
#include "oglext/OglExt.h"
#include "system.h"
#include "texture.h"
#include "error.h"
#include <stdexcept>
#include <fstream>
using namespace std;


int glsl_program::glsl_supported = -1;
bool glsl_program::supported()
{
	if (glsl_supported < 0) {
		glsl_supported = (sys().extension_supported("GL_ARB_fragment_shader") &&
				  sys().extension_supported("GL_ARB_shader_objects") &&
				  /* sys().extension_supported("GL_ARB_shading_language_100") && */
				  sys().extension_supported("GL_ARB_vertex_shader")) ? 1 : 0;
	}
	return glsl_supported == 1;
}



glsl_shader::glsl_shader(const string& filename, type stype, const glsl_shader::defines_list& dl)
	: id(0)
{
	if (!glsl_program::supported())
		throw std::runtime_error("GLSL shaders are not supported!");
	switch (stype) {
	case VERTEX:
		id = glCreateShader(GL_VERTEX_SHADER);
		break;
	case FRAGMENT:
		id = glCreateShader(GL_FRAGMENT_SHADER);
		break;
	default:
		throw invalid_argument("invalid shader type");
	}
	if (id == 0)
		throw runtime_error("can't create glsl shader");
	try {
		// read shader source
		ifstream ifprg(filename.c_str(), ios::in);
		if (ifprg.fail())
			throw file_read_error(filename);

		// the program as string
		string prg;

		// add defines to top of list for preprocessor
		for (defines_list::const_iterator it = dl.begin(); it != dl.end(); ++it) {
			prg += string("#define ") + *it + "\n";
		}

		// read lines.
		while (!ifprg.eof()) {
			string s;
			getline(ifprg, s);
			prg += s + "\n";
		}

		const char* prg_cstr = prg.c_str();
		glShaderSource(id, 1, &prg_cstr, 0);

		glCompileShader(id);

		GLint compiled = GL_FALSE;
		glGetShaderiv(id, GL_COMPILE_STATUS, &compiled);

		// get compile log
		GLint maxlength = 0;
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &maxlength);
		string log(maxlength+1, ' ');
		GLint length = 0;
		glGetShaderInfoLog(id, maxlength, &length, &log[0]);

		if (!compiled) {
			sys().add_console("compiling failed, log:");
			sys().add_console(log);
			printf("compiling of shader failed:\n%s\n", log.c_str());
			throw runtime_error(string("compiling of shader failed : ") + filename);
		}

		sys().add_console("shader compiled successfully, log:");
		sys().add_console(log);
	}
	catch (exception& e) {
		glDeleteShader(id);
		throw;
	}
}



glsl_shader::~glsl_shader()
{
	glDeleteShader(id);
}



glsl_program::glsl_program()
	: id(0), linked(false)
{
	if (!supported())
		throw std::runtime_error("GLSL programs are not supported!");
	id = glCreateProgram();
	if (id == 0)
		throw runtime_error("can't create glsl program");
}



glsl_program::~glsl_program()
{
	for (list<glsl_shader*>::iterator it = attached_shaders.begin(); it != attached_shaders.end(); ++it)
		glDetachShader(id, (*it)->id);
	glDeleteProgram(id);
}



void glsl_program::attach(glsl_shader& s)
{
	glAttachShader(id, s.id);
	attached_shaders.push_front(&s);
	linked = false;
}



void glsl_program::detach(glsl_shader& s)
{
	glDetachShader(id, s.id);
	for (list<glsl_shader*>::iterator it = attached_shaders.begin(); it != attached_shaders.end(); )
		if (*it == &s) {
			glDetachShader(id, (*it)->id);
			it = attached_shaders.erase(it);
		} else {
			++it;
		}
	linked = false;
}



void glsl_program::link()
{
	glLinkProgram(id);
	GLint waslinked = GL_FALSE;
	glGetProgramiv(id, GL_LINK_STATUS, &waslinked);

	if (!waslinked) {
		sys().add_console("linking failed, log:");
		GLint maxlength = 0;
		glGetProgramiv(id, GL_INFO_LOG_LENGTH, &maxlength);
		string log(maxlength+1, ' ');
		GLint length = 0;
		glGetProgramInfoLog(id, maxlength, &length, &log[0]);
		sys().add_console(log);
		throw runtime_error("linking of program failed");
	}

	linked = true;
}



void glsl_program::use() const
{
	if (!linked)
		throw runtime_error("glsl_program::use() : program not linked");
	glUseProgram(id);
}



void glsl_program::set_gl_texture(texture& tex, const std::string& texname, unsigned texunit) const
{
	GLint uniloc = glGetUniformLocation(id, texname.c_str());
	glActiveTexture(GL_TEXTURE0 + texunit);
	tex.set_gl_texture();
	glUniform1i(uniloc, texunit);
}



void glsl_program::use_fixed()
{
	glUseProgram(0);
}



glsl_shader_setup::glsl_shader_setup(const std::string& filename_vshader,
				     const std::string& filename_fshader,
				     const glsl_shader::defines_list& dl)
	: vs(filename_vshader, glsl_shader::VERTEX, dl),
	  fs(filename_fshader, glsl_shader::FRAGMENT, dl)
{
	prog.attach(vs);
	prog.attach(fs);
	prog.link();
}



void glsl_shader_setup::use() const
{
	prog.use();
}



void glsl_shader_setup::use_fixed()
{
	glUseProgram(0);
}
