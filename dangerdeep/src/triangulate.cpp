// a simple polygon triangulation algorithm
// (C) Thorsten Jordan

#include "triangulate.h"

/*
how it works:
polygon (point list) are points p_0....p_n-1
i = 0
while point list contains more than three points
	take first point on list (index i)
	if p_i, p_i+1, p_i+2 don't form a triangle (angle at p_i+1 is >= 180°), i:=i+1, continue
	compute circle c through p_i, p_i+1, p_i+2
	for all j in from [i+3, i[ (cyclic indices!) check if p_j is in c
	if all points are outside c, then add triangle i,i+1,i+2, remove p_i+1 from point list
	else i:=i+1 (rotate list)
end
*/

pair<vector2f, float> triangulate::get_circle(const vector2f& a, const vector2f& b, const vector2f& c)
{
	vector2f bao = (b-a).orthogonal();
	vector2f cbo = (c-b).orthogonal();
	float s, t;
	((c-a)*0.5f).solve(bao, cbo, s, t);
	vector2f ct = ((a+b)*0.5f) + bao*s;
	return make_pair(ct, ct.square_distance(a));
}

vector<unsigned> triangulate::compute(const vector<vector2f>& vertices)
{
	vector<unsigned> indices;
	if (vertices.size() < 3) return indices;	// error!
	indices.reserve(3*vertices.size());
	list<unsigned> vl;
	for (unsigned l = 0; l < vertices.size(); ++l)
		vl.push_back(l);
	list<unsigned>::iterator i0 = vl.begin();
	list<unsigned>::iterator i1 = i0; next(vl, i1);
	list<unsigned>::iterator i2 = i1; next(vl, i2);
	while (vl.size() > 3) {
//cout<<"loop0 "<<vl.size()<<"\n";
		if (!is_correct_triangle(vertices[*i0], vertices[*i1], vertices[*i2])) {
			next(vl, i0);
			next(vl, i1);
			next(vl, i2);
			continue;
		}
//cout<<"is ok\n";		
		pair<vector2f, float> circle = get_circle(vertices[*i0], vertices[*i1], vertices[*i2]);
		list<unsigned>::iterator i3 = i2; next(vl, i3);
		for ( ; i3 != i0; next(vl, i3)) {
//cout<<"checking fourth...\n";
			// length^2 of (v0-center) is < radius^2 fixme
			if (is_inside_circle(circle, vertices[*i3]))
				break;
		}
		if (i3 == i0) {	// triangle is part of Delaunay triangulation
//cout<<"delaunay found!\n";
			indices.push_back(*i0);
			indices.push_back(*i1);
			indices.push_back(*i2);
			vl.erase(i1);
			i1 = i2;
			next(vl, i2);
		} else {
//cout<<"failed, ct\n";
			next(vl, i0);
			next(vl, i1);
			next(vl, i2);
		}
	}
	// add remaining triangle
	indices.push_back(*i0);
	indices.push_back(*i1);
	indices.push_back(*i2);
	return indices;
}
