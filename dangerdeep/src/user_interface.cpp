// user interface common code
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL/SDL.h>
#endif
#include <sstream>
#include "user_interface.h"
#include "system.h"
#include "game.h"
#include "texts.h"
#include "sound.h"
#include "logbook.h"
#include "model.h"
#include "airplane.h"
#include "depth_charge.h"
#include "gun_shell.h"
#include "water_splash.h"
#include "ships_sunk_display.h"

#define MAX_PANEL_SIZE 256

#define WAVE_PHASES 128		// no. of phases for wave animation
#define WAVES_PER_AXIS 16	// no. of waves along x or y axis
#define FACES_PER_WAVE 4	// resolution of wave model in x/y dir.
#define WAVE_LENGTH 40.0	// in meters, total length of one wave (one sine function)
#define WAVE_HEIGHT 2.0		// half of difference top/bottom of wave -> fixme, depends on weather
#define TIDECYCLE_TIME 8.0

#define CLOUD_ANIMATION_CYCLE_TIME 3600.0

/*
	a note on our coordinate system (11/10/2003):
	We simulate earth by projecting objects according to curvature from earth
	space to Euclidian space. This projection is yet a identity projection, that means
	we ignore curvature yet.
	The map forms a cylinder around the earth, that means x,y position on the map translates
	to longitude,latitude values. Hence valid coordinates go from -20000km...20000km in x
	direction and -10000km to 10000km in y direction. (we could use exact values, around
	20015km). The wrap around is a problem, but that's somewhere in the Pacific ocean, so
	we just ignore it. This mapping leads to some distorsion and wrong distance values
	when coming to far north or south on the globe. We just ignore this for simplicities
	sake. The effect should'nt be noticeable.
*/

user_interface::user_interface() :
	quit(false), pause(false), time_scale(1), player_object(0),
	panel_height(128), panel_visible(true), bearing(0),
	viewmode(4), target(0),	zoom_scope(false), mapzoom(0.1), viewsideang(0),
	viewupang(-90),	viewpos(0, 0, 10)
{
	clouds = 0;
	init ();
}

user_interface::user_interface(sea_object* player) :
	quit(false), pause(false), time_scale(1), player_object ( player ),
	panel_height(128), panel_visible(true), bearing(0),
	viewmode(4), target(0), zoom_scope(false), mapzoom(0.1), viewsideang(0),
	viewupang(-90),	viewpos(0, 0, 10)
{
	clouds = 0;
	init ();
}

user_interface::~user_interface ()
{
	deinit();
}

void user_interface::init ()
{
	// if the constructors of these classes may ever fail, we should
	// use C++ exceptions.
	captains_logbook = new captains_logbook_display;
//	system::sys()->myassert ( captains_logbook != 0, "Error while creating captains_logbook!" );
	ships_sunk_disp = new ships_sunk_display;
//	system::sys()->myassert ( ships_sunk_disp != 0, "Error while creating ships_sunk!" );

//fixme: create normals too!!!! (for correct lighting)
//that is a problem because we scale the result later and thus can't precompute normals,
//they're screwed up by scaling. Enable auto normaling? But the display lists have borders.
//what does auto normaling there? recreate Lists when wave height/length changes and don't
//scale?!
	// create and fill display lists for water
	/*
		this model leads to waves rolling from NE to SW.
		We could determine wind direction or tide related water movement and
		simulate wave rolling accordingly. But this would made a recomputation
		of this display lists necessary, whenever the wind or tide changes.
		That is very costly.
		Another idea: rotate the waves before displaying according to wind/water
		movement direction and let get_water_height/get_water_normal functions
		use the rotation, too. fixme
	*/
	wavedisplaylists = glGenLists(WAVE_PHASES);
	system::sys()->myassert(wavedisplaylists != 0, "no more display list indices available");
	for (unsigned i = 0; i < WAVE_PHASES; ++i) {
		glNewList(wavedisplaylists+i, GL_COMPILE);
		glBegin(GL_QUADS);
		vector<vector2f> csv(FACES_PER_WAVE+1);
		for (unsigned j = 0; j <= FACES_PER_WAVE; ++j) {
			csv[j].x = float(j)/FACES_PER_WAVE;
			csv[j].y = sin(2.0*M_PI*(float(i)/WAVE_PHASES+float(j)/FACES_PER_WAVE));
		}
		for (unsigned y = 0; y < FACES_PER_WAVE; ++y) {
			for (unsigned x = 0; x < FACES_PER_WAVE; ++x) {
				glTexCoord2f(csv[x].x, csv[y].x);
				glVertex3f(csv[x].x, csv[y].x, csv[x].y*csv[y].y);
				glTexCoord2f(csv[x+1].x, csv[y].x);
				glVertex3f(csv[x+1].x, csv[y].x, csv[x+1].y*csv[y].y);
				glTexCoord2f(csv[x+1].x, csv[y+1].x);
				glVertex3f(csv[x+1].x, csv[y+1].x, csv[x+1].y*csv[y+1].y);
				glTexCoord2f(csv[x].x, csv[y+1].x);
				glVertex3f(csv[x].x, csv[y+1].x, csv[x].y*csv[y+1].y);
			}
		}
		glEnd();
		glEndList();
	}
	
	// load and init map
	mapw = maph = 0;
	maprealw = 0;
	ifstream in("1.map", ios::in | ios::binary);
	if (in.good()) {	// does file exist?
		mapw = read_u16(in);
		maph = read_u16(in);
		maprealw = read_double(in);
		mappos.x = read_double(in);
		mappos.y = read_double(in);
		mapmaxpos.x = mappos.x + maprealw;
		mapmaxpos.y = mappos.y + maprealw * double(maph)/double(mapw);
		mapmperpixel = maprealw / double(mapw);
//		landsea.resize(mapw*maph);
//		for (unsigned i = 0; i < mapw*maph; ++i)
//			landsea[i] = read_u8(in);
		unsigned s = read_u32(in);
		for ( ; s > 0; --s)
			coastlines.push_back(coastline(in));
	}
	
	// init clouds
	// clouds are generated with Perlin noise.
	// one big texture is rendered (1024x1024, could be dynamic) and distributed
	// over 4x4 textures.
	// We generate m levels of noise maps, m <= n, texture width = 2^n.
	// here n = 10.
	// Each level is a noise map of 2^(n-m+1) pixels wide and high, scaled to full size.
	// For m = 4 we have 4 maps, 128x128 pixel each, scaled to 1024x1024, 512x512, 256x256
	// and 128x128.
	// The maps are tiled and added on a 1024x1024 map (the result), with descending
	// factors, that means level m gives factor 1/(2^(m-1)).
	// So we add the 4 maps: 1*map0 + 1/2*map1 + 1/4*map2 + 1/8*map3.
	// To animate clouds, just interpolate one level's noise map between two random
	// noise maps.
	// The result s is recomputed: cover (0-255), sharpness (0-255)
	// clamp(clamp_at_zero(s - cover) * sharpness)
	// This is used as alpha value for the texture.
	// 0 = no clouds, transparent, 255 full cloud (white/grey)
	// The final map is mapped to a hemisphere:
	// Texture coords = height_in_sphere * cos/sin(direction_in_sphere).
	// That means a circle with radius 512 of the original map is used.

	cloud_levels = 5;
	cloud_coverage = 128;	// 0-256 (none-full)
	cloud_sharpness = 256;	// 0-256
	cloud_animphase = 0;

	noisemaps_0 = compute_noisemaps();
	noisemaps_1 = compute_noisemaps();
	compute_clouds();

	// create sky hemisphere display list
	unsigned skyvsegs = 16;
	unsigned skyhsegs = 4*skyvsegs;
	clouds_dl = glGenLists(1);
	glNewList(clouds_dl, GL_COMPILE);
	glBegin(GL_QUADS);
	glTexCoord2f(0,1);
	glVertex3f(-1,1,1);
	glTexCoord2f(1,1);
	glVertex3f(1,1,1);
	glTexCoord2f(1,0);
	glVertex3f(1,-1,1);
	glTexCoord2f(0,0);
	glVertex3f(-1,-1,1);
/*
	for (unsigned beta = 0; beta < skyvsegs; ++beta) {
		float t = (1.0-float(beta)/skyvsegs)/2;
		float t2 = (1.0-float(beta+1)/skyvsegs)/2;
		float r = cos(M_PI/2*beta/skyvsegs)/2;
		float h = sin(M_PI/2*beta/skyvsegs)/2;
		float r2 = cos(M_PI/2*(beta+1)/skyvsegs)/2;
		float h2 = sin(M_PI/2*(beta+1)/skyvsegs)/2;
		for (unsigned alpha = 0; alpha < skyhsegs; ++alpha) {
			float x = cos(2*M_PI*alpha/skyhsegs);
			float y = sin(2*M_PI*alpha/skyhsegs);
			float x2 = cos(2*M_PI*(alpha+1)/skyhsegs);
			float y2 = sin(2*M_PI*(alpha+1)/skyhsegs);
			glTexCoord2f(x*t+0.5, y*t+0.5);
			glVertex3f(x*r, y*r, h);
			glTexCoord2f(x2*t+0.5, y2*t+0.5);
			glVertex3f(x2*r, y2*r, h);
			glTexCoord2f(x2*t2+0.5, y2*t2+0.5);
			glVertex3f(x2*r2, y2*r2, h2);
			glTexCoord2f(x*t2+0.5, y*t2+0.5);
			glVertex3f(x*r2, y*r2, h2);
		}
	}
*/	
	glEnd();
	glEndList();

}

void user_interface::deinit ()
{
	delete captains_logbook;
	delete ships_sunk_disp;
	
	delete clouds;
	glDeleteLists(clouds_dl, 1);

	// delete display lists for water
	glDeleteLists(wavedisplaylists, WAVE_PHASES);
}

void user_interface::advance_cloud_animation(float fac)
{
	int oldphase = int(cloud_animphase*256);
	cloud_animphase += fac;
	int newphase = int(cloud_animphase*256);
	if (cloud_animphase >= 1.0) {
		cloud_animphase -= 1.0;
		noisemaps_0 = noisemaps_1;
		noisemaps_1 = compute_noisemaps();
	} else {
		if (newphase > oldphase)
			compute_clouds();
	}
}

void user_interface::compute_clouds(void)
{
	unsigned mapsize = 8 - cloud_levels;
	unsigned mapsize2 = (2<<mapsize);

	vector<vector<Uint8> > cmaps = noisemaps_0;
	float f = cloud_animphase;
	for (unsigned i = 0; i < cloud_levels; ++i)
		for (unsigned j = 0; j < mapsize2 * mapsize2; ++j)
			cmaps[i][j] = Uint8(noisemaps_0[i][j]*(1-f) + noisemaps_1[i][j]*f);

	// create full map
	vector<Uint8> fullmap(256 * 256 * 4);
	unsigned fullmapptr = 0;
	for (unsigned y = 0; y < 256; ++y) {
		for (unsigned x = 0; x < 256; ++x) {
			unsigned v = 0;
			// accumulate values
			for (unsigned k = 0; k < cloud_levels; ++k) {
				unsigned tv = get_value_from_bytemap(x, y, cloud_levels-1-k, mapsize2, cmaps[k]);
				v += (tv >> k);
			}
			if (v > 255) v = 255;
			if (v < (256 - cloud_coverage))
				v = 0;
			else
				v -= (256 - cloud_coverage);
			// use sharpness for exp function
			v = 255 - v * 256 / cloud_coverage;	// equalize
			fullmap[fullmapptr++] = 255;
			fullmap[fullmapptr++] = 255;
			fullmap[fullmapptr++] = 255;
			fullmap[fullmapptr++] = v;
		}
	}

	delete clouds;	
	clouds = new texture(&fullmap[0], 256, 256, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE,
		GL_LINEAR, GL_CLAMP);
}

vector<vector<Uint8> > user_interface::compute_noisemaps(void)
{
	unsigned mapsize = 8 - cloud_levels;
	unsigned mapsize2 = (2<<mapsize);
	vector<vector<Uint8> > noisemaps(cloud_levels);
	for (unsigned i = 0; i < cloud_levels; ++i) {
		noisemaps[i].resize(mapsize2 * mapsize2);
		for (unsigned j = 0; j < mapsize2 * mapsize2; ++j)
			noisemaps[i][j] = (unsigned char)(255*rnd());
//		smooth_and_equalize_bytemap(mapsize2, noisemaps[i]);
	}
	return noisemaps;
}

Uint8 user_interface::get_value_from_bytemap(unsigned x, unsigned y, unsigned level,
	unsigned s, const vector<Uint8>& nmap)
{
	unsigned rest = (1<<level);
	unsigned xr = x % rest;
	unsigned yr = y % rest;
	x = (x >> level) % s;
	y = (y >> level) % s;
	unsigned x2 = (x+1) % s;
	unsigned y2 = (y+1) % s;
	unsigned v0 = nmap[y*s+x];
	unsigned v1 = nmap[y*s+x2];
	unsigned v2 = nmap[y2*s+x];
	unsigned v3 = nmap[y2*s+x2];
	unsigned v4 = (v0*(rest-xr)+v1*xr);
	unsigned v5 = (v2*(rest-xr)+v3*xr);
	unsigned v6 = (v4*(rest-yr)+v5*yr);
	return v6 / (rest*rest);
}

void user_interface::smooth_and_equalize_bytemap(unsigned s, vector<Uint8>& map)
{
	vector<Uint8> map2 = map;
	unsigned maxv = 0, minv = 255;
	for (unsigned y = 0; y < s; ++y) {
		unsigned y1 = (y+s-1)%s, y2 = (y+1)%s;
		for (unsigned x = 0; x < s; ++x) {
			unsigned x1 = (x+s-1)%s, x2 = (x+1)%s;
			unsigned v = (unsigned(map2[y1*s+x]) + unsigned(map2[y*s+x1]) + unsigned(map2[y*s+x]) + unsigned(map2[y*s+x2]) + unsigned(map2[y2*s+x])) / 5;
			map[y*s+x] = Uint8(v);
			if (v < minv) minv = v;
			if (v > maxv) maxv = v;
		}
	}
	for (unsigned y = 0; y < s; ++y) {
		for (unsigned x = 0; x < s; ++x) {
			unsigned v = map[y*s+x];
			map[y*s+x] = Uint8((v - minv)*255/(maxv-minv));
		}
	}
}

/* 2003/07/04 idea.
   simulate earth curvature by drawing several horizon faces
   approximating the curvature.
   earth has medium radius of 6371km, that means 40030km around it.
   A ship with 15m height above the waterline disappears behind
   the horizon at ca. 13.825km distance (7.465 sm)
   
   exact value 40030.17359km. (u), earth radius (r)
   
   height difference in view: (h), distance (d). Formula:
   
   h = r * (1 - cos( 360deg * d / u ) )
   
   or
   
   d = arccos ( 1 - h / r ) * u / 360deg
   
   draw ships with height -h. so (dis)appearing of ships can be
   simulated properly.
   
   highest ships are battleships (approx. 30meters), they disappear
   at 19.551km (10.557 sm).
   
   That's much shorter than I thought! But there is a mistake:
   The viewer's height is not 0 but around 6-8m for submarines,
   so the formulas are more difficult:
   
   The real distance is twice the formula, once for the viewer's
   height, once for the object:
   
   d = (arccos(1 - myh/r) + arccos(1 - h/r)) * u / 360deg
   
   or for the watched object
   
   h = r * (1 - cos( 360deg * (d - (arccos(1 - myh/r)) / u ) )
   
   so for a watcher in 6m height and other ships we have
   arccos(1-myh/r) = 0.07863384deg
   15m in height -> dist: 22.569km (12.186sm)
   30m in height -> dist: 28.295km (15.278sm)
   
   This values are useful for computing "normal" simulation's
   maximum visibility.
   Waves are disturbing sight but are ignored here.
*/	   

// ------------------------------------- water --------------------------------
/*
	The water surface is given by the following implicit function:
	z = WAVE_HEIGHT * sin(2*PI*(x/WAVE_LENGTH+t)) * sin(2*PI*(y/WAVE_LENGTH+t))
	with t is a time factor in [0...1) that determines tide rise and fall speed.
	That means that the water surface is made of one tile repeated infinitly.
	This tile is WAVE_LENGTH * WAVE_LENGTH in size and contains two waves.
	(Function sin(x)*sin(y) for x,y in [0...2PI] )
	The normal of the surface at any position x,y is thus given by:
		Tangential plane is spanned by
			(1, 0, dz/dx), (0, 1, dz/dy) with
			b = WAVEHEIGHT, a = 2*PI/WAVE_LENGTH, c=2*PI
			z = b * sin(a*x+c*t) * sin(a*y+c*t)
			dz/dx = b * a * cos(a*x+c*t) * sin(a*y+c*t)
			dz/dy = b * a * sin(a*x+c*t) * cos(a*y+c*t)
		Cross product gives normal:
			(-dz/dx, -dz/dy, 1)
	Total: (-b * a * cos(a*x+c*t) * sin(a*y+c*t), -b * a * sin(a*x+c*t) * cos(a*y+c*t), 1)
	The normal is needed to simulate ship rolling.
*/	

void user_interface::rotate_by_pos_and_wave(const vector3& pos, double timefac,
	double rollfac, bool inverse) const
{
	vector3 rz = get_water_normal(pos.xy(), timefac, rollfac);
	vector3 rx = vector3(1, 0, -rz.x).normal();
	vector3 ry = vector3(0, 1, -rz.y).normal();
	if (inverse) {
		double mat[16] = {
			rx.x, ry.x, rz.x, 0,
			rx.y, ry.y, rz.y, 0,
			rx.z, ry.z, rz.z, 0,
			0,    0,    0,    1 };
		glMultMatrixd(&mat[0]);
	} else {
		double mat[16] = {
			rx.x, rx.y, rx.z, 0,
			ry.x, ry.y, ry.z, 0,
			rz.x, rz.y, rz.z, 0,
			0,    0,    0,    1 };
		glMultMatrixd(&mat[0]);
	}
}

double user_interface::get_water_height(const vector2& pos, double t) const
{
	double b = WAVE_HEIGHT;
	double a = 2.0*M_PI/WAVE_LENGTH;
	double x = myfmod(pos.x, WAVE_LENGTH);
	double y = myfmod(pos.y, WAVE_LENGTH);
	double pt = 2.0*M_PI*t;
	return b * sin(a*x+pt) * sin(a*y+pt);
}

vector3 user_interface::get_water_normal(const vector2& pos, double t, double f) const
{
	double b = WAVE_HEIGHT * f;
	double a = 2.0*M_PI/WAVE_LENGTH;
	double x = myfmod(pos.x, WAVE_LENGTH);
	double y = myfmod(pos.y, WAVE_LENGTH);
	double pt = 2.0*M_PI*t;
	return vector3(-b * a * cos(a*x+pt) * sin(a*y+pt), -b * a * sin(a*x+pt) * cos(a*y+pt), 1).normal();
}

void user_interface::draw_water(const vector3& viewpos, angle dir, double t,
	double max_view_dist) const
{

	glPushMatrix();
	glTranslatef(-myfmod(viewpos.x, WAVE_LENGTH), -myfmod(viewpos.y, WAVE_LENGTH), -viewpos.z);

	// fixme use animated water texture(s)

	// draw polygons to the horizon
	float wr = WAVES_PER_AXIS * WAVE_LENGTH / 2;
	float c0 = -max_view_dist;
	float c1 = -wr;
	float c2 = wr;
	float c3 = max_view_dist;
	float t0 = c0/64;
	float t1 = c1/64;
	float t2 = c2/64;
	float t3 = c3/64;
	float wz = 0;//-WAVE_HEIGHT; // this leads to hole between waves and horizon faces, fixme

	glBindTexture(GL_TEXTURE_2D, water->get_opengl_name());
	glBegin(GL_TRIANGLE_STRIP);
	glTexCoord2f(t0,t3);
	glVertex3f(c0,c3,0);
	glTexCoord2f(t1,t2);
	glVertex3f(c1,c2,wz);
	glTexCoord2f(t3,t3);
	glVertex3f(c3,c3,0);
	glTexCoord2f(t2,t2);
	glVertex3f(c2,c2,wz);
	glTexCoord2f(t3,t0);
	glVertex3f(c3,c0,0);
	glTexCoord2f(t2,t1);
	glVertex3f(c2,c1,wz);
	glTexCoord2f(t0,t0);
	glVertex3f(c0,c0,0);
	glTexCoord2f(t1,t1);
	glVertex3f(c1,c1,wz);
	glTexCoord2f(t0,t3);
	glVertex3f(c0,c3,0);
	glTexCoord2f(t1,t2);
	glVertex3f(c1,c2,wz);
	glEnd();

	// draw waves
	glScalef(WAVE_LENGTH, WAVE_LENGTH, WAVE_HEIGHT);
	glBindTexture(GL_TEXTURE_2D, water->get_opengl_name());
	unsigned dl = wavedisplaylists + int(WAVE_PHASES*t);
	for (int y = 0; y < WAVES_PER_AXIS; ++y) {
		for (int x = 0; x < WAVES_PER_AXIS; ++x) {
			glPushMatrix();
			glTranslatef(-WAVES_PER_AXIS/2+x, -WAVES_PER_AXIS/2+y, 0);
			glCallList(dl);
			glPopMatrix();
		}
	}
	glPopMatrix();
}

void user_interface::draw_terrain(const vector3& viewpos, angle dir,
	double max_view_dist) const
{
#if 0
	double dist = max_view_dist;
	vector2 minp(viewpos.x - dist, viewpos.y - dist);
	vector2 maxp(viewpos.x + dist, viewpos.y + dist);
//cout << "minp " << minp << "\n";
//cout << "maxp " << maxp << "\n";
	vector2 drawmax = mapmaxpos.min(maxp), drawmin = mappos.max(minp);
//cout << "mappos " << mappos << "\n";	
//cout << "mapmaxpos " << mapmaxpos << "\n";	
//cout << "drawmax " << drawmax << "\n";	
//cout << "drawmin " << drawmin << "\n";	
	vector2 drawarea = drawmax - drawmin;
//cout << "draw map area double " << drawmin.x << "," << drawmin.y << "," << drawmax.x << "," << drawmax.y << "\n";
	vector2 mapoffset = drawmin - mappos;
	int minx = mapoffset.x / mapmperpixel;
	int miny = mapoffset.y / mapmperpixel;
	int maxx = minx + drawarea.x / mapmperpixel;
	int maxy = miny + drawarea.y / mapmperpixel;
//cout << "draw map area " << minx << "," << miny << "," << maxx << "," << maxy << "\n";
	if (minx < 1) minx = 1;
	if (miny < 1) miny = 1;
	if (maxx >= mapw-1) maxx = mapw-1;
	if (maxy >= maph-1) maxy = maph-1;
#endif
	/* the top vertices are translated along the negative normal of the polyline.
		that gives an ascending shape for the coast */
	glPushMatrix();
	terraintex->set_gl_texture();
	float cls = mapmperpixel;
	glTranslatef((mappos.x-viewpos.x), (-mappos.y-viewpos.y), -viewpos.z);
	glScalef(cls, cls, 1);
	for (list<coastline>::const_iterator it = coastlines.begin(); it != coastlines.end(); ++it) {
		unsigned ps = it->points.size();
		unsigned prevpt = ps-1;
		unsigned thispt = 0;
		unsigned nextpt = 1;
		glBegin(GL_QUAD_STRIP);
		double coastheight = 100 + double(ps > 1000 ? 1000 : ps) * 0.2;
		float t = 0.0;
		for (unsigned s = 0; s < ps; ++s) {
			const vector2f& p = it->points[thispt];
			vector2f n = (it->points[nextpt] - it->points[prevpt]).orthogonal().normal() * -3.0;
			if (s > 0) {
				glTexCoord2f(t, 1);
				glVertex3f(p.x, p.y, -10);
				glTexCoord2f(t, 0);
				glVertex3f(p.x+n.x, p.y+n.y, coastheight);
			} else {
				glTexCoord2f(t, 0);
				glVertex3f(p.x+n.x, p.y+n.y, coastheight);
				glTexCoord2f(t, 1);
				glVertex3f(p.x, p.y, -10);
			}
			t += 1.0;
			prevpt = thispt;
			thispt = nextpt;
			nextpt = (nextpt+1)%ps;
		}
		glEnd();
	}
	glPopMatrix();
	
}

void user_interface::draw_view(class system& sys, class game& gm, const vector3& viewpos,
	angle dir, bool aboard, bool drawbridge, bool withunderwaterweapons)
{
	double max_view_dist = gm.get_max_view_distance();

	sea_object* player = get_player();

	float cf = myfmod(gm.get_time(), CLOUD_ANIMATION_CYCLE_TIME)/CLOUD_ANIMATION_CYCLE_TIME - cloud_animphase;
	if (cf < 0) cf += 1.0;
	advance_cloud_animation(cf);

	double timefac = myfmod(gm.get_time(), TIDECYCLE_TIME)/TIDECYCLE_TIME;
	
	glRotatef(-90,1,0,0);
	// if we're aboard the player's vessel move the world instead of the ship
	if (aboard) {
		double rollfac = (dynamic_cast<ship*>(player))->get_roll_factor();
		rotate_by_pos_and_wave(player->get_pos(), timefac, rollfac, true);
	}

	// This should be a negative angle, but nautical view dir is clockwise, OpenGL uses ccw values, so this is a double negation
	glRotatef(dir.value(),0,0,1);

	// ************ sky ***************************************************************
	glPushMatrix();
	glTranslatef(0, 0, -viewpos.z);
	glPushMatrix();
	glScalef(max_view_dist, max_view_dist, max_view_dist);	// fixme dynamic
	double dt = get_day_time(gm.get_time());
	color skycol1, skycol2, lightcol;
	double colscal;
	if (dt < 1) { colscal = 0; }
	else if (dt < 2) { colscal = fmod(dt,1); }
	else if (dt < 3) { colscal = 1; }
	else { colscal = 1-fmod(dt,1); }
	lightcol = color(color(64, 64, 64), color(255,255,255), colscal);
	skycol1 = color(color(8,8,32), color(165,192,247), colscal);
	skycol2 = color(color(0, 0, 16), color(74,114,236), colscal);

	// compute light source position and brightness
	GLfloat lambient[4] = {0.2, 0.2, 0.2, 1};//lightcol.r/255.0/2.0, lightcol.g/255.0/2.0, lightcol.b/255.0/2.0, 1};
	GLfloat ldiffuse[4] = {lightcol.r/255.0, lightcol.g/255.0, lightcol.b/255.0, 1};
	GLfloat lposition[4] = {0,0,1,0};	//fixed for now. fixme
	glLightfv(GL_LIGHT1, GL_AMBIENT, lambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, ldiffuse);
	glLightfv(GL_LIGHT1, GL_POSITION, lposition);

	glDisable(GL_LIGHTING);
	skycol2.set_gl_color();
	float tmp = 1.0/30000.0;
	glScalef(tmp, tmp, tmp);	// sky hemisphere is stored as 30km in radius
	skyhemisphere->display();
	color::white().set_gl_color();
	glEnable(GL_LIGHTING);
	glPopMatrix();	// remove scale
	
	// ******** the sun and the moon **********
	glDisable(GL_LIGHTING);
	/* How to compute the sun's/moon's position:
	   Earth and moon rotate around the sun's y-axis and around their own y-axes.
	   The z-axis is pointing outward and the x-axis around the equator.
	   Earth's rotational axis differs by 23 degrees from sun's y-axis.
	   Given that knowledge one can write transformations between local spaces earth->sun, moon->sun,
	   viewer's position->earth. With them one can compute any transform between any object.
	   What is when all angles are zero?
	   1. The earth's rotational axis is pointing towards the sun then meaning we have the 21st. of June
	      That means we have to translate the zero position backwards to 1st. January.
	   2. Earth's coordinate system is just that of the sun translated by distance earth-sun along the z-axis.
	      That means the sun is at +-180 degrees W/E, exactly at the opposite of the null meridian, exact midnight.
	   3. The moon is exactly in line with sun and earth behind earth and at the southmost position relative to
	      earth's latitude coordinates.
	   Summary: Nullposition means top of summer, midnight and new moon (earth hides him) at southmost point.
	   We have to adjust sun position by 31+28+31+30+31+21=172 days back (ignoring that february may have 29 days)
	   The "easiest" thing would be to know where sun and moon were at 1st. january 1939, 00:00am.
	*/
	// fixme: adjust OpenGL light position to infinite in sun/moon direction.
	const double EARTH_RADIUS = 6378e3;
	const double SUN_RADIUS = 696e6;
	const double MOON_RADIUS = 1738e3;
	const double EARTH_SUN_DISTANCE = 149.6e9;
	const double MOON_EARTH_DISTANCE = 384.4e6;
	const double EARTH_ROT_AXIS_ANGLE = 23.45;
	const double MOON_ORBIT_TIME = 29.5306 * 86400.0;
	const double EARTH_ROTATION_TIME = 86400.0;
	const double EARTH_CIRCUMFERENCE = 2.0 * M_PI * EARTH_RADIUS;
	const double EARTH_ORBIT_TIME = 31556926.5;	// in seconds. 365 days, 5 hours, 48 minutes, 46.5 seconds
	// these values are difficult to get. SUN_POS_ADJUST should be around +9.8deg (10days of 365 later) but that gives
	// a roughly right position but wrong sun rise time by about 40min. fixme
	const double SUN_POS_ADJUST = 0;	// in degrees. 10 days from 21st. Dec. to 1st. Jan. * 360deg/365.24days
	const double MOON_POS_ADJUST = 300.0;	// in degrees. Moon pos in its orbit on 1.1.1939 fixme: this value is a rude guess
	double universaltime = gm.get_time();//    * 8640;	// fixme: substract local time

	// draw sun
	glPushMatrix();
	// Transform earth space to viewer space
	// to avoid mixing very differently sized floating point values, we scale distances before translation.
	double sun_scale_fac = max_view_dist / EARTH_SUN_DISTANCE;
	glTranslated(0, 0, -EARTH_RADIUS * sun_scale_fac);
	glRotated(360.0 * -viewpos.y * 4 / EARTH_CIRCUMFERENCE, 0, 1, 0);
	glRotated(360.0 * -viewpos.x * 2 / EARTH_CIRCUMFERENCE, 1, 0, 0);
	// Transform sun space to earth space
	glRotated(360.0 * -myfmod(universaltime, EARTH_ROTATION_TIME)/EARTH_ROTATION_TIME, 0, 1, 0);
	glRotated(-EARTH_ROT_AXIS_ANGLE, 1, 0, 0);
	glRotated(SUN_POS_ADJUST + 360.0 * myfmod(universaltime, EARTH_ORBIT_TIME)/EARTH_ORBIT_TIME, 0, 1, 0);
	glTranslated(0, 0, -EARTH_SUN_DISTANCE * sun_scale_fac * 0.96);	// to keep it inside sky hemisphere
	// draw quad
	double suns = SUN_RADIUS * sun_scale_fac;
	glColor3f(1,1,1);
	the_sun->set_gl_texture();
	glDisable(GL_LIGHTING);
	glBegin(GL_QUADS);
	glTexCoord2f(0,0);
	glVertex3f(-suns, -suns, 0);
	glTexCoord2f(1,0);
	glVertex3f(suns, -suns, 0);
	glTexCoord2f(1,1);
	glVertex3f(suns, suns, 0);
	glTexCoord2f(0,1);
	glVertex3f(-suns, suns, 0);
	glEnd();

	// this is not correct because of the shortened translation above.
	// instead take the transform matrix multiply with an vector (0,0,1,1) instead of last translation
	// to get direction of sun (4th. component is 0) fixme
//	float m[16];
//	glGetFloatv(GL_MODELVIEW_MATRIX, &m[0]);
//	glLoadIdentity();
	GLfloat lp[4] = {0,0,0,1};//{m[12], m[13], m[14], 0};	// light comes from sun (maybe use directional light)
//	glLightfv(GL_LIGHT1, GL_POSITION, lp);

	glPopMatrix();	// remove sun space

	// draw moon
	glPushMatrix();
	// Transform earth space to viewer space
	double moon_scale_fac = max_view_dist / MOON_EARTH_DISTANCE;
	glTranslated(0, 0, -EARTH_RADIUS * moon_scale_fac);
	glRotated(360.0 * -viewpos.y * 4 / EARTH_CIRCUMFERENCE, 0, 1, 0);
	glRotated(360.0 * -viewpos.x * 2 / EARTH_CIRCUMFERENCE, 1, 0, 0);
	// Transform moon space to earth space
	glRotated(360.0 * -myfmod(universaltime, EARTH_ROTATION_TIME)/EARTH_ROTATION_TIME, 0, 1, 0);
	glRotated(-EARTH_ROT_AXIS_ANGLE, 1, 0, 0);
	glRotated(MOON_POS_ADJUST + 360.0 * myfmod(universaltime, MOON_ORBIT_TIME)/MOON_ORBIT_TIME, 0, 1, 0);
	glTranslated(0, 0, MOON_EARTH_DISTANCE * moon_scale_fac * 0.95);	// to keep it inside sky hemisphere
	// draw quad	
	double moons = MOON_RADIUS * moon_scale_fac;
	glColor3f(1,1,1);
	the_moon->set_gl_texture();
	glBegin(GL_QUADS);
	glTexCoord2f(0,0);
	glVertex3f(-moons, -moons, 0);
	glTexCoord2f(0,1);
	glVertex3f(-moons, moons, 0);
	glTexCoord2f(1,1);
	glVertex3f(moons, moons, 0);
	glTexCoord2f(1,0);
	glVertex3f(moons, -moons, 0);
	glEnd();
	glPopMatrix();	// remove moon space
	glEnable(GL_LIGHTING);
	
	
	// ******** clouds *******
	glDisable(GL_LIGHTING);		// direct lighting turned off
	glDisable(GL_DEPTH_TEST);	// draw all clouds
	lightcol.set_gl_color();	// cloud color depends on day time

	float clsc = max_view_dist * 0.9;
	glScalef(clsc, clsc, 3000/*clsc fixme*/);
	glBindTexture(GL_TEXTURE_2D, clouds->get_opengl_name());
	glCallList(clouds_dl);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	color::white().set_gl_color();

	glPopMatrix();	// remove z translate for sky
	
	// modelview matrix is around viewpos now.

	// ********* fog test ************ fog color is skycol2
	GLfloat fog_color[4] = {skycol2.r/255.0, skycol2.g/255.0, skycol2.b/255.0, 1.0};
	glFogi(GL_FOG_MODE, GL_LINEAR );
	glFogfv(GL_FOG_COLOR, fog_color);
	glFogf(GL_FOG_DENSITY, 1.0);	// not used in linear mode
	glHint(GL_FOG_HINT, GL_NICEST /*GL_FASTEST*/ /*GL_DONT_CARE*/);
	glFogf(GL_FOG_START, max_view_dist*0.75);	// ships disappear earlier :-(
	glFogf(GL_FOG_END, max_view_dist);
	glEnable(GL_FOG);	

	// ******* water *********
					// fixme: program directional light caused by sun
					// or moon should be reflected by water.
	draw_water(viewpos, dir, timefac, max_view_dist);
	

	// ******** terrain/land *******
	
	draw_terrain(viewpos, dir, max_view_dist);


	// ******************** ships & subs *************************************************
	
	// rest of scene is displayed relative to world coordinates
	glTranslatef(-viewpos.x, -viewpos.y, -viewpos.z);

	list<ship*> ships;
	gm.visible_ships(ships, player);
	for (list<ship*>::const_iterator it = ships.begin(); it != ships.end(); ++it) {
		if (aboard && *it == player) continue;	// only ships or subs playable!
		glPushMatrix();
		glTranslatef((*it)->get_pos().x, (*it)->get_pos().y, (*it)->get_pos().z);
		glRotatef(-(*it)->get_heading().value(), 0, 0, 1);
		rotate_by_pos_and_wave((*it)->get_pos(), timefac, (*it)->get_roll_factor());
		(*it)->display();
		glPopMatrix();

		if ((*it)->has_smoke()) {
			double view_dir = 90.0f - angle ( (*it)->get_pos ().xy () - player->get_pos ().xy () ).value ();
			(*it)->smoke_display (view_dir);
		}
	}

	list<submarine*> submarines;
	gm.visible_submarines(submarines, player);
	for (list<submarine*>::const_iterator it = submarines.begin(); it != submarines.end(); ++it) {
		if (aboard && *it == player) continue; // only ships or subs playable!
		glPushMatrix();
		glTranslatef((*it)->get_pos().x, (*it)->get_pos().y, (*it)->get_pos().z);
		glRotatef(-(*it)->get_heading().value(), 0, 0, 1);
		if ((*it)->get_pos().z > -15) {
			rotate_by_pos_and_wave((*it)->get_pos(), timefac, (*it)->get_roll_factor());
		}
		(*it)->display();
		glPopMatrix();
	}

	list<airplane*> airplanes;
	gm.visible_airplanes(airplanes, player);
	for (list<airplane*>::const_iterator it = airplanes.begin(); it != airplanes.end(); ++it) {
		glPushMatrix();
		glTranslatef((*it)->get_pos().x, (*it)->get_pos().y, (*it)->get_pos().z);
		glRotatef(-(*it)->get_heading().value(), 0, 0, 1);	// simulate pitch, roll etc.
		(*it)->display();
		glPopMatrix();
	}

	if (withunderwaterweapons) {
		list<torpedo*> torpedoes;
		gm.visible_torpedoes(torpedoes, player);
		for (list<torpedo*>::const_iterator it = torpedoes.begin(); it != torpedoes.end(); ++it) {
			glPushMatrix();
			glTranslatef((*it)->get_pos().x, (*it)->get_pos().y, (*it)->get_pos().z);
			glRotatef(-(*it)->get_heading().value(), 0, 0, 1);
			(*it)->display();
			glPopMatrix();
		}
		list<depth_charge*> depth_charges;
		gm.visible_depth_charges(depth_charges, player);
		for (list<depth_charge*>::const_iterator it = depth_charges.begin(); it != depth_charges.end(); ++it) {
			glPushMatrix();
			glTranslatef((*it)->get_pos().x, (*it)->get_pos().y, (*it)->get_pos().z);
			glRotatef(-(*it)->get_heading().value(), 0, 0, 1);
			(*it)->display();
			glPopMatrix();
		}
	}

	list<gun_shell*> gun_shells;
	gm.visible_gun_shells(gun_shells, player);
	for (list<gun_shell*>::const_iterator it = gun_shells.begin(); it != gun_shells.end(); ++it) {
		glPushMatrix();
		glTranslatef((*it)->get_pos().x, (*it)->get_pos().y, (*it)->get_pos().z);
		glRotatef(-(*it)->get_heading().value(), 0, 0, 1);
		glScalef(10,10,10);//fixme: to control functionality for now
		(*it)->display();
		glPopMatrix();
	}

	list<water_splash*> water_splashs;
	gm.visible_water_splashes ( water_splashs, player );
	for ( list<water_splash*>::const_iterator it = water_splashs.begin ();
		it != water_splashs.end (); it ++ )
	{
		double view_dir = 90.0f - angle ( (*it)->get_pos ().xy () - player->get_pos ().xy () ).value ();
		glPushMatrix ();
		glTranslatef ( (*it)->get_pos ().x, (*it)->get_pos ().y, (*it)->get_pos ().z );
		glRotatef ( view_dir, 0.0f, 0.0f, 1.0f );
		(*it)->display ();
		glPopMatrix ();
	}

	if (aboard && drawbridge) {
		// after everything was drawn, draw conning tower (new projection matrix needed)
		glClear(GL_DEPTH_BUFFER_BIT);
//		glDisable(GL_DEPTH_TEST);
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		sys.gl_perspective_fovx (90.0, 4.0/3.0 /* fixme may change */, 0.5, 100.0);
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();
		glRotatef(-90,1,0,0);
		glRotatef((dir - player->get_heading()).value(),0,0,1);
		conning_tower_typeVII->display();
		glPopMatrix();
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
//		glEnable(GL_DEPTH_TEST);
	}
	
	glDisable(GL_FOG);	
	glColor3f(1,1,1);
}

bool user_interface::time_scale_up(void)
{
	if (time_scale < 4096) {
		time_scale *= 2;
		return true;
	}
	return false;
}

bool user_interface::time_scale_down(void)
{
	if (time_scale > 1) {
		time_scale /= 2;
		return true;
	}
	return false;
}

void user_interface::draw_infopanel(class system& sys, class game& gm) const
{
	int panel_border_pos = 768 - panelbackgr->get_height();
	int panel_pos = 768;
	if (panel_visible) {
		panel_border_pos -= panel_height;
		panel_pos -= panel_height;
		color::black().set_gl_color(128);
		sys.no_tex();
		sys.draw_rectangle(0, panel_pos, 1024, 768);
		color::white().set_gl_color(255);

		ostringstream os;
		os << texts::get(1) << ": " << get_player()->get_heading().ui_value()
			<< "\t" << texts::get(4) << ": "
			<< unsigned(fabs(round(sea_object::ms2kts(get_player()->get_speed()))))
			<< "\t" << texts::get(5) << ": "
			<< unsigned(round(-get_player()->get_pos().z))
			<< "\t" << texts::get(2) << ": "
			<< bearing.ui_value()
			<< "\t" << texts::get(98) << ": "
			<< time_scale;
		int fph = font_panel->get_height();	// should be 24.
		int y = 768 - fph;
		font_panel->print(0, y, os.str(), color::green(), true);
		y -= fph + (panel_height % fph);
		
		for (list<string>::const_reverse_iterator it = panel_texts.rbegin(); 
	             it != panel_texts.rend(); ++it) {
			font_panel->print(0, y, *it, color::white(), true);
			y -= fph;
			if (y < panel_pos) break;
		}
	}

	color::white().set_gl_color();			
	panelbackgr->draw(0, panel_border_pos, 1024, panel_pos-panel_border_pos);
}


texture* user_interface::torptex(unsigned type)
{
	switch (type) {
		case torpedo::T1: return torpt1;
		case torpedo::T2: return torpt2;
		case torpedo::T3: return torpt3;
		case torpedo::T3a: return torpt3a;
		case torpedo::T4: return torpt4;
		case torpedo::T5: return torpt5;
		case torpedo::T11: return torpt11;
		case torpedo::T1FAT: return torpt1fat;
		case torpedo::T3FAT: return torpt3fat;
		case torpedo::T6LUT: return torpt6lut;
	}
	return torpempty;
}

void user_interface::draw_gauge(class system& sys, class game& gm,
	unsigned nr, int x, int y, unsigned wh, angle a, const string& text, angle a2) const
{
	set_display_color ( gm );
	switch (nr) {
		case 1:	gauge1->draw(x, y, wh, wh); break;
		case 2:	gauge2->draw(x, y, wh, wh); break;
		case 3:	gauge3->draw(x, y, wh, wh); break;
		case 4:	gauge4->draw(x, y, wh, wh); break;
		default: return;
	}
	vector2 d = a.direction();
	int xx = x+wh/2, yy = y+wh/2;
	pair<unsigned, unsigned> twh = font_arial->get_size(text);

	color font_color ( 255, 255, 255 );
	if ( !gm.is_day_mode () )
		font_color = color ( 255, 127, 127 );

	font_arial->print(xx-twh.first/2, yy-twh.second/2, text, font_color);
	glBindTexture(GL_TEXTURE_2D, 0);
	if (a2 != a) {
		vector2 d2 = a2.direction();
		glColor3f(0.2,0.8,1);
		glBegin(GL_LINES);
		glVertex2i(xx, yy);
		glVertex2i(xx + int(d2.x*wh*3/8),yy - int(d2.y*wh*3/8));
		glEnd();
	}
	glColor3f(1,0,0);
	glBegin(GL_TRIANGLES);
	glVertex2i(xx - int(d.y*4),yy - int(d.x*4));
	glVertex2i(xx + int(d.y*4),yy + int(d.x*4));
	glVertex2i(xx + int(d.x*wh*3/8),yy - int(d.y*wh*3/8));
	glEnd();
	glColor3f(1,1,1);
}

void user_interface::draw_clock(class system& sys, class game& gm,
	int x, int y, unsigned wh, double t, const string& text) const
{
	unsigned seconds = unsigned(fmod(t, 86400));
	unsigned minutes = seconds / 60;
	bool is_day_mode = gm.is_day_mode ();

	set_display_color ( gm );
	if (minutes < 12*60)
		clock12->draw(x, y, wh, wh);
	else
		clock24->draw(x, y, wh, wh);
	minutes %= 12*60;
	int xx = x+wh/2, yy = y+wh/2;
	pair<unsigned, unsigned> twh = font_arial->get_size(text);

	color font_color ( 255, 255, 255 );
	if ( !is_day_mode )
		font_color = color ( 255, 127, 127 );

	font_arial->print(xx-twh.first/2, yy-twh.second/2, text, font_color);
	vector2 d;
	int l;

	glBindTexture(GL_TEXTURE_2D, 0);
	glBegin(GL_TRIANGLES);

	d = (angle(minutes * 360 / (12*60))).direction();
	l = wh/4;
	if ( is_day_mode )
		glColor3f(0,0,0.5);
	else
		glColor3f ( 0.5f, 0.0f, 0.5f );
	glVertex2i(xx - int(d.y*4),yy - int(d.x*4));
	glVertex2i(xx + int(d.y*4),yy + int(d.x*4));
	glVertex2i(xx + int(d.x*l),yy - int(d.y*l));

	d = (angle((minutes%60) * 360 / 60)).direction();
	l = wh*3/8;
	if ( is_day_mode )
		glColor3f(0,0,1);
	else
		glColor3f ( 0.5f, 0.0f, 1.0f );
	glVertex2i(xx - int(d.y*4),yy - int(d.x*4));
	glVertex2i(xx + int(d.y*4),yy + int(d.x*4));
	glVertex2i(xx + int(d.x*l),yy - int(d.y*l));

	d = (angle((seconds%60) * 360 / 60)).direction();
	l = wh*7/16;
	glColor3f(1,0,0);
	glVertex2i(xx - int(d.y*4),yy - int(d.x*4));
	glVertex2i(xx + int(d.y*4),yy + int(d.x*4));
	glVertex2i(xx + int(d.x*l),yy - int(d.y*l));

	glEnd();
	glColor3f(1,1,1);
}

void user_interface::draw_turnswitch(class system& sys, class game& gm, int x, int y,
	unsigned firstdescr, unsigned nrdescr, unsigned selected, unsigned extradescr, unsigned title) const
{
	double full_turn = (nrdescr <= 2) ? 90 : 270;
	double begin_turn = (nrdescr <= 2) ? -45 : -135;
	turnswitchbackgr->draw(x, y);
	double degreesperpos = (nrdescr > 1) ? full_turn/(nrdescr-1) : 0;
	glColor4f(1,1,1,1);
	for (unsigned i = 0; i < nrdescr; ++i) {
		vector2 d = angle(begin_turn+degreesperpos*i).direction();
		sys.no_tex();
		glBegin(GL_LINES);
		glVertex2f(x+128+d.x*36,y+128-d.y*36);
		glVertex2f(x+128+d.x*80,y+128-d.y*80);
		glEnd();
		font_arial->print_c(x+int(d.x*96)+128, y-int(d.y*96)+128, texts::get(firstdescr+i));
	}
	font_arial->print_c(x+128, y+196, texts::get(extradescr));
	turnswitch->draw_rot(x+128, y+128, begin_turn+degreesperpos*selected);
	font_arial->print_c(x+128, y+228, texts::get(title));
}

unsigned user_interface::turnswitch_input(int x, int y, unsigned nrdescr) const
{
	if (nrdescr <= 1) return 0;
	angle a(vector2(x-128, 128-y));
	double full_turn = (nrdescr <= 2) ? 90 : 270;
	double begin_turn = (nrdescr <= 2) ? -45 : -135;
	double degreesperpos = full_turn/(nrdescr-1);
	double ang = a.value_pm180() - begin_turn;
	if (ang < 0) ang = 0;
	if (ang > full_turn) ang = full_turn;
	return unsigned(round(ang/degreesperpos));
}

void user_interface::draw_vessel_symbol(class system& sys,
	const vector2& offset, sea_object* so, color c)
{
	vector2 d = so->get_heading().direction();
	float w = so->get_width()*mapzoom/2, l = so->get_length()*mapzoom/2;
	vector2 p = (so->get_pos().xy() + offset) * mapzoom;
	p.x += 512;
	p.y = 384 - p.y;

	double clickd = mapclick.square_distance(p);
	if (clickd < mapclickdist) {
		target = so;	// fixme: message?
		mapclickdist = clickd;
	}

	c.set_gl_color();
	glBindTexture(GL_TEXTURE_2D, 0);
	glBegin(GL_QUADS);
	glVertex2f(p.x - d.y*w, p.y - d.x*w);
	glVertex2f(p.x - d.x*l, p.y + d.y*l);
	glVertex2f(p.x + d.y*w, p.y + d.x*w);
	glVertex2f(p.x + d.x*l, p.y - d.y*l);
	glEnd();
	glBegin(GL_LINES);
	glVertex2f(p.x - d.x*l, p.y + d.y*l);
	glVertex2f(p.x + d.x*l, p.y - d.y*l);
	glEnd();
	glColor3f(1,1,1);
}

void user_interface::draw_trail(sea_object* so, const vector2& offset)
{
	list<vector2> l = so->get_previous_positions();
	glColor4f(1,1,1,1);
	glBegin(GL_LINE_STRIP);
	vector2 p = (so->get_pos().xy() + offset)*mapzoom;
	glVertex2f(512+p.x, 384-p.y);
	float la = 1.0/float(l.size()), lc = 0;
	for (list<vector2>::const_iterator it = l.begin(); it != l.end(); ++it) {
		glColor4f(1,1,1,1-lc);
		vector2 p = (*it + offset)*mapzoom;
		glVertex2f(512+p.x, 384-p.y);
		lc += la;
	}
	glEnd();
	glColor4f(1,1,1,1);
}

void user_interface::display_gauges(class system& sys, game& gm)
{
	sea_object* player = get_player ();
	sys.prepare_2d_drawing();
	set_display_color ( gm );
	for (int y = 0; y < 3; ++y)	// fixme: replace with gauges
		for (int x = 0; x < 4; ++x)
			psbackgr->draw(x*256, y*256, 256, 256);
	angle player_speed = player->get_speed()*360.0/sea_object::kts2ms(36);
	angle player_depth = -player->get_pos().z;
	draw_gauge(sys, gm, 1, 0, 0, 256, player->get_heading(), texts::get(1),
		player->get_head_to());
	draw_gauge(sys, gm, 2, 256, 0, 256, player_speed, texts::get(4));
	draw_gauge(sys, gm, 4, 2*256, 0, 256, player_depth, texts::get(5));
	draw_clock(sys, gm, 3*256, 0, 256, gm.get_time(), texts::get(61));

	draw_infopanel(sys, gm);
	sys.unprepare_2d_drawing();

	// mouse handling
	int mx, my, mb = sys.get_mouse_buttons();
	sys.get_mouse_position(mx, my);

	if (mb & sys.left_button) {
		int marea = (my/256)*4+(mx/256);
		int mareax = (mx/256)*256+128;
		int mareay = (my/256)*256+128;
		angle mang(vector2(mx - mareax, mareay - my));
		if ( marea == 0 )
		{
			player->head_to_ang(mang, mang.is_cw_nearer(
				player->get_heading()));
		}
		else if ( marea == 1 )
		{}
		else if ( marea == 2 )
		{
			submarine* sub = dynamic_cast<submarine*> ( player );
			if ( sub )
			{
				sub->dive_to_depth(mang.ui_value());
			}
		}
	}

	// keyboard processing
	int key = sys.get_key().sym;
	while (key != 0) {
		if (!keyboard_common(key, sys, gm)) {
			// specific keyboard processing
		}
		key = sys.get_key().sym;
	}
}

void user_interface::display_bridge(class system& sys, game& gm)
{
	sea_object* player = get_player();
    
	glClear(GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	vector2 phd = player->get_heading().direction();
	vector3 viewpos = player->get_pos() + vector3(0, 0, 6) + phd.xy0();
	// no torpedoes, no DCs, with player
	draw_view(sys, gm, viewpos, player->get_heading()+bearing, true, true, false);

	sys.prepare_2d_drawing();
	draw_infopanel(sys, gm);
	sys.unprepare_2d_drawing();

	// keyboard processing
	int key = sys.get_key().sym;
	while (key != 0) {
		if (!keyboard_common(key, sys, gm)) {
			// specific keyboard processing
			switch ( key )
			{
				// Zoom view
				case SDLK_y:
					zoom_scope = true;
					break;
			}
		}
		key = sys.get_key().sym;
	}
}

void user_interface::draw_pings(class game& gm, const vector2& offset)
{
	// draw pings (just an experiment, you can hear pings, locate their direction
	//	a bit fuzzy but not their origin or exact shape).
	const list<game::ping>& pings = gm.get_pings();
	for (list<game::ping>::const_iterator it = pings.begin(); it != pings.end(); ++it) {
		const game::ping& p = *it;
		vector2 r = player_object->get_pos ().xy () - p.pos;
		vector2 p1 = (p.pos + offset)*mapzoom;
		vector2 p2 = p1 + (p.dir + p.pingAngle).direction() * p.range * mapzoom;
		vector2 p3 = p1 + (p.dir - p.pingAngle).direction() * p.range * mapzoom;
		glBegin(GL_TRIANGLES);
		glColor4f(0.5,0.5,0.5,1);
		glVertex2f(512+p1.x, 384-p1.y);
		glColor4f(0.5,0.5,0.5,0);
		glVertex2f(512+p2.x, 384-p2.y);
		glVertex2f(512+p3.x, 384-p3.y);
		glEnd();
		glColor4f(1,1,1,1);
	}
}

void user_interface::draw_sound_contact(class game& gm, const sea_object* player,
	double max_view_dist)
{
    // draw sound contacts
	list<ship*> ships;
	gm.sonar_ships(ships, player);
	for (list<ship*>::iterator it = ships.begin(); it != ships.end(); ++it) {
		vector2 ldir = (*it)->get_pos().xy() - player->get_pos().xy();
		ldir = ldir.normal() * 0.666666 * max_view_dist*mapzoom;
		if ((*it)->is_merchant())
			glColor3f(0,0,0);
		else if ((*it)->is_warship())
			glColor3f(0,0.5,0);
		else if ((*it)->is_escort())
			glColor3f(1,0,0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glBegin(GL_LINES);
		glVertex2f(512,384);
		glVertex2f(512+ldir.x, 384-ldir.y);
		glEnd();
		glColor3f(1,1,1);
	}

	list<submarine*> submarines;
	gm.sonar_submarines ( submarines, player );
	for ( list<submarine*>::iterator it = submarines.begin ();
		it != submarines.end (); it ++ )
	{
		vector2 ldir = (*it)->get_pos().xy() - player->get_pos().xy();
		ldir = ldir.normal() * 0.666666 * max_view_dist*mapzoom;
		// Submarines are drawn in blue.
		glColor3f(0,0,1);
		glBindTexture(GL_TEXTURE_2D, 0);
		glBegin(GL_LINES);
		glVertex2f(512,384);
		glVertex2f(512+ldir.x, 384-ldir.y);
		glEnd();
		glColor3f(1,1,1);
	}
}

void user_interface::draw_visual_contacts(class system& sys, class game& gm,
    const sea_object* player, const vector2& offset)
{
	// draw vessel trails and symbols (since player is submerged, he is drawn too)
	list<ship*> ships;
	gm.visible_ships(ships, player);
	list<submarine*> submarines;
	gm.visible_submarines(submarines, player);
	list<airplane*> airplanes;
	gm.visible_airplanes(airplanes, player);
	list<torpedo*> torpedoes;
	gm.visible_torpedoes(torpedoes, player);

   	// draw trails
   	for (list<ship*>::iterator it = ships.begin(); it != ships.end(); ++it)
   		draw_trail(*it, offset);
   	for (list<submarine*>::iterator it = submarines.begin(); it != submarines.end(); ++it)
   		draw_trail(*it, offset);
   	for (list<airplane*>::iterator it = airplanes.begin(); it != airplanes.end(); ++it)
   		draw_trail(*it, offset);
   	for (list<torpedo*>::iterator it = torpedoes.begin(); it != torpedoes.end(); ++it)
   		draw_trail(*it, offset);

   	// draw vessel symbols
   	for (list<ship*>::iterator it = ships.begin(); it != ships.end(); ++it)
   		draw_vessel_symbol(sys, offset, *it, color(192,255,192));
   	for (list<submarine*>::iterator it = submarines.begin(); it != submarines.end(); ++it)
   		draw_vessel_symbol(sys, offset, *it, color(255,255,128));
   	for (list<airplane*>::iterator it = airplanes.begin(); it != airplanes.end(); ++it)
   		draw_vessel_symbol(sys, offset, *it, color(0,0,64));
   	for (list<torpedo*>::iterator it = torpedoes.begin(); it != torpedoes.end(); ++it)
   		draw_vessel_symbol(sys, offset, *it, color(255,0,0));
}

void user_interface::draw_square_mark ( class system& sys, class game& gm,
	const vector2& mark_pos, const vector2& offset, const color& c )
{
	c.set_gl_color ();
	glBegin ( GL_LINE_LOOP );
	vector2 p = ( mark_pos + offset ) * mapzoom;
	int x = int ( round ( p.x ) );
	int y = int ( round ( p.y ) );
	glVertex2i ( 512-4+x,384-4-y );
	glVertex2i ( 512+4+x,384-4-y );
	glVertex2i ( 512+4+x,384+4-y );
	glVertex2i ( 512-4+x,384+4-y );
	glEnd ();
}

void user_interface::display_map(class system& sys, game& gm)
{
	// get mouse values before drawing (mapclick is used in draw_vessel_symbol)
	int mx, my, mb = sys.get_mouse_buttons();
	sys.get_mouse_position(mx, my);
	mapclick = vector2(mx, my);
	mapclickdist = (mb & sys.left_button) ? 1e20 : -1;

	sea_object* player = get_player ();
	bool is_day_mode = gm.is_day_mode ();

	if ( is_day_mode )
		glClearColor ( 0.0f, 0.0f, 1.0f, 1.0f );
	else
		glClearColor ( 0.0f, 0.0f, 0.75f, 1.0f );
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	double max_view_dist = gm.get_max_view_distance();

	vector2 offset = player->get_pos().xy();

	sys.prepare_2d_drawing();

	float delta = MAPGRIDSIZE*mapzoom;
	float sx = myfmod(512, delta)-myfmod(offset.x, MAPGRIDSIZE)*mapzoom;
	float sy = 768.0 - (myfmod(384.0f, delta)-myfmod(offset.y, MAPGRIDSIZE)*mapzoom);
	int lx = int(1024/delta)+2, ly = int(768/delta)+2;

	// draw grid
	if (mapzoom >= 0.01) {
		glColor3f(0.5, 0.5, 1);
		glBindTexture(GL_TEXTURE_2D, 0);
		glBegin(GL_LINES);
		for (int i = 0; i < lx; ++i) {
			glVertex2f(sx, 0);
			glVertex2f(sx, 768);
			sx += delta;
		}
		for (int i = 0; i < ly; ++i) {
			glVertex2f(0, sy);
			glVertex2f(1024, sy);
			sy -= delta;
		}
		glEnd();
	}

/*
	// draw terrain (mapzoom is pixel/m)
	double dist = max_view_dist;
	vector2 minp(offset.x - dist, offset.y - dist);
	vector2 maxp(offset.x + dist, offset.y + dist);
	vector2 drawmax = mapmaxpos.min(maxp), drawmin = mappos.max(minp);
	vector2 drawarea = drawmax - drawmin;
	vector2 mapoffset = drawmin - mappos;
	int minx = mapoffset.x / mapmperpixel;
	int miny = mapoffset.y / mapmperpixel;
	int maxx = minx + drawarea.x / mapmperpixel;
	int maxy = miny + drawarea.y / mapmperpixel;
	if (minx < 0) minx = 0;
	if (miny < 0) miny = 0;
	if (maxx >= mapw) maxx = mapw;
	if (maxy >= maph) maxy = maph;
cout << "map display\n";	
cout << "minp " << minp << "\n";
cout << "maxp " << maxp << "\n";
cout << "mappos " << mappos << "\n";	
cout << "mapmaxpos " << mapmaxpos << "\n";	
cout << "drawmax " << drawmax << "\n";	
cout << "drawmin " << drawmin << "\n";	
cout << "draw map area double " << drawmin.x << "," << drawmin.y << "," << drawmax.x << "," << drawmax.y << "\n";
cout << "draw map area " << minx << "," << miny << "," << maxx << "," << maxy << "\n";
	glColor3f(0, 0.25, 0);
	glPointSize(mapmperpixel * mapzoom);
	glBegin(GL_POINTS);
	double yp = 384 + drawmin.y * mapzoom;
	unsigned mapptr = miny*mapw+minx;
	double h = 100.0;
	for (int y = miny; y < maxy; ++y) {
		double xp = 512 + drawmin.x * mapzoom;
		for (int x = minx; x < maxx; ++x) {
			char mv = landsea[mapptr];
			if (mv == 1) {
				glVertex2i(xp, yp);
			}
			++mapptr;
			xp += mapmperpixel * mapzoom;
		}
		mapptr += mapw - (maxx - minx);
		yp += mapmperpixel * mapzoom;
	}
	glEnd();
	glPointSize(1.0);
*/
	glColor3f(0,0.5,0);
	glPushMatrix();
	float cls = mapmperpixel * mapzoom;
	glTranslatef(512 + (mappos.x-offset.x)*mapzoom, 384 + (mappos.y+offset.y)*mapzoom, 0);
	glScalef(cls, cls, 1);
	for (list<coastline>::const_iterator it = coastlines.begin(); it != coastlines.end(); ++it) {
		if (it->cyclic)
			glBegin(GL_LINE_LOOP);
		else
			glBegin(GL_LINE_STRIP);
		for (vector<vector2f>::const_iterator it2 = it->points.begin(); it2 != it->points.end(); ++it2) {
			glVertex2f(it2->x, -it2->y);
		}
		glEnd();
	}
	glPopMatrix();

	// draw convoy positions	fixme: should be static and fade out after some time
	glColor3f(1,1,1);
	list<vector2> convoy_pos;
	gm.convoy_positions(convoy_pos);
	glBegin(GL_LINE_LOOP);
	for (list<vector2>::iterator it = convoy_pos.begin(); it != convoy_pos.end(); ++it) {
		draw_square_mark ( sys, gm, (*it), -offset, color ( 0, 0, 0 ) );
	}
	glEnd();
	glColor3f(1,1,1);

	// draw view range
	glColor3f(1,0,0);
	float range = max_view_dist*mapzoom;
	glBegin(GL_LINE_LOOP);
	for (int i = 0; i < 32+range/4; ++i) {
		float a = i*2*M_PI/(32+range/4);
		glVertex2f(512+sin(a)*range, 384-cos(a)*range);
	}
	glEnd();
	glColor3f(1,1,1);

	// draw vessel symbols (or noise contacts)
	submarine* sub_player = dynamic_cast<submarine*> ( player );
	if (sub_player && sub_player->is_submerged ()) {
		// draw pings
		draw_pings(gm, -offset);

		// draw sound contacts
		draw_sound_contact(gm, sub_player, max_view_dist);

		// draw player trails and player
		draw_trail(player, -offset);
		draw_vessel_symbol(sys, -offset, sub_player, color(255,255,128));

		// Special handling for submarine player: When the submarine is
		// on periscope depth and the periscope is up the visual contact
		// must be drawn on map.
		if ((sub_player->get_depth() <= sub_player->get_periscope_depth()) &&
			sub_player->is_scope_up())
		{
			draw_visual_contacts(sys, gm, sub_player, -offset);

			// Draw a red box around the selected target.
			if ( target )
			{
				draw_square_mark ( sys, gm, target->get_pos ().xy (), -offset,
					color ( 255, 0, 0 ) );
				glColor3f ( 1.0f, 1.0f, 1.0f );
			}
		}
	} 
	else	 	// enable drawing of all object as testing hack by commenting this, fixme
	{
		draw_visual_contacts(sys, gm, player, -offset);

		// Draw a red box around the selected target.
		if ( target )
		{
			draw_square_mark ( sys, gm, target->get_pos ().xy (), -offset,
				color ( 255, 0, 0 ) );
			glColor3f ( 1.0f, 1.0f, 1.0f );
		}
	}

	draw_infopanel(sys, gm);
	sys.unprepare_2d_drawing();

	// further Mouse handling

	// keyboard processing
	int key = sys.get_key().sym;
	while (key != 0) {
		if (!keyboard_common(key, sys, gm)) {
			// specific keyboard processing
			switch(key) {
				case SDLK_EQUALS :
				case SDLK_PLUS : if (mapzoom < 1) mapzoom *= 1.5; break;
				case SDLK_MINUS : if (mapzoom > 0.001) mapzoom /= 1.5; break;
			}
		}
		key = sys.get_key().sym;
	}
}

void user_interface::display_logbook(class system& sys, game& gm)
{
	// glClearColor ( 0.5f, 0.25f, 0.25f, 0 );
	glClearColor ( 0, 0, 0, 0 );
	glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	sys.prepare_2d_drawing ();
	captains_logbook->display ( sys, gm );
	draw_infopanel ( sys, gm );
	sys.unprepare_2d_drawing ();

	// mouse processing;
	int mx;
	int my;
	int mb = sys.get_mouse_buttons();
	sys.get_mouse_position(mx, my);
	if ( mb & sys.left_button )
		captains_logbook->check_mouse ( mx, my, mb );

	// keyboard processing
	int key = sys.get_key().sym;
	while (key != 0) {
		if (!keyboard_common(key, sys, gm)) {
			// specific keyboard processing
			captains_logbook->check_key ( key, sys, gm );
		}
		key = sys.get_key().sym;
	}
}

void user_interface::display_successes(class system& sys, game& gm)
{
	// glClearColor ( 0, 0, 0, 0 );
	// glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	sys.prepare_2d_drawing ();
	ships_sunk_disp->display ( sys, gm );
	draw_infopanel ( sys, gm );
	sys.unprepare_2d_drawing ();

	// keyboard processing
	int key = sys.get_key ().sym;
	while ( key != 0 )
	{
		if ( !keyboard_common ( key, sys, gm ) )
		{
			// specific keyboard processing
			ships_sunk_disp->check_key ( key, sys, gm );
		}
		key = sys.get_key ().sym;
	}
}

#ifdef OLD
void user_interface::display_successes(class system& sys, game& gm)
{
	glClearColor(0.25, 0.25, 0.25, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	sys.prepare_2d_drawing();
	font_arial->print(0, 0, "success records - fixme");
	font_arial->print(0, 100, "Ships sunk\n----------\n");
	unsigned ships = 0, tons = 0;
	for (list<unsigned>::const_iterator it = tonnage_sunk.begin(); it != tonnage_sunk.end(); ++it) {
		++ships;
		char tmp[20];
		sprintf(tmp, "%u BRT", *it);
		font_arial->print(0, 100+(ships+2)*font_arial->get_height(), tmp);
		tons += *it;
	}
	char tmp[40];
	sprintf(tmp, "total: %u BRT", tons);
	font_arial->print(0, 100+(ships+4)*font_arial->get_height(), tmp);
	sys.unprepare_2d_drawing();

	// keyboard processing
	int key = sys.get_key().sym;
	while (key != 0) {
		if (!keyboard_common(key, sys, gm)) {
			// specific keyboard processing
		}
		key = sys.get_key().sym;
	}
}
#endif // OLD

void user_interface::display_freeview(class system& sys, game& gm)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glRotatef(viewupang,1,0,0);
	glRotatef(viewsideang,0,1,0);
	float viewmatrix[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, viewmatrix);
	vector3 sidestep(viewmatrix[0], viewmatrix[4], viewmatrix[8]);
	vector3 upward(viewmatrix[1], viewmatrix[5], viewmatrix[9]);
	vector3 forward(viewmatrix[2], viewmatrix[6], viewmatrix[10]);
	glTranslatef(-viewpos.x, -viewpos.y, -viewpos.z);

	// draw everything
	draw_view(sys, gm, viewpos, 0, false, false, true);

	int mx, my;
	sys.get_mouse_motion(mx, my);
	viewsideang += mx*0.5;
	viewupang -= my*0.5;

	sys.prepare_2d_drawing();
	draw_infopanel(sys, gm);
	sys.unprepare_2d_drawing();

	// keyboard processing
	int key = sys.get_key().sym;
	while (key != 0) {
		if (!keyboard_common(key, sys, gm)) {
			// specific keyboard processing
			switch(key) {
				case SDLK_w: viewpos -= forward * 5; break;
				case SDLK_x: viewpos += forward * 5; break;
				case SDLK_a: viewpos -= sidestep * 5; break;
				case SDLK_d: viewpos += sidestep * 5; break;
				case SDLK_q: viewpos -= upward * 5; break;
				case SDLK_e: viewpos += upward * 5; break;
			}
		}
		key = sys.get_key().sym;
	}
}

void user_interface::add_message(const string& s)
{
	panel_texts.push_back(s);
	if (panel_texts.size() > 1+MAX_PANEL_SIZE/font_panel->get_height())
		panel_texts.pop_front();
}

void user_interface::display_glasses(class system& sys, class game& gm)
{
	sea_object* player = get_player();

	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	unsigned res_x = sys.get_res_x(), res_y = sys.get_res_y();

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	sys.gl_perspective_fovx (5.0, 2.0/1.0, 2.0, gm.get_max_view_distance());
	glViewport(0, res_y/3, res_x, res_x/2);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	vector3 viewpos = player->get_pos() + vector3(0, 0, 6);
	// no torpedoes, no DCs, no player
	draw_view(sys, gm, viewpos, player->get_heading()+bearing, true, false, false);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glViewport(0, 0, res_x, res_y);
	glMatrixMode(GL_MODELVIEW);

	sys.prepare_2d_drawing();
	glasses->draw(0, 0, 512, 512);
	glasses->draw_hm(512, 0, 512, 512);
	draw_infopanel(sys, gm);
	sys.unprepare_2d_drawing();

	// keyboard processing
	int key = sys.get_key().sym;
	while (key != 0) {
		if (!keyboard_common(key, sys, gm)) {
			// specific keyboard processing
			switch ( key )
			{
				case SDLK_y:
					zoom_scope = false;
					break;
			}
		}
		key = sys.get_key().sym;
	}
}

void user_interface::add_rudder_message()
{
    switch (player_object->get_rudder())
    {
        case player_object->rudderfullleft:
            add_message(texts::get(35));
            break;
        case player_object->rudderleft:
            add_message(texts::get(33));
            break;
        case player_object->ruddermid:
            add_message(texts::get(42));
            break;
        case player_object->rudderright:
            add_message(texts::get(34));
            break;
        case player_object->rudderfullright:
            add_message(texts::get(36));
            break;
    }
}

#define DAY_MODE_COLOR() glColor3f ( 1.0f, 1.0f, 1.0f )

#define NIGHT_MODE_COLOR() glColor3f ( 1.0f, 0.4f, 0.4f )

void user_interface::set_display_color ( color_mode mode ) const
{
	switch ( mode )
	{
		case night_color_mode:
			NIGHT_MODE_COLOR ();
			break;
		default:
			DAY_MODE_COLOR ();
			break;
	}
}

void user_interface::set_display_color ( const class game& gm ) const
{
	if ( gm.is_day_mode () )
		DAY_MODE_COLOR ();
	else
		NIGHT_MODE_COLOR ();
}

sound* user_interface::get_sound_effect ( sound_effect se ) const
{
	sound* s = 0;

	switch ( se )
	{
		case se_submarine_torpedo_launch:
			s = torpedo_launch_sound;
			break;
		case se_torpedo_detonation:
			{
				submarine* sub = dynamic_cast<submarine*>( player_object );

				if ( sub && sub->is_submerged () )
				{
					double sid = rnd ( 2 );
					if ( sid < 1.0f )
						s = torpedo_detonation_submerged[0];
					else if ( sid < 2.0f )
						s = torpedo_detonation_submerged[1];
				}
				else
				{
					double sid = rnd ( 2 );
					if ( sid < 1.0f )
						s = torpedo_detonation_surfaced[0];
					else if ( sid < 2.0f )
						s = torpedo_detonation_surfaced[1];
				}
			}
			break;
	}

	return s;
}

void user_interface::play_sound_effect ( sound_effect se, double volume ) const
{
	sound* s = get_sound_effect ( se );

	if ( s )
		s->play ( volume );
}

void user_interface::play_sound_effect_distance ( sound_effect se, double distance ) const
{
	sound* s = get_sound_effect ( se );

	if ( s )
		s->play ( ( 1.0f - player_object->get_noise_factor () ) * exp ( - distance / 3000.0f ) );
}

void user_interface::add_captains_log_entry ( class game& gm, const string& s)
{
	date d;
	get_date ( gm.get_time (), d );

	if ( captains_logbook )
		captains_logbook->add_entry( d, s );
}

inline void user_interface::record_sunk_ship ( const ship* so )
{
	ships_sunk_disp->add_sunk_ship ( so );
}

void user_interface::draw_manometer_gauge ( class system& sys, class game& gm,
	unsigned nr, int x, int y, unsigned wh, float value, const string& text) const
{
	set_display_color ( gm );
	switch (nr)
	{
		case 1:
			gauge5->draw ( x, y, wh, wh / 2 );
			break;
		default:
			return;
	}
	angle a ( 292.5f + 135.0f * value );
	vector2 d = a.direction ();
	int xx = x + wh / 2, yy = y + wh / 2;
	pair<unsigned, unsigned> twh = font_arial->get_size(text);

	// Draw text.
	color font_color ( 0, 0, 0 );
	font_arial->print ( xx - twh.first / 2, yy - twh.second / 2 - wh / 6,
		text, font_color );

	// Draw pointer.
	glColor3f ( 0.0f, 0.0f, 0.0f );
	glBindTexture ( GL_TEXTURE_2D, 0 );
	glBegin ( GL_LINES );
	glVertex2i ( xx + int ( d.x * wh / 16 ), yy - int ( d.y * wh / 16 ) );
	glVertex2i ( xx + int ( d.x * wh * 3 / 8 ), yy - int ( d.y * wh * 3 / 8 ) );
	glEnd ();
	glColor3f ( 1.0f, 1.0f, 1.0f );
}
