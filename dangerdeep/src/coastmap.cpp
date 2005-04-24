// a coastmap
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include "oglext/OglExt.h"

#include "coastmap.h"
#include "binstream.h"
#include "global_data.h"
#include "texture.h"
#include "system.h"
#include "triangulate.h"
#include "tinyxml/tinyxml.h"
#include <SDL_image.h>
#include <cassert>
#include <fstream>
#include <list>
#include <vector>
using namespace std;



const unsigned BSPLINE_SMOOTH_FACTOR = 1;//16;	// should be 3...16

// order (0-3):
// 32
// 01
const int coastmap::dmx[4] = { 0, 1, 1, 0 };
const int coastmap::dmy[4] = { 0, 0, 1, 1 };
// order: left, down, right, up
const int coastmap::dx[4] = { -1,  0, 1, 0 };
const int coastmap::dy[4] = {  0, -1, 0, 1 };


/*
2005/04/24
fixme: define what is meant how and check that.
rules:
segcls are connected in ccw order.
vertices are stored in ccw order.
this means when iterating along the coast, land is left and sea is right (ccw).
lakes are not allowed (when they're completely inside a seg, because the
triangulation failes there).
borders are stored from 0-3 in ccw order: left,bottom,right,top.
*/


void coastline::create_points(vector<vector2>& points, float begint, float endt, int detail) const
{
	sys().myassert(begint < endt, "error: begint >= endt (%f %f)", begint, endt);

	// compute number of points to be generated
	unsigned totalpts = curve.control_points().size();
	unsigned nrpts = unsigned(round(float(totalpts) * (endt-begint)));
	nrpts = (nrpts << detail);
	if (nrpts < 2) nrpts = 2;

	points.reserve(points.size() + nrpts);
	float t = begint, tstep = (endt - begint) / float(nrpts - 1);
	for (unsigned i = 0; i < nrpts; ++i) {
		if (t > 1.0f) t = 1.0f;	// fixme: can this happen? only for last point?
		points.push_back(curve.value(t));
		t += tstep;
	}
}



void coastline::draw_as_map(int detail) const
{
	glBindTexture(GL_TEXTURE_2D, 0);
	glBegin(GL_LINE_STRIP);
	int z = 0;
	for (vector<vector2>::const_iterator it = curve.control_points().begin(); it != curve.control_points().end(); ++it) {
		float p = float(z % 8 + 1)/8;
		float p2 = float(z % 4 + 1)/4;
		glColor3f(0,p,p2);
		glVertex2f(it->x, it->y);
		++z;
	}
	glEnd();
	glColor3f(1,1,1);
}



#if 0
void coastline::render(const vector2& p, int detail) const
{
	// fixme: cache that somehow
	vector<vector2> pts = create_points(p, 1, 0, points.size(), detail);
	
	glPushMatrix();
	glTranslatef(-p.x, -p.y, 0);
	
	unsigned ps = pts.size();
	glBegin(GL_QUAD_STRIP);
	double coastheight = 50;
	float t = 0.0;
	for (unsigned s = 0; s < ps; ++s) {
		const vector2& p = pts[s];
		unsigned prevpt = (s > 0) ? s-1 : s;
		unsigned nextpt = (s < ps-1) ? s+1 : s;
		vector2 n = (pts[nextpt] - pts[prevpt]).orthogonal().normal() * 50.0;
		if (s == 0) {
			glTexCoord2f(t, 1);
			glVertex3f(p.x, p.y, -10);
			glTexCoord2f(t, 0);
			glVertex3f(p.x+n.x, p.y+n.y, coastheight);
		} else {
			glTexCoord2f(t, 0);	// texture coordinates are wrong, fixme
			glVertex3f(p.x+n.x, p.y+n.y, coastheight);
			glTexCoord2f(t, 1);
			glVertex3f(p.x, p.y, -10);
		}
		t += 1.0;
	}
	glEnd();
	
	glPopMatrix();
}
#endif



// the least distance between two points is around 0.014, fixme test again
void coastsegment::cacheentry::push_back_point(const vector2& p)
{
	if (points.size() > 0) {
		double d = points.back().square_distance(p);
		if (d < 1.0f) return;//fixme test hack
		sys().myassert(d >= 1.0f, "error: points are too close %f    %f %f  %f %f", d, points.back().x, points.back().y, p.x, p.y);
	}
	points.push_back(p);
}



// fixme: 2004/05/29
// this code has to be reviewed again. remove bugs, etc.
float gmd=1e30;
void coastsegment::generate_point_cache(const class coastmap& cm, const vector2& roff, int detail,
					const vector2& segoff) const
{
	if (type > 1) {
		// cache generated and unchanged?
		if (pointcache.size() > 0 && pointcachedetail == detail) return;

//	cout << "creating cache entry for segment " << roff << " empty? " << (pointcache.size() > 0) << " cached detail " << pointcachedetail << " new detail " << detail << "\n";
		// invalidate cache (detail changed or initial generation)
		pointcachedetail = detail;
		pointcache.clear();

		unsigned nrcl = segcls.size();
		vector<bool> cl_handled(nrcl, false);
		for (unsigned i = 0; i < nrcl; ++i) {
			if (cl_handled[i]) continue;

			cacheentry ce;

		//cout << "about to find area!\n";
			// find area
			unsigned current = i;
			do {
				/* NOTE:
				   doubles shouldn't occour by design. But they do. So we do
				   an quick, simple and dirty workaround here and just avoid
				   inserting them. Finally we check if the last point is the
				   same as the first, in that case we remove it, too.
				   Reason for double points:
				   this seem to happen because sometimes endborder/beginborder
				   is >= 0 even if the corresponding points are not on the border.
				   so the coast lines are treated as coast, not as part of an
				   island -> end of first cl is equal to begin of next ->
				   double points result.
				   2004/05/28: fixme: is this because we take beginborder/endborder
				   values from the cl for island? (illegal combination
				   of island/non-island pair for beginborder/endborder)???
				*/
				const segcl& cl = segcls[current];
				cm.coastlines[cl.mapclnr].create_points(ce.points, cl.begint, cl.endt, detail);
				unsigned next = cl.next;
			//cout << "current="<<current<<" next="<<next<<"\n";
				cl_handled[current] = true;
				if(next!=i){
					if(cl_handled[next])break;
					//fixme: happens with segcls that have length 0 (on corners etc.)
					//fixme: this failes sometimes. related to island bug???
					sys().myassert(!cl_handled[next], "cl already handled?");
				}
				//if(next==0xffffffff)break;
				sys().myassert(next!=-1, "next unset?");//fixme: another unexplainable bug.
				//if(current==next)break;
				// insert corners if needed
				int ed = cl.endborder, bg = segcls[next].beginborder;
				if (ed >= 0 && bg >= 0) {
					if (ed == bg) {
						if (borderpos(ed, cl.endp - segoff, cm.segw_real) >
						    borderpos(bg, segcls[next].beginp, cm.segw_real)) {

						//fixme!!!!!!! segment offset is not subtracted here!!!!!!
//						if (dist_to_corner(ed, cl.endp, cm.segw_real) < dist_to_corner(bg, segcls[next].beginp, cm.segw_real))
							bg += 4;
						}
					} else if (bg < ed) {
							bg += 4;
					}
					for (int j = ed; j < bg; ++j) {
						int k = j % 4;
						// push back destination point of edge (0-3: bl,br,tr,tl)
						//fixme: many wrong corners are inserted...
						if (k == 0) ce.push_back_point(roff);
						else if (k == 1) ce.push_back_point(roff + vector2(cm.segw_real, 0));
						else if (k == 2) ce.push_back_point(roff + vector2(cm.segw_real, cm.segw_real));
						else ce.push_back_point(roff + vector2(0, cm.segw_real));
					}
				}
				current = next;
			} while (current != i);
		//cout << "area found!\n";

			//more a quick hack, it shouldn't be necessary, fixme!
			if (ce.points.back().square_distance(ce.points.front()) < 1.0f)
				ce.points.pop_back();
				

//detect double points, debug code, not needed any longer
//fixme: one time on the new map there are double points (d==0), but there are 8 triangulation faults
float mdd=1e30;
for(unsigned m=0;m<ce.points.size();++m){
float d=ce.points[m].distance(ce.points[(m+1)%ce.points.size()]);
if(d<mdd)mdd=d;
if(d<gmd)gmd=d;
if(d <= 0.01f)cout<<"fault: "<<d<<","<<m<<","<<(m+1)%ce.points.size()<<"\n";
//assert(d > 0.01f);
}


//			cout << "compute triang for segment " << this << "\n";
			
			// fixme: 2004/05/30:
			// the remaining triangulation bugs have two reasons:
			// 1. double points (hack-fix with this code)
			// 2. "flat" polygons (degenerated), which can't be triangulated
			// we have 11 failed triangulations and with double point removal
			// only 6, all degenerated polygon cases.
			// this gives a clue to fix the triangulation problems finally...
			// check which segment fails to triangulate and check the map
			// image what case it is.
			// first case: north of iceland: the coastline lies exactly on a
			// segment border!!!
			cout << "segment no: " << (this - &cm.coastsegments[0]) << "\n";
			unsigned dbl = 0;
			for (unsigned i = 0; i + 1 < ce.points.size(); ++i) {
				unsigned j = i+1;
				if (ce.points[j].square_distance(ce.points[i]) < 0.1f) {
					ce.points.erase(ce.points.begin()+j);
					--i;
					++dbl;
				}
			}
			if (dbl > 0) cout << "erased " << dbl << " double points!\n";
			
			ce.indices = triangulate::compute(ce.points);
			pointcache.push_back(ce);
		}
	}
}



void coastsegment::draw_as_map(const class coastmap& cm, int x, int y, const vector2& roff, int detail) const
{
//cout<<"segment draw " << x << "," << y << " dtl " << detail << " tp " << type << "\n";
	if (type == 1) {
		glBegin(GL_QUADS);
		glTexCoord2d(roff.x, roff.y);
		glVertex2d(roff.x, roff.y);
		glTexCoord2d(roff.x+cm.segw_real, roff.y);
		glVertex2d(roff.x+cm.segw_real, roff.y);
		glTexCoord2f(roff.x+cm.segw_real, roff.y+cm.segw_real);
		glVertex2d(roff.x+cm.segw_real, roff.y+cm.segw_real);
		glTexCoord2f(roff.x, roff.y+cm.segw_real);
		glVertex2d(roff.x, roff.y+cm.segw_real);
		glEnd();
	} else if (type > 1) {
#if 1
		vector2 segoff(x*cm.segw_real + cm.realoffset.x, y*cm.segw_real + cm.realoffset.y);
		generate_point_cache(cm, roff, detail+2, segoff);
		glBegin(GL_TRIANGLES);
		for (vector<cacheentry>::const_iterator cit = pointcache.begin(); cit != pointcache.end(); ++cit) {
			for (vector<unsigned>::const_iterator tit = cit->indices.begin(); tit != cit->indices.end(); ++tit) {
				const vector2& v = cit->points[*tit];
				glTexCoord2f(v.x, v.y);
				glVertex2f(v.x, v.y);
			}
		}
		glEnd();
#endif		
#if 1
		// test
		glBindTexture(GL_TEXTURE_2D, 0);
		glBegin(GL_LINES);
		for (unsigned i = 0; i < segcls.size(); ++i) {
			glColor3f(1.0f, 1.0f, 0.0f);
			glVertex2f(segcls[i].beginp.x, segcls[i].beginp.y);
			glColor3f(1.0f, 0.0f, 1.0f);
			glVertex2f(segcls[i].endp.x, segcls[i].endp.y);
		}
		glEnd();
		glColor3f(1,1,1);
#endif
#if 0
		// test
		for (unsigned i = 0; i < segcls.cm.segw_real(); ++i) {
			segcls[i].draw_as_map(off, cm.segw_real, t, ts, detail);
		}
#endif		
	}
}



//fixme:
//cache results.
//remove unnecessary flat quads in the distance/don't generate them
//introduces LOD.
//many more effects...
#include <sstream>
void coastsegment::render(const class coastmap& cm, int x, int y, const vector2& p, int detail) const
{
#if 0
	unsigned res = cm.pixels_per_seg;
	res = (res << detail);
	vector<vector3f> coords;
	coords.reserve((res+1)*(res+1));
	vector<vector2> uv0;
	uv0.reserve((res+1)*(res+1));
	vector2 segoff(x*cm.segw_real + cm.realoffset.x, y*cm.segw_real + cm.realoffset.y);
	for (unsigned yy = 0; yy <= res; ++yy) {
		for (unsigned xx = 0; xx <= res; ++xx) {
			float s = float(xx)/res;
			float t = float(yy)/res;
			float h = topo.value(s, t);
			coords.push_back(vector3f(segoff.x + s*cm.segw_real, segoff.y + t*cm.segw_real, h));
			uv0.push_back(vector2(s, t));
		}
	}
	vector<unsigned> indices;
	indices.reserve(res*res*4);
	for (unsigned yy = 0; yy < res; ++yy) {
		for (unsigned xx = 0; xx < res; ++xx) {
			indices.push_back(yy*(res+1)+xx);
			indices.push_back(yy*(res+1)+xx+1);
			indices.push_back((yy+1)*(res+1)+xx+1);
			indices.push_back((yy+1)*(res+1)+xx);
		}
	}
	
/*
	ostringstream oss;
	oss << "test_render_" << x << "_" << y << ".off";
	ofstream out(oss.str().c_str());
	out << "OFF\n" << coords.size() << " " << indices.size()/2 << " 0\n";
	for (unsigned i = 0; i < coords.size(); ++i)
		out << coords[i].x << " " << coords[i].y << " " << coords[i].z << "\n";
	for (unsigned i = 0; i < indices.size()/4; ++i) {
		out << "3 " << indices[i*4] << " " << indices[i*4+1] << " " << indices[i*4+3] << "\n";
		out << "3 " << indices[i*4+1] << " " << indices[i*4+3] << " " << indices[i*4+2] << "\n";
	}
*/
	
	glPushMatrix();

	terraintex->set_gl_texture();
	glColor4f(1,1,1,1);
	glNormal3f(0,0,1);//fixme
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, sizeof(vector3f), &coords[0].x);
	glClientActiveTexture(GL_TEXTURE0);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, sizeof(vector2), &uv0[0].x);
	glDrawElements(GL_QUADS, indices.size(), GL_UNSIGNED_INT, &(indices[0]));
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	glPopMatrix();

/*
	if (type > 1) {
		for (vector<coastline>::const_iterator it = coastlines.begin(); it != coastlines.end(); ++it) {
			it->render(p, detail);
		}
	}
*/
#endif
}



// return position on segment border. 0...1 left, 1...2 bottom, 2...3 right, 3...4 top.
//fixme: border number goes from top to left in cw order, borderpos in ccw order left to top, ouch.
float coastsegment::borderpos(int b, const vector2& p, float segw) const
{
	if (b == 0) return 0.0 + 1.0 - p.y/segw;
	else if (b == 1) return 1.0f + p.x/segw;
	else if (b == 2) return 2.0f + p.y/segw;
	else return 3.0f + 1.0f - p.x/segw;
}



float coastsegment::compute_border_dist(int b0, const vector2& p0, int b1, const vector2& p1, float segw) const
{
	float bp0 = borderpos(b0, p0, segw);
	float bp1 = borderpos(b1, p1, segw);
	if (bp1 < bp0) bp1 += 4.0f;
	return (bp1 - bp0) * segw;
}



void coastsegment::segcl::print() const
{
	cout << "segcl:\nmapclnr: " << mapclnr << " begint " << begint << " endt " << endt
	     << "\nbeginp: " << beginp << " endp: " << endp
	     << "\nbeginborder: " << beginborder << " endborder " << endborder
	     << "\ncyclic? " << cyclic << " next: " << next << "\n";
}

int coastsegment::get_successor_for_cl(unsigned cln, const vector2& segoff, double segw) const
{
//fixme: this function sometimes returns shit.
//this seems to be the reason for the triangulation bugs. several coastlines are connected
//and given to the triangulation algo, that are obviously not adjacent!!!
//(e.g. two parts of an island)
//fixme: another reason: if an island cl gets distributed to several segments the first and
//last part of the cl falls into the same segment, but CAN NOT get united! pieces are
//t=0...x1 and x2...1. Both pieces must get connected via "next" info instead!

/*
how it should work:
for a segcl cln:
if endp(cln) = island then next(cln) = cln itself.
else
  find a segcl x so that beginp(x) is the nearest point to endp(cln) on the border.
  just translate endborder/endp info to a number (float) in [0,4) =: n(cln)
  distance is (n(x) - n(cln) + 4) fmod 4
end

fixme: check wether we have to count cw or ccw around the segment border!
*/

	const segcl& scl0 = segcls[cln];

	// handle islands totally contained in one segment
	if (scl0.cyclic) {
		sys().myassert(scl0.beginborder < 0, "paranoia4");
		sys().myassert(scl0.endborder < 0, "paranoia5");
		return cln;
	}

/*
	cout << "scl0 " << cln << " mapclnr " << scl0.mapclnr << " begint " << scl0.begint << " endt " << scl0.endt
	     << " beginp " << (scl0.beginp.x - segoff.x)/segw << "," << (scl0.beginp.y - segoff.y)/segw << " endp " << (scl0.endp.x - segoff.x)/segw << "," << (scl0.endp.y-segoff.y)/segw << " bb " << scl0.beginborder << " eb " <<
		scl0.endborder << " cyc " << scl0.cyclic << "\n";

	sys().myassert(scl0.beginborder >= 0, "paranoia7");
	sys().myassert(scl0.endborder >= 0, "paranoia8");
*/

	if (scl0.beginborder < 0) sys().myassert(scl0.endborder >= 0, "paranoia8");

	// handle special case when endborder is < 0 (island that is distributed
	// to more than one segment, and we have first and last segcl of island here)
	if (scl0.endborder < 0) {
		sys().myassert(scl0.beginborder >= 0, "paranoia7");
		for (unsigned i = 0; i < segcls.size(); ++i) {
			const segcl& scl1 = segcls[i];
			if (scl0.mapclnr == scl1.mapclnr) {
				printf("cln %i i %i\n",cln,i);
				scl0.print();
				segcls[i].print();
			}
			if (scl0.mapclnr == scl1.mapclnr && scl1.beginborder < 0) {
				return i;
			}
		}
		//fixme: what is if island begin/end points fall on a segment border or even a corner?
		return cln;//fixme hack, return self.
		sys().myassert(false, "paranoia9");
	}

	float scl0num = borderpos(scl0.endborder, scl0.endp - segoff, segw);

	float mindist = 4.0f;
	int next = -1;
	for (unsigned i = 0; i < segcls.size(); ++i) {
		// i's successor can be i itself, when i enters and leaves the seg without any
		// other cl in between
		const segcl& scl1 = segcls[i];
		if (scl1.beginborder < 0) continue;	// not compareable
		float scl1num = borderpos(scl1.beginborder, scl1.beginp - segoff, segw);
		printf("get_succ %i %i   %f %f\n",cln,i,scl0num,scl1num);
		float numd = myfmod(scl1num - scl0num + 4.0f, 4.0f);
		if (numd < mindist) {
			mindist = numd;
			next = int(i);
		}
	}
	
	if (next == -1) {
		cout << "# segcls " << segcls.size() << "\n";
		for (unsigned i = 0; i < segcls.size(); ++i) {
			cout << "segcl #" << i << " : " << segcls[i].beginborder << "," <<
			segcls[i].endborder << " cyclic " << segcls[i].cyclic << ", begint "
			<< segcls[i].begint << " endt " << segcls[i].endt << " beginp "
			<< segcls[i].beginp << " endp " << segcls[i].endp << " borderpos begin "
			<< borderpos(segcls[i].beginborder, segcls[i].beginp - segoff, segw) << " end borderpos "
			<< borderpos(segcls[i].endborder, segcls[i].endp - segoff, segw) << "\n";
		}
	}

	sys().myassert(next != -1, "paranoia6");
	return next;
	

#if 0
	float mindist = 1e30;
	unsigned next = 0xffffffff;
//coastlines[cln].debug_print(cout);
//cout << " succ?\n";
	for (unsigned i = 0; i < segcls.size(); ++i) {
//coastlines[i].debug_print(cout);
		// i's successor can be i itself and may already have been handled (i itself!)
		const segcl& scl1 = segcls[i];

		// compare island begin and end piece
		if (scl0.mapclnr == scl1.mapclnr && scl0.endt == 1.0f && scl1.begint == 0.0f) {
//cout<<"island close fix happens.\n";
//this happens 138 times. do we have 138 island larger than one segment?!
			return i;
		}
		
		// compare for distance, 5 cases
		if (scl0.endborder < 0 && scl1.beginborder < 0) {	// part of an island
			float dist = scl0.endp.distance(scl1.beginp);
			if (dist < mindist) {
				mindist = dist;
				next = i;
			}
		} else if (scl0.endborder >= 0 && scl1.beginborder >= 0) {
			float dist = compute_border_dist(scl0.endborder, scl0.endp - segoff, scl1.beginborder, scl1.beginp - segoff, segw);
			if (dist < mindist) {
				mindist = dist;
				next = i;
			}
		} // else not compareable
	}
//	assert(next != 0xffffffff);
//cout << "next was " << next << "\n";	
	return next;
#endif
}









/*
2004/05/17
idea.
Store map as b/w png image.
Create coastlines from pixel border at runtime.
Create coast line with bsplines on the fly from border.
#Vertices = 2^detaillevel.
Compute vertices with binary subdivision: at t=0,1
then at t=0.5, later at t=0.25 and 0.75, t=0.125,0.375,0.625,0.875 etc.
would be fast enough for display at runtime. OK
Locally increased detail is possible, e.g. subdivide that part of a coastline TODO
that is inside the segment to a certain amount (5-10km/line) and subdivide
it more near the player (measure distance to line segment)

For each segment store which coastlines are inside with start t and end t
(t along the whole coastline). OK
When drawing multiple sectors unite the coastline lists of several sectors
(e.g. cl a is in segm 1 with t=[0.2...0.4] and cl a is also in segm2 with
t=[0.4...0.6] -> draw cl a with t=[0.2...0.6])
That way we have seamless coastlines. NO
only for rendering! complexity of triangulation is o(n^2) hence increasing n is not that
clever.

coastmap should move from user_interface to game because it's not only stored
for displaying purposes but should influence the game!
*/





//
// coastmap functions
//

Uint8& coastmap::mapf(int cx, int cy)
{
	cx = clamp_zero(cx);
	cy = clamp_zero(cy);
	cx = mapw - 1 - clamp_zero(mapw - 1 - cx);
	cy = maph - 1 - clamp_zero(maph - 1 - cy);
	return themap[cy*mapw+cx];
}



bool coastmap::find_begin_of_coastline(int& x, int& y)
{
	bool cyclic = false;
	int sx = x, sy = y, j2 = 0;
	while (true) {
		// compute next x,y
		int j = 0;
		Uint8 mv[4];
		for (j = 0; j < 4; ++j) {
			mv[j] = mapf(x+dmx[j], y+dmy[j]) & 0x7f;
		}
		// mirrored direction to find_coastline
		if (mv[0] == 0 && mv[2] == 0 && mv[1] > 0 && mv[3] > 0) {
			// check pattern:
			// #.
			// .#
			// direction: only 0,2 valid. 3->0, 1->2
			j = (j2 == 3) ? 0 : 2;
		} else if (mv[0] > 0 && mv[2] > 0 && mv[1] == 0 && mv[3] == 0) {
			// check pattern:
			// .#
			// #.
			// direction: only 1,3 valid. 0->1, 2->3
			j = (j2 == 0) ? 1 : 3;
		} else {
			// check patterns:
			// #. .# .. .. ## ## .# #. ## .# .. #.
			// .. .. .# #. #. .# ## ## .. .# ## #.
			// we need a direction so that right of it is land and left of it sea.
			// left: take mv 0,3  down: take mv 1,0  right: take mv 2,1  up: take mv 3,2
			// so take for direction j mv[j],mv[(j+3)%4]. first one must be sea.
			for (j = 0; j < 4; ++j) {
				if (mv[j] == 0 && mv[(j+3)%4] > 0) break;
			}
			assert(j < 4);
		}
		j2 = j;
		int nx = x + dx[j];
		int ny = y + dy[j];
		if (nx < 0 || ny < 0 || nx >= int(mapw) || ny >= int(maph))	// border reached
			break;
		x = nx;
		y = ny;

		if (sx == x && sy == y) {
			cyclic = true;
			break;	// island found
		}
	}
	return cyclic;
}



bool coastmap::find_coastline(int x, int y, vector<vector2i>& points, bool& cyclic, int& beginborder,
			      int& endborder)	// returns true if cl is valid
{
	// run backward at the coastline until we reach the border or round an island.
	// start there creating the coastline. this avoids coastlines that can never be seen.
	// In reality: north pole, ice, America to the west, Asia/africa to the east.
	// generate points in ccw order, that means land is left, sea is right.

	assert((mapf(x, y) & 0x80) == 0);
	
	cyclic = find_begin_of_coastline(x, y);

	beginborder = -1;	
	if (!cyclic) {
		if (x == 0) beginborder = 0;
		else if (y == 0) beginborder = 1;
		else if (x == int(mapw)-1) beginborder = 2;
		else if (y == int(maph)-1) beginborder = 3;
	}
	
	int sx = x, sy = y, j2 = 0, lastj = -1, turncount = 0;
	while (true) {
		// store x,y
		points.push_back(vector2i(x, y));

		assert(x>=0&&y>=0&&x<int(mapw)&&y<int(maph));
		mapf(x, y) |= 0x80;

		// compute next x,y	
		int j = 0;
		Uint8 mv[4];
		for (j = 0; j < 4; ++j) {
			mv[j] = mapf(x+dmx[j], y+dmy[j]) & 0x7f;
		}
		if (mv[0] == 0 && mv[2] == 0 && mv[1] > 0 && mv[3] > 0) {
			// check pattern:
			// #.
			// .#
			// direction: only 1,3 valid. 2->1, 0->3
			j = (j2 == 2) ? 1 : 3;
		} else if (mv[0] > 0 && mv[2] > 0 && mv[1] == 0 && mv[3] == 0) {
			// check pattern:
			// .#
			// #.
			// direction: only 0,2 valid. 1->0, 3->2
			j = (j2 == 1) ? 0 : 2;
		} else {
			// check patterns:
			// #. .# .. .. ## ## .# #. ## .# .. #.
			// .. .. .# #. #. .# ## ## .. .# ## #.
			// we need a direction so that left of it is land and right of it sea.
			// left: take mv 0,3  down: take mv 1,0  right: take mv 2,1  up: take mv 3,2
			// so take for direction j mv[j],mv[(j+3)%4]. first one must be land.
			for (j = 0; j < 4; ++j) {
				if (mv[j] > 0 && mv[(j+3)%4] == 0) break;
			}
			assert(j < 4);
		}
		j2 = j;

		if (lastj != -1 && cyclic) {
			int jd = lastj-j;
			if (jd < 0) jd += 4;
			if (jd == 3) jd = -1;
			//if(jd<-1||jd>1)cout<<"j="<<j<<" lastj="<<lastj<<"\n";
			assert(jd>=-1&&jd<=1);
			turncount += jd;
		}
		lastj = j;
		
		int nx = x + dx[j];
		int ny = y + dy[j];
		if (nx < 0 || ny < 0 || nx >= int(mapw) || ny >= int(maph)) // border reached
			break;
		x = nx;
		y = ny;
		if (sx == x && sy == y) {
			break;	// island found
		}
	}
	
	endborder = -1;
	if (!cyclic) {
		if (x == 0) endborder = 0;
		else if (y == 0) endborder = 1;
		else if (x == int(mapw)-1) endborder = 2;
		else if (y == int(maph)-1) endborder = 3;
	}

	return turncount >= 0;
}



void coastmap::divide_and_distribute_cl(const coastline& cl, unsigned clnr, int beginb, int endb, const vector<vector2>& points, bool clcyclic)
{
	coastsegment::segcl scl;
	scl.mapclnr = clnr;
	scl.cyclic = false;
	scl.next = -1;

	// divide coastline at segment borders
	// find segment that first point is into
	vector2 cp0 = cl.curve.value(0);
	int sx = int((cp0.x - realoffset.x) / segw_real);
	int sy = int((cp0.y - realoffset.y) / segw_real);
	sys().myassert(sx < segsx && sy < segsy, "paranoia");
	int seg = sy*segsx+sx;
	vector2 segoff(sx * segw_real + realoffset.x, sy * segw_real + realoffset.y);

	scl.begint = 0;
	scl.beginp = cp0;
	scl.beginborder = beginb;
	
	const double eps = 1e-5;
	const int segdx[4] = { 0, 1, 0, -1 };
	const int segdy[4] = { 1, 0, -1, 0 };

	bool sameseg = true;
	float t0 = 0;
	vector2 cpoldi = cp0;
	// loop over coastline
	const int detailfac = 1;
	for (unsigned i = 1; i < points.size() * detailfac; ++i) {
		float t1 = float(i)/float(points.size() * detailfac);
		// destination point and segment
		vector2 cpi = cl.curve.value(t1);
		int csx = int((cpi.x - realoffset.x) / segw_real);
		int csy = int((cpi.y - realoffset.y) / segw_real);
		int cseg = csy*segsx+csx;
		sys().myassert(csx < segsx && csy < segsy, "paranoia2");

		if (seg != cseg) {
			sameseg = false;
			vector2 delta = cpi - cpoldi;

			// now go along the line between cpoldi -> cpi through all segments
			// that the line crosses.
			// loop over u = 0...1 along the line.
			printf("sx %i sy %i csx %i csy %i   [[ %i %i ]]\n",sx,sy,csx,csy,csx-sx,csy-sy);
			int count = 20;
			while (seg != cseg) {
				if (--count <= 0) { printf("deadlock!\n"); exit(1); }
				// segment borders are segoff.x/y and segoff.x/y + segw/segw
				// find nearest border of segment (find smallest u >= 0 so that line crosses
				// segment border)
				// but only in direction of delta
				int border = -1;
				double umin = 1e30;
				// check top
				if (delta.y > eps) {
					double u = (segoff.y + segw_real - cpoldi.y)/delta.y;
					if (u < umin) {
						border = 0;
						umin = u;
					}
				}
				// check right
				if (delta.x > eps) {
					double u = (segoff.x + segw_real - cpoldi.x)/delta.x;
					if (u < umin) {
						border = 1;
						umin = u;
					}
				}
				// check bottom
				if (delta.y < -eps) {
					double u = (segoff.y             - cpoldi.y)/delta.y;
					if (u < umin) {
						border = 2;
						umin = u;
					}
				}
				// check left
				if (delta.x < -eps) {
					double u = (segoff.x             - cpoldi.x)/delta.x;
					if (u < umin) {
						border = 3;
						umin = u;
					}
				}

				sys().myassert(border >= 0, "minu internal error paranoia");
			
				vector2 borderp = cpoldi + delta * umin;
				float ct = t0 + (t1 - t0) * umin;

				printf("border %i, t0 %f t1 %f ct %f u %f  epscheck %f\n",border,t0,t1,ct,umin,ct-scl.begint);

				scl.endt = ct;
				scl.endp = borderp;
				scl.endborder = border;
				if (scl.endt - scl.begint > eps)
					coastsegments[seg].segcls.push_back(scl);
				else
					printf("trying to add too small segcl %f %f %f\n",scl.begint,scl.endt,scl.endt-scl.begint);

/*
				if (umin + eps > 1.0) {
					seg = cseg;
					sx = csx;
					sy = csy;
					scl.begint = t1;
				} else
*/
				{
					// just switch to next segment. Also happens when u < eps
					sx += segdx[border];
					sy += segdy[border];
					seg = sy*segsx+sx;
					scl.begint = ct;
				}

				segoff = vector2(sx * segw_real + realoffset.x, sy * segw_real + realoffset.y);
				scl.beginp = borderp;
				scl.beginborder = (border + 2) % 4;
			}
		}
		t0 = t1;
		cpoldi = cpi;
	}

	// store remaining segment
	scl.endt = 1;
	scl.endp = cl.curve.value(1);
	scl.endborder = endb;
	scl.cyclic = sameseg && clcyclic;
	if (scl.endt - scl.begint > eps)
		coastsegments[seg].segcls.push_back(scl);
	else
		printf("2trying to add too small segcl %f %f\n",scl.begint,scl.endt);
}



void coastmap::process_coastline(int x, int y)
{
	assert ((mapf(x, y) & 0x80) == 0);
	
	// find coastline, avoid "lakes", (inverse of islands), because the triangulation will fault there
	vector<vector2i> points;
	bool cyclic;
	int beginborder, endborder;
	bool valid = find_coastline(x, y, points, cyclic, beginborder, endborder);
	
	if (!valid) return;	// skip

	// create bspline curve
	vector<vector2> tmp;
	tmp.reserve(points.size());
	for (unsigned i = 0; i < points.size(); ++i)
		tmp.push_back(vector2(points[i].x * pixelw_real + realoffset.x, points[i].y * pixelw_real + realoffset.y));

//	cout << "points.size()="<<points.size()<<" cyclic?"<<cyclic<<"\n";
	if (cyclic) {
		// close polygon and make sure the first and last line are linear dependent
		assert(tmp.size()>2);
		vector2 p0 = tmp[0];
		vector2 p1 = tmp[1];
		vector2 p01 = (p0 + p1) * 0.5;
		tmp[0] = p01;
		tmp.push_back(p0);
		tmp.push_back(p01);
	}

	unsigned n = tmp.size() - 1;
	// fixme: a high n on small islands leads to a non-uniform spatial distribution of
	// bspline generated points. This looks ugly and is a serious drawback to the
	// old technique.
	// Maybe limiting n to tmp.size()/2 or tmp.size()/4 would be a solution. test it!
	if (n > BSPLINE_SMOOTH_FACTOR) n = BSPLINE_SMOOTH_FACTOR;
	if (n > tmp.size()/4) n = tmp.size()/4;
	coastline result(n, tmp);
	divide_and_distribute_cl(result, coastlines.size(), beginborder, endborder, tmp, cyclic);

	coastlines.push_back(result);
}



void coastmap::process_segment(int sx, int sy)
{
	coastsegment& cs = coastsegments[sy*segsx+sx];
	if (cs.segcls.size() == 0) {	// no coastlines in segment.
		// segment is fully land or sea. determine what it is
		if (mapf(sx*pixels_per_seg + pixels_per_seg/2, sy*pixels_per_seg + pixels_per_seg/2) & 0x7f) {
			cs.type = 1;
		} else {
			cs.type = 0;
		}
	} else {		// there are coastlines in segment
		cs.type = 2;
		
		vector2 segoff = vector2(sx * segw_real + realoffset.x, sy * segw_real + realoffset.y);

		// compute cl.next info
		for (unsigned i = 0; i < cs.segcls.size(); ++i) {
			cs.segcls[i].next = cs.get_successor_for_cl(i, segoff, segw_real);
		}

#if 0
		// mega-paranois debug test
		vector<bool> dbg(cs.segcls.size());
		printf("nr of segcls: %u\n", dbg.size());
		for (unsigned i = 0; i < cs.segcls.size(); ++i) {
			if (dbg[i]) continue;
			printf("loop begin %i\n",i);
			dbg[i] = true;
			if (cs.segcls[i].cyclic) continue;
			unsigned j = i;
			while (cs.segcls[j].next != i) {
				unsigned k = cs.segcls[j].next;
				printf("   %i -> %i\n",j,k);
//fixme: fehler kommen von segcls, die von einer ecke zu derselben gehen!!!!
				sys().myassert(dbg[k] == false, "paranoia10");
				j = k;
				dbg[j] = true;
				printf("...%i\n",j);
			}
		}
		unsigned i = 0;
		for ( ; i < dbg.size(); ++i)
			if (!dbg[i]) break;
		sys().myassert(i == dbg.size(), "paranoia11 at %i\n",i);
#endif
	}
}



// load from xml description file
coastmap::coastmap(const string& filename)
{
	TiXmlDocument doc(filename);
	doc.LoadFile();
	TiXmlElement* root = doc.FirstChildElement("dftd-map");
	sys().myassert(root != 0, string("coastmap: no root element found in ") + filename);
	TiXmlElement* etopology = root->FirstChildElement("topology");
	sys().myassert(etopology != 0, string("coastmap: no topology node found in ") + filename);
	const char* img = etopology->Attribute("image");
	sys().myassert(img != 0, string("coastmap: no image attribute found in ") + filename);
	realwidth = 0;
	etopology->Attribute("realwidth", &realwidth);
	sys().myassert(realwidth != 0, string("coastmap: realwidth not given or zero in ") + filename);
	etopology->Attribute("realoffsetx", &realoffset.x);
	etopology->Attribute("realoffsety", &realoffset.y);
	TiXmlElement* ecities = root->FirstChildElement("cities");
	if (ecities) {
		TiXmlElement* ecity = ecities->FirstChildElement("city");
		for ( ; ecity != 0; ecity = ecity->NextSiblingElement()) {
			// parse name, posx, posy  fixme
		}
	}

	SDL_Surface* surf = IMG_Load((get_map_dir() + img).c_str());
	add_loading_screen("map image loaded");
	sys().myassert(surf != 0, string("coastmap: error loading image ") + img + string(" referenced in file ") + filename);

	mapw = surf->w;
	maph = surf->h;
	pixelw_real = realwidth/mapw;
	realheight = maph*realwidth/mapw;
	pixels_per_seg = 1 << unsigned(ceil(log2(60000/pixelw_real)));
	segsx = mapw/pixels_per_seg;
	segsy = maph/pixels_per_seg;
	segw_real = pixelw_real * pixels_per_seg;
	sys().myassert((segsx*pixels_per_seg == mapw) && (segsy*pixels_per_seg == maph), string("coastmap: map size must be integer multiple of segment size, in") + filename);

	themap.resize(mapw*maph);

	SDL_LockSurface(surf);
	sys().myassert(surf->format->BytesPerPixel == 1 && surf->format->palette != 0 && surf->format->palette->ncolors == 2, string("coastmap: image is no black/white 1bpp paletted image, in ") + filename);

	Uint8* offset = (Uint8*)(surf->pixels);
	int mapoffy = maph*mapw;
	for (int yy = 0; yy < int(maph); yy++) {
		mapoffy -= mapw;
		for (int xx = 0; xx < int(mapw); ++xx) {
			Uint8 c = (*offset++);
			themap[mapoffy+xx] = (c > 0) ? 1 : 0;
		}
		offset += surf->pitch - mapw;
	}

	SDL_UnlockSurface(surf);
	SDL_FreeSurface(surf);

	add_loading_screen("image transformed");

	// they are filled in by process_coastline
	coastsegments.reserve(segsx*segsy);
	for (unsigned yy = 0; yy < segsy; ++yy) {
		for (unsigned xx = 0; xx < segsx; ++xx) {
			/*
			vector<float> d(pixels_per_seg*pixels_per_seg);
			for (unsigned y2 = 0; y2 < pixels_per_seg; ++y2) {
				for (unsigned x2 = 0; x2 < pixels_per_seg; ++x2) {
				// the values don't seem to match the png map?! fixme
					Uint8 m = mapf(xx*pixels_per_seg+x2, yy*pixels_per_seg+y2);
					float h = 30.0f + rnd() * 500.0f;
					float elev = (m == 0) ? -h : h;
					d[y2*pixels_per_seg+x2] = elev;
				}
			}
			coastsegments.push_back(coastsegment(pixels_per_seg/4, d));
			*/
			coastsegments.push_back(coastsegment());
		}
	}

	// find coastlines
	for (int yy = 0; yy < int(maph); ++yy) {
		for (int xx = 0; xx < int(mapw); ++xx) {
			if (mapf(xx, yy) & 0x80) continue;
			Uint8 m0 = mapf(xx+dmx[0], yy+dmy[0]) & 0x7f;
			Uint8 m1 = mapf(xx+dmx[1], yy+dmy[1]) & 0x7f;
			Uint8 m2 = mapf(xx+dmx[2], yy+dmy[2]) & 0x7f;
			Uint8 m3 = mapf(xx+dmx[3], yy+dmy[3]) & 0x7f;
			Uint8 m4 = m0 & m1 & m2 & m3;
			Uint8 m5 = m0 | m1 | m2 | m3;
			if (m4 == 0 && m5 == 1) {
				process_coastline(xx, yy);
			}
		}
	}

/*
	//check for double pts in cls
	for (unsigned a = 0; a < coastlines.size(); ++a) {
		for (unsigned b = 0; b < coastlines[a].curve.control_points().size(); ++b) {
			const vector2& pa = coastlines[a].curve.control_points()[b];
			unsigned eq = 1;
			for (unsigned c = 0; c < coastlines.size(); ++c) {
				for (unsigned d = 0; d < coastlines[c].curve.control_points().size(); ++d) {
					if (a == c && b == d) continue;
					const vector2& pb = coastlines[c].curve.control_points()[d];
					float di = pa.square_distance(pb);
					if (di < 0.1f) {
						++eq;
						cout << "double found a="<<a<<" b="<<b<<" c="<<c<<" d="<<d<<" (#"<<eq<<")\n";
					}
				}
			}
		}
	}
*/					

	// find coastsegment type
	for (unsigned yy = 0; yy < segsy; ++yy) {
		for (unsigned xx = 0; xx < segsx; ++xx) {
			process_segment(xx, yy);
		}
	}

/*
	// read cities, fixme move to coastmap
	parser cityfile(get_map_dir() + "cities.txt");
	while (!cityfile.is_empty()) {
		bool xneg = cityfile.type() == TKN_MINUS;
		cityfile.consume();
		int xd = cityfile.parse_number();
		cityfile.parse(TKN_COMMA);
		int xm = cityfile.parse_number();
		cityfile.parse(TKN_COMMA);
		bool yneg = cityfile.type() == TKN_MINUS;
		cityfile.consume();
		int yd = cityfile.parse_number();
		cityfile.parse(TKN_COMMA);
		int ym = cityfile.parse_number();
		cityfile.parse(TKN_COMMA);
		string n = cityfile.parse_string();
		double x, y;
		sea_object::degrees2meters(xneg, xd, xm, yneg, yd, ym, x, y);
		cities.push_back(make_pair(vector2(x, y), n));
	}
*/

	// fixme: clear "themap" so save space.
	// information wether a position on the map is land or sea can be computed from
	// segment data. This will save 6MB of space at least.

	add_loading_screen("coastmap created");
}



void coastmap::draw_as_map(const vector2& droff, double mapzoom, int detail) const
{
	int x, y, w, h;
//cout << "mapzoom pix/m = " << mapzoom << " segwreal " << segw_real << "\n";
	w = int(ceil((1024/mapzoom)/segw_real)) +2;
	h = int(ceil((768/mapzoom)/segw_real)) +2;	// fixme: use 640 and translate map y - 64
//cout << " w " << w << " h " << h << " waren's mal.\n";
	x = int(floor((droff.x - realoffset.x)/segw_real)) - w/2;
	y = int(floor((droff.y - realoffset.y)/segw_real)) - h/2;
	if (x < 0) {
		w += x;
		x = 0;
	}
	if (y < 0) {
		h += y;
		y = 0;
	}
	if (x + w > int(segsx)) {
		w = int(segsx) - x;
	}
	if (y + h > int(segsy)) {
		h = int(segsy) - y;
	}
//cout<<"draw map   segsx " << segsx << " segsy " << segsy << " x " << x << " y " << y << " w " << w << " h " << h << "\n";

#if 1
	// testing
	for (unsigned i = 0; i < coastlines.size(); ++i)
		coastlines[i].draw_as_map();
#endif

	atlanticmap->set_gl_texture();
	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glLoadIdentity();
	glTranslated(0.0, 1.0, 0.0);
	glScaled(1.0/realwidth, -1.0/realheight, 1.0);
	glTranslated(-realoffset.x, -realoffset.y, 0.0);
	glMatrixMode(GL_MODELVIEW);
	for (int yy = y; yy < y + h; ++yy) {
		for (int xx = x; xx < x + w; ++xx) {
			vector2 offset(realoffset.x + segw_real * xx, realoffset.y + segw_real * yy);
		//cout<<"drawing segment " << xx << "," << yy << "\n";			
			coastsegments[yy*segsx+xx].draw_as_map(*this, xx, yy, offset, detail);
		}
	}

	glMatrixMode(GL_TEXTURE);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);

/*
	// draw cities, fixme move to coastmap
	for (list<pair<vector2, string> >::const_iterator it = cities.begin(); it != cities.end(); ++it) {
		glBindTexture(GL_TEXTURE_2D, 0);
		draw_square_mark(gm, it->first, -offset, color(255, 0, 0));
		vector2 pos = (it->first - offset) * mapzoom;
		font_arial->print(int(512 + pos.x), int(384 - pos.y), it->second);
	}
*/
}



void coastmap::render(const vector2& p, int detail, bool withterraintop) const
{
	vector2 pnew = p - realoffset;

	// determine which coast segment can be seen (at most 4)
	// fixme
	double rsegw = pixelw_real * pixels_per_seg;
	int moffx = int(pnew.x/rsegw);
	int moffy = int(pnew.y/rsegw);

	glPushMatrix();
	glTranslated(-p.x, -p.y, 0.0);

	if (moffx >= 0 && moffy >= 0 && moffx < int(segsx) && moffy < int(segsy))	
		coastsegments[moffy*segsx+moffx].render(*this, moffx, moffy, pnew - vector2(moffx*rsegw, moffy*rsegw), detail +2);

	glPopMatrix();
}
