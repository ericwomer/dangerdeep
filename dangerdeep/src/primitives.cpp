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

// OpenGL primitives container
// (C)+(W) by Thorsten Jordan. See LICENSE

#include "primitives.h"

primitives::primitives(int type_,
		       bool with_colors,
		       bool with_texcoords,
		       unsigned size)
	: type(type_)
{
	vertices.resize(size);
	if (with_colors) colors.resize(size);
	if (with_texcoords) texcoords.resize(size);
}



void primitives::render()
{
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, sizeof(vector3f), &vertices[0]);
	if (!colors.empty()) {
		glEnableClientState(GL_COLOR_ARRAY);
		glColorPointer(4, GL_UNSIGNED_BYTE, 0, &colors[0]);
		glsl_shader_setup::default_col->use();
	} else if (!texcoords.empty()) {
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2, GL_FLOAT, sizeof(vector2f), &texcoords[0]);
		//fixme: set uniform location to texture unit 0, or better current active unit
		if (colors.empty()) {
			glsl_shader_setup::default_tex->use();
			glUniform1i(glsl_shader_setup::loc_t_tex_color, 0);
		} else {
			glsl_shader_setup::default_coltex->use();
			glUniform1i(glsl_shader_setup::loc_ct_tex_color, 0);
		}
	} else {
		glsl_shader_setup::default_opaque->use();
	}
	glDrawArrays(type, 0, vertices.size());
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glsl_shader_setup::use_fixed();//fixme replace later, when everything is rendered with shaders
}



primitive_tex<4> primitives::textured_quad(const vector2f& xy0,
					   const vector2f& xy1,
					   const vector2f& texc0,
					   const vector2f& texc1)
{
	primitive_tex<4> result(GL_QUADS);
	result.vertices[0].x = xy0.x;
	result.vertices[0].y = xy0.y;
	result.vertices[1].x = xy1.x;
	result.vertices[1].y = xy0.y;
	result.vertices[2].x = xy1.x;
	result.vertices[2].y = xy1.y;
	result.vertices[3].x = xy0.x;
	result.vertices[3].y = xy1.y;
	result.texcoords[0].x = texc0.x;
	result.texcoords[0].y = texc0.y;
	result.texcoords[1].x = texc1.x;
	result.texcoords[1].y = texc0.y;
	result.texcoords[2].x = texc1.x;
	result.texcoords[2].y = texc1.y;
	result.texcoords[3].x = texc0.x;
	result.texcoords[3].y = texc1.y;
	return result;
}



primitive_tex<4> primitives::quad(const vector2f& xy0,
				  const vector2f& xy1)
{
	primitive_tex<4> result(GL_QUADS);
	result.vertices[0].x = xy0.x;
	result.vertices[0].y = xy0.y;
	result.vertices[1].x = xy1.x;
	result.vertices[1].y = xy0.y;
	result.vertices[2].x = xy1.x;
	result.vertices[2].y = xy1.y;
	result.vertices[3].x = xy0.x;
	result.vertices[3].y = xy1.y;
	return result;
}



primitive_tex<4> primitives::rectangle(const vector2f& xy0,
				       const vector2f& xy1)
{
	primitive_tex<4> result(GL_LINE_LOOP);
	result.vertices[0].x = xy0.x;
	result.vertices[0].y = xy0.y;
	result.vertices[1].x = xy1.x;
	result.vertices[1].y = xy0.y;
	result.vertices[2].x = xy1.x;
	result.vertices[2].y = xy1.y;
	result.vertices[3].x = xy0.x;
	result.vertices[3].y = xy1.y;
	return result;
}



primitive_tex<4> primitives::diamond(const vector2f& xy,
				     float r)
{
	primitive_tex<4> result(GL_LINE_LOOP);
	result.vertices[0].x = xy.x;
	result.vertices[0].y = xy.y+r;
	result.vertices[1].x = xy.x+r;
	result.vertices[1].y = xy.y;
	result.vertices[2].x = xy.x;
	result.vertices[2].y = xy.y-r;
	result.vertices[3].x = xy.x-r;
	result.vertices[3].y = xy.y;
	return result;
}



primitives primitives::circle(const vector2f& xy,
			      float radius)
{
	// use 2 pixels per line each
	primitives result(GL_LINE_LOOP, false, false, unsigned(floor(M_PI * radius)));
	for (unsigned i = 0; i < result.vertices.size(); ++i) {
		float a = i*2*M_PI/result.vertices.size();
		result.vertices[i].x = xy.x + sin(a)*radius;
		result.vertices[i].y = xy.y + cos(a)*radius;
	}
	return result;
}



primitive<2> primitives::line(const vector2f& xy0,
			      const vector2f& xy1)
{
	primitive<2> result(GL_LINES);
	result.vertices[0].x = xy0.x;
	result.vertices[0].y = xy0.y;
	result.vertices[1].x = xy1.x;
	result.vertices[1].y = xy1.y;
	return result;
}


primitive<2> primitives::line(const vector3f& xyz0,
			      const vector3f& xyz1)
{
	primitive<2> result(GL_LINES);
	result.vertices[0] = xyz0;
	result.vertices[1] = xyz1;
	return result;
}
