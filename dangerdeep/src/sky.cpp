// sky simulation and display (OpenGL)
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#pragma warning (disable : 4786)
#endif

#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL.h>

#include "oglext/OglExt.h"

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
//const double MOON_ORBIT_TIME = 27.3333333 * 86400.0;	// fixme: the moon rotates in 27 1/3 days around the earth, but full moon is every 29,5 days
const double MOON_ORBIT_TIME = 29.5306 * 86400.0;	// fixme: the moon rotates in 27 1/3 days around the earth, but full moon is every 29,5 days
//moon rotational plane is ~ 5 degrees rotated to that of earth/sun, fixme
const double EARTH_ROTATION_TIME = 86400.0;		// exactly 1 day, really? compare to moon: earth moves and rotates!!!
const double EARTH_PERIMETER = 2.0 * M_PI * EARTH_RADIUS;
const double EARTH_ORBIT_TIME = 31556926.5;	// in seconds. 365 days, 5 hours, 48 minutes, 46.5 seconds
// these values are difficult to get. SUN_POS_ADJUST should be around +9.8deg (10days of 365 later) but that gives
// a roughly right position but wrong sun rise time by about 40min. fixme
const double SUN_POS_ADJUST = 9.8;	// in degrees. 10 days from 21st. Dec. to 1st. Jan. * 360deg/365.24days
const double MOON_POS_ADJUST = 300.0;	// in degrees. Moon pos in its orbit on 1.1.1939 fixme: this value is a rude guess



sky::sky(double tm) : mytime(tm), skyhemisphere(0), stars(0), skycolor(0), sunglow(0),
	clouds(0), suntex(0), moontex(0), clouds_dl(0), skyhemisphere_dl(0)
{
	skyhemisphere = new model(get_model_dir() + "skyhemisphere.3ds", false, false);

/*


	// generate uv coordinates if desired	
	if (auto_uv == 1) {
		vector3f center = (min + max) * 0.5f;
		center.z = min.z;
		for (vector<mesh>::iterator it = meshes.begin(); it != meshes.end(); ++it) {
			for (vector<mesh::vertex>::iterator vit = it->vertices.begin(); vit != it->vertices.end(); ++vit) {
				vector3f d = (vit->pos - center).normal();
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
				vit->uv.x = u;
				vit->uv.y = v;
			}
			it->use_uv_coords = true;
		}
	}





	glColorPointer(3, GL_UNSIGNED_BYTE, sizeof(model::mesh::vertex));

	glDisableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, &coords[0]);
	glDisableClientState(GL_NORMAL_ARRAY);
	glClientActiveTexture(GL_TEXTURE1);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glClientActiveTexture(GL_TEXTURE0);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);



GLAPI void GLAPIENTRY glVertexPointer( GLint size, GLenum type,
                                       GLsizei stride, const GLvoid *ptr );

GLAPI void GLAPIENTRY glNormalPointer( GLenum type, GLsizei stride,
                                       const GLvoid *ptr );

GLAPI void GLAPIENTRY glColorPointer( GLint size, GLenum type,
                                      GLsizei stride, const GLvoid *ptr );

GLAPI void GLAPIENTRY glIndexPointer( GLenum type, GLsizei stride,
                                      const GLvoid *ptr );

GLAPI void GLAPIENTRY glTexCoordPointer( GLint size, GLenum type,
                                         GLsizei stride, const GLvoid *ptr );

GLAPI void GLAPIENTRY glEdgeFlagPointer( GLsizei stride, const GLvoid *ptr );

GLAPI void GLAPIENTRY glGetPointerv( GLenum pname, GLvoid **params );

GLAPI void GLAPIENTRY glArrayElement( GLint i );

GLAPI void GLAPIENTRY glDrawArrays( GLenum mode, GLint first, GLsizei count );

GLAPI void GLAPIENTRY glDrawElements( GLenum mode, GLsizei count,
                                      GLenum type, const GLvoid *indices );

GLAPI void GLAPIENTRY glInterleavedArrays( GLenum format, GLsizei stride,
                                           const GLvoid *pointer );



*/


	// create maps for stars, sky color and sun glow
	vector<Uint8> starmap(256*256);
	vector<Uint8> skycolormap(32*32*3);
	vector<Uint8> sunglowmap(256*256);
	for (int y = 0; y < 256; ++y) {
		for (int x = 0; x < 256; ++x) {
			Uint8 s = 0;
			if (rnd() < 0.005)
				s = Uint8(255*rnd());
			starmap[y*256+x] = s;
		}
	}
//	ofstream oss("test2.pgm");
//	oss << "P5\n256 256\n255\n";
//	oss.write((const char*)(&starmap[0]), 256*256);
	for (int y = 0; y < 32; ++y) {
		for (int x = 0; x < 32; ++x) {
			float dist = sqrt(float(vector2i(x-16, y-16).square_length()))/16.0f;
			if (dist > 1.0f) dist = 1.0f;
			dist = dist*dist*dist;
			float dist1 = 1.0f-dist;
			skycolormap[(y*32+x)*3+0] = Uint8(173*dist +  73*dist1);
			skycolormap[(y*32+x)*3+1] = Uint8(200*dist + 164*dist1);
			skycolormap[(y*32+x)*3+2] = Uint8(219*dist + 255*dist1);
		}
	}
//	ofstream osc("test.ppm");
//	osc << "P6\n32 32\n255\n";
//	osc.write((const char*)(&skycolormap[0]), 32*32*3);
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
//	ofstream osg("test.pgm");
//	osg << "P5\n256 256\n255\n";
//	osg.write((const char*)(&sunglowmap[0]), 256*256);

	stars = new texture(&starmap[0], 256, 256, GL_LUMINANCE, GL_LUMINANCE, GL_UNSIGNED_BYTE, GL_LINEAR, GL_REPEAT);
	skycolor = new texture(&skycolormap[0], 32, 32, GL_RGB, GL_RGB, GL_UNSIGNED_BYTE, GL_LINEAR, GL_CLAMP);
	sunglow = new texture(&sunglowmap[0], 256, 256, GL_LUMINANCE, GL_LUMINANCE, /*GL_ALPHA, GL_ALPHA,*/ GL_UNSIGNED_BYTE, GL_LINEAR, GL_CLAMP);
	starmap.clear();
	skycolormap.clear();
	sunglowmap.clear();

	// init sun/moon	
	suntex = new texture(get_texture_dir() + "thesun.png", GL_LINEAR);
	moontex = new texture(get_texture_dir() + "themoon.png", GL_LINEAR);
	
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
	delete stars;
	delete skycolor;
	delete sunglow;
	delete clouds;
	delete suntex;
	delete moontex;
	glDeleteLists(clouds_dl, 1);
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
		GL_LINEAR, GL_CLAMP);
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
	tm = myfmod(tm, 86400.0);
	double cf = myfmod(tm, CLOUD_ANIMATION_CYCLE_TIME)/CLOUD_ANIMATION_CYCLE_TIME - cloud_animphase;
	if (fabs(cf) < (1.0/(3600.0*256.0))) cf = 0.0;
	if (cf < 0) cf += 1.0;
	advance_cloud_animation(cf);
}



void sky::display(const vector3& viewpos, double max_view_dist)
{
	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, /*stars*/ skycolor->get_opengl_name());
	
	// skyplanes:
	// 1) the stars (black with grey random dots)
	// 2) blue color, shading to grey haze to the horizon
	// 3) sun glow
	// 4) clouds layer
	// 5) sun lens flares
	// draw: interpolate between 1 and 2 according to time
	// add sun glow, same faces
	// draw lower cloud layer
	// optionally add lens flares
	// stars must come from a texture, also sun glow (it moves), sky color may be drawn with
	// glColor. So three sources: color (c), stars (t0), glow (t1) ->
	// formula is: pixel color = c*(1-timefac)+stars*timefac + glow
	// timefac is blending factor (alpha) and const while drawing (global color alpha?)
	// sky hemisphere is missing multitexture coordinates! fixme
	// read mesh data from model, add anything else? this removes auto texgen in model!fixme
	
	// sky color in primary color. sunglow in tex0, stars in tex1.
	// add tex0 to primary. mix result with tex1 according to primary_alpha. prim_alpha is
	// set before display (if it works... - seems not. use texenv color alpha instead - what else?)

/*
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);//tex0 is sky color, but should also be scaleable (texsubimage?)
	glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_ADD);
	glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_PRIMARY_COLOR);
	glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
	glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_TEXTURE);
	glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
*/

	// fixme: texture coords for unit 1 are missing. this is why sky display fails.
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);
/*
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, sunglow->get_opengl_name());
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD);
*/
/*
	glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE);
	glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_PREVIOUS);
	glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
	glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_TEXTURE);
	glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
*/

/*	
	glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_PRIMARY_COLOR);//primary color
	glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
	glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_TEXTURE);
	glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);

*/

	// fixme: sky should be brighter near the sun
	// two texture components: top-down color gradient (blue->grey e.g.)
	// brightness glow around sun position (exponential luminance map), border is black, clamped
	// brightness map controls mix between sky color and light color
	// this allows sky display without texture recomputation
	glPushMatrix();
	glTranslatef(0, 0, -viewpos.z);
	glPushMatrix();
	glScalef(max_view_dist, max_view_dist, max_view_dist);	// fixme dynamic
	double dt = get_day_time(mytime);
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
	GLfloat lambient[4] = {0,0,0,1};//{0.2, 0.2, 0.2, 1};//lightcol.r/255.0/2.0, lightcol.g/255.0/2.0, lightcol.b/255.0/2.0, 1};
	GLfloat ldiffuse[4] = {lightcol.r/255.0, lightcol.g/255.0, lightcol.b/255.0, 1};
	GLfloat lposition[4] = {0,1,1,0};	//fixed for now. fixme
	glLightfv(GL_LIGHT1, GL_AMBIENT, lambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, ldiffuse);
	glLightfv(GL_LIGHT1, GL_POSITION, lposition);

	glDisable(GL_LIGHTING);
	float tmp = 1.0/30000.0;
	glScalef(tmp, tmp, tmp);	// sky hemisphere is stored as 30km in radius
	//lightcol.set_gl_color();
	color::white().set_gl_color();
	glActiveTexture(GL_TEXTURE0);
	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glLoadIdentity();
//	glScalef(8.0f, 8.0f, 1.0f);
//	glTranslatef(0.2f, 0.3f, 0.0f);
	glTexCoord2f(0.5, 0.5);	// fix hack for sky color, real tex coords for sky hemisphere are missing

	skyhemisphere->display();
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	color::white().set_gl_color();

	// clean up texture units 1 and 0
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glEnable(GL_LIGHTING);

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
	glRotated(360.0 * -myfmod(universaltime, EARTH_ROTATION_TIME)/EARTH_ROTATION_TIME, 0, 1, 0);
	glRotated(-EARTH_ROT_AXIS_ANGLE, 1, 0, 0);
	glRotated(SUN_POS_ADJUST + 360.0 * myfmod(universaltime, EARTH_ORBIT_TIME)/EARTH_ORBIT_TIME, 0, 1, 0);
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
//	glLightfv(GL_LIGHT1, GL_POSITION, lp);

	glPopMatrix();	// remove sun space

	// draw moon
	glPushMatrix();
	// Transform earth space to viewer space
	double moon_scale_fac = max_view_dist / MOON_EARTH_DISTANCE;
	glTranslated(0, 0, -EARTH_RADIUS * moon_scale_fac);
	glRotated(360.0 * -viewpos.y * 4 / EARTH_PERIMETER, 0, 1, 0);
	glRotated(360.0 * -viewpos.x * 2 / EARTH_PERIMETER, 1, 0, 0);
	// Transform moon space to earth space
	glRotated(360.0 * -myfmod(universaltime, EARTH_ROTATION_TIME)/EARTH_ROTATION_TIME, 0, 1, 0);
	glRotated(-EARTH_ROT_AXIS_ANGLE, 1, 0, 0);
	glRotated(MOON_POS_ADJUST + 360.0 * myfmod(universaltime, MOON_ORBIT_TIME)/MOON_ORBIT_TIME, 0, 1, 0);
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
