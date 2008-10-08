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

/// this class models OpenGL primitives
///@note needs OpenGL 2.0.
class primitives
{
 public:
	primitives(int type = GL_TRIANGLES,
		   bool with_colors = false, bool with_texcoords = false,
		   bool with_normals = false, unsigned size = 0);
	void render();

	static primitives textured_quad(const vector2f& xy0,
					const vector2f& xy1,
					const vector2f& texc0,
					const vector2f& texc1);

	std::vector<vector3f> vertices;
	std::vector<color> colors;
	std::vector<vector2f> texcoords;
	std::vector<vector3f> normals;
	int type;
};

#endif
