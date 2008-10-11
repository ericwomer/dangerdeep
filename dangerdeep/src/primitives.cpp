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

primitives_plain::primitives_plain(int type_, unsigned size, bool with_colors, bool with_tex)
	: type(type_),
	  vertices(size)
{
	if (with_colors) colors.resize(size);
	if (with_tex) texcoords.resize(size);
}



void primitives_plain::render()
{
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, sizeof(vector3f), &vertices[0]);
	if (!colors.empty()) {
		glEnableClientState(GL_COLOR_ARRAY);
		glColorPointer(4, GL_UNSIGNED_BYTE, 0, &colors[0]);
	} else if (!texcoords.empty()) {
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2, GL_FLOAT, sizeof(vector2f), &texcoords[0]);
	}
	glDrawArrays(type, 0, vertices.size());
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}



primitives::primitives(int type_, unsigned size, const colorf& col_)
	: primitives_plain(type_, size, false, false),
	  col(col_),
	  tex(0)
{
}



primitives::primitives(int type_, unsigned size)
	: primitives_plain(type_, size, true, false),
	  tex(0)
{
}



primitives::primitives(int type_, unsigned size, const colorf& col_, const texture& tex_)
	: primitives_plain(type_, size, false, true),
	  col(col_),
	  tex(&tex_)
{
}



primitives::primitives(int type_, unsigned size, const texture& tex_)
	: primitives_plain(type_, size, true, true),
	  tex(&tex_)
{
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
		if (colors.empty()) {
			glsl_shader_setup::default_tex->use();
			glsl_shader_setup::default_tex->set_uniform(glsl_shader_setup::loc_t_color, col);
			glsl_shader_setup::default_tex->set_gl_texture(*tex, glsl_shader_setup::loc_t_tex, 0);
		} else {
			glsl_shader_setup::default_coltex->use();
			glsl_shader_setup::default_coltex->set_gl_texture(*tex, glsl_shader_setup::loc_ct_tex, 0);
		}
	} else {
		glsl_shader_setup::default_opaque->use();
		glsl_shader_setup::default_opaque->set_uniform(glsl_shader_setup::loc_o_color, col);
	}
	glDrawArrays(type, 0, vertices.size());
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glsl_shader_setup::use_fixed();//fixme replace later, when everything is rendered with shaders
}



primitive_tex<4> primitives::textured_quad(const vector2f& xy0,
					   const vector2f& xy1,
					   const texture& tex,
					   const vector2f& texc0,
					   const vector2f& texc1,
					   const colorf& col)
{
	primitive_tex<4> result(GL_QUADS, col, tex);
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



primitive<4> primitives::quad(const vector2f& xy0,
			      const vector2f& xy1,
			      const colorf& col)
{
	primitive<4> result(GL_QUADS, col);
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



primitive<4> primitives::rectangle(const vector2f& xy0,
				   const vector2f& xy1,
				   const colorf& col)
{
	primitive<4> result(GL_LINE_LOOP, col);
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



primitive<4> primitives::diamond(const vector2f& xy,
				 float r,
				 const colorf& col)
{
	primitive<4> result(GL_LINE_LOOP, col);
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
			      float radius,
			      const colorf& col)
{
	// use 2 pixels per line each
	primitives result(GL_LINE_LOOP, unsigned(floor(M_PI * radius)), col);
	for (unsigned i = 0; i < result.vertices.size(); ++i) {
		float a = i*2*M_PI/result.vertices.size();
		result.vertices[i].x = xy.x + sin(a)*radius;
		result.vertices[i].y = xy.y + cos(a)*radius;
	}
	return result;
}



primitive<2> primitives::line(const vector2f& xy0,
			      const vector2f& xy1,
			      const colorf& col)
{
	primitive<2> result(GL_LINES, col);
	result.vertices[0].x = xy0.x;
	result.vertices[0].y = xy0.y;
	result.vertices[1].x = xy1.x;
	result.vertices[1].y = xy1.y;
	return result;
}


primitive<2> primitives::line(const vector3f& xyz0,
			      const vector3f& xyz1,
			      const colorf& col)
{
	primitive<2> result(GL_LINES, col);
	result.vertices[0] = xyz0;
	result.vertices[1] = xyz1;
	return result;
}



primitive_tex<4> primitives::textured_quad(const vector3f& xyz0,
					   const vector3f& xyz1,
					   const vector3f& xyz2,
					   const vector3f& xyz3,
					   const texture& tex,
					   const vector2f& texc0,
					   const vector2f& texc1,
					   const colorf& col)
{
	primitive_tex<4> result(GL_QUADS, col, tex);
	result.vertices[0] = xyz0;
	result.vertices[1] = xyz1;
	result.vertices[2] = xyz2;
	result.vertices[3] = xyz3;
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
