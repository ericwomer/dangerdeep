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
#include "model.h"
#include "texture.h"
#include "datadirs.h"
#include "global_data.h"
#include "game.h"
#include "matrix4.h"


const double CLOUD_ANIMATION_CYCLE_TIME = 3600.0;

/* fixme: idea:
   use perlin noise data as height (3d depth) of cloud.
   compute normals from it and light clouds with bump mapping
   + ambient

   clouds have self-shadowing effects, and may be lit by sun from behind if they
   are thin!

   add rain! (GL_LINES with varying alpha)
*/

sky::sky(const double tm, const unsigned int sectors_x, const unsigned int sectors_y)
	: mytime(tm), sunglow(0), clouds(0),
	suntex(0), moontex(0), clouds_dl(0), skyhemisphere_dl(0),
	m_sun_azimuth(10.0f), m_sun_elevation(10.0f), m_turbidity(2.0f), m_needsrebuild(true)
{

	// ******************************** create stars
	const unsigned nr_of_stars = 2000;
	stars_pos.reserve(nr_of_stars);
	stars_lumin.reserve(nr_of_stars*4);
	for (unsigned i = 0; i < nr_of_stars; ++i) {
		vector3f p(rnd() * 2.0f - 1.0f, rnd() * 2.0f - 1.0f, rnd());
		stars_pos.push_back(p.normal());
		float fl = rnd();
		fl = 1.0f - fl*fl*fl;
		Uint8 l = Uint8(255*fl);
		stars_lumin.push_back(l);
		stars_lumin.push_back(l);
		stars_lumin.push_back(l);
		stars_lumin.push_back(l);
	}

	// ******************************** create sky geometry
	build_dome(sectors_x, sectors_y);

	// ********************************** init sun/moon
	sunglow = texture::ptr(new texture(get_texture_dir() + "sunglow.png", texture::LINEAR));
	suntex = texture::ptr(new texture(get_texture_dir() + "thesun.png", texture::LINEAR));
	moontex = texture::ptr(new texture(get_texture_dir() + "themoon.png", texture::LINEAR));

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
	cloud_coverage = 192;//128;	// 0-256 (none-full)
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

	// create cloud mesh display list, FIXME get it from sky hemisphere mesh data!
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
	glDeleteLists(clouds_dl, 1);
	glDeleteLists(skyhemisphere_dl, 1);
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
			// FIXME generate a lookup table for this function, depending on coverage/sharpness
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

	clouds = texture::ptr(new texture(fullmap, 256, 256, GL_LUMINANCE_ALPHA, texture::LINEAR, texture::CLAMP_TO_EDGE));
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
}



/*
* REBUILD_EPSILON_* values define threshold that needs to be exceeded to
* rebuild a sky colors.
*/
#define REBUILD_EPSILON_AZIMUTH		8.73E-3		//	0.5 deg
#define REBUILD_EPSILON_ELEVATION		8.73E-3		//	0.5 deg
void sky::display(const game& gm, const vector3& viewpos, double max_view_dist, bool isreflection) const
{
	// FIXME sky for reflection should be drawn different, because earth is curved!
	// far reflections of the sky point outwards into the atmosphere and thus are blueish, but the grey haze color
	// of the horizon!

	// FIXME for reflections this is wrong, so get LIGHT_POS!

	vector3 sundir = gm.compute_sun_pos(viewpos).normal();
	float sun_azimuth = atan2(sundir.y, sundir.x);
	float sun_elevation = asin(sundir.z);
	float sky_alpha = (sundir.z < -0.25) ? 0.0f : ((sundir.z < 0.0) ? 4*(sundir.z+0.25) : 1.0f);

	vector3 moondir = gm.compute_moon_pos(viewpos).normal();
	float moon_azimuth = atan2(moondir.y, moondir.x);
	float moon_elevation = asin(moondir.z);
	float moon_alpha = 0.5 + (0.5-sky_alpha/2);

	// fixme: this works only if time update comes in small steps...
	if( fabs( m_sun_azimuth-sun_azimuth )>REBUILD_EPSILON_AZIMUTH ||
		fabs( m_sun_elevation-sun_elevation )>REBUILD_EPSILON_ELEVATION )
	{
		m_sun_azimuth = sun_azimuth;
		m_sun_elevation = sun_elevation;
		m_needsrebuild = true;
	}

	if (m_needsrebuild)
		rebuild_colors(sky_alpha);

	glDisable(GL_LIGHTING);
	glDepthMask(GL_FALSE);
	glDisable(GL_DEPTH_TEST);

	// the brighter the sun, the deeper is the sky color
	//float sky_alpha = (sundir.z < -0.25) ? 0.0f : ((sundir.z < 0.25) ? 2*(sundir.z+0.20) : 1.0f);

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
		glPushMatrix();
		glScalef(max_view_dist * 0.95, max_view_dist * 0.95, max_view_dist * 0.95);
		glBindTexture(GL_TEXTURE_2D, 0);
		glEnableClientState(GL_COLOR_ARRAY);
		glColorPointer(4, GL_UNSIGNED_BYTE, 0, &stars_lumin[0]);
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(3, GL_FLOAT, sizeof(vector3f), &stars_pos[0].x);
		glDrawArrays(GL_POINTS, 0, stars_pos.size());
		glDisableClientState(GL_COLOR_ARRAY);
		glDisableClientState(GL_VERTEX_ARRAY);
		glPopMatrix();
	}

	glPushMatrix();
	float scale = 10;
	glScaled(scale, scale, scale);
	glTranslated(0, 0, -0.005);	// FIXME move down 10m? to compensate for waves moving up/down (avoid gaps because of that)

	//	render sky
	glEnableClientState(GL_COLOR_ARRAY);
	glColorPointer(4, GL_FLOAT, 0, &(m_skycolors[0]));
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, &(m_skyverts[0]));
	glDrawArrays(GL_QUAD_STRIP, 0, m_skyverts.size());
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);

#if 0	//	debug (hemisphere mesh)
	glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
	glColor4f(1, 1, 1, 1);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, &(m_skyverts[0]));
	glDrawArrays(GL_QUAD_STRIP, 0, m_skyverts.size());
	glDisableClientState(GL_VERTEX_ARRAY);
	glPolygonMode( GL_FRONT, GL_FILL );
#endif

	glPopMatrix();

	//	FIXME render sun
	glPushMatrix();
	glRotatef(angle::from_rad(sun_azimuth).value(), 0, 0, -1);
	glRotatef(angle::from_rad(sun_elevation).value(), 0, -1, 0);
	glEnable(GL_TEXTURE_2D);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, sunglow->get_opengl_name());

	glColor4f(1, 1, 1, 1);
	glBegin(GL_QUADS);
		glMultiTexCoord2f(GL_TEXTURE0, 0, 0);
		glVertex3f(10, -1, 1);
		glMultiTexCoord2f(GL_TEXTURE0, 1, 0);
		glVertex3f(10, 1, 1);
		glMultiTexCoord2f(GL_TEXTURE0, 1, 1);
		glVertex3f(10, 1, -1);
		glMultiTexCoord2f(GL_TEXTURE0, 0, 1);
		glVertex3f(10, -1, -1);
	glEnd();
	glPopMatrix();

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);


	//	render moon
	glPushMatrix();
	glRotatef(angle::from_rad(moon_azimuth).value(), 0, 0, -1);
	glRotatef(angle::from_rad(moon_elevation).value(), 0, -1, 0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, moontex->get_opengl_name());
	glColor4f(1, 1, 1, moon_alpha);
	glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex3f(20, -1, 1);
		glTexCoord2f(1, 0);
		glVertex3f(20, 1, 1);
		glTexCoord2f(1, 1);
		glVertex3f(20, 1, -1);
		glTexCoord2f(0, 1);
		glVertex3f(20, -1, -1);
	glEnd();
	glPopMatrix();

	if (sundir.z > 0.0) {
		vector3 sunpos = sundir * max_view_dist;
		GLfloat lightpos[4] = { sunpos.x, sunpos.y, sunpos.z, 0.0f };
		glLightfv(GL_LIGHT0, GL_POSITION, lightpos);
	}
	else
	{
		vector3 moonpos = moondir * max_view_dist;
		GLfloat lightpos[4] = { moonpos.x, moonpos.y, moonpos.z, 0.0f };
		glLightfv(GL_LIGHT0, GL_POSITION, lightpos);
	}

	// ******** clouds ********************************************************************
	color lightcol = gm.compute_light_color(viewpos);

	lightcol.set_gl_color();	// cloud color depends on day time

	// FIXME cloud color varies with direction to sun (clouds aren't flat, but round, so
	// border are brighter if sun is above/nearby)
	// also thin clouds appear bright even when facing away from the sun (sunlight
	// passes through them, diffuse lighting in a cloud, radiosity).
	// Dynamic clouds are nice, but "real" clouds (fotographies) look much more realistic.
	// Realistic clouds can be computed, but the question is, how much time this would take.
	// Fotos are better, but static...
	// FIXME add flares after cloud layer (?)

	glPushMatrix();
	glScalef(3000, 3000, 333);	// bottom of cloud layer has altitude of 3km., fixme varies with weather
	clouds->set_gl_texture();
	glCallList(clouds_dl);
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	color::white().set_gl_color();
	glPopMatrix();
}



color sky::get_horizon_color(const game& gm, const vector3& viewpos) const
{
	vector3 sundir = gm.compute_sun_pos(viewpos).normal();
	float sun_azimuth = atan2(sundir.y, sundir.x);
	float sun_elevation = asin(sundir.z);
	float sky_alpha = (sundir.z < -0.25) ? 0.0f : ((sundir.z < 0.0) ? 4*(sundir.z+0.25) : 1.0f);

	bool recalc = false;
	if( fabs( m_sun_azimuth-sun_azimuth )>REBUILD_EPSILON_AZIMUTH ||
		fabs( m_sun_elevation-sun_elevation )>REBUILD_EPSILON_ELEVATION )
			recalc = true;

	if(recalc || m_needsrebuild) {
		daysky skycol(sun_azimuth, sun_elevation, m_turbidity);
		return skycol.get_color(0, 0.001).to_uint8();
	} else {
		return m_skycolors[0].to_uint8();
	}
}


void sky::build_dome(const unsigned int sectors_x, const unsigned int sectors_y)
{
	const float radius = 1.0f;

	float phi1,phi2,theta;
	float x,y,z;

	m_skyverts.reserve(sectors_x*sectors_y);
	m_skyangles.reserve(sectors_x*sectors_y);
	m_skycolors.reserve(sectors_x*sectors_y);

	colorf defaultColor(0,0,0,0);

	// this will define the sphere in height
	for(int i=0; i<sectors_y; i++){

		phi1 = i * (M_PI*0.5) / sectors_y;
		if(phi1 == 0.0) phi1 = 0.001;	//	if phi==1 skycolor = -INF, -INF, -INF (BAD)
		phi2 = (i+1) * (M_PI*0.5) / sectors_y;

		// definition of the circular sections
		for(int j=0; j<=sectors_x; j++){

			theta = 2 * j * M_PI / sectors_x;

			// lower point of the quad_strip
			x = radius * cos(theta) * cos(phi1);
			y = radius * sin(theta) * cos(phi1);
			z = radius * sin(phi1);

			m_skyangles.push_back( vector2f( (1-theta/(M_PI*2))*2*M_PI, phi1 ) );
			m_skyverts.push_back( vector3f(x,y,z) );
			m_skycolors.push_back(defaultColor);

			// upper point of the quad strip
			x = radius * cos(theta) * cos(phi2);
			y = radius * sin(theta) * cos(phi2);
			z = radius * sin(phi2);

			m_skyangles.push_back( vector2f( (1-theta/(M_PI*2))*2*M_PI, phi2 ) );
			m_skyverts.push_back( vector3f(x,y,z) );
			m_skycolors.push_back(defaultColor);
		}
	}
}



void sky::rebuild_colors(const float alpha) const
{
	if (!m_needsrebuild) return;

	// fixme: recreating that everytime is a bit wastefully...
	daysky skycol(m_sun_azimuth, m_sun_elevation, m_turbidity);

	std::vector<vector2f>::const_iterator skyangle = m_skyangles.begin();
	std::vector<colorf>::iterator skycolors = m_skycolors.begin();
	while(skyangle != m_skyangles.end()) {
#if 0	//	debug (angles mapped on hemisphere)
		*skycolors = colorf( skyangle->x/(M_PI*2), skyangle->y/(M_PI/2.0f), 0, alpha );
#else
		colorf c = skycol.get_color(skyangle->x, skyangle->y);
		c.a = alpha;
		*skycolors = c;
#endif

		skycolors++; skyangle++;
	}

	m_needsrebuild = false;
}
