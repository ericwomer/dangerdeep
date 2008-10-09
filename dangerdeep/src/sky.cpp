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

// sky simulation and display (OpenGL)
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "oglext/OglExt.h"
#include <GL/glu.h>
#include <SDL.h>

#include "sky.h"
#include "daysky.h"
#include "moon.h"
#include "model.h"
#include "texture.h"
#include "datadirs.h"
#include "global_data.h"
#include "game.h"
#include "matrix4.h"
#include "cfg.h"
#include "primitives.h"

#include <iostream>
using std::vector;


const double CLOUD_ANIMATION_CYCLE_TIME = 3600.0;

/* fixme: idea:
   use perlin noise data as height (3d depth) of cloud.
   compute normals from it and light clouds with bump mapping
   + ambient

   clouds have self-shadowing effects, and may be lit by sun from behind if they
   are thin!

   add rain! (GL_LINES with varying alpha)
*/

sky::sky(const double tm, const unsigned int sectors_h, const unsigned int sectors_v)
	: mytime(tm),
	  sunglow(0),
	  clouds(0),
	  suntex(0),
	  clouds_texcoords(false),
	  sky_vertices(false),
	  sky_indices(true),
	  sun_azimuth(10.0f), sun_elevation(10.0f),
	  turbidity(2.0f)//was 2.0, fixme
{
	// ******************************** create sky geometry
	build_dome(sectors_h, sectors_v);

	// ********************************** init sun/moon
	sunglow = texture::ptr(new texture(get_texture_dir() + "sunglow.png", texture::LINEAR));
	suntex = texture::ptr(new texture(get_texture_dir() + "thesun.png", texture::LINEAR));

	// ********************************** init clouds
	// clouds are generated with Perlin noise.
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
	cloud_coverage = 192;//128;	// 0-256 (none-full)
	cloud_sharpness = 256;	// 0-256
	cloud_animphase = 0;

	cloud_interpolate_func.resize(256);
	for (unsigned n = 0; n < 256; ++n)
		cloud_interpolate_func[n] = unsigned(128-cos(n*M_PI/256)*128);

	noisemaps_0 = compute_noisemaps();
	noisemaps_1 = compute_noisemaps();
	compute_clouds();

	clouds_texcoords.init_data(nr_sky_vertices*2*4, 0, GL_STATIC_DRAW);
	float* ptr = (float*)clouds_texcoords.map(GL_WRITE_ONLY);
	for (unsigned beta = 0; beta <= sectors_v; ++beta) {
		float t = (1.0-float(beta)/sectors_v)/2;
		for (unsigned alpha = 0; alpha <= sectors_h; ++alpha) {
			float x = cos(2*M_PI*alpha/sectors_h);
			float y = sin(2*M_PI*alpha/sectors_h);
			*ptr++ = x*t+0.5;
			*ptr++ = y*t+0.5;
		}
	}
	clouds_texcoords.unmap();

	glsl_clouds.reset(new glsl_shader_setup(get_shader_dir() + "clouds.vshader",
						get_shader_dir() + "clouds.fshader"));
	glsl_clouds->use();
	loc_cloudstex = glsl_clouds->get_uniform_location("tex_cloud");
	glsl_clouds->use_fixed();
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



void sky::compute_clouds()
{
	unsigned mapsize = 8 - cloud_levels;
	unsigned mapsize2 = (2<<mapsize);

	// FIXME could we interpolate between accumulated noise maps
	// to further speed up the process?
	// in theory, yes. formula says that we can...
	// NO! different noise levels must get animated with different speeds
	// for realistic cloud form change (isn't done yet)...
	// also bump mapping for clouds is missing.
	// clouds facing away from the sun shouldn't be black though (because of
	// bump mapping).
	// FIXME use perlin noise generator here!
	vector<vector<Uint8> > cmaps = noisemaps_0;
	float f = cloud_animphase;
	for (unsigned i = 0; i < cloud_levels; ++i)
		for (unsigned j = 0; j < mapsize2 * mapsize2; ++j)
			cmaps[i][j] = Uint8(noisemaps_0[i][j]*(1-f) + noisemaps_1[i][j]*f);

	// create full map
	const unsigned res = 256;
	vector<Uint8> fullmap(res * res);
	unsigned fullmapptr = 0;
	for (unsigned y = 0; y < res; ++y) {
		for (unsigned x = 0; x < res; ++x) {
			unsigned v = 0;
			// accumulate values
			for (unsigned k = 0; k < cloud_levels; ++k) {
				unsigned tv = get_value_from_bytemap(x, y, k, cmaps[k]);
				v += (tv >> k);
			}
			// FIXME generate a lookup table for this function, depending on coverage/sharpness
			if (v < 96) v = 96;
			v -= 96;
			if (v > 255) v = 255;
			fullmap[fullmapptr++] = v;
		}
	}

	clouds = texture::ptr(new texture(fullmap, res, res, GL_LUMINANCE, texture::LINEAR, texture::REPEAT));
}



vector<vector<Uint8> > sky::compute_noisemaps()
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
}



void sky::display(const colorf& lightcolor, const vector3& viewpos, double max_view_dist, bool isreflection) const
{
	// 25th jan 2007, after switch to VBO: skipping sky render brings 4fps.
	// 50->54 in editor

	// FIXME sky for reflection should be drawn different, because earth is curved!
	// far reflections of the sky point outwards into the atmosphere and thus are blueish, but the grey haze color
	// of the horizon!

	// FIXME for reflections this is wrong, so get LIGHT_POS!

	glDisable(GL_LIGHTING);
	glDepthMask(GL_FALSE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_FOG); // no fog on the sky, sun, moon

	// if stars are drawn after the sky, they can appear in front of the sun glow, which is wrong.
	// sky should be blended into the stars, not vice versa, but then we would have to clear
	// the back buffer, that takes 2 frames... DST_ALPHA could be used IF sky stores its alpha

	// because the reflection map may have a lower resolution than the screen
	// we shouldn't draw stars while computing this map. They would appear to big.
	// in fact star light is to weak for water reflections, isn't it?
	// FIXME maybe rotate star positions every day a bit? rotate with earth rotation? that would be
	// more realistic/cooler
	if (isreflection) {
		// no stars, blend sky into black
		glBlendFunc(GL_SRC_ALPHA, GL_ZERO);
	} else {
		_stars.display(max_view_dist);
	}

	glPushMatrix(); // sky scale
	float scale = max_view_dist * 0.95;
	// we need to handle viewpos.z here, or sky moves with viewer when he ascends
	// we subtract some extra height to compensate for wave amplitude.
	// 5m should be enough, but gap can be seen when using less than 100m, strange!
	glTranslated(0, 0, -viewpos.z - 100.0);
	glScaled(scale, scale, scale);

	// render sky
	glBindTexture(GL_TEXTURE_2D, 0);
	glEnableClientState(GL_COLOR_ARRAY);
	sky_colors.bind();
	glColorPointer(4, GL_UNSIGNED_BYTE, 0, 0);
	glEnableClientState(GL_VERTEX_ARRAY);
	sky_vertices.bind();
	glVertexPointer(3, GL_FLOAT, sizeof(vector3f), 0);
	sky_vertices.unbind();
	sky_indices.bind();
	glDrawRangeElements(GL_QUAD_STRIP, 0, nr_sky_vertices-1, nr_sky_indices, GL_UNSIGNED_INT, 0);
	sky_indices.unbind();
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);

	glPopMatrix(); // sky scale

	// restore blend function
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// ******** the sun and the moon *****************************************************

	// draw moon

	// alter blend function so that the moon renders ok in daylight
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);
	_moon.display(moonpos, sunpos, max_view_dist);
	// restore blend function
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// set gl light position.
	if (sundir.z > 0.0) {
		vector3 sunpos = sundir * max_view_dist;
		GLfloat lightpos[4] = { sunpos.x, sunpos.y, sunpos.z, 0.0f };
		glLightfv(GL_LIGHT0, GL_POSITION, lightpos);
	} else {
		// fixme: what is when moon is also below horizon?!
		vector3 moonpos = moondir * max_view_dist;
		GLfloat lightpos[4] = { moonpos.x, moonpos.y, moonpos.z, 0.0f };
		glLightfv(GL_LIGHT0, GL_POSITION, lightpos);
	}

	// draw sun, fixme draw flares/halo
	vector3 sunpos = sundir * (0.96 * max_view_dist);
	double suns = max_view_dist/20; // was 10, 17x17px, needs to be smaller
	glColor4f(1,1,1,1);
	//glColor4f(1,1,1,0.25);
	sunglow /*suntex*/->set_gl_texture();
	glPushMatrix();
	glTranslated(sunpos.x, sunpos.y, sunpos.z);
	matrix4 tmpmat = matrix4::get_gl(GL_MODELVIEW_MATRIX);
	tmpmat.clear_rot();
	tmpmat.set_gl(GL_MODELVIEW);
	primitives::textured_quad(vector2f(-suns,-suns), vector2f(suns,suns)).render();
	glPopMatrix();

	// ******** clouds ********************************************************************
	lightcolor.set_gl_color();	// cloud color depends on day time
	//printf("sunpos.z = %f\n", sunpos.z);

	// FIXME cloud color varies with direction to sun (clouds aren't flat, but round, so
	// border are brighter if sun is above/nearby)
	// also thin clouds appear bright even when facing away from the sun (sunlight
	// passes through them, diffuse lighting in a cloud, radiosity).
	// Dynamic clouds are nice, but "real" clouds (fotographies) look much more realistic.
	// Realistic clouds can be computed, but the question is, how much time this would take.
	// Fotos are better, but static...
	// FIXME add flares after cloud layer (?)

	glPushMatrix();
	glScalef(10000, 10000, 3330);	// bottom of cloud layer has altitude of 3km., fixme varies with weather
	glsl_clouds->use();
	glsl_clouds->set_gl_texture(*clouds, loc_cloudstex, 0);
	sky_vertices.bind();
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, sizeof(vector3f), 0);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	clouds_texcoords.bind();
	glTexCoordPointer(2, GL_FLOAT, 2*4, (float*)0);
	glEnableClientState(GL_COLOR_ARRAY);
	sky_colors.bind();
	glColorPointer(4, GL_UNSIGNED_BYTE, 0, 0);
	sky_colors.unbind();
	sky_indices.bind();
	glDrawRangeElements(GL_QUAD_STRIP, 0, nr_sky_vertices-1, nr_sky_indices, GL_UNSIGNED_INT, 0);
	sky_indices.unbind();
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glsl_clouds->use_fixed();
	glPopMatrix();

	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_FOG);
}



color sky::get_horizon_color(const game& gm, const vector3& viewpos) const
{
	// but why is reading of _first_ color done here? this depends on view direction?!
	// fixme !!!
	return skycolors[0];
}


void sky::build_dome(const unsigned int sectors_h, const unsigned int sectors_v)
{
	const float radius = 1.0f;

	float phi,theta;
	float x,y,z;
	nr_sky_vertices = (sectors_h+1)*(sectors_v+1);
	// 2 start indices, two indices per quad, 4 indices for sector change (2 degenerated quads)
	// but not for last sector.
	nr_sky_indices = 2 + sectors_v*sectors_h*2 + (sectors_v-1)*2*2;
	sky_colors.init_data(nr_sky_vertices * 4, 0, GL_DYNAMIC_DRAW);
	skycolors.resize(nr_sky_vertices);
	std::vector<vector3f> skyverts;
	std::vector<unsigned> skyindices;
	skyverts.reserve(nr_sky_vertices);
	skyangles.reserve(nr_sky_vertices);
	skyindices.reserve(nr_sky_indices);

	// this will define the sphere in height
	for(unsigned int i=0; i<=sectors_v; i++) {
		if(i == 0)
			phi = 0.001;	//	if phi==0 skycolor = -INF, -INF, -INF (BAD)
		else if(i == sectors_v)
			phi = M_PI*0.5;	//	zenith
		else
		{
			float gap = pow((float)i/sectors_v, 1.3f);	//	more strips near horizon
			if(gap<0.5) gap = 0.5;
			phi = (i * (M_PI*0.5) / sectors_v) * gap;
		}

		// definition of the circular sections
		for(unsigned int j=0; j<= sectors_h; j++) {
			theta = 2 * j * M_PI / sectors_h;

			x = radius * cos(theta) * cos(phi);
			y = radius * sin(theta) * cos(phi);
			z = radius * sin(phi);

			skyangles.push_back( vector2f( (1-theta/(M_PI*2))*2*M_PI, phi ) );
			skyverts.push_back( vector3f(x,y,z) );
		}
	}

	// build index list
	for(unsigned i=0; i<sectors_v; i++) {
		// traverse indices in reserve order because angles are ccw, so we walk around
		// the horizontal sector in ccw order (quad-strips normally clockwise)
		for(unsigned j=0; j<=sectors_h;j++) {
			skyindices.push_back((i+1)*(sectors_h+1) + sectors_h - j);
			skyindices.push_back( i   *(sectors_h+1) + sectors_h - j);
		}
		// degenerated quads for sector conjunction
		if (i + 1 < sectors_v) {
			skyindices.push_back( i   *(sectors_h+1));
			skyindices.push_back((i+2)*(sectors_h+1) + sectors_h);
		}
	}
	sky_vertices.init_data(nr_sky_vertices*sizeof(vector3f), &skyverts[0], GL_STATIC_DRAW);
	sky_indices.init_data(nr_sky_indices*sizeof(unsigned), &skyindices[0], GL_STATIC_DRAW);
}



/*
* REBUILD_EPSILON_* values define threshold that needs to be exceeded to
* rebuild a sky colors.
*/
#define REBUILD_EPSILON_AZIMUTH			8.73E-3		//	0.5 deg
#define REBUILD_EPSILON_ELEVATION		8.73E-3		//	0.5 deg
void sky::rebuild_colors(const vector3& sunpos_, const vector3& moonpos_, const vector3& viewpos) const
{
	// compute position of sun and moon
	//fixme: compute_sun_pos works relativly to the viewer!!!
	// but for sky simulation we need "global" azimuth/elevation?!
	sunpos = sunpos_;
	sundir = sunpos.normal();
	float sun_azimuth2 = atan2(sundir.y, sundir.x);
	float sun_elevation2 = asin(sundir.z);
	float sky_alpha = (sundir.z < -0.25) ? 0.0f : ((sundir.z < 0.0) ? 4*(sundir.z+0.25) : 1.0f);

	moonpos = moonpos_;
	moondir = moonpos.normal();

	//	sun_azimuth and sun_elevation are used only for sky color rendering
	//	so they are refreshed every 0.5 deg of sun movement.
	if (fabs(sun_azimuth-sun_azimuth2) > REBUILD_EPSILON_AZIMUTH ||
	    fabs(sun_elevation-sun_elevation2) > REBUILD_EPSILON_ELEVATION) {
		sun_azimuth = sun_azimuth2;
		sun_elevation = sun_elevation2;

		// rebuild colors
		daysky skycol(-sun_azimuth, sun_elevation, turbidity);

		std::vector<vector2f>::const_iterator skyangle = skyangles.begin();
		std::vector<color>::iterator skycolor = skycolors.begin();
		while (skyangle != skyangles.end()) {
#if 0	//	debug (angles mapped on hemisphere)
			*skycolor = colorf( skyangle->x/(M_PI*2), skyangle->y/(M_PI/2.0f), 0, sky_alpha ).to_uint8();
#else
			colorf c = skycol.get_color(skyangle->x, skyangle->y, sun_elevation);
			c.a = sky_alpha;
			if(c.r>1.0) c.r=1.0; if(c.g>1.0) c.g=1.0; if(c.b>1.0) c.b=1.0;
			*skycolor = c.to_uint8();
#endif

			skycolor++; skyangle++;
		}
	}
	sky_colors.init_data(nr_sky_vertices * 4, &skycolors[0], GL_DYNAMIC_DRAW);
}
