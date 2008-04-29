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
#include "log.h"
#include <stdexcept>
#include <fstream>
using namespace std;


/*
Note!
Linux/Nvidia, use:

export __GL_WriteProgramObjectAssembly=1
export __GL_WriteProgramObjectSource=1

to get ASM source.
*/

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



const glsl_program* glsl_program::used_program = 0;



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

		// add version string for Nvidia cards (leave it out for ATI cards to work around some
		// crappy ATI drivers).
		if (sys().extension_supported("GL_NV_texture_env_combine4")) {
			// we have an Nvidia card (most probably)
			prg += "#version 110\n";
			// add some more performance boost stuff if requested
			if (1) { // fixme: later add cfg-switch for it
				prg += "#pragma optionNV(fastmath on)\n"
					"#pragma optionNV(fastprecision on)\n"
					"#pragma optionNV(ifcvt all)\n"
					"#pragma optionNV(inline all)\n"
					"#pragma optionNV(unroll all)\n";
			}
		}

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
		string compilelog(maxlength+1, ' ');
		GLsizei length = 0;
		glGetShaderInfoLog(id, maxlength, &length, &compilelog[0]);

		if (!compiled) {
			log_warning("compiling failed, log:");
			log_warning(compilelog);
			throw runtime_error(string("compiling of shader failed : ") + filename);
		}

		log_info("shader compiled successfully, log:");
		log_info(compilelog);
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
	if (used_program == this) {
		// rather use some kind of "bug!"-exception here
		log_warning("deleting bound glsl program!");
		use_fixed();
	}
	// if shaders are still attached, it is rather a bug...
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
		log_warning("linking failed, log:");
		GLint maxlength = 0;
		glGetProgramiv(id, GL_INFO_LOG_LENGTH, &maxlength);
		string compilelog(maxlength+1, ' ');
		GLsizei length = 0;
		glGetProgramInfoLog(id, maxlength, &length, &compilelog[0]);
		log_warning(compilelog);
		throw runtime_error("linking of program failed");
	}

	linked = true;
}



void glsl_program::use() const
{
	if (!linked)
		throw runtime_error("glsl_program::use() : program not linked");
	glUseProgram(id);
	used_program = this;
}



unsigned glsl_program::get_uniform_location(const std::string& name) const
{
	if (used_program != this)
		throw runtime_error("glsl_program::set_gl_texture, program not bound!");
	return unsigned(glGetUniformLocation(id, name.c_str()));
}



void glsl_program::set_gl_texture(texture& tex, unsigned loc, unsigned texunit) const
{
	if (used_program != this)
		throw runtime_error("glsl_program::set_gl_texture, program not bound!");
	glActiveTexture(GL_TEXTURE0 + texunit);
	tex.set_gl_texture();
	glUniform1i(loc, texunit);
}



void glsl_program::set_uniform(unsigned loc, const vector3f& value) const
{
	if (used_program != this)
		throw runtime_error("glsl_program::set_uniform, program not bound!");
	glUniform3f(loc, value.x, value.y, value.z);
}



void glsl_program::set_uniform(unsigned loc, const vector2f& value) const
{
	if (used_program != this)
		throw runtime_error("glsl_program::set_uniform, program not bound!");
	glUniform2f(loc, value.x, value.y);
}



void glsl_program::set_uniform(unsigned loc, float value) const
{
	if (used_program != this)
		throw runtime_error("glsl_program::set_uniform, program not bound!");
	glUniform1f(loc, value);
}



void glsl_program::set_uniform(unsigned loc, const vector3& value) const
{
	if (used_program != this)
		throw runtime_error("glsl_program::set_uniform, program not bound!");
	glUniform3f(loc, value.x, value.y, value.z);
}



void glsl_program::set_uniform(unsigned loc, const matrix4& value) const
{
	if (used_program != this)
		throw runtime_error("glsl_program::set_uniform, program not bound!");
	float tmp[16];
	const double* ea = value.elemarray();
	for (int i = 0; i < 16; ++i)
		tmp[i] = float(ea[i]);
	glUniformMatrix4fv(loc, 1, GL_TRUE, tmp);
}



unsigned glsl_program::get_vertex_attrib_index(const std::string& name) const
{
	if (used_program != this)
		throw runtime_error("glsl_program::get_vertex_attrib_index, program not bound!");
	return glGetAttribLocation(id, name.c_str());
	
}



void glsl_program::use_fixed()
{
	glUseProgram(0);
	used_program = 0;
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
	glsl_program::use_fixed();
}
