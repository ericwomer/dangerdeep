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
	for all j in from [i+3, i[ (cyclic indices!) check if p_j is in triangle p_i, p_i+1, p_i+2
	if all points are outside, then add triangle i,i+1,i+2, remove p_i+1 from point list
	else i:=i+1 (rotate list)
end
*/

bool triangulate::is_inside_triangle(const vector2f& a, const vector2f& b, const vector2f& c,
	const vector2f& p)
{
	float s, t;
	bool solved = (p-a).solve(b-a, c-a, s, t);
	if (!solved) return true;
	return (s >= 0 && t >= 0 && s <= 1 && t <= 1 && s+t <= 1);
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
//int haengt=0;	// fixme: hack to avoid lock ups. why do they occour? reasons maybe:
		// 1) there are double points in the input, that means polygon edges are degenerated
		// or too short. OCCOUR, fixme, SEE COASTSEGMENT.CPP
		// 2) the polygon is self-intersecting. ???? CHECK
		// 3) polygon is inverted (hole), that means polylines are all in wrong direction AVOIDED, FIXED
		// 4) if several points are on one line a,b,c,d and one is beside (e)
		// and a triangle d,e,a is formed, the polygon's AREA is triangulated, but
		// b,c are on line a-d. -> change is_inside test, what about epsilon?! AVOIDED, FIXED
		// check these cases (1,2)
	while (vl.size() > 3) {
//++haengt;
//if(haengt>2000){cout<<"TRIANGULATE: LOCKUP DETECTED!\n";assert(false);return indices;}
		if (!is_correct_triangle(vertices[*i0], vertices[*i1], vertices[*i2])) {
			next(vl, i0);
			next(vl, i1);
			next(vl, i2);
			continue;
		}
		list<unsigned>::iterator i3 = i2; next(vl, i3);
		for ( ; i3 != i0; next(vl, i3)) {
			if (is_inside_triangle(vertices[*i0], vertices[*i1], vertices[*i2], vertices[*i3]))
				break;
		}
		if (i3 == i0) {
			indices.push_back(*i0);
			indices.push_back(*i1);
			indices.push_back(*i2);
			vl.erase(i1);
			i1 = i2;
			next(vl, i2);
		} else {
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


#include <fstream>
void triangulate::debug_test(const vector<vector2f>& vertices, const string& outputfile)
{
	cout << "testing in \"" << outputfile << "\"\n";
	int nverts = int(vertices.size());
	vector<unsigned> idx = compute(vertices);

#if 1
	for (int j = 0; j < nverts; ++j) {	// show poly also
		idx.push_back(j);
		idx.push_back(j);
		idx.push_back((j+1)%nverts);
	}
#endif	
	
	int nfaces = int(idx.size())/3;
	ofstream out(outputfile.c_str());
	out << "OFF\n" << nverts << " " << nfaces << " 0\n";
	for (int i = 0; i < nverts; ++i) {
		out << vertices[i].x << " " << vertices[i].y << " 0.0\n";
	}
	for (int i = 0; i < nfaces; ++i) {
		out << "3 " << idx[i*3+0] << " " << idx[i*3+1] << " " << idx[i*3+2] << "\n";
	}
}

/*
// triang test
int main(int argc, char** argv)
{
	int nverts = 50;
	if (argc > 1) nverts = atoi(argv[1]);
	vector<vector2f> verts;
	for (unsigned i = 0; i < nverts; ++i) {
		float f = 2*3.14159f*i/nverts;
		vector2f d(cos(f), sin(f));
		float l = 50.0f+30.0f*rand()/RAND_MAX;
		verts.push_back(d*l);
	}
	
	vector<unsigned> idx = triangulate::debug_test(verts, "test.off");
}
*/
