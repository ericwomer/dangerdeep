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
#include "datadirs.h"
#include "frustum.h"
#include "shader.h"
#include "font.h"
#include "fpsmeasure.h"
#include "log.h"
#include "mymain.cpp"

#include <time.h>

class system* mysys;
int res_x, res_y;

void run();

font* font_arial;

texture* metalbackgr;
texture* woodbackgr;
texture* terraintex;

texture* reliefwall_diffuse;
texture* reliefwall_bump;
texture* stonewall_diffuse;
texture* stonewall_bump;
glsl_shader_setup* glsl_reliefmapping;
int loc_tex_color;
int loc_tex_normal;
int vertex_attrib_index;
int loc_depth_factor;

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

	// fixme: also allow 1280x1024, set up gl viewport for 4:3 display
	// with black borders at top/bottom (height 2*32pixels)
	res_y = res_x*3/4;
	// weather conditions and earth curvature allow 30km sight at maximum.
	system::create_instance(new class system(1.0, 30000.0+500.0, res_x, res_y, fullscreen));
	sys().set_res_2d(1024, 768);
//	sys().set_max_fps(60);
	
	log_info("Danger from the Deep");

	GLfloat lambient[4] = {0.3,0.3,0.3,1};
	GLfloat ldiffuse[4] = {1,1,1,1};
	GLfloat lposition[4] = {0,0,1,0};
	glLightfv(GL_LIGHT0, GL_AMBIENT, lambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, ldiffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, lposition);
	glEnable(GL_LIGHT0);

 	font_arial = new font(get_font_dir() + "font_arial");
 	sys().draw_console_with(font_arial, 0);

	run();

 	delete font_arial;

	system::destroy_instance();

	return 0;
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
	mutable bool visited;	// needed during rendering.
	sector() : walls(0), displayed(false), visited(false) {}
	void display(const frustum& f) const;
	bool check_movement(const vector3& currpos, const vector3& nextpos, sector*& nextseg) const;
};



void sector::display(const frustum& f) const
{
	visited = true;
	if (!displayed) {
		glColor3f(1,1,1);
		glsl_reliefmapping->use();
		glsl_reliefmapping->set_gl_texture(*stonewall_diffuse, loc_tex_color, 0);
		glsl_reliefmapping->set_gl_texture(*stonewall_bump, loc_tex_normal, 1);
		glsl_reliefmapping->set_uniform(loc_depth_factor, float(0.015));
		if (walls & 1) {
			glBegin(GL_QUADS);
			glTexCoord2f(0,0);
			glNormal3f(0,-1,0);
			glVertexAttrib3f(vertex_attrib_index, 1,0,0);
			glVertex3f(basepos.x, basepos.y+1, basepos.z);
			glTexCoord2f(1,0);
			glNormal3f(0,-1,0);
			glVertexAttrib3f(vertex_attrib_index, 1,0,0);
			glVertex3f(basepos.x+1, basepos.y+1, basepos.z);
			glTexCoord2f(1,1);
			glNormal3f(0,-1,0);
			glVertexAttrib3f(vertex_attrib_index, 1,0,0);
			glVertex3f(basepos.x+1, basepos.y+1, basepos.z+1);
			glTexCoord2f(0,1);
			glNormal3f(0,-1,0);
			glVertexAttrib3f(vertex_attrib_index, 1,0,0);
			glVertex3f(basepos.x, basepos.y+1, basepos.z+1);
			glEnd();
		}
		glsl_reliefmapping->set_gl_texture(*reliefwall_diffuse, loc_tex_color, 0);
		glsl_reliefmapping->set_gl_texture(*reliefwall_bump, loc_tex_normal, 1);
		if (walls & 2) {
			glBegin(GL_QUADS);
			glTexCoord2f(0,0);
			glNormal3f(0,1,0);
			glVertexAttrib3f(vertex_attrib_index, -1,0,0);
			glVertex3f(basepos.x+1, basepos.y, basepos.z);
			glTexCoord2f(1,0);
			glNormal3f(0,1,0);
			glVertexAttrib3f(vertex_attrib_index, -1,0,0);
			glVertex3f(basepos.x, basepos.y, basepos.z);
			glTexCoord2f(1,1);
			glNormal3f(0,1,0);
			glVertexAttrib3f(vertex_attrib_index, -1,0,0);
			glVertex3f(basepos.x, basepos.y, basepos.z+1);
			glTexCoord2f(0,1);
			glNormal3f(0,1,0);
			glVertexAttrib3f(vertex_attrib_index, -1,0,0);
			glVertex3f(basepos.x+1, basepos.y, basepos.z+1);
			glEnd();
		}
		if (walls & 4) {
			glBegin(GL_QUADS);
			glTexCoord2f(0,0);
			glNormal3f(1,0,0);
			glVertexAttrib3f(vertex_attrib_index, 0,1,0);
			glVertex3f(basepos.x, basepos.y, basepos.z);
			glTexCoord2f(1,0);
			glNormal3f(1,0,0);
			glVertexAttrib3f(vertex_attrib_index, 0,1,0);
			glVertex3f(basepos.x, basepos.y+1, basepos.z);
			glTexCoord2f(1,1);
			glNormal3f(1,0,0);
			glVertexAttrib3f(vertex_attrib_index, 0,1,0);
			glVertex3f(basepos.x, basepos.y+1, basepos.z+1);
			glTexCoord2f(0,1);
			glNormal3f(1,0,0);
			glVertexAttrib3f(vertex_attrib_index, 0,1,0);
			glVertex3f(basepos.x, basepos.y, basepos.z+1);
			glEnd();
		}
		if (walls & 8) {
			glBegin(GL_QUADS);
			glTexCoord2f(0,0);
			glNormal3f(-1,0,0);
			glVertexAttrib3f(vertex_attrib_index, 0,-1,0);
			glVertex3f(basepos.x+1, basepos.y+1, basepos.z);
			glTexCoord2f(1,0);
			glNormal3f(-1,0,0);
			glVertexAttrib3f(vertex_attrib_index, 0,-1,0);
			glVertex3f(basepos.x+1, basepos.y, basepos.z);
			glTexCoord2f(1,1);
			glNormal3f(-1,0,0);
			glVertexAttrib3f(vertex_attrib_index, 0,-1,0);
			glVertex3f(basepos.x+1, basepos.y, basepos.z+1);
			glTexCoord2f(0,1);
			glNormal3f(-1,0,0);
			glVertexAttrib3f(vertex_attrib_index, 0,-1,0);
			glVertex3f(basepos.x+1, basepos.y+1, basepos.z+1);
			glEnd();
		}
		if (walls & 16) {
			//metalbackgr->set_gl_texture();
			glBegin(GL_QUADS);
			glTexCoord2f(0,0);
			glNormal3f(0,0,1);
			glVertexAttrib3f(vertex_attrib_index, 1,0,0);
			glVertex3f(basepos.x, basepos.y, basepos.z);
			glTexCoord2f(1,0);
			glNormal3f(0,0,1);
			glVertexAttrib3f(vertex_attrib_index, 1,0,0);
			glVertex3f(basepos.x+1, basepos.y, basepos.z);
			glTexCoord2f(1,1);
			glNormal3f(0,0,1);
			glVertexAttrib3f(vertex_attrib_index, 1,0,0);
			glVertex3f(basepos.x+1, basepos.y+1, basepos.z);
			glTexCoord2f(0,1);
			glNormal3f(0,0,1);
			glVertexAttrib3f(vertex_attrib_index, 1,0,0);
			glVertex3f(basepos.x, basepos.y+1, basepos.z);
			glEnd();
		}
		if (walls & 32) {
			//woodbackgr->set_gl_texture();
			glBegin(GL_QUADS);
			glTexCoord2f(0,0);
			glNormal3f(0,0,-1);
			glVertexAttrib3f(vertex_attrib_index, 1,0,0);
			glVertex3f(basepos.x+1, basepos.y, basepos.z+1);
			glTexCoord2f(1,0);
			glNormal3f(0,0,-1);
			glVertexAttrib3f(vertex_attrib_index, 1,0,0);
			glVertex3f(basepos.x, basepos.y, basepos.z+1);
			glTexCoord2f(1,1);
			glNormal3f(0,0,-1);
			glVertexAttrib3f(vertex_attrib_index, 1,0,0);
			glVertex3f(basepos.x, basepos.y+1, basepos.z+1);
			glTexCoord2f(0,1);
			glNormal3f(0,0,-1);
			glVertexAttrib3f(vertex_attrib_index, 1,0,0);
			glVertex3f(basepos.x+1, basepos.y+1, basepos.z+1);
			glEnd();
		}
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
		displayed = true;
	}

	// check for other portals
//	f.draw();
	for (unsigned i = 0; i < portals.size(); ++i) {
		const portal& p = portals[i];
		// avoid portals facing away.
		// this means viewpos must be on the inner side of the portal plane.
		// fixme: if we are too close to a portal this is a problem and leads to bugs.
		// compare distance to portal and znear.
		// if we're beyond the portal, ignore it.
		// the problem still appears when we have four sectors with portals between each other, so they form one room.
		// if we step exactly on the corner between the four sectors (or maybe the problem exists already with three),
		// sometimes nothing is drawn...
		// if we're too close to a portal, render both sectors, but avoid re-visit (or infinite loops will result).
		plane portal_plane = p.shape.get_plane();
		double dist_to_portal = portal_plane.distance(f.viewpos);
		//cout << "viewpos dist: " << portal_plane.distance(f.viewpos) << "\n";
		if (fabs(dist_to_portal) < f.znear) {
			// we're to close to the portal.
			// render both adjacent sectors with frustum f, but only if they haven't been visited.
			// we already rendered this sector, so render other sector
			if (!p.adj_sector->visited) {
				p.adj_sector->display(f);
			}
		} else if (portal_plane.is_left(f.viewpos)) {
			polygon newfpoly = f.clip(p.shape);
			if (!newfpoly.empty()) {
				frustum fnew(newfpoly, f.viewpos, f.znear);
				p.adj_sector->display(fnew);
			}
		}
	}
	visited = false;
}



bool sector::check_movement(const vector3& currpos, const vector3& nextpos, sector*& nextseg) const
{
	// we assume that curros is inside this sector.
	// check for crossed portal.
	for (unsigned i = 0; i < portals.size(); ++i) {
		plane pl = portals[i].shape.get_plane();
		if (pl.test_side(nextpos) <= 0) {
			// we crossed the plane of that portal, switch sector.
			nextseg = portals[i].adj_sector;
			return false;
		}
	}
	// the value needs to be greater than the largest distance between the viewer's position and one of the
	// corner points of the near rectangle of the viewing frustum. assume sqrt(3*znear^2) or so...
	// znear=0.1 -> dist2wall = 0.173...
	const double DIST2WALL = 0.175;
	nextseg = 0;
	if (nextpos.x < basepos.x + DIST2WALL && (walls & 4)) return false;
	if (nextpos.x > basepos.x + 1 - DIST2WALL && (walls & 8)) return false;
	if (nextpos.y < basepos.y + DIST2WALL && (walls & 2)) return false;
	if (nextpos.y > basepos.y + 1 - DIST2WALL && (walls & 1)) return false;
	if (nextpos.z < basepos.z + DIST2WALL && (walls & 16)) return false;
	if (nextpos.z > basepos.z + 1 - DIST2WALL && (walls & 32)) return false;
	return true;
}



void line(const vector3& a, const vector3& b)
{
	glVertex3dv(&a.x);
	glVertex3dv(&b.x);
}



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

	metalbackgr = new texture(get_texture_dir() + "foam.png", texture::LINEAR_MIPMAP_LINEAR);
	woodbackgr = new texture(get_texture_dir() + "wooden_desk.png", texture::LINEAR_MIPMAP_LINEAR);
	terraintex = new texture(get_texture_dir() + "terrain.jpg", texture::LINEAR_MIPMAP_LINEAR);

	stonewall_diffuse = new texture(get_texture_dir() + "stonewall_diffuse.jpg", texture::LINEAR_MIPMAP_LINEAR);
	stonewall_bump = new texture(get_texture_dir() + "stonewall_bump.png", texture::LINEAR_MIPMAP_LINEAR);
	reliefwall_diffuse = new texture(get_texture_dir() + "reliefwall_diffuse.jpg", texture::LINEAR_MIPMAP_LINEAR);
	reliefwall_bump = new texture(get_texture_dir() + "reliefwall_bump.png", texture::LINEAR_MIPMAP_LINEAR);
	glsl_reliefmapping = new glsl_shader_setup(get_shader_dir() + "reliefmapping.vshader",
						   get_shader_dir() + "reliefmapping.fshader");
	glsl_reliefmapping->use();
	loc_tex_normal = glsl_reliefmapping->get_uniform_location("tex_normal");
	loc_tex_color = glsl_reliefmapping->get_uniform_location("tex_color");
	vertex_attrib_index = glsl_reliefmapping->get_vertex_attrib_index("tangentx");
	loc_depth_factor = glsl_reliefmapping->get_uniform_location("depth_factor");
	glsl_shader_setup::use_fixed();

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

	double tm0 = sys().millisec();
	int mv_forward = 0, mv_upward = 0, mv_sideward = 0;

	fpsmeasure fpsm(1.0f);

	while (true) {
		double tm1 = sys().millisec();
		double delta_t = tm1 - tm0;
		tm0 = tm1;

		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		glBindTexture(GL_TEXTURE_2D, 0);

		// compute mvp etc. for user
		glLoadIdentity();
		// make camera look to pos. y-axis.
		glRotatef(-90, 1, 0, 0);

		glRotatef(-viewangles.x, 1, 0, 0);
		glRotatef(-viewangles.y, 0, 1, 0);
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
		polygon viewwindow(wbln, wbrn, wtrn, wtln);
		frustum viewfrustum(viewwindow, pos, 0.1 /* fixme: read from matrix! */);

		// set light
		vector3 ld(cos((sys().millisec()%10000)*2*3.14159/10000), sin((sys().millisec()%10000)*2*3.14159/10000), 1.0);
		ld.normalize();
		GLfloat lposition[4] = {ld.x,ld.y,ld.z,0};
		glLightfv(GL_LIGHT0, GL_POSITION, lposition);

		// render sectors.
		for (unsigned i = 0; i < sectors.size(); ++i)
			sectors[i].displayed = false;
		currsector->display(viewfrustum);
		glsl_shader_setup::use_fixed();

		vector3 oldpos = pos;
		const double movesc = 0.25;
		list<SDL_Event> events = sys().poll_event_queue();
		vector3 forward = -invmvr.column3(2) * movesc;
		vector3 upward = invmvr.column3(1) * movesc;
		vector3 sideward = invmvr.column3(0) * movesc;
		for (list<SDL_Event>::iterator it = events.begin(); it != events.end(); ++it) {
			SDL_Event& event = *it;
			if (event.type == SDL_KEYDOWN) {
				switch (event.key.keysym.sym) {
				case SDLK_ESCAPE:
					return;
				case SDLK_KP4: mv_sideward = -1; break;
				case SDLK_KP6: mv_sideward = 1; break;
				case SDLK_KP8: mv_upward = 1; break;
				case SDLK_KP2: mv_upward = -1; break;
				case SDLK_KP1: mv_forward = 1; break;
				case SDLK_KP3: mv_forward = -1; break;
				default: break;
				}
			} else if (event.type == SDL_KEYUP) {
				switch (event.key.keysym.sym) {
				case SDLK_KP4: mv_sideward = 0; break;
				case SDLK_KP6: mv_sideward = 0; break;
				case SDLK_KP8: mv_upward = 0; break;
				case SDLK_KP2: mv_upward = 0; break;
				case SDLK_KP1: mv_forward = 0; break;
				case SDLK_KP3: mv_forward = 0; break;
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
		const double move_speed = 0.003;
		pos += forward * mv_forward * delta_t * move_speed
			+ sideward * mv_sideward * delta_t * move_speed
			+ upward * mv_upward * delta_t * move_speed;

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

		// record fps
		float fps = fpsm.account_frame();

		sys().prepare_2d_drawing();
		std::ostringstream oss; oss << "FPS: " << fps << "\n(all time total " << fpsm.get_total_fps() << ")";
		font_arial->print(0, 0, oss.str());
		sys().unprepare_2d_drawing();
		
		sys().swap_buffers();
	}

	delete glsl_reliefmapping;
	delete reliefwall_bump;
	delete reliefwall_diffuse;
	delete stonewall_bump;
	delete stonewall_diffuse;

	delete metalbackgr;
	delete woodbackgr;
	delete terraintex;
}
