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

bool triangulate::is_inside_triangle(const vector2& a, const vector2& b, const vector2& c,
	const vector2& p)
{
	double s, t;
	bool solved = (p-a).solve(b-a, c-a, s, t);
	if (!solved) return true;
	return (s >= 0 && t >= 0 && s <= 1 && t <= 1 && s+t <= 1);
}

#include <sstream>
#include <cassert>
#include <fstream>
int failcount = 0;
vector<unsigned> triangulate::compute(const vector<vector2>& vertices)
{
	vector<unsigned> indices;
	if (vertices.size() < 3) return indices;	// error!
	indices.reserve(3*vertices.size());
	// fixme: use a vector and mark entries as "erased" (-1)
	// next then steps over erased entries.
	// count number of valid entries separately.
	// that is much easier and faster.
	// fixme 2: use delaunay condition to check triangles. But maybe the triangulation then fails some
	// times...
	vector<unsigned> vl;
	vl.reserve(vertices.size());
	unsigned vl_size = vertices.size();
	for (unsigned l = 0; l < vertices.size(); ++l)
		vl.push_back(l);
	unsigned i0 = 0;
	unsigned i1 = 1;
	unsigned i2 = 2;
// fixme: 2004/02/14, with the new map the lockups reoccour. why?!	
int iscorrecttests=0;
int notriangpossible=0;
int polyscreated=0;
int haengt=0;	// fixme: hack to avoid lock ups. why do they occour? reasons maybe:
		// 1) there are double points in the input, that means polygon edges are degenerated
		// or too short. OCCOUR, fixme, SEE COASTSEGMENT.CPP
		// 2) the polygon is self-intersecting. ???? CHECK
		// 3) polygon is inverted (hole), that means polylines are all in wrong direction AVOIDED, FIXED
		// 4) if several points are on one line a,b,c,d and one is beside (e)
		// and a triangle d,e,a is formed, the polygon's AREA is triangulated, but
		// b,c are on line a-d. -> change is_inside test, what about epsilon?! AVOIDED, FIXED
		// check these cases (1,2)
	while (vl_size > 3) {
++haengt;
if(haengt>8000){
	cout<<"TRIANGULATE: LOCKUP DETECTED! ("<<polyscreated<<","<<iscorrecttests<<","<<notriangpossible<<")\n";
	ostringstream oss; oss << "failed_triang_" << failcount++ << ".off";
	ofstream out(oss.str().c_str());
	unsigned vs = vertices.size();
	out << "OFF\n" << vs+1 << " " << vs << " 0\n";
	vector2 median;
	double dist2median = 0;
	for (unsigned i = 0; i < vs; ++i) {
		out << vertices[i].x << " " << vertices[i].y << " 0.0\n";
		median += vertices[i];
	}
	median = median * (1.0f/vs);
	for (unsigned i = 0; i < vs; ++i) {
		dist2median += median.distance(vertices[i]);
	}
	out << median.x << " " << median.y << " " << (2*dist2median/vs) << "\n";
	for (unsigned i = 0; i < vs; ++i)
		out << "3 " << i << " " << vs << " " << (i+1)%vs << "\n";
	return indices;
}
		if (!is_correct_triangle(vertices[i0], vertices[i1], vertices[i2])) {
++iscorrecttests;
			i0 = next(vl, i0);
			i1 = next(vl, i1);
			i2 = next(vl, i2);
			continue;
		}
		unsigned i3 = next(vl, i2);
		for ( ; i3 != i0; i3 = next(vl, i3)) {
			if (is_inside_triangle(vertices[i0], vertices[i1], vertices[i2], vertices[i3]))
				break;
		}
		if (i3 == i0) {
++polyscreated;		
			indices.push_back(i0);
			indices.push_back(i1);
			indices.push_back(i2);
//			cout << "TRI: adding triangle " << i0 << "/" << i1 << "/" << i2 << "\n";
			vl[i1] = unsigned(-1);
			--vl_size;
			i1 = i2;
			i2 = next(vl, i2);
		} else {
++notriangpossible;
			i0 = next(vl, i0);
			i1 = next(vl, i1);
			i2 = next(vl, i2);
		}
	}
	// add remaining triangle
	//fixme: wird letztes dreieck eingefügt, aber es ist nicht ccw????
	indices.push_back(i0);
	indices.push_back(i1);
	indices.push_back(i2);
//	cout << "TRI: adding triangle " << i0 << "/" << i1 << "/" << i2 << "\n";
	return indices;
}


#include <fstream>
void triangulate::debug_test(const vector<vector2>& vertices, const string& outputfile)
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
	vector<vector2> verts;
	for (unsigned i = 0; i < nverts; ++i) {
		double f = 2*3.14159f*i/nverts;
		vector2 d(cos(f), sin(f));
		double l = 50.0f+30.0f*rand()/RAND_MAX;
		verts.push_back(d*l);
	}
	
	vector<unsigned> idx = triangulate::debug_test(verts, "test.off");
}
*/
