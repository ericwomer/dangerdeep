// a coastmap
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef COASTMAP_H
#define COASTMAP_H

#include "vector2.h"
#include "vector3.h"
#include "bspline.h"
#include <vector>
#include <string>
#include <list>
#include <SDL.h>
using namespace std;



struct coastline
{
	bsplinet<vector2f, float> curve;	// points in map coordinates (meters)

	bool cyclic;		// is cyclic, that means an island?
	
	coastline(int n, const vector<vector2f>& p, bool cyclic_) : curve(n, p), cyclic(cyclic_) {}
	~coastline() {}
	coastline(const coastline& o) : curve(o.curve), cyclic(o.cyclic) {}
	coastline& operator= (const coastline& o) { curve = o.curve; cyclic = o.cyclic; return *this; }

	// create vector of real points, detail can be > 0 (additional detail with bspline
	// interpolation) or even < 0 (reduced detail)
	// create points between begint and endt with 0<=t<points.size()
	vector<vector2f> create_points(unsigned begint, unsigned endt, int detail = 0) const;
	//fixme: obsolete.
	void draw_as_map(int detail = 0) const;
	// possibly obsolete.fixme
	void render(const vector2& p, int detail = 0) const;
};




struct coastsegment
{
	struct segcl
	{
		unsigned mapclnr;
		unsigned begint, endt;
		vector2f beginp, endp;
		int beginborder;// 0-3, top,right,bottom,left of segment (clockwise), -1 = (part of an) island
		int endborder;	// dito
		bool cyclic;
		//int next;
		segcl(unsigned n, unsigned s, unsigned e) : mapclnr(n), begint(s), endt(e) {}
	};

	unsigned type;	// 0 - sea, 1 - land, 2 mixed
	vector<segcl> segcls;

	// cache generated points.
	struct cacheentry {
		vector<vector2f> points;
		vector<unsigned> indices;
		cacheentry() {}
		~cacheentry() {}
		cacheentry(const cacheentry& c) : points(c.points), indices(c.indices) {}
		cacheentry& operator=(const cacheentry& c) { points = c.points; indices = c.indices; return *this; }
		void push_back_point(const vector2f& p);	// avoids double points.
	};
	unsigned pointcachedetail;
	vector<cacheentry> pointcache;
	// check if cache needs to be (re)generated, and do that
	void generate_point_cache(double size/*what is size?fixme*/, unsigned detail);

	static float dist_to_corner(int b, const vector2f& p, float segw);
	float compute_border_dist(int b0, const vector2f& p0, int b1, const vector2f& p1);

	unsigned get_successor_for_cl(unsigned cln) const;

	coastsegment() : type(0) {}	
	~coastsegment() {}
	coastsegment(const coastsegment& o) : type(o.type), segcls(o.segcls), pointcachedetail(o.pointcachedetail), pointcache(o.pointcache) {}
	coastsegment& operator= (const coastsegment& o) { type = o.type; segcls = o.segcls; pointcachedetail = o.pointcachedetail; pointcache = o.pointcache; return *this; }
	
	void draw_as_map(int x, int y, int detail = 0) const;
	void render(const vector2& p, int detail = 0) const;
};




class coastmap
{
	// some attributes used for map reading/processing
	vector<Uint8> themap;		// pixel data of map file
	double segw;			// real width of a segment
	static const int dmx[4];	// some helper constants.
	static const int dmy[4];
	static const int dx[4];
	static const int dy[4];

	unsigned pixels_per_seg;	// "n"
	unsigned mapw, maph;		// map width/height in pixels.
	unsigned segsx, segsy;		// nr of segs in x/y dimensions
	double realwidth;		// width of map in reality (meters)
	double pixelw_real;		// width/height of one pixel in reality, in meters
	vector2 realoffset;		// offset in meters for map (position of pixel pos 0,0)
	vector<coastsegment> coastsegments;
	vector<coastline> coastlines;

	list<pair<vector2f, string> > cities;	// city positions (real) and names
	
	coastmap();
	coastmap(const coastmap& );
	coastmap& operator= (const coastmap& );

	// very fast integer clamping (no branch needed, only for 32bit signed integers!)
	Sint32 clamp_zero(Sint32 x) { return x & ~(x >> 31); }
	unsigned find_seg_for_point(const vector2i& p) const;
	Uint8& mapf(int cx, int cy);
	bool find_begin_of_coastline(int& x, int& y);
	bool find_coastline(int x, int y, vector<vector2i>& points, bool& cyclic, int& beginborder, int& endborder);
	void process_coastline(int x, int y);
	void process_segment(int x, int y);

public:	
	// create from xml file
	coastmap(const string& filename);
	~coastmap() {}

	// fixme: handle zoom and offset with OpenGL transformation matrix.
	// fixme: maybe it's better to give top,left and bottom,right corner of sub area to draw
	void draw_as_map(const vector2& droff, double mapzoom, int detail = 0) const;
	// here give point and viewrange for rendering?
	void render(const vector2& p, int detail = 0, bool withterraintop = false) const;
};

#endif
