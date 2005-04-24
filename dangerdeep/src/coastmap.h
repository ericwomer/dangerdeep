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
	//float has not enough resolution to store world wide coordinates with meter accuracy.
	//earth diameter ~ 40 million meters, float with 24bit has ~ 16 million precision. so we use double.
	bsplinet<vector2> curve;	// points in map coordinates (meters)

	// create coastline
	// n  - bspline smooth detail
	// p  - bspline control points
	coastline(int n, const vector<vector2>& p) : curve(n, p) {}
	~coastline() {}

	// create vector of real points, detail can be > 0 (additional detail with bspline
	// interpolation) or even < 0 (reduced detail)
	// create points between begint and endt with 0<=t<=1
	// new points are appended on vector "points"
	void create_points(vector<vector2>& points, float begint, float endt, int detail = 0) const;
	
	// just for testing purposes.
	void draw_as_map(int detail = 0) const;
};



class coastsegment
{
//	coastsegment();
public:
	struct segcl
	{
		unsigned mapclnr;	// pointer to coastmap::coastlines, global cl number
		float begint, endt;	// point on coastline where segment begins and ends, 0...1
		vector2 beginp, endp;	// = coastlines[mapclnr].curve.value(begint) but storing is faster!
		int beginborder;	// 0-3, top,right,bottom,left of segment, -1 = (part of an) island
		int endborder;		// dito
		bool cyclic;		// is segcl cyclic?
		int next;		// successor of segcl in segment, needed for triangulation
	};

	unsigned type;	// 0 - sea, 1 - land, 2 mixed
	vector<segcl> segcls;
	
	// terrain elevation (no matter if land or sea, total elevation in meters)
	// user for computation of water depth or terrain height (not yet)
	//bspline2dt<float> topo;

	// cache generated points.
	struct cacheentry {
		vector<vector2> points;	// that is a 2d mesh.
		vector<unsigned> indices;
		void push_back_point(const vector2& p);	// avoids double points.
	};
	mutable int pointcachedetail;
	mutable vector<cacheentry> pointcache;
	// check if cache needs to be (re)generated, and do that
	void generate_point_cache(const class coastmap& cm, const vector2& roff, int detail) const;

	// computes distance to next corner around segment border.
	// p must be inside the segment, hence 0 <= p.x,p.y < segw
	static float dist_to_corner(int b, const vector2& p, float segw);
	float borderpos(int b, const vector2& p, float segw) const;
	// computes the distance on the segment border between two points.
	// p0,p1 must be inside the segment.
	float compute_border_dist(int b0, const vector2& p0, int b1, const vector2& p1, float segw) const;

	int get_successor_for_cl(unsigned cln, const vector2& segoff, double segw) const;

	coastsegment(/*unsigned topon, const vector<float>& topod*/) : type(0), /*topo(topon, topod),*/ pointcachedetail(0) {}
	
	void draw_as_map(const class coastmap& cm, int x, int y, const vector2& roff, int detail = 0) const;
	void render(const class coastmap& cm, int x, int y, const vector2& p, int detail = 0) const;
};



class coastmap
{
	friend class coastsegment;	// just request some values

	// some attributes used for map reading/processing
	vector<Uint8> themap;		// pixel data of map file, y points up, like in OpenGL
	static const int dmx[4];	// some helper constants.
	static const int dmy[4];
	static const int dx[4];
	static const int dy[4];

	unsigned pixels_per_seg;	// "n"
	unsigned mapw, maph;		// map width/height in pixels.
	unsigned segsx, segsy;		// nr of segs in x/y dimensions
	double realwidth, realheight;	// width/height of map in reality (meters)
	double pixelw_real;		// width/height of one pixel in reality, in meters
	double segw_real;		// width/height of one segment in reality, in meters
	vector2 realoffset;		// offset in meters for map (position of pixel pos 0,0)
	vector<coastsegment> coastsegments;
	vector<coastline> coastlines;

	list<pair<vector2, string> > cities;	// city positions (real) and names
	
	coastmap();
	coastmap(const coastmap& );
	coastmap& operator= (const coastmap& );

	// very fast integer clamping (no branch needed, only for 32bit signed integers!)
	Sint32 clamp_zero(Sint32 x) { return x & ~(x >> 31); }
	Uint8& mapf(int cx, int cy);
	bool find_begin_of_coastline(int& x, int& y);
	bool find_coastline(int x, int y, vector<vector2i>& points, bool& cyclic, int& beginborder, int& endborder);
	void divide_and_distribute_cl(const coastline& cl, unsigned clnr, int beginb, int endb, const vector<vector2>& points, bool clcyclic);
	void process_coastline(int x, int y);
	void process_segment(int x, int y);

public:	
	// create from xml file
	coastmap(const string& filename);

	// fixme: maybe it's better to give top,left and bottom,right corner of sub area to draw
	void draw_as_map(const vector2& droff, double mapzoom, int detail = 0) const;
	// here give point and viewrange for rendering?
	void render(const vector2& p, int detail = 0, bool withterraintop = false) const;
};

#endif
