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

	coastline(int n, const vector<vector2f>& p, int bb, int eb) : curve(n, p) {}
	~coastline() {}
	coastline(const coastline& o) : curve(o.curve) {}
	coastline& operator= (const coastline& o) { curve = o.curve; return *this; }

	// create vector of real points, detail can be > 0 (additional detail with bspline
	// interpolation) or even < 0 (reduced detail)
	// create points between begint and endt with 0<=t<=1
	// new points are appended on vector "points"
	void create_points(vector<vector2f>& points, float begint, float endt, int detail = 0) const;
	
	// just for testing purposes.
	void draw_as_map(int detail = 0) const;
};



struct coastsegment
{
	struct segcl
	{
		unsigned mapclnr;	// pointer to coastmap::coastlines, global cl number
		float begint, endt;
		vector2f beginp, endp;	// = coastlines[mapclnr].curve.value(begint) but storing is faster!
		int beginborder;// 0-3, top,right,bottom,left of segment, -1 = (part of an) island
		int endborder;	// dito
		bool cyclic;
		int next;
	};

	unsigned type;	// 0 - sea, 1 - land, 2 mixed
	vector<segcl> segcls;

	// cache generated points.
	struct cacheentry {
		vector<vector2f> points;	// that is a 2d mesh.
		vector<unsigned> indices;
		cacheentry() {}
		~cacheentry() {}
		cacheentry(const cacheentry& c) : points(c.points), indices(c.indices) {}
		cacheentry& operator=(const cacheentry& c) { points = c.points; indices = c.indices; return *this; }
		void push_back_point(const vector2f& p);	// avoids double points.
	};
	mutable int pointcachedetail;
	mutable vector<cacheentry> pointcache;
	// check if cache needs to be (re)generated, and do that
	void generate_point_cache(const class coastmap& cm, const vector2f& roff, int detail) const;

	// computes distance to next corner around segment border.
	// p must be inside the segment, hence 0 <= p.x,p.y < segw
	static float dist_to_corner(int b, const vector2f& p, float segw);
	// computes the distance on the segment border between two points.
	// p0,p1 must be inside the segment.
	float compute_border_dist(int b0, const vector2f& p0, int b1, const vector2f& p1, float segw) const;

	unsigned get_successor_for_cl(unsigned cln, const vector2f& segoff, float segw) const;

	coastsegment() : type(0), pointcachedetail(0) {}	
	~coastsegment() {}
	coastsegment(const coastsegment& o) : type(o.type), segcls(o.segcls), pointcachedetail(o.pointcachedetail), pointcache(o.pointcache) {}
	coastsegment& operator= (const coastsegment& o) { type = o.type; segcls = o.segcls; pointcachedetail = o.pointcachedetail; pointcache = o.pointcache; return *this; }
	
	void draw_as_map(const class coastmap& cm, int x, int y, const vector2f& roff, int detail = 0) const;
	void render(const vector2& p, int detail = 0) const;
};



class coastmap
{
	friend class coastsegment;	// just request some values

	// some attributes used for map reading/processing
	vector<Uint8> themap;		// pixel data of map file
	static const int dmx[4];	// some helper constants.
	static const int dmy[4];
	static const int dx[4];
	static const int dy[4];

	//fixme: everywhere i use floats. Why do i use doubles here?
	unsigned pixels_per_seg;	// "n"
	unsigned mapw, maph;		// map width/height in pixels.
	unsigned segsx, segsy;		// nr of segs in x/y dimensions
	double realwidth, realheight;	// width/height of map in reality (meters)
	double pixelw_real;		// width/height of one pixel in reality, in meters
	double segw_real;		// width/height of one segment in reality, in meters
	vector2 realoffset;		// offset in meters for map (position of pixel pos 0,0)
	vector<coastsegment> coastsegments;
	vector<coastline> coastlines;

	list<pair<vector2f, string> > cities;	// city positions (real) and names
	
	coastmap();
	coastmap(const coastmap& );
	coastmap& operator= (const coastmap& );

	// very fast integer clamping (no branch needed, only for 32bit signed integers!)
	Sint32 clamp_zero(Sint32 x) { return x & ~(x >> 31); }
	Uint8& mapf(int cx, int cy);
	bool find_begin_of_coastline(int& x, int& y);
	bool find_coastline(int x, int y, vector<vector2i>& points, bool& cyclic, int& beginborder, int& endborder);
	void divide_and_distribute_cl(const coastline& cl, unsigned clnr, int beginb, int endb, const vector<vector2f>& points);
	void process_coastline(int x, int y);
	void process_segment(int x, int y);

public:	
	// create from xml file
	coastmap(const string& filename);
	~coastmap() {}

	// fixme: maybe it's better to give top,left and bottom,right corner of sub area to draw
	void draw_as_map(const vector2& droff, double mapzoom, int detail = 0) const;
	// here give point and viewrange for rendering?
	void render(const vector2& p, int detail = 0, bool withterraintop = false) const;
};

#endif
