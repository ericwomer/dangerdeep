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
#ifndef MAPCOMPILER
#include "texture.h"
#endif
#include <cassert>
#include <fstream>


/*
2004/05/17
idea.
Store map as b/w png image.
Create coastlines from pixel border at runtime.
Create coast line with bsplines on the fly from border.
#Vertices = 2^detaillevel.
Compute vertices with binary subdivision: at t=0,1
then at t=0.5, later at t=0.25 and 0.75, t=0.125,0.375,0.625,0.875 etc.
would be fast enough for display at runtime.
Locally increased detail is possible, e.g. subdivide that part of a coastline
that is inside the segment to a certain amount (5-10km/line) and subdivide
it more near the player (measure distance to line segment)

For each segment store which coastlines are inside with start t and end t
(t along the whole coastline).
When drawing multiple sectors unite the coastline lists of several sectors
(e.g. cl a is in segm 1 with t=[0.2...0.4] and cl a is also in segm2 with
t=[0.4...0.6] -> draw cl a with t=[0.2...0.6])
That way we have seamless coastlines.

The map takes little memory on disk, no map compiler needed (or at least not that
time consuming one), much advantages

coastmap should move from user_interface to game because it's not only stored
for displaying purposes but should influence the game!
*/



void coastmap::save(const string& filename) const
{
	ofstream out((get_map_dir() + filename).c_str(), ios::binary|ios::out);
	assert(out.good() && "can't write map file");
	write_u16(out, pixels_per_seg);
	write_u16(out, segsx);
	write_u16(out, segsy);
	write_double(out, pixelw_real);
	write_double(out, offsetx);
	write_double(out, offsety);
	for (vector<coastsegment>::const_iterator it = coastsegments.begin(); it != coastsegments.end(); ++it) {
		it->save(out);
	}
}

coastmap::coastmap(const string& filename)
{
	ifstream in((get_map_dir() + filename).c_str(), ios::in | ios::binary);
	assert(in.good() && "can't read map file");
	pixels_per_seg = read_u16(in);
	segsx = read_u16(in);
	segsy = read_u16(in);
	pixelw_real = read_double(in);
	offsetx = read_double(in);
	offsety = read_double(in);
	unsigned s = segsx * segsy;
	coastsegments.reserve(s);
	for ( ; s > 0; --s) {
		coastsegments.push_back(coastsegment(in));
	}
}

void coastmap::draw_as_map(const vector2& droff, double mapzoom, unsigned detail)
{
	int x, y, w, h;
	double mperseg = pixels_per_seg * pixelw_real;
	w = int(ceil((1024/mapzoom)/mperseg)) +2;
	h = int(ceil((768/mapzoom)/mperseg)) +2;	// fixme: use 640 and translate map y - 64
	x = int(floor((droff.x - offsetx)/mperseg)) - w/2;
	y = int(floor((droff.y - offsety)/mperseg)) - h/2;
	glBindTexture(GL_TEXTURE_2D, 0);
	double rsegw = pixelw_real * pixels_per_seg;
	double tsx = 1.0/segsx;
	double tsy = 1.0/segsy;
#ifndef MAPCOMPILER
	atlanticmap->set_gl_texture();
#endif	
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
	double ry = offsety + rsegw * y;
	double ty = tsy * y;
	for (int yy = y; yy < y + h; ++yy) {
		double rx = offsetx + rsegw * x;
		double tx = tsx * x;
		for (int xx = x; xx < x + w; ++xx) {
			coastsegments[yy*segsx+xx].draw_as_map(rx, ry, rsegw, tx, ty, tsx, tsy, detail);
			rx += rsegw;
			tx += tsx;
		}
		ry += rsegw;
		ty += tsy;
	}
}

void coastmap::render(double px, double py, unsigned detail, bool withterraintop) const
{
	px -= offsetx;
	py -= offsety;

	// determine which coast segment can be seen (at most 4)
	// fixme
	double rsegw = pixelw_real * pixels_per_seg;
	int moffx = int(px/rsegw);
	int moffy = int(py/rsegw);

	if (moffx >= 0 && moffy >= 0 && moffx < int(segsx) && moffy < int(segsy))	
		coastsegments[moffy*segsx+moffx].render(px - moffx*rsegw, py - moffy*rsegw, detail);
}
