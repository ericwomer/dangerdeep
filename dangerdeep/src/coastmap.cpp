// a coastmap
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include <GL/gl.h>

#include "coastmap.h"
#include "binstream.h"
#include "global_data.h"
#include "texture.h"
#include "bspline.h"
#include "vector2.h"
#include "system.h"
#include "triangulate.h"
#include "tinyxml/tinyxml.h"
#include <SDL_image.h>
#include <cassert>
#include <fstream>
#include <list>
#include <vector>
using namespace std;



const unsigned BSPLINE_SMOOTH_FACTOR = 16;	// should be 3...16

const int coastmap::dmx[4] = { -1, 0, 0, -1 };
const int coastmap::dmy[4] = { -1, -1, 0, 0 };
const int coastmap::dx[4] = { 0, 1, 0, -1 };
const int coastmap::dy[4] = { -1, 0, 1, 0 };



/* fixme:
  maybe reuse already generated data: give old point array to. if new detail is higher than
  old detail, just generate new points (50% computation time saving on increasing the detail),
  or drop old points with decreasing detail (100% computation time saving).
*/
vector<vector2f> coastline::create_points(const vector2& offset, float scal, unsigned begint, unsigned endt, int detail) const
{
	double t0 = double(begint)/double(points.size());
	double t1 = double(endt)/double(points.size());
	unsigned nrpts = endt - begint + 1;
	if (detail < 0) nrpts = (nrpts >> -detail); else nrpts = (nrpts << detail);
	if (nrpts < 2) nrpts = 2;
	double tstep = (t1 - t0) / double(nrpts - 1);
	vector<vector2f> result;
	result.reserve(nrpts);
	double t = t0;
	vector<vector2f> pointsf;
	pointsf.reserve(nrpts);
	for (unsigned i = 0; i < nrpts; ++i) {
		pointsf.push_back(vector2f(
			points[begint+i].x * scal + offset.x,
			points[begint+i].y * scal + offset.y ));
	}

	unsigned n = points.size() - 1;
	if (n > BSPLINE_SMOOTH_FACTOR) n = BSPLINE_SMOOTH_FACTOR;
	bsplinet<vector2f> spl(n, pointsf);	
	for (unsigned n = 0; n < nrpts; ++n) {
		result.push_back(spl.value(t));
		t += tstep;
	}
	return result;
}



void coastline::draw_as_map(const vector2f& off, float size, const vector2f& t, const vector2f& ts, int detail) const
{
	// fixme: cache that somehow
	vector<vector2f> pts = create_points(vector2(off.x, off.y), size, 0, points.size(), detail);

	// fixme: move calculations to opengl's transform matrix.	
	glBegin(GL_LINE_STRIP);
	for (vector<vector2f>::const_iterator it = pts.begin(); it != pts.end(); ++it) {
		glTexCoord2f(t.x + ts.x * it->x / size, 1.0f - (t.y + ts.y * it->y / size));
		glVertex2f(off.x + it->x, off.y + it->y);
	}
	glEnd();

#if 0
	glPointSize(2.0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBegin(GL_POINTS);
	for (vector<vector2f>::const_iterator it = pts.begin(); it != pts.end(); ++it) {
		glVertex2f(off.x + it->x, off.y + it->y);
	}
	glEnd();
#endif	
}



void coastline::render(const vector2& p, int detail) const
{
	// fixme: cache that somehow
	vector<vector2f> pts = create_points(p, 1, 0, points.size(), detail);
	
	glPushMatrix();
	glTranslatef(-p.x, -p.y, 0);
	
	unsigned ps = pts.size();
	glBegin(GL_QUAD_STRIP);
	double coastheight = 50;
	float t = 0.0;
	for (unsigned s = 0; s < ps; ++s) {
		const vector2f& p = pts[s];
		unsigned prevpt = (s > 0) ? s-1 : s;
		unsigned nextpt = (s < ps-1) ? s+1 : s;
		vector2f n = (pts[nextpt] - pts[prevpt]).orthogonal().normal() * 50.0;
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






#if 0
// the least distance between two points is around 0.014
void coastsegment::cacheentry::push_back_point(const vector2f& p)
{
//	debug code that avoids double points, not needed any longer
//	if (points.size() == 0 || (points.back().square_distance(p) >= 1.0f))
		points.push_back(p);
}

float gmd=1e30;
void coastsegment::generate_point_cache(double size, unsigned detail)
{
	if (type > 1) {
		// cache generated and unchanged?
		if (pointcache.size() > 0 && pointcachedetail == detail) return;

		// invalidate cache (detail changed or initial generation)
		pointcachedetail = detail;
		pointcache.clear();

		unsigned nrcl = coastlines.size();
		vector<bool> cl_handled(nrcl, false);
		for (unsigned i = 0; i < nrcl; ++i) {
			if (cl_handled[i]) continue;

			cacheentry ce;

			// find area
			unsigned current = i;
			do {
				/* NOTE:
				   double shouldn't occour by design. But they do. So we do
				   an quick, simple and dirty workaround here and just avoid
				   inserting them. Finally we check if the last point is the
				   same as the first, in that case we remove it, too.
				   Reason for double points:
				   this seem to happen because sometimes endborder/beginborder
				   is >= 0 even if the corresponding points are not on the border.
				   so the coast lines are treated as coast, not as part of an
				   island -> end of first cl is equal to begin of next ->
				   double points result.
				*/
				const coastline& cl = coastlines[current];
				ce.push_back_point(cl.p0);
				cl.create_points(ce.points, detail);
				if (cl.endborder >= 0)	// else: part of an island, in that case avoid double points
					ce.push_back_point(cl.p1);
				unsigned next = cl.next;
				cl_handled[current] = true;
				// insert corners if needed
				int ed = cl.endborder, bg = coastlines[next].beginborder;
				if (ed >= 0 && bg >= 0) {
					if (ed == bg) {
						if (coastline::dist_to_corner(ed, cl.p1, size) < coastline::dist_to_corner(bg, coastlines[next].p0, size))
							bg += 4;
					} else if (bg < ed) {
							bg += 4;
					}
					for (int j = ed; j < bg; ++j) {
						int k = j % 4;
						if (k == 0) ce.push_back_point(vector2f(size, 0));
						else if (k == 1) ce.push_back_point(vector2f(size, size));
						else if (k == 2) ce.push_back_point(vector2f(0, size));
						else ce.push_back_point(vector2f(0, 0));
					}
				}
				current = next;
			} while (current != i);
			
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


			ce.indices = triangulate::compute(ce.points);
			pointcache.push_back(ce);
		}
	}
}
#endif



void coastsegment::draw_as_map(const vector2f& off, float size, const vector2f& t, const vector2f& ts, int detail) const
{
	if (type == 1) {
		glBegin(GL_QUADS);
		glTexCoord2f(t.x, 1.0f-t.y);
		glVertex2d(off.x, off.y);
		glTexCoord2f(t.x+ts.x, 1.0f-t.y);
		glVertex2d(off.x+size, off.y);
		glTexCoord2f(t.x+ts.x, 1.0f-(t.y+ts.y));
		glVertex2d(off.x+size, off.y+size);
		glTexCoord2f(t.x, 1.0f-(t.y+ts.y));
		glVertex2d(off.x, off.y+size);
		glEnd();
	} else if (type > 1) {
#if 0
		generate_point_cache(size, detail);
	
		glBegin(GL_TRIANGLES);
		for (vector<cacheentry>::const_iterator cit = pointcache.begin(); cit != pointcache.end(); ++cit) {
			for (vector<unsigned>::const_iterator tit = cit->indices.begin(); tit != cit->indices.end(); ++tit) {
				const vector2f& v = cit->points[*tit];
				glTexCoord2f(tx + tsx * v.x / size, 1.0f - (ty + tsy * v.y / size));
				glVertex2f(xoff + v.x, yoff + v.y);
			}
		}
		glEnd();
#endif
#if 0
		// test
		for (unsigned i = 0; i < coastlines.size(); ++i) {
			coastlines[i].draw_as_map(off, size, t, ts, detail);
		}
#endif		
	}
}



void coastsegment::render(const vector2& p, int detail) const
{
/*
	if (type > 1) {
		for (vector<coastline>::const_iterator it = coastlines.begin(); it != coastlines.end(); ++it) {
			it->render(p, detail);
		}
	}
*/
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
That way we have seamless coastlines. TODO

The map takes little memory on disk, no map compiler needed (or at least not that
time consuming one), much advantages

coastmap should move from user_interface to game because it's not only stored
for displaying purposes but should influence the game!
*/






unsigned coastmap::find_seg_for_point(const vector2i& p) const
{
	return (p.x/pixels_per_seg)+(p.y/pixels_per_seg)*segsx;
}



Uint8& coastmap::mapf(int cx, int cy)
{
	cx = clamp_zero(cx);
	cy = clamp_zero(cy);
	cx = mapw - 1 - clamp_zero(mapw - 1 - cx);
	cy = maph - 1 - clamp_zero(maph - 1 - cy);
	return themap[(maph-1-cy)*mapw+cx];
}



bool coastmap::find_begin_of_coastline(int& x, int& y)
{
	bool cyclic = false;
	int sx = x, sy = y, j2 = 0;
	while (true) {
		// compute next x,y
		int j = 0;
		char mv[4];
		for (j = 0; j < 4; ++j) {
			mv[j] = mapf(x+dmx[j], y+dmy[j]) & 0x7f;
		}
		if (mv[0] == 0 && mv[2] == 0 && mv[1] > 0 && mv[3] > 0) {
			j = (j2 + 1) % 4;
		} else if (mv[0] > 0 && mv[2] > 0 && mv[1] == 0 && mv[3] == 0) {
			j = (j2 + 1) % 4;
		} else {
			for (j = 0; j < 4; ++j) {
				if (mv[j] > 0 && mv[(j+1)%4] == 0) break;
			}
			assert(j < 4);
		}
		j2 = j;
		int nx = x + dx[j];
		int ny = y + dy[j];
		if (nx < 0 || ny < 0 || nx >= mapw || ny >= maph)	// border reached
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



bool coastmap::find_coastline(int x, int y, coastline& cl)	// returns true if cl is valid
{
	// run backward at the coastline until we reach the border or round an island.
	// start there creating the coastline. this avoids coastlines that can never be seen.
	// In reality: north pole, ice, America to the west, Asia/africa to the east.

	assert((mapf(x, y) & 0x80) == 0);
	
	cl.cyclic = find_begin_of_coastline(x, y);

	cl.beginborder = -1;	
	if (!cl.cyclic) {
		if (x == 0) cl.beginborder = 3;
		else if (y == 0) cl.beginborder = 0;
		else if (x == mapw-1) cl.beginborder = 1;
		else if (y == maph-1) cl.beginborder = 2;
	}
	
	int sx = x, sy = y, j2 = 0, lastj = -1, turncount = 0;
	while (true) {
		// store x,y
		cl.points.push_back(vector2i(x, y));

		assert(x>=0&&y>=0&&x<mapw&&y<maph);
		mapf(x, y) |= 0x80;

		// compute next x,y	
		int j = 0;
		char mv[4];
		for (j = 0; j < 4; ++j) {
			mv[j] = mapf(x+dmx[j], y+dmy[j]) & 0x7f;
		}
		if (mv[0] == 0 && mv[2] == 0 && mv[1] > 0 && mv[3] > 0) {
			j = (j2 + 3) % 4;
		} else if (mv[0] > 0 && mv[2] > 0 && mv[1] == 0 && mv[3] == 0) {
			j = (j2 + 3) % 4;
		} else {
			for (j = 0; j < 4; ++j) {
				if (mv[j] == 0 && mv[(j+1)%4] > 0) break;
			}
			assert(j < 4);
		}
		j2 = j;

		if (lastj != -1 && cl.cyclic) {
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
		if (nx < 0 || ny < 0 || nx >= mapw || ny >= maph)	// border reached
			break;
		x = nx;
		y = ny;
		if (sx == x && sy == y) {
			break;	// island found
		}
	}
	
	cl.endborder = -1;
	if (!cl.cyclic) {
		if (x == 0) cl.endborder = 3;
		else if (y == 0) cl.endborder = 0;
		else if (x == mapw-1) cl.endborder = 1;
		else if (y == maph-1) cl.endborder = 2;
	}

	return turncount <= 0;
}



/* for islands, add:
		assert(tmp.size()>2);
		vector2 p0 = tmp[0];
		vector2 p1 = tmp[1];
		vector2 p01 = p0 * 0.5 + p1 * 0.5;
		tmp[0] = p01;
		tmp.push_back(p0);
		tmp.push_back(p01);
*/



/*
float compute_border_dist(int b0, const vector2f& p0, int b1, const vector2f& p1)
{
	float dist0 = coastline::dist_to_corner(b0, p0, segw), dist1 = coastline::dist_to_corner(b1, p1, segw);
	if (b0 == b1 && dist1 > dist0) b1 += 4;
	else if (b1 < b0) b1 += 4;
	float fulldist = dist0;
	for (int i = b0; i < b1; ++i)
		fulldist += segw;
	fulldist -= dist1;
	return fulldist;
}
*/


#if 0
unsigned coastsegment::get_successor_for_cl(unsigned cln) const
{
	float mindist = 1e30;
	unsigned next = 0xffffffff;
//coastlines[cln].debug_print(cout);
//cout << " succ?\n";
	for (unsigned i = 0; i < coastlines.size(); ++i) {
//coastlines[i].debug_print(cout);
		// i's successor can be i itself and may already have been handled (i itself!)
		const coastline& cl0 = coastlines[cln];
		const coastline& cl1 = coastlines[i];
		// compare for distance, 5 cases
		if (cl0.endborder < 0 && cl1.beginborder < 0) {	// part of an island
			float dist = cl0.p1.distance(cl1.p0);
			if (dist < mindist) {
				mindist = dist;
				next = i;
			}
		} else if (cl0.endborder >= 0 && cl1.beginborder >= 0) {
			float dist = compute_border_dist(cl0.endborder, cl0.p1, cl1.beginborder, cl1.p0);
			if (dist < mindist) {
				mindist = dist;
				next = i;
			}
		} // else not compareable
	}
	assert(next != 0xffffffff);
//cout << "next was " << next << "\n";	
	return next;
}
#endif



#if 0
void divide_and_distribute_cl(const intcl& icl)
//const doublecl& dcl, vector<vector<doublecl> >& cl_per_seg)
{
	doublecl segcl;

	// divide coastline at segment borders
	// find segment that first point is into
	segcl.beginborder = dcl.beginborder;
	int sx = int(dcl.points.front().x / pixelw) / n;
	int sy = int(dcl.points.front().y / pixelw) / n;
	int segsx = w/n;
	int seg = sy*segsx+sx;
	vector2 segoff(sx * segw, sy * segw);

	segcl.points.push_back(dcl.points.front() - segoff);
	bool sameseg = true;
	// fixme: avoid just one line of an island/coast to get distributed to another segment.
	// thus treat borders as part of current segment, for both sides!
	for (unsigned i = 1; i < dcl.points.size(); ++i) {
		int csx = int(dcl.points[i].x / pixelw) / n;
		int csy = int(dcl.points[i].y / pixelw) / n;
		int cseg = csy*segsx+csx;
		if (seg == cseg) {
			segcl.points.push_back(dcl.points[i] - segoff);
		} else {
			sameseg = false;
			vector2 b = dcl.points[i-1];
			vector2 d = dcl.points[i] - b;

			// fixme: what is if the next segment is right AND down (line crosses corner)?
			
			// find t so that b + t * d is on segment border between seg and cseg
			double t;
			int border = -1;
			if (cseg < seg) {	// border is left or top
				if (cseg == seg-1) {	// left
					t = (segoff.x - b.x) / d.x;
					border = 3;
				} else {		// top
					t = (segoff.y - b.y) / d.y;
					border = 0;
				}
			} else {		// border is right or bottom
				if (cseg == seg+1) {	// right
					t = (segoff.x + segw - b.x) / d.x;
					border = 1;
				} else {		// bottom
					t = (segoff.y + segw - b.y) / d.y;
					border = 2;
				}
			}
			
			vector2 borderp = b + d * t;	// das hier kommt weg. punkte liegen sowieso schon auf integer coordinaten!
			
			segcl.endborder = border;

			// avoid storing too short line segments
			if (t > 0.0001)
				segcl.points.push_back(borderp - segoff);
			cl_per_seg[seg].push_back(segcl);
			sx = csx;
			sy = csy;
			seg = cseg;
			segoff = vector2(sx * segw, sy * segw);
			segcl.points.clear();
			segcl.beginborder = (border + 2) % 4;
			
			// avoid storing too short line segments
			if (t < 0.9999)
				segcl.points.push_back(borderp - segoff);
			segcl.points.push_back(dcl.points[i] - segoff);
		}
	}

	segcl.endborder = dcl.endborder;

	segcl.cyclic = sameseg;	// we have an island complete in one segment
	
//	if (segcl.cyclic) assert(segcl.beginborder==-1 && segcl.endborder==-1);

	cl_per_seg[seg].push_back(segcl);
}
#endif


void coastmap::process_coastline(int x, int y)
{
	assert ((mapf(x, y) & 0x80) == 0);
	coastline cl;
	
	// find coastline, avoid "lakes", (inverse of islands), because the triangulation will fault there
	bool valid = find_coastline(x, y, cl);
	
	if (!valid) return;	// skip

	assert(cl.points.size() > 0);

	// fill in coastseg info, which segments are covered by cl?
	unsigned curseg = find_seg_for_point(cl.points[0]);
	unsigned begint = 0, endt = 0;
	for (unsigned i = 1; i < cl.points.size(); ++i) {
		endt = i;
		unsigned curseg2 = find_seg_for_point(cl.points[i]);
		//fixme: fill in startborder/endborder info
		if (curseg2 != curseg) {
			coastsegments[curseg].segcls.push_back(coastsegment::segcl(coastlines.size()-1, begint, endt));
			curseg = curseg2;
			begint = endt;
		}
	}
	coastsegments[curseg].segcls.push_back(coastsegment::segcl(coastlines.size()-1, begint, endt));
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
/*
		// compute cl.next info
		unsigned ncl = cs.segcls.size();
		vector<bool> handledcl(ncl, false);
		for (unsigned i = 0; i < ncl; ++i) {
			if (handledcl[i]) continue;
			unsigned current = i;
			do {
				unsigned next = cs.get_successor_for_cl(current);
				cs.segcls[current].next = next;
				handledcl[current] = true;
			} while (current != i);
		}
*/
	}
}



// load from xml description file
coastmap::coastmap(const string& filename)
{
	TiXmlDocument doc(filename);
	doc.LoadFile();
	TiXmlElement* root = doc.FirstChildElement("dftd-map");
	system::sys().myassert(root != 0, string("coastmap: no root element found in ") + filename);
	TiXmlElement* etopology = root->FirstChildElement("topology");
	system::sys().myassert(etopology != 0, string("coastmap: no topology node found in ") + filename);
	const char* img = etopology->Attribute("image");
	system::sys().myassert(img != 0, string("coastmap: no image attribute found in ") + filename);
	realwidth = 0;
	etopology->Attribute("realwidth", &realwidth);
	system::sys().myassert(realwidth != 0, string("coastmap: realwidth not given or zero in ") + filename);
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
	system::sys().myassert(surf != 0, string("coastmap: error loading image ") + img + string(" referenced in file ") + filename);

	mapw = surf->w;
	maph = surf->h;
	pixelw_real = realwidth/mapw;
	pixels_per_seg = unsigned(ceil(log2(60000/pixelw_real)));
	segw = pixelw_real * pixels_per_seg;
	segsx = mapw/pixels_per_seg;
	segsy = maph/pixels_per_seg;
	system::sys().myassert((segsx*pixels_per_seg == mapw) && (segsy*pixels_per_seg == maph), string("coastmap: map size must be integer multiple of segment size, in") + filename);

	themap.resize(mapw*maph);

	SDL_LockSurface(surf);
	system::sys().myassert(surf->format->BytesPerPixel == 1 && surf->format->palette != 0 && surf->format->palette->ncolors == 2, string("coastmap: image is no black/white 1bpp paletted image, in ") + filename);

	Uint8* offset = (Uint8*)(surf->pixels);
	for (int yy = 0; yy < maph; yy++) {
		for (int xx = 0; xx < mapw; ++xx) {
			Uint8 c = (*offset++);
			themap[yy*mapw+xx] = c ? 1 : 0;
		}
		offset += surf->pitch - mapw;
	}

	SDL_UnlockSurface(surf);
	SDL_FreeSurface(surf);

	// they are filled in by process_coastline
	coastsegments.resize(segsx*segsy);

	// find coastlines
	for (int yy = 0; yy < maph; ++yy) {
		for (int xx = 0; xx < mapw; ++xx) {
			if (mapf(xx, yy) & 0x80) continue;
			char m0 = mapf(xx+dmx[0], yy+dmy[0]) & 0x7f;
			char m1 = mapf(xx+dmx[1], yy+dmy[1]) & 0x7f;
			char m2 = mapf(xx+dmx[2], yy+dmy[2]) & 0x7f;
			char m3 = mapf(xx+dmx[3], yy+dmy[3]) & 0x7f;
			char m4 = m0 & m1 & m2 & m3;
			char m5 = m0 | m1 | m2 | m3;
			if (m4 == 0 && m5 == 1) {
				process_coastline(xx, yy);
			}
		}
	}

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
}



void coastmap::draw_as_map(const vector2& droff, double mapzoom, int detail) const
{
	int x, y, w, h;
	double mperseg = pixels_per_seg * pixelw_real;
	w = int(ceil((1024/mapzoom)/mperseg)) +2;
	h = int(ceil((768/mapzoom)/mperseg)) +2;	// fixme: use 640 and translate map y - 64
	x = int(floor((droff.x - realoffset.x)/mperseg)) - w/2;
	y = int(floor((droff.y - realoffset.y)/mperseg)) - h/2;
	double rsegw = pixelw_real * pixels_per_seg;
	vector2f ts(1.0/segsx, 1.0/segsy);
	atlanticmap->set_gl_texture();
	if (x < 0) {
		w += x;
		x = 0;
	}
	if (y < 0) {
		h += y;
		y = 0;
	}
	if (x + w > segsx) {
		w = segsx - x;
	}
	if (y + h > segsy) {
		h = segsy - y;
	}

	vector2f r(0, realoffset.y + rsegw * y);
	vector2f t(0, ts.y * y);
	for (int yy = y; yy < y + h; ++yy) {
		r.x = realoffset.x + rsegw * x;
		t.x = ts.x * x;
		for (int xx = x; xx < x + w; ++xx) {
			coastsegments[yy*segsx+xx].draw_as_map(r, rsegw, t, ts, detail);
			r.x += rsegw;
			t.x += ts.x;
		}
		r.y += rsegw;
		t.y += ts.y;
	}

//	for (unsigned i = 0; i < coastlines.size(); ++i)
//		coastlines[i].draw_as_map(vector2f(realoffset.x, realoffset.y), float(pixelw_real), vector2f(1,1), vector2f(1,1));

/*
	// draw cities, fixme move to coastmap
	for (list<pair<vector2, string> >::const_iterator it = cities.begin(); it != cities.end(); ++it) {
		sys.no_tex();
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

	if (moffx >= 0 && moffy >= 0 && moffx < int(segsx) && moffy < int(segsy))	
		coastsegments[moffy*segsx+moffx].render(pnew - vector2(moffx*rsegw, moffy*rsegw), detail);
}
