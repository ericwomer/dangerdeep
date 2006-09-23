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

// a simple polygon triangulation algorithm
// (C) Thorsten Jordan

// give a line loop (polygon) of vertices, clockwise order
// returns vector of vertex indices (triangles, 3*m indices, m = #triangles, ccw order)

#ifndef TRIANGULATE_H
#define TRIANGULATE_H

#include "vector2.h"
#include <vector>

///\brief Triangulation algorithm for planar meshes.
struct triangulate
{
	static unsigned next(const std::vector<unsigned>& vl, unsigned i) {
		do {
			++i;
			if (i == vl.size()) i = 0;
		} while (vl[i] == unsigned(-1));
		return i;
	}

	static bool is_correct_triangle(const vector2& a, const vector2& b, const vector2& c) {
		return (b.x-a.x)*(c.y-a.y) > (b.y-a.y)*(c.x-a.x);
	}
	
	static bool is_inside_triangle(const vector2& a, const vector2& b, const vector2& c, const vector2& p);

	static std::vector<unsigned> compute(const std::vector<vector2>& vertices);
	
	static void debug_test(const std::vector<vector2>& vertices, const std::string& outputfile);
};

#endif
