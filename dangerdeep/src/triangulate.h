// a simple polygon triangulation algorithm
// (C) Thorsten Jordan

// give a line loop (polygon) of vertices, clockwise order
// returns vector of vertex indices (triangles, 3*m indices, m = #triangles, ccw order)

#ifndef TRIANGULATE_H
#define TRIANGULATE_H

#include "vector2.h"
#include <list>
#include <vector>
using namespace std;

struct triangulate
{
	static void next(list<unsigned>& vl, list<unsigned>::iterator& i) {
		++i; if (i == vl.end()) i = vl.begin();
	}

	static bool is_correct_triangle(const vector2f& a, const vector2f& b, const vector2f& c) {
		return (b.x-a.x)*(c.y-a.y) > (b.y-a.y)*(c.x-a.x);
	}
	
	static bool is_inside_triangle(const vector2f& a, const vector2f& b, const vector2f& c, const vector2f& p);

	static vector<unsigned> compute(const vector<vector2f>& vertices);
};

#endif
