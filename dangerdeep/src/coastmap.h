// a coastmap
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef COASTMAP_H
#define COASTMAP_H

#include <string>
using namespace std;

#include "coastsegment.h"


struct coastmap {
	unsigned pixels_per_seg;	// "n"
	unsigned segsx, segsy;		// nr of segs in x/y dim.
	double pixelw_real;		// width/height of one pixel in reality, in meters
	double offsetx, offsety;	// offset in meters for map (position of pixel pos 0,0)
	vector<coastsegment> coastsegments;
	
	coastmap() : pixels_per_seg(0), segsx(0), segsy(0), pixelw_real(0), offsetx(0), offsety(0) {}
	~coastmap() {}
	coastmap(const coastmap& o) : pixels_per_seg(o.pixels_per_seg), segsx(o.segsx), segsy(o.segsy),
		pixelw_real(o.pixelw_real), offsetx(o.offsetx), offsety(o.offsety) {}
	coastmap& operator=(const coastmap& o) { pixels_per_seg = o.pixels_per_seg; segsx = o.segsx; segsy = o.segsy;
		pixelw_real = o.pixelw_real; offsetx = o.offsetx; offsety = o.offsety; return *this; }
	void save(const string& filename) const;
	coastmap(const string& filename);
	
	void draw_as_map(const vector2& droff, double mapzoom, unsigned detail = 0xffffffff);
	void render(double px, double py, unsigned detail = 0xffffffff, bool withterraintop = false) const;
};

#endif
