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



class coastsegment
{
public:
	typedef vector2t<unsigned short> segpos;

	struct segcl
	{
		vector<segpos> points;	// coordinates of the segcl. relative to segment.
		int beginpos;		// positions are on border in 0-64k scale: s=65535. (-1 = not on border)
		int endpos;		// then bottom,right,top,left border of segment are 0s+x, 1s+x, 2s+x, 3s+x
		int next;		// successor of this cl. is itself for cyclic segcl's.
		void print() const;	// for debugging
		segcl() : beginpos(-1), endpos(-1), next(-1) {}
		void push_back_point(const segpos& sp);	// avoids double points
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
	void generate_point_cache(const class coastmap& cm, int x, int y, int detail) const;

	void compute_successor_for_cl(unsigned cln);

	void push_back_segcl(const segcl& scl);	// avoids segcls with < 2 points.

	coastsegment(/*unsigned topon, const vector<float>& topod*/) : type(0), /*topo(topon, topod),*/ pointcachedetail(0) {}
	
	void draw_as_map(const class coastmap& cm, int x, int y, int detail = 0) const;
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

	list<pair<vector2, string> > cities;	// city positions (real) and names
	
	coastmap();
	coastmap(const coastmap& );
	coastmap& operator= (const coastmap& );

	// very fast integer clamping (no branch needed, only for 32bit signed integers!)
	Sint32 clamp_zero(Sint32 x) { return x & ~(x >> 31); }
	Uint8& mapf(int cx, int cy);

	// compute position on border (0...4*(2^16-1)-1) or -1 if not on border
	// give border number (-1,0...3) and position in segment.
	int borderpos(const coastsegment::segpos& p) const;

	bool find_begin_of_coastline(int& x, int& y);
	bool find_coastline(int x, int y, vector<vector2i>& points, bool& cyclic);
	void divide_and_distribute_cl(const vector<vector2i>& cl, bool clcyclic);
	void process_coastline(int x, int y);
	void process_segment(int x, int y);

	vector2 segcoord_to_real(int segx, int segy, const coastsegment::segpos& sp) const;

public:	
	// create from xml file
	coastmap(const string& filename);

	// fixme: maybe it's better to give top,left and bottom,right corner of sub area to draw
	void draw_as_map(const vector2& droff, double mapzoom, int detail = 0) const;
	// here give point and viewrange for rendering?
	void render(const vector2& p, int detail = 0, bool withterraintop = false) const;
};

#endif
