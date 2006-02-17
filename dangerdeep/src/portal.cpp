/*
Danger from the Deep - Open source submarine simulation
Copyright (C) 2003-2006  Thorsten Jordan, Luis Barrancos and others.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

// main program
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#ifdef USE_NATIVE_GL
#include <gl.h>
#else
#include "oglext/OglExt.h"
#endif
#include <glu.h>
#include <SDL.h>
#include <SDL_net.h>

#include "system.h"
#include "vector3.h"
#include "model.h"
#include "texture.h"
#include <iostream>
#include <sstream>
#include "image.h"
#include "faulthandler.h"

#include "mymain.cpp"

class system* mysys;
int res_x, res_y;

void run();


texture* metalbackgr;
texture* woodbackgr;
texture* terraintex;

extern string get_data_dir();

string get_texture_dir()
{
	return get_data_dir() + "textures/";
}


#define LVL_X 13
#define LVL_Y 13
#define LVL_Z 3
const char* level[3][13] = {
	{
		"xxxxxxxxxxxxx",
		"x x     x   x",
		"x x xxx x x x",
		"x   x x x x x",
		"xxx x   x x x",
		"x x x x x x x",
		"x         x x",
		"x   x x xxx x",
		"xxxxx x x   x",
		"x     x x xxx",
		"x  x  xxx   x",
		"x           x",
		"xxxxxxxxxxxxx"
	},
	{
		"xxxxxxxxxxxxx",
		"x xxxxxxxxxxx",
		"xxxxxxxxxxxxx",
		"xxxxxxxxxxx x",
		"xxxxxxxxxxxxx",
		"xxx xxxxxxxxx",
		"xxxxxxxxxxxxx",
		"xxxxxxxxxxxxx",
		"xxxxxxxxxxxxx",
		"xxx xxxx xxxx",
		"xxxxxxxxxxxxx",
		"xxxxxxxxxxx x",
		"xxxxxxxxxxxxx"
	},
	{
		"xxxxxxxxxxxxx",
		"x       x   x",
		"xxxxxxx x x x",
		"x     x x x x",
		"x xxx x x   x",
		"x   x   x x x",
		"xxx xxxxx   x",
		"x   x x   x x",
		"x xxx x x   x",
		"x x   x x x x",
		"x xxx xxx x x",
		"x           x",
		"xxxxxxxxxxxxx"
	},
};
unsigned level_at(int x, int y, int z)
{
	if (x < 0 || x >= LVL_X) return 1;
	if (y < 0 || y >= LVL_Y) return 1;
	if (z < 0 || z >= LVL_Z) return 1;
	return level[z][LVL_Y-1-y][x] == 'x' ? 1 : 0;
}


int mymain(list<string>& args)
{
	// report critical errors (on Unix/Posix systems)
	install_segfault_handler();

	// randomize
	srand(time(0));

	// command line argument parsing
	res_x = 1024;
	bool fullscreen = true;

	// parse commandline
	for (list<string>::iterator it = args.begin(); it != args.end(); ++it) {
		if (*it == "--help") {
			cout << "*** Danger from the Deep ***\nusage:\n--help\t\tshow this\n"
			<< "--res n\t\tuse resolution n horizontal,\n\t\tn is 512,640,800,1024 (recommended) or 1280\n"
			<< "--nofullscreen\tdon't use fullscreen\n"
			<< "--debug\t\tdebug mode: no fullscreen, resolution 800\n"
			<< "--editor\trun mission editor directly\n"
			<< "--mission fn\trun mission from file fn (just the filename in the mission directory)\n"
			<< "--nosound\tdon't use sound\n";
			return 0;
		} else if (*it == "--nofullscreen") {
			fullscreen = false;
		} else if (*it == "--debug") {
			fullscreen = false;
			res_x = 800;
		} else if (*it == "--res") {
			list<string>::iterator it2 = it; ++it2;
			if (it2 != args.end()) {
				int r = atoi(it2->c_str());
				if (r==512||r==640||r==800||r==1024||r==1280)
					res_x = r;
				++it;
			}
		}
	}

	model::enable_shaders = true;

	// fixme: also allow 1280x1024, set up gl viewport for 4:3 display
	// with black borders at top/bottom (height 2*32pixels)
	res_y = res_x*3/4;
	// weather conditions and earth curvature allow 30km sight at maximum.
	mysys = new class system(1.0, 30000.0+500.0, res_x, fullscreen);
	mysys->set_res_2d(1024, 768);
	mysys->set_max_fps(60);
	
	mysys->add_console("$ffffffDanger $c0c0c0from the $ffffffDeep");
	mysys->add_console("$ffff00copyright and written 2003 by $ff0000Thorsten Jordan");
	mysys->add_console("$80ff80*** welcome ***");

	GLfloat lambient[4] = {0.1,0.1,0.1,1};
	GLfloat ldiffuse[4] = {1,1,1,1};
	GLfloat lposition[4] = {0,0,1,0};
	glLightfv(GL_LIGHT0, GL_AMBIENT, lambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, ldiffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, lposition);
	glEnable(GL_LIGHT0);

// 	font_arial = new font(get_font_dir() + "font_arial");
// 	mysys->draw_console_with(font_arial, 0);

	run();

	mysys->write_console(true);

// 	delete font_arial;
	delete mysys;

	return 0;
}


template<class D>
class plane_t
{
public:
	vector3t<D> N;
	D d;
	plane_t() : d(0) {}
	plane_t(const vector3t<D>& N_, const D& d_) : N(N_), d(d_) {}
	plane_t(const D& a, const D& b, const D& c, const D& d_) : N(a, b, c), d(d_) {}
	/// construct from three points.
	plane_t(const vector3t<D>& a, const vector3t<D>& b, const vector3t<D>& c)
		: N((b-a).cross(c-a).normal())
	{
		d = -(N * a);
	}
	/// determine if point is left of plane (on side that normal points to).
	bool is_left(const vector3t<D>& a) const {
		return (N * a >= -d);
	}
	/// compute intersection point of line a->b, returns true if intersection is valid
	vector3t<D> intersection(const vector3t<D>& a, const vector3t<D>& b) const {
		D divi = N * (b - a);
		// if abs(divi) is near zero then a,b are both on the plane
		D t = - (d + N * a) / divi;
		return a + (b - a) * t;
	}
	/// compute intersection point of line a->b, returns true if intersection is valid
	bool test_intersection(const vector3t<D>& a, const vector3t<D>& b, vector3t<D>& result) const {
		bool a_left = is_left(a), b_left = is_left(b);
		// both are left or not left? no intersection
		if (a_left == b_left)
			return false;
		result = intersection(a, b);
		return true;
	}
};

typedef plane_t<double> plane;
typedef plane_t<float> planef;



template<class D>
class polygon_t
{
public:
	std::vector<vector3t<D> > points;
	/// empty polygon
	polygon_t() {}
	/// polygon with prepared space
	polygon_t(unsigned capacity_) { points.reserve(capacity_); }
	/// make from three points.
	polygon_t(const vector3t<D>& a, const vector3t<D>& b, const vector3t<D>& c) {
		points.reserve(3);
		points.push_back(a);
		points.push_back(b);
		points.push_back(c);
	}
	/// make from four points
	polygon_t(const vector3t<D>& a, const vector3t<D>& b, const vector3t<D>& c, const vector3t<D>& d) {
		points.reserve(4);
		points.push_back(a);
		points.push_back(b);
		points.push_back(c);
		points.push_back(d);
	}
	/// check if polygon is empty. Each polygon must have at least three vertices or it is empty
	bool empty() const { return points.size() < 3; }
	/// compute normal of polygon. Only valid if all points are in a plane.
	vector3t<D> normal() const {
		if (empty()) return vector3t<D>();
		return (points[1] - points[0]).cross(points[2] - points[0]).normal();
	}
	/// clip polygon against plane.
	polygon_t clip(const plane_t<D>& plan) const {
		// do not clip empty polygons.
		if (empty()) return polygon_t();
		// test for each point if it is left of the plane.
		// do not use bitvector here, too slow.
		std::vector<unsigned> pleft(points.size());
		unsigned prightleft_ctr[2] = { 0, 0 };	// count #points on right and left side.
		for (unsigned i = 0; i < points.size(); ++i) {
			unsigned pli = plan.is_left(points[i]);
			pleft[i] = pli;
			++prightleft_ctr[pli ? 1 : 0];
		}
		// we can have at most two clip points.
		// this means all points are splitted in at most two sets (left / right).
		// if all points are left, return *this, if all are right, return an empty polygon.
		if (prightleft_ctr[0] == 0)
			return *this;	// no points are right.
		if (prightleft_ctr[1] == 0)
			return polygon_t();	// no points are left.
		// now we have the case that this polygon really intersects the plane (or touches it...)
		// we assume that no point touches the plane, but is really left or right (depends
		// on rounding precision...)
		// this means the result has #left + 2 points.
		polygon_t result(prightleft_ctr[1] + 2);
		// copy all points that are left of the plane to the result
		// insert the two intersection points.
		for (unsigned i = 0; i < points.size(); ++i) {
			unsigned j = (i+1) % points.size();
			if (pleft[i]) {
				if (pleft[j]) {
					// both points are left, just copy this point
					result.points.push_back(points[i]);
				} else {
					// next point is right, insert this point and intersection point
					result.points.push_back(points[i]);
					result.points.push_back(plan.intersection(points[i], points[j]));
				}
			} else {
				if (pleft[j]) {
					// next point is left, insert intersection point
					result.points.push_back(plan.intersection(points[i], points[j]));
				} else {
					// both points are right, do nothing
				}
			}
		}
		return result;
	}
	/// print polygon
	void print() const {
		cout << "Poly, pts=" << points.size() << "\n";
		for (unsigned i = 0; i < points.size(); ++i)
			cout << "P[" << i << "] = " << points[i] << "\n";
	}
	/// render polygon
	void draw() const
	{
		glBegin(GL_LINE_LOOP);
		for (unsigned i = 0; i < points.size(); ++i)
			glVertex3d(points[i].x, points[i].y, points[i].z);
		glEnd();
	}
	/// compute plane that poly lies in
	plane_t<D> get_plane() const {
		if (empty())
			return plane_t<D>();
		return plane_t<D>(points[0], points[1], points[2]);
	}
};

typedef polygon_t<double> polygon;
typedef polygon_t<float> polygonf;



class frustum
{
	frustum();
public:
	std::vector<plane> planes;
	vector3 viewpos;
	vector3 viewdir;
	frustum(polygon poly, const vector3& viewp, const vector3& viewd);
	polygon clip(polygon p) const;
};



frustum::frustum(polygon poly, const vector3& viewp, const vector3& viewd)
	: viewpos(viewp), viewdir(viewd)
{
	planes.reserve(poly.points.size());
	for (unsigned i = 0; i < poly.points.size(); ++i) {
		unsigned j = (i+1)%poly.points.size();
		planes.push_back(plane(poly.points[i], viewpos, poly.points[j]));
	}
}

polygon frustum::clip(polygon p) const
{
	polygon result = p;
	for (unsigned i = 0; i < planes.size(); ++i)
		result = result.clip(planes[i]);
	return result;
}



class portal
{
public:
	polygon shape;
	class sector* adj_sector;
	// bool mirror;
	portal(polygon shp, class sector* adsc)	: shape(shp), adj_sector(adsc) {}
};



class sector
{
public:
	//model geometry;
	vector3 basepos;
	unsigned walls;
	std::vector<portal> portals;
	mutable bool displayed;
	void display(const frustum& f) const;
	bool check_movement(const vector3& currpos, const vector3& nextpos, sector*& nextseg) const;
};



void sector::display(const frustum& f) const
{
	if (!displayed) {
		glColor3f(1,1,1);
		terraintex->set_gl_texture();
		if (walls & 1) {
			glBegin(GL_QUADS);
			glTexCoord2f(0,1);
			glVertex3f(basepos.x, basepos.y+1, basepos.z);
			glTexCoord2f(1,1);
			glVertex3f(basepos.x+1, basepos.y+1, basepos.z);
			glTexCoord2f(1,0);
			glVertex3f(basepos.x+1, basepos.y+1, basepos.z+1);
			glTexCoord2f(0,0);
			glVertex3f(basepos.x, basepos.y+1, basepos.z+1);
			glEnd();
		}
		if (walls & 2) {
			glBegin(GL_QUADS);
			glTexCoord2f(0,1);
			glVertex3f(basepos.x+1, basepos.y, basepos.z);
			glTexCoord2f(1,1);
			glVertex3f(basepos.x, basepos.y, basepos.z);
			glTexCoord2f(1,0);
			glVertex3f(basepos.x, basepos.y, basepos.z+1);
			glTexCoord2f(0,0);
			glVertex3f(basepos.x+1, basepos.y, basepos.z+1);
			glEnd();
		}
		if (walls & 4) {
			glBegin(GL_QUADS);
			glTexCoord2f(0,1);
			glVertex3f(basepos.x, basepos.y, basepos.z);
			glTexCoord2f(1,1);
			glVertex3f(basepos.x, basepos.y+1, basepos.z);
			glTexCoord2f(1,0);
			glVertex3f(basepos.x, basepos.y+1, basepos.z+1);
			glTexCoord2f(0,0);
			glVertex3f(basepos.x, basepos.y, basepos.z+1);
			glEnd();
		}
		if (walls & 8) {
			glBegin(GL_QUADS);
			glTexCoord2f(0,1);
			glVertex3f(basepos.x+1, basepos.y+1, basepos.z);
			glTexCoord2f(1,1);
			glVertex3f(basepos.x+1, basepos.y, basepos.z);
			glTexCoord2f(1,0);
			glVertex3f(basepos.x+1, basepos.y, basepos.z+1);
			glTexCoord2f(0,0);
			glVertex3f(basepos.x+1, basepos.y+1, basepos.z+1);
			glEnd();
		}
		if (walls & 16) {
			metalbackgr->set_gl_texture();
			glBegin(GL_QUADS);
			glTexCoord2f(0,1);
			glVertex3f(basepos.x, basepos.y, basepos.z);
			glTexCoord2f(1,1);
			glVertex3f(basepos.x+1, basepos.y, basepos.z);
			glTexCoord2f(1,0);
			glVertex3f(basepos.x+1, basepos.y+1, basepos.z);
			glTexCoord2f(0,0);
			glVertex3f(basepos.x, basepos.y+1, basepos.z);
			glEnd();
		}
		if (walls & 32) {
			woodbackgr->set_gl_texture();
			glBegin(GL_QUADS);
			glTexCoord2f(0,1);
			glVertex3f(basepos.x+1, basepos.y, basepos.z+1);
			glTexCoord2f(1,1);
			glVertex3f(basepos.x, basepos.y, basepos.z+1);
			glTexCoord2f(1,0);
			glVertex3f(basepos.x, basepos.y+1, basepos.z+1);
			glTexCoord2f(0,0);
			glVertex3f(basepos.x+1, basepos.y+1, basepos.z+1);
			glEnd();
		}
		glBindTexture(GL_TEXTURE_2D, 0);
		displayed = true;
	}

	// check for other portals
	for (unsigned i = 0; i < portals.size(); ++i) {
		const portal& p = portals[i];
		// avoid portals facing away.
		// this means viewpos must be on the inner side of the portal plane.
		if (p.shape.get_plane().is_left(f.viewpos)) {
			polygon newfpoly = f.clip(p.shape);
			if (!newfpoly.empty()) {
// 				glColor3f(0.5,0.5,1);
// 				newfpoly.draw();
				frustum fnew(newfpoly, f.viewpos, f.viewdir);
				p.adj_sector->display(fnew);
			}
		}
	}
}



bool sector::check_movement(const vector3& currpos, const vector3& nextpos, sector*& nextseg) const
{
	// we assume that nextpos is inside this sector.
	if (nextpos.x >= basepos.x && nextpos.x < basepos.x + 1
	    && nextpos.y >= basepos.y && nextpos.y < basepos.y + 1
	    && nextpos.z >= basepos.z && nextpos.z < basepos.z + 1) {
		nextseg = 0;
		return true;
	}
	// check for crossed portal.
	for (unsigned i = 0; i < portals.size(); ++i) {
		plane pl = portals[i].shape.get_plane();
		if (pl.is_left(currpos) && ! pl.is_left(nextpos)) {
			// we crossed the plane of that portal, switch sector.
			nextseg = portals[i].adj_sector;
			return false;
		}
	}
	nextseg = 0;
	return false;
}



void line(const vector3& a, const vector3& b)
{
	glVertex3dv(&a.x);
	glVertex3dv(&b.x);
}



#if 0
void run()
{
	// plane parallel to z=0 plane, but 2 units in z-direction.
// 	plane pl(0, 0, 1, -2);
// 	plane pl2(0, 0, -1, 3);
// 	polygon p0(vector3(0,0,0), vector3(-4,0,4), vector3(4,0,4));
// 	polygon p1(vector3(0,0,0), vector3(-4,0,-4), vector3(4,0,-4));
// 	polygon p2(vector3(0,0,8), vector3(-4,0,4), vector3(4,0,4));
// 	polygon pc0 = p0.clip(pl);
// 	polygon pc1 = p1.clip(pl);
// 	polygon pc2 = p2.clip(pl);
// 	pc0.print();
// 	pc1.print();
// 	pc2.print();
// 	p0.clip(pl).clip(pl2).print();

	/* 3d portal rendering:
	   define frustum as list of planes (4 at begin, depends on FOV etc, maybe get plane
	   equations from projection matrix etc.
	   clip portal against frustum by clipping the portal polygon against all frustum planes
	   sequentially.
	   Avoid portals that are facing away.
	   Resulting polygon is either empty or valid.
	   Construct new frustum by making planes from points of polygon and camera position.
	   Continue...
	   mark each sector as rendered when you render it (could be visible through > 1 portals).
	   avoid rerender. clear all tags before rendering.
	*/

#define LVLSZ 13
	vector<sector> sectors(LVLSZ*LVLSZ);
	for (int y = 0; y < LVLSZ; ++y) {
		int ya = LVLSZ-1-y;
		for (int x = 0; x < LVLSZ; ++x) {
			if (level[ya][x] != 'x') {
				// create sector
				sector& s = sectors[y*LVLSZ+x];
				s.basepos = vector3(x, y, 0);
				s.walls = 0x00;
				polygon pup   (s.basepos+vector3(0,1,0), s.basepos+vector3(1,1,0), s.basepos+vector3(1,1,1), s.basepos+vector3(0,1,1));
				polygon pleft (s.basepos+vector3(0,0,0), s.basepos+vector3(0,1,0), s.basepos+vector3(0,1,1), s.basepos+vector3(0,0,1));
				polygon pright(s.basepos+vector3(1,1,0), s.basepos+vector3(1,0,0), s.basepos+vector3(1,0,1), s.basepos+vector3(1,1,1));
				polygon pdown (s.basepos+vector3(1,0,0), s.basepos+vector3(0,0,0), s.basepos+vector3(0,0,1), s.basepos+vector3(1,0,1));
				// look for adjacent sectors, create portals to them
				if (level[ya-1][x] != 'x')
					s.portals.push_back(portal(pup, &sectors[(y+1)*LVLSZ+x]));
				else
					s.walls |= 1;
				if (level[ya+1][x] != 'x')
					s.portals.push_back(portal(pdown, &sectors[(y-1)*LVLSZ+x]));
				else
					s.walls |= 2;
				if (level[ya][x-1] != 'x')
					s.portals.push_back(portal(pleft, &sectors[y*LVLSZ+x-1]));
				else
					s.walls |= 4;
				if (level[ya][x+1] != 'x')
					s.portals.push_back(portal(pright, &sectors[y*LVLSZ+x+1]));
				else
					s.walls |= 8;
			}
		}
	}

	sector* currsector = &sectors[1*LVLSZ+1];

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	sys().gl_perspective_fovx(70, 4.0/3.0, 0.1, 1000);
	glMatrixMode(GL_MODELVIEW);

	glDisable(GL_LIGHTING);

	vector3 viewangles_cam;
	vector3 pos_cam(0, 0, 2);
	vector3 viewangles_usr(90, 0, 0);
	vector3 pos_usr(1.5, 1.5, 0.3);

	bool movecamera = true;

	while (true) {
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		glBindTexture(GL_TEXTURE_2D, 0);

		// only allow turning as test
		viewangles_usr.x = 90;
		viewangles_usr.z = 0;

		// compute mvp etc. for user
		glLoadIdentity();
		glRotatef(viewangles_usr.z, 0, 0, 1);
		glRotatef(viewangles_usr.y, 0, 1, 0);
		glRotatef(viewangles_usr.x, 1, 0, 0);
		matrix4 mvr = matrix4::get_gl(GL_MODELVIEW_MATRIX);
		glTranslated(-pos_usr.x, -pos_usr.y, -pos_usr.z);
		matrix4 mv = matrix4::get_gl(GL_MODELVIEW_MATRIX);
		matrix4 prj = matrix4::get_gl(GL_PROJECTION_MATRIX);
		matrix4 mvp = prj * mv;
		matrix4 invmv = mv.inverse();
		matrix4 invmvr = mvr.inverse();
		matrix4 invmvp = mvp.inverse();
		vector3 wbln = invmvp * vector3(-1,-1,-1);
		vector3 wbrn = invmvp * vector3(+1,-1,-1);
		vector3 wtln = invmvp * vector3(-1,+1,-1);
		vector3 wtrn = invmvp * vector3(+1,+1,-1);
		vector3 vd = invmvr * vector3(0,0,-1);
		polygon viewwindow(wbln, wbrn, wtrn, wtln);
		frustum viewfrustum(viewwindow, pos_usr, vd);

		// set real values for camera
		glLoadIdentity();
		glTranslated(-pos_cam.x, -pos_cam.y, -pos_cam.z);
		glRotatef(viewangles_cam.z, 0, 0, 1);
		glRotatef(viewangles_cam.y, 0, 1, 0);
		glRotatef(viewangles_cam.x, 1, 0, 0);

		// coordinate system
		glBegin(GL_LINES);
		glColor3f(1,0,0);
		glVertex3f(0,0,0);
		glVertex3f(1,0,0);
		glColor3f(1,1,0);
		glVertex3f(0,0,0);
		glVertex3f(0,1,0);
		glColor3f(0,1,0);
		glVertex3f(0,0,0);
		glVertex3f(0,0,1);
		glColor3f(1,1,1);
		glEnd();
		glColor3f(1, 1, 1);

		// show user pos and frustum
		glBegin(GL_LINES);
		glColor3f(1,0.2,0.2);
		line(pos_usr, wbln);
		line(pos_usr, wbrn);
		line(pos_usr, wtln);
		line(pos_usr, wtrn);
		glColor3f(1,0.6,0.6);
		line(wbln, wbrn);
		line(wbrn, wtrn);
		line(wtrn, wtln);
		line(wtln, wbln);
		glColor3f(1,1,0.5);
		line(pos_usr, pos_usr + vd);
		glEnd();

		// render sectors.
		for (unsigned i = 0; i < sectors.size(); ++i)
			sectors[i].displayed = false;
		currsector->display(viewfrustum);

		bool movedcam = movecamera;
		vector3& pos = (movecamera) ? pos_cam : pos_usr;
		vector3& viewangles = (movecamera) ? viewangles_cam : viewangles_usr;
		vector3 oldpos = pos;
		const double movesc = 0.1;
		list<SDL_Event> events = mysys->poll_event_queue();
		for (list<SDL_Event>::iterator it = events.begin(); it != events.end(); ++it) {
			SDL_Event& event = *it;
			if (event.type == SDL_KEYDOWN) {
				switch (event.key.keysym.sym) {
				case SDLK_ESCAPE:
					return;
				case SDLK_KP4: pos.x -= movesc; break;
				case SDLK_KP6: pos.x += movesc; break;
				case SDLK_KP8: pos.y -= movesc; break;
				case SDLK_KP2: pos.y += movesc; break;
				case SDLK_KP1: pos.z -= movesc; break;
				case SDLK_KP3: pos.z += movesc; break;
				case SDLK_SPACE: movecamera = !movecamera; break;
				default: break;
				}
			} else if (event.type == SDL_MOUSEMOTION) {
				if (event.motion.state & SDL_BUTTON_LMASK) {
					viewangles.y += event.motion.xrel * 0.5;
					viewangles.x += event.motion.yrel * 0.5;
				} else if (event.motion.state & SDL_BUTTON_RMASK) {
					viewangles.z += event.motion.xrel * 0.5;
					viewangles.x += event.motion.yrel * 0.5;
				} else if (event.motion.state & SDL_BUTTON_MMASK) {
					pos.x += event.motion.xrel * 0.05;
					pos.y += event.motion.yrel * 0.05;
				}
			} else if (event.type == SDL_MOUSEBUTTONDOWN) {
				if (event.button.button == SDL_BUTTON_WHEELUP) {
					pos.z -= movesc;
				} else if (event.button.button == SDL_BUTTON_WHEELDOWN) {
					pos.z += movesc;
				}
			}
		}
		if (!movedcam) {
			sector* seg = 0;
			bool movementok = currsector->check_movement(oldpos, pos, seg);
			if (!movementok) {
				if (seg) {
					// switch sector
					currsector = seg;
				} else {
					// avoid movement
					pos_usr = oldpos;
				}
			}
		}
		mysys->swap_buffers();
	}
}
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////

void run()
{
	/* 3d portal rendering:
	   define frustum as list of planes (4 at begin, depends on FOV etc, maybe get plane
	   equations from projection matrix etc.
	   clip portal against frustum by clipping the portal polygon against all frustum planes
	   sequentially.
	   Avoid portals that are facing away.
	   Resulting polygon is either empty or valid.
	   Construct new frustum by making planes from points of polygon and camera position.
	   Continue...
	   mark each sector as rendered when you render it (could be visible through > 1 portals).
	   avoid rerender. clear all tags before rendering.
	*/

	metalbackgr = new texture(get_texture_dir() + "metalbackgr.png", texture::LINEAR);
	woodbackgr = new texture(get_texture_dir() + "wooden_desk.png", texture::LINEAR);
	terraintex = new texture(get_texture_dir() + "terrain.jpg", texture::LINEAR);

	vector<sector> sectors(LVL_X * LVL_Y * LVL_Z);
	for (int z = 0; z < LVL_Z; ++z) {
		for (int y = 0; y < LVL_Y; ++y) {
			for (int x = 0; x < LVL_X; ++x) {
				if (level_at(x, y, z) == 0) {
					// create sector
					sector& s = sectors[x+LVL_X*(y+LVL_Y*z)];
					s.basepos = vector3(x, y, z);
					s.walls = 0x00;
					polygon pup   (s.basepos+vector3(0,1,0), s.basepos+vector3(1,1,0), s.basepos+vector3(1,1,1), s.basepos+vector3(0,1,1));
					polygon pleft (s.basepos+vector3(0,0,0), s.basepos+vector3(0,1,0), s.basepos+vector3(0,1,1), s.basepos+vector3(0,0,1));
					polygon pright(s.basepos+vector3(1,1,0), s.basepos+vector3(1,0,0), s.basepos+vector3(1,0,1), s.basepos+vector3(1,1,1));
					polygon pdown (s.basepos+vector3(1,0,0), s.basepos+vector3(0,0,0), s.basepos+vector3(0,0,1), s.basepos+vector3(1,0,1));
					polygon ptop  (s.basepos+vector3(1,0,1), s.basepos+vector3(0,0,1), s.basepos+vector3(0,1,1), s.basepos+vector3(1,1,1));
					polygon pbott (s.basepos+vector3(0,0,0), s.basepos+vector3(1,0,0), s.basepos+vector3(1,1,0), s.basepos+vector3(0,1,0));
					// look for adjacent sectors, create portals to them
					if (level_at(x, y+1, z) == 0)
						s.portals.push_back(portal(pup, &sectors[x+LVL_X*(y+1+LVL_Y*z)]));
					else
						s.walls |= 1;
					if (level_at(x, y-1, z) == 0)
						s.portals.push_back(portal(pdown, &sectors[x+LVL_X*(y-1+LVL_Y*z)]));
					else
						s.walls |= 2;
					if (level_at(x-1, y, z) == 0)
						s.portals.push_back(portal(pleft, &sectors[x-1+LVL_X*(y+LVL_Y*z)]));
					else
						s.walls |= 4;
					if (level_at(x+1, y, z) == 0)
						s.portals.push_back(portal(pright, &sectors[x+1+LVL_X*(y+LVL_Y*z)]));
					else
						s.walls |= 8;
					if (level_at(x, y, z-1) == 0)
						s.portals.push_back(portal(pbott, &sectors[x+LVL_X*(y+LVL_Y*(z-1))]));
					else
						s.walls |= 16;
					if (level_at(x, y, z+1) == 0)
						s.portals.push_back(portal(ptop, &sectors[x+LVL_X*(y+LVL_Y*(z+1))]));
					else
						s.walls |= 32;
				}
			}
		}
	}

	sector* currsector = &sectors[1+LVL_X*(1+LVL_Y*0)];
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	sys().gl_perspective_fovx(70, 4.0/3.0, 0.1, 1000);
	glMatrixMode(GL_MODELVIEW);

	glDisable(GL_LIGHTING);

	vector3 viewangles(0, 0, 0);
	vector3 pos(1.5, 1.5, 0.3);

	while (true) {
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		glBindTexture(GL_TEXTURE_2D, 0);

		// compute mvp etc. for user
		glLoadIdentity();
		// make camera look to pos. y-axis.
		glRotatef(-90, 1, 0, 0);
		glRotatef(-viewangles.y, 0, 1, 0);
		glRotatef(-viewangles.x, 1, 0, 0);
		glRotatef(-viewangles.z, 0, 0, 1);
		matrix4 mvr = matrix4::get_gl(GL_MODELVIEW_MATRIX);
		glTranslated(-pos.x, -pos.y, -pos.z);
		matrix4 mv = matrix4::get_gl(GL_MODELVIEW_MATRIX);
		matrix4 prj = matrix4::get_gl(GL_PROJECTION_MATRIX);
		matrix4 mvp = prj * mv;
		matrix4 invmv = mv.inverse();
		matrix4 invmvr = mvr.inverse();
		matrix4 invmvp = mvp.inverse();
		vector3 wbln = invmvp * vector3(-1,-1,-1);
		vector3 wbrn = invmvp * vector3(+1,-1,-1);
		vector3 wtln = invmvp * vector3(-1,+1,-1);
		vector3 wtrn = invmvp * vector3(+1,+1,-1);
		vector3 vd = invmvr * vector3(0,0,-1);
		polygon viewwindow(wbln, wbrn, wtrn, wtln);
		frustum viewfrustum(viewwindow, pos, vd);

		// render sectors.
		for (unsigned i = 0; i < sectors.size(); ++i)
			sectors[i].displayed = false;
		currsector->display(viewfrustum);

		vector3 oldpos = pos;
		const double movesc = 0.25;
		list<SDL_Event> events = mysys->poll_event_queue();
		vector3 forward = -invmvr.column3(2) * movesc;
		vector3 upward = invmvr.column3(1) * movesc;
		vector3 sideward = invmvr.column3(0) * movesc;
		for (list<SDL_Event>::iterator it = events.begin(); it != events.end(); ++it) {
			SDL_Event& event = *it;
			if (event.type == SDL_KEYDOWN) {
				switch (event.key.keysym.sym) {
				case SDLK_ESCAPE:
					return;
				case SDLK_KP4: pos -= sideward; break;
				case SDLK_KP6: pos += sideward; break;
				case SDLK_KP8: pos += upward; break;
				case SDLK_KP2: pos -= upward; break;
				case SDLK_KP1: pos += forward; break;
				case SDLK_KP3: pos -= forward; break;
				default: break;
				}
			} else if (event.type == SDL_MOUSEMOTION) {
				if (event.motion.state & SDL_BUTTON_LMASK) {
					viewangles.z -= event.motion.xrel * 0.5;
					viewangles.x -= event.motion.yrel * 0.5;
				} else if (event.motion.state & SDL_BUTTON_RMASK) {
					viewangles.y += event.motion.xrel * 0.5;
// 				} else if (event.motion.state & SDL_BUTTON_MMASK) {
// 					pos.x += event.motion.xrel * 0.05;
// 					pos.y += event.motion.yrel * 0.05;
				}
			} else if (event.type == SDL_MOUSEBUTTONDOWN) {
// 				if (event.button.button == SDL_BUTTON_WHEELUP) {
// 					pos.z -= movesc;
// 				} else if (event.button.button == SDL_BUTTON_WHEELDOWN) {
// 					pos.z += movesc;
// 				}
			}
		}

		// check for sector switch by movement
		sector* seg = 0;
		bool movementok = currsector->check_movement(oldpos, pos, seg);
		if (!movementok) {
			if (seg) {
				// switch sector
				currsector = seg;
			} else {
				// avoid movement
				pos = oldpos;
			}
		}

		mysys->swap_buffers();
	}

	delete metalbackgr;
	delete woodbackgr;
	delete terraintex;
}
