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

#ifndef PRIMITIVES_H
#define PRIMITIVES_H

#include "vector3.h"
#include "color.h"
#include <vector>



/// this class models OpenGL primitives with fix vertex count
template<unsigned size>
class primitive
{
 public:
	primitive(int type_ = GL_TRIANGLES) : type(type_) {}
	void render() {
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(3, GL_FLOAT, sizeof(vector3f), &vertices[0]);
		glDrawArrays(type, 0, size);
		glDisableClientState(GL_VERTEX_ARRAY);
	}

	vector3f vertices[size];
	int type;
};

/// this class models OpenGL primitives with fix vertex count and colors
template<unsigned size>
class primitive_col
{
 public:
	primitive_col(int type_ = GL_TRIANGLES) : type(type_) {}
	void render() {
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(3, GL_FLOAT, sizeof(vector3f), &vertices[0]);
		glEnableClientState(GL_COLOR_ARRAY);
		glColorPointer(4, GL_UNSIGNED_BYTE, 0, &colors[0]);
		glDrawArrays(type, 0, size);
		glDisableClientState(GL_COLOR_ARRAY);
		glDisableClientState(GL_VERTEX_ARRAY);
	}

	vector3f vertices[size];
	color colors[size];
	int type;
};

/// this class models OpenGL primitives with fix vertex count and texcoords
template<unsigned size>
class primitive_tex
{
 public:
	primitive_tex(int type_ = GL_TRIANGLES) : type(type_) {}
	void render() {
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(3, GL_FLOAT, sizeof(vector3f), &vertices[0]);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2, GL_FLOAT, sizeof(vector2f), &texcoords[0]);
		glDrawArrays(type, 0, size);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisableClientState(GL_VERTEX_ARRAY);
	}

	vector3f vertices[size];
	vector2f texcoords[size];
	int type;
};

/// this class models OpenGL primitives with fix vertex count and colors + texcoords
template<unsigned size>
class primitive_coltex
{
 public:
	primitive_coltex(int type_ = GL_TRIANGLES) : type(type_) {}
	void render() {
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(3, GL_FLOAT, sizeof(vector3f), &vertices[0]);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2, GL_FLOAT, sizeof(vector2f), &texcoords[0]);
		glEnableClientState(GL_COLOR_ARRAY);
		glColorPointer(4, GL_UNSIGNED_BYTE, 0, &colors[0]);
		glDrawArrays(type, 0, size);
		glDisableClientState(GL_COLOR_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisableClientState(GL_VERTEX_ARRAY);
	}

	vector3f vertices[size];
	color colors[size];
	vector2f texcoords[size];
	int type;
};



/// this class models OpenGL primitives with variable vertex count
class primitives
{
 public:
	primitives(int type = GL_TRIANGLES,
		   bool with_colors = false,
		   bool with_texcoords = false,
		   unsigned size = 0);
	void render();

	/// render a 2d textured quad, face is back-sided
	static primitive_tex<4> textured_quad(const vector2f& xy0,
					      const vector2f& xy1,
					      const vector2f& texc0,
					      const vector2f& texc1);
	/// render a 2d quad, face is back-sided
	static primitive_tex<4> quad(const vector2f& xy0,
				     const vector2f& xy1);
	/// render a 2d line
	static primitive<2> line(const vector2f& xy0,
				 const vector2f& xy1);
	/// render a 3d line
	static primitive<2> line(const vector3f& xyz0,
				 const vector3f& xyz1);

	std::vector<vector3f> vertices;
	std::vector<color> colors;
	std::vector<vector2f> texcoords;
	int type;
};

#endif
