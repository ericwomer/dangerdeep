// sky simulation and display (OpenGL)
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#pragma warning (disable : 4786)
#endif

#include "oglext/OglExt.h"
#include <GL/glu.h>
#include <SDL.h>

#include "sky.h"
#include "model.h"
#include "texture.h"
#include "global_data.h"


const double CLOUD_ANIMATION_CYCLE_TIME = 3600.0;
const double EARTH_RADIUS = 6.378e6;			// 6378km
const double SUN_RADIUS = 696e6;			// 696.000km
const double MOON_RADIUS = 1.738e6;			// 1738km
const double EARTH_SUN_DISTANCE = 149600e6;		// 149.6 million km.
const double MOON_EARTH_DISTANCE = 384.4e6;		// 384.000km
const double EARTH_ROT_AXIS_ANGLE = 23.45;
const double MOON_ORBIT_TIME = 27.3333333 * 86400.0;	// sidereal month is 27 1/3 days, time from full moon to next is 29.5306 days because of earth's rotation around sun
//moon rotational plane is ~ 5 degrees rotated to that of earth/sun, fixme
const double EARTH_ROTATION_TIME = 86160.0;		// 23h56min, one sidereal day!
// as a result, earth takes ~366 rotations around its axis per year.
const double EARTH_PERIMETER = 2.0 * M_PI * EARTH_RADIUS;
const double EARTH_ORBIT_TIME = 31556926.5;	// in seconds. 365 days, 5 hours, 48 minutes, 46.5 seconds
// these values are difficult to get. SUN_POS_ADJUST should be around +9.8deg (10days of 365 later) but that gives
// a roughly right position but wrong sun rise time by about 40min. fixme
const double SUN_POS_ADJUST = 9.8;	// in degrees. 10 days from 21st. Dec. to 1st. Jan. * 360deg/365.24days
const double MOON_POS_ADJUST = 300.0;	// in degrees. Moon pos in its orbit on 1.1.1939 fixme: this value is a rude guess

// moon rotates from west to east ie. ccw ?
// sun rises in the east so earth turns ccw. around itself.

/*
what has to be fixed for sun/earth/moon simulation:
get exact distances and diameters (done)
get exact rotation times (sidereal day, not solar day) for earth and moon (done)
get exact orbit times for earth and moon around sun / earth (done)
get angle of rotational axes for earth and moon (fixme, 23.45 and ~5)
get direction of rotation for earth and moon relative to each other (fixme)
get position of objects and axis states for a fix date (optimum 1.1.1939) (fixme)
compute formulas for determining the positions for the following years (fixme)
write code that computes sun/moon pos relative to earth and relative to local coordinates (fixme)
draw moon with phases (fixme)
*/




sky::sky(double tm) : mytime(tm), skycolorfac(0.0f), atmosphericlight(0.0),
	skyhemisphere(0), skycol(0), sunglow(0),
	clouds(0), suntex(0), moontex(0), clouds_dl(0), skyhemisphere_dl(0)
{
	// ******************************* create display list for sky background
	skyhemisphere = new model(get_model_dir() + "skyhemisphere.3ds", false, false);
	model::mesh skyhemisphere_mesh = skyhemisphere->get_mesh(0);

	unsigned smv = skyhemisphere_mesh.vertices.size();
	vector<vector2f> uv0(smv);
	vector<vector2f> uv1(smv);
	vector3f center = (skyhemisphere->get_min() + skyhemisphere->get_max()) * 0.5f;
	center.z = skyhemisphere->get_min().z;
	for (unsigned i = 0; i < smv; ++i) {
		vector3f d = (skyhemisphere_mesh.vertices[i].pos - center).normal();
		d.z = fabs(d.z);
		if (d.z > 1.0f) d.z = 1.0f;
		float alpha = acos(fabs(d.z));
		float sinalpha = sin(alpha);
		float u = 0.5f;
		float v = 0.5f;
		if (sinalpha != 0.0f) {
			u += (alpha*d.x)/(sinalpha*M_PI);
			v += (alpha*d.y)/(sinalpha*M_PI);
			if (u < 0.0f) u = 0.0f; if (u > 1.0f) u = 1.0f;
			if (v < 0.0f) v = 0.0f; if (v > 1.0f) v = 1.0f;
		}
		uv0[i] = vector2f(0.0f, float(2.0*alpha/M_PI));
		uv1[i] = vector2f(u, v);
	}

	skyhemisphere_dl = glGenLists(1);
	glNewList(skyhemisphere_dl, GL_COMPILE);
	glVertexPointer(3, GL_FLOAT, sizeof(model::mesh::vertex), &(skyhemisphere_mesh.vertices[0].pos.x));
	glEnableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glClientActiveTexture(GL_TEXTURE0);
	glTexCoordPointer(2, GL_FLOAT, sizeof(vector2f), &uv0[0].x);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glClientActiveTexture(GL_TEXTURE1);
	glTexCoordPointer(2, GL_FLOAT, sizeof(vector2f), &uv1[0].x);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glDrawElements(GL_TRIANGLES, 3*skyhemisphere_mesh.faces.size(), GL_UNSIGNED_INT, &(skyhemisphere_mesh.faces[0].v[0]));
	glDisableClientState(GL_VERTEX_ARRAY);
	glClientActiveTexture(GL_TEXTURE1);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glClientActiveTexture(GL_TEXTURE0);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glEndList();

	// ******************************** create stars
	const unsigned nr_of_stars = 2000;
	stars_pos.reserve(nr_of_stars);
	stars_lumin.reserve(nr_of_stars*3);
	for (unsigned i = 0; i < nr_of_stars; ++i) {
		vector3f p(rnd() * 2.0f - 1.0f, rnd() * 2.0f - 1.0f, rnd());
		stars_pos.push_back(p.normal());
		float fl = rnd();
		fl = 1.0f - fl*fl*fl;
		Uint8 l = Uint8(255*fl);
		stars_lumin.push_back(l);
		stars_lumin.push_back(l);
		stars_lumin.push_back(l);
	}

	// ******************************** create map for sky color
	vector<Uint8> skycolmap(256*256*3);
	for (int y = 0; y < 256; ++y) {
		// y is height
		color a = color(color( 73, 164, 255), color(173, 200, 219), float(y)/255);
		color b = color(color(143, 148, 204), color(138, 156, 168), float(y)/255);
		for (int x = 0; x < 256; ++x) {
			// x is weather type (sunny -> storm)
			color c = color(a, b, float(x)/255);
			skycolmap[(y*256+x)*3+0] = c.r;
			skycolmap[(y*256+x)*3+1] = c.g;
			skycolmap[(y*256+x)*3+2] = c.b;
		}
	}
/*
	ofstream osg("testsky.ppm");
	osg << "P6\n256 256\n255\n";
	osg.write((const char*)(&skycolmap[0]), 256*256*3);
*/
	skycol = new texture(&skycolmap[0], 256, 256, GL_RGB, GL_RGB, GL_UNSIGNED_BYTE, GL_LINEAR, GL_CLAMP_TO_EDGE);
	skycolmap.clear();

	// ******************************** create maps for sun glow
	vector<Uint8> sunglowmap(256*256);
	const double expa = 2.0;
	const double expb = exp(-expa);
	for (int y = 0; y < 256; ++y) {
		for (int x = 0; x < 256; ++x) {
			float dist = sqrt(float(vector2i(x-128, y-128).square_length()))/64.0f;
			if (dist > 1.0f) dist = 1.0f;
			float val = 255*((exp(-dist*expa) - expb)/(1-expb));
			if (val < 0) val = 0;
			if (val > 255) val = 255;
			sunglowmap[y*256+x] = Uint8(val);
		}
	}
/*	
	ofstream osg("testglow.pgm");
	osg << "P5\n256 256\n255\n";
	osg.write((const char*)(&sunglowmap[0]), 256*256);
*/
	sunglow = new texture(&sunglowmap[0], 256, 256, GL_LUMINANCE, GL_LUMINANCE, GL_UNSIGNED_BYTE, GL_LINEAR, GL_CLAMP_TO_EDGE);
	sunglowmap.clear();

	// ********************************** init sun/moon	
	suntex = new texture(get_texture_dir() + "thesun.png", GL_LINEAR);
	moontex = new texture(get_texture_dir() + "themoon.png", GL_LINEAR);
	
	// ********************************** init clouds
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
	
	cloud_interpolate_func.resize(256);
	for (unsigned n = 0; n < 256; ++n)
		cloud_interpolate_func[n] = unsigned(128-cos(n*M_PI/256)*128);

	cloud_alpha.resize(256*256);
	for (unsigned y = 0; y < 256; ++y) {
		for (unsigned x = 0; x < 256; ++x) {
			float fx = float(x)-128;
			float fy = float(y)-128;
			float d = 1.0-sqrt(fx*fx+fy*fy)/128.0;
			d = 1.0-exp(-d*5);
			if (d < 0) d = 0;
			cloud_alpha[y*256+x] = Uint8(255*d);
		}
	}

	noisemaps_0 = compute_noisemaps();
	noisemaps_1 = compute_noisemaps();
	compute_clouds();

	// create cloud mesh display list, fixme: get it from sky hemisphere mesh data!
	unsigned skyvsegs = 16;
	unsigned skyhsegs = 4*skyvsegs;
	clouds_dl = glGenLists(1);
	glNewList(clouds_dl, GL_COMPILE);
	unsigned skysegs = skyvsegs*skyhsegs;
	vector<vector3f> points;
	points.reserve(skysegs+1);
	vector<vector2f> texcoords;
	texcoords.reserve(skysegs+1);
	for (unsigned beta = 0; beta < skyvsegs; ++beta) {
		float t = (1.0-float(beta)/skyvsegs)/2;
		float r = cos(M_PI/2*beta/skyvsegs);
		float h = sin(M_PI/2*beta/skyvsegs);
		for (unsigned alpha = 0; alpha < skyhsegs; ++alpha) {
			float x = cos(2*M_PI*alpha/skyhsegs);
			float y = sin(2*M_PI*alpha/skyhsegs);
			points.push_back(vector3f(x*r, y*r, h));
			texcoords.push_back(vector2f(x*t+0.5, y*t+0.5));
		}
	}
	points.push_back(vector3f(0, 0, 1));
	texcoords.push_back(vector2f(0.5, 0.5));
	glVertexPointer(3, GL_FLOAT, sizeof(vector3f), &points[0]);
	glTexCoordPointer(2, GL_FLOAT, sizeof(vector2f), &texcoords[0]);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	vector<unsigned> indices;
	indices.reserve(skysegs*4);
	for (unsigned beta = 0; beta < skyvsegs; ++beta) {
		for (unsigned alpha = 0; alpha < skyhsegs; ++alpha) {
			unsigned i0 = beta*skyhsegs+alpha;
//			if((alpha+beta)&1)continue;
			unsigned i1 = beta*skyhsegs+(alpha+1)%skyhsegs;
			unsigned i2 = (beta==skyvsegs-1) ? skysegs : (beta+1)*skyhsegs+alpha;
			unsigned i3 = (beta==skyvsegs-1) ? skysegs : (beta+1)*skyhsegs+(alpha+1)%skyhsegs;
			indices.push_back(i0);
			indices.push_back(i2);
			indices.push_back(i3);
			indices.push_back(i1);
		}
	}
	glDrawElements(GL_QUADS, skysegs*4 /* /2 */, GL_UNSIGNED_INT, &indices[0]);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glEndList();
}



sky::~sky()
{
	delete skyhemisphere;
	delete skycol;
	delete sunglow;
	delete clouds;
	delete suntex;
	delete moontex;
	glDeleteLists(clouds_dl, 1);
	glDeleteLists(skyhemisphere_dl, 1);
}



void sky::setup_textures(void) const
{
	glPushAttrib(GL_ALL_ATTRIB_BITS);

	// *********************** set up texture unit 0
	// skycol (tex0) is blended into previous color
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, skycol->get_opengl_name());
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
	glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE);
	glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE);
	glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
/*
	// not needed.
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_PRIMARY_COLOR);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
*/
	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glLoadIdentity();
	glTranslatef(skycolorfac, 0.0f, 0.0f);
	glMatrixMode(GL_MODELVIEW);	

	// *********************** set up texture unit 1
	glActiveTexture(GL_TEXTURE1);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, sunglow->get_opengl_name());
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_ADD);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_PREVIOUS);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_TEXTURE1);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
/*
	// not needed.
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_PRIMARY_COLOR);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
*/
	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glLoadIdentity();
	glTranslatef(0.25, 0.35, 0.0);
	glMatrixMode(GL_MODELVIEW);	
	// fixme: set up texture matrix to move sunglow according to sun position

	glDisable(GL_LIGHTING);
}



void sky::cleanup_textures(void) const
{
	glColor4f(1,1,1,1);

	// ******************** clean up texture unit 0
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glMatrixMode(GL_TEXTURE);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);

	// ******************** clean up texture unit 1
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glMatrixMode(GL_TEXTURE);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);

	glEnable(GL_LIGHTING);

	glPopAttrib();
}



void sky::advance_cloud_animation(double fac)
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



void sky::compute_clouds(void)
{
	unsigned mapsize = 8 - cloud_levels;
	unsigned mapsize2 = (2<<mapsize);

	// fixme: could we interpolate between accumulated noise maps
	// to further speed up the process?
	vector<vector<Uint8> > cmaps = noisemaps_0;
	float f = cloud_animphase;
	for (unsigned i = 0; i < cloud_levels; ++i)
		for (unsigned j = 0; j < mapsize2 * mapsize2; ++j)
			cmaps[i][j] = Uint8(noisemaps_0[i][j]*(1-f) + noisemaps_1[i][j]*f);

	// create full map
	vector<Uint8> fullmap(256 * 256 * 2);
	unsigned fullmapptr = 0;
	vector2i sunpos(64,64);		// store sun coordinates here! fixme
	float maxsundist = 362;		// sqrt(2*256^2)
	for (unsigned y = 0; y < 256; ++y) {
		for (unsigned x = 0; x < 256; ++x) {
			unsigned v = 0;
			// accumulate values
			for (unsigned k = 0; k < cloud_levels; ++k) {
				unsigned tv = get_value_from_bytemap(x, y, k, cmaps[k]);
				v += (tv >> k);
			}
			// fixme generate a lookup table for this function, depending on coverage/sharpness
			if (v > 255) v = 255;
			unsigned invcover = 256-cloud_coverage;
			if (v < invcover)
				v = 0;
			else
				v -= invcover;
			// use sharpness for exp function
			v = 255 - v * 256 / cloud_coverage;	// equalize
			int sundist = int(255-192*sqrt(float(vector2i(x,y).square_distance(sunpos)))/maxsundist);
			fullmap[fullmapptr++] = sundist;	// luminance info is wasted here, but should be used, fixme
			fullmap[fullmapptr++] = Uint8(v * unsigned(cloud_alpha[y*256+x]) / 255);
		}
	}

	delete clouds;	
	clouds = new texture(&fullmap[0], 256, 256, GL_LUMINANCE_ALPHA, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE,
		GL_LINEAR, GL_CLAMP_TO_EDGE);
}



vector<vector<Uint8> > sky::compute_noisemaps(void)
{
	unsigned mapsize = 8 - cloud_levels;
	unsigned mapsize2 = (2<<mapsize);
	vector<vector<Uint8> > noisemaps(cloud_levels);
	for (unsigned i = 0; i < cloud_levels; ++i) {
		noisemaps[i].resize(mapsize2 * mapsize2);
		for (unsigned j = 0; j < mapsize2 * mapsize2; ++j)
			noisemaps[i][j] = (unsigned char)(255*rnd());
		smooth_and_equalize_bytemap(mapsize2, noisemaps[i]);
	}
	return noisemaps;
}



Uint8 sky::get_value_from_bytemap(unsigned x, unsigned y, unsigned level,
	const vector<Uint8>& nmap)
{
	// x,y are in 0...255, shift them according to level
	unsigned shift = cloud_levels - 1 - level;
	unsigned mapshift = 9 - cloud_levels;
	unsigned mapmask = (1 << mapshift) - 1;
	unsigned rshift = 8 - shift;
	unsigned xfrac = ((x << rshift) & 255);
	unsigned yfrac = ((y << rshift) & 255);
	x = (x >> shift);
	y = (y >> shift);
	x = x & mapmask;
	y = y & mapmask;
	unsigned x2 = (x+1) & mapmask;
	unsigned y2 = (y+1) & mapmask;

	xfrac = cloud_interpolate_func[xfrac];
	yfrac = cloud_interpolate_func[yfrac];
	
	unsigned v0 = nmap[(y<<mapshift)+x];
	unsigned v1 = nmap[(y<<mapshift)+x2];
	unsigned v2 = nmap[(y2<<mapshift)+x];
	unsigned v3 = nmap[(y2<<mapshift)+x2];
	unsigned v4 = (v0*(256-xfrac)+v1*xfrac);
	unsigned v5 = (v2*(256-xfrac)+v3*xfrac);
	unsigned v6 = (v4*(256-yfrac)+v5*yfrac);
	return Uint8(v6 >> 16);
}



void sky::smooth_and_equalize_bytemap(unsigned s, vector<Uint8>& map1)
{
	vector<Uint8> map2 = map1;
	unsigned maxv = 0, minv = 255;
	for (unsigned y = 0; y < s; ++y) {
		unsigned y1 = (y+s-1)%s, y2 = (y+1)%s;
		for (unsigned x = 0; x < s; ++x) {
			unsigned x1 = (x+s-1)%s, x2 = (x+1)%s;
			unsigned v = (unsigned(map2[y1*s+x1]) + unsigned(map2[y1*s+x2]) + unsigned(map2[y2*s+x1]) + unsigned(map2[y2*s+x2])) / 16
				+ (unsigned(map2[y*s+x1]) + unsigned(map2[y*s+x2]) + unsigned(map2[y1*s+x]) + unsigned(map2[y2*s+x])) / 8
				+ (unsigned(map2[y*s+x])) / 4;
			map1[y*s+x] = Uint8(v);
			if (v < minv) minv = v;
			if (v > maxv) maxv = v;
		}
	}
	for (unsigned y = 0; y < s; ++y) {
		for (unsigned x = 0; x < s; ++x) {
			unsigned v = map1[y*s+x];
			map1[y*s+x] = Uint8((v - minv)*255/(maxv-minv));
		}
	}
}



void sky::set_time(double tm)
{
	mytime = tm;
	tm = myfmod(tm, 86400.0);
	double cf = myfrac(tm/CLOUD_ANIMATION_CYCLE_TIME) - cloud_animphase;
	if (fabs(cf) < (1.0/(3600.0*256.0))) cf = 0.0;
	if (cf < 0) cf += 1.0;
	advance_cloud_animation(cf);

	double dt = get_day_time(mytime);
	// fixme: get light color from sun position. 0 at night, 1 when sun is more than 10 degrees above the horizon or something the like
	double colscal;
	if (dt < 1) { colscal = 0; }
	else if (dt < 2) { colscal = fmod(dt,1); }
	else if (dt < 3) { colscal = 1; }
	else { colscal = 1-fmod(dt,1); }
	
	atmosphericlight = colscal;
}



void sky::display(const vector3& viewpos, double max_view_dist, bool isreflection) const
{
	double dt = get_day_time(mytime);
	// fixme: get light color from sun position. 0 at night, 1 when sun is more than 10 degrees above the horizon or something the like
	double colscal;
	if (dt < 1) { colscal = 0; }
	else if (dt < 2) { colscal = fmod(dt,1); }
	else if (dt < 3) { colscal = 1; }
	else { colscal = 1-fmod(dt,1); }
//colscal = fabs(myfrac(mytime/40.0)*2.0-1.0);//testing
	color lightcol = color(color(64, 64, 64), color(255,255,255), colscal);

	// 1) draw the stars on a black background

	// skyplanes:
	// 2) blue color, shading to grey haze to the horizon	(tex0, blend into background)
	// 3) sun glow						(tex1, added)
	// 4) clouds layer					(separate faces)
	// 5) sun lens flares					(later, added, one quad)
	// draw: time (night/day) changes blend factor for 2, maybe 3
	// blend factor may be set via global color alpha
	// set up texture matrix for texture unit 0 (x translation for weather conditions,
	// x in [0,1] means sunny...storm)
	// sun glow (it moves) must come from a texture

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glPushMatrix();
	glTranslatef(0, 0, -viewpos.z);

	// because the reflection map may have a lower resolution than the screen
	// we shouldn't draw stars while computing this map. They would appear to big.
	// in fact star light is to weak for water reflections, isn't it?
	if (!isreflection) {
		glPushMatrix();
		glScalef(max_view_dist * 0.95, max_view_dist * 0.95, max_view_dist * 0.95);
		glDisable(GL_LIGHTING);
		glBindTexture(GL_TEXTURE_2D, 0);
		glEnableClientState(GL_COLOR_ARRAY);
	        glColorPointer(3, GL_UNSIGNED_BYTE, 0, &stars_lumin[0]);
        	glEnableClientState(GL_VERTEX_ARRAY);
	        glVertexPointer(3, GL_FLOAT, sizeof(vector3f), &stars_pos[0].x);
		glDrawArrays(GL_POINTS, 0, stars_pos.size());
		glDisableClientState(GL_COLOR_ARRAY);
		glDisableClientState(GL_VERTEX_ARRAY);
		glPopMatrix();
	}

	glColor4f(1, 1, 1, atmosphericlight);

	setup_textures();

	// fixme: maybe rotate star positions every day a bit? rotate with earth rotation? that would be more realistic/cooler

	glPushMatrix();
	double scal = max_view_dist / 30000.0;	// sky hemisphere is stored as 30km in radius
	glScaled(scal, scal, scal);

	// ********* set up sky textures and call list
	glDisable(GL_DEPTH_TEST);	// to avoid the stars appearing in front of the sun etc.
	glCallList(skyhemisphere_dl);	// this overdraws the stars! why?!
	glEnable(GL_DEPTH_TEST);
	
	color::white().set_gl_color();
	
	cleanup_textures();

	glPopMatrix();	// remove scale
	

	// ******** the sun and the moon *****************************************************
	glDisable(GL_LIGHTING);
	/* How to compute the sun's/moon's position:
	   Earth and moon rotate around the sun's y-axis and around their own y-axes.
	   The z-axis is pointing outward and the x-axis around the equator.
	   Earth's rotational axis differs by 23.45 degrees from sun's y-axis.
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
	   fixme: moon rotation plane is not equal to earth rotation plane, it differs by 5,15 degr.
	   this is why the moon is not always in earth's shadow when it is a full moon.
	*/
	// fixme: adjust OpenGL light position to infinite in sun/moon direction.
	double universaltime = mytime;//    * 8640;	// fixme: substract local time

	// draw sun, fixme draw glow/flares/halo
	glPushMatrix();
	// Transform earth space to viewer space
	// to avoid mixing very differently sized floating point values, we scale distances before translation.
	double sun_scale_fac = max_view_dist / EARTH_SUN_DISTANCE;
	glTranslated(0, 0, -EARTH_RADIUS * sun_scale_fac);
	glRotated(360.0 * -viewpos.y * 4 / EARTH_PERIMETER, 0, 1, 0);
	glRotated(360.0 * -viewpos.x * 2 / EARTH_PERIMETER, 1, 0, 0);
	// Transform sun space to earth space
	glRotated(360.0 * -myfrac(universaltime/EARTH_ROTATION_TIME), 0, 1, 0);
	glRotated(-EARTH_ROT_AXIS_ANGLE, 1, 0, 0);
	glRotated(SUN_POS_ADJUST + 360.0 * myfrac(universaltime/EARTH_ORBIT_TIME), 0, 1, 0);
	glTranslated(0, 0, -EARTH_SUN_DISTANCE * sun_scale_fac * 0.96);	// to keep it inside sky hemisphere
	// draw quad
	double suns = SUN_RADIUS * sun_scale_fac;
	glColor3f(1,1,1);
	suntex->set_gl_texture();
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
	// GLfloat lp[4] = {0,0,0,1};//{m[12], m[13], m[14], 0};	// light comes from sun (maybe use directional light)
//	glLightfv(GL_LIGHT0, GL_POSITION, lp);

	glPopMatrix();	// remove sun space

	// draw moon
	glPushMatrix();
	// Transform earth space to viewer space
	double moon_scale_fac = max_view_dist / MOON_EARTH_DISTANCE;
	glTranslated(0, 0, -EARTH_RADIUS * moon_scale_fac);
	glRotated(360.0 * -viewpos.y * 4 / EARTH_PERIMETER, 0, 1, 0);
	glRotated(360.0 * -viewpos.x * 2 / EARTH_PERIMETER, 1, 0, 0);
	// Transform moon space to earth space
	glRotated(360.0 * -myfrac(universaltime/EARTH_ROTATION_TIME), 0, 1, 0);
	glRotated(-EARTH_ROT_AXIS_ANGLE, 1, 0, 0);
	glRotated(MOON_POS_ADJUST + 360.0 * myfrac(universaltime/MOON_ORBIT_TIME), 0, 1, 0);
	glTranslated(0, 0, MOON_EARTH_DISTANCE * moon_scale_fac * 0.95);	// to keep it inside sky hemisphere
	// draw quad	
	double moons = MOON_RADIUS * moon_scale_fac;
	glColor3f(1,1,1);
	moontex->set_gl_texture();
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
	
	

	// ******** clouds ********************************************************************
	glDisable(GL_LIGHTING);		// direct lighting turned off
	glDisable(GL_DEPTH_TEST);	// draw all clouds
	lightcol.set_gl_color();	// cloud color depends on day time

	// fixme: cloud color varies with direction to sun (clouds aren't flat, but round, so
	// border are brighter if sun is above/nearby)
	// fixme: add flares after cloud layer (?)

	float clsc = max_view_dist * 0.9;
	glScalef(clsc, clsc, 3000);	// bottom of cloud layer has altitude of 3km., fixme varies with weather
	clouds->set_gl_texture();
	glCallList(clouds_dl);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	color::white().set_gl_color();

	glPopMatrix();	// remove z translate for sky
	
	// modelview matrix is around viewpos now.

}
