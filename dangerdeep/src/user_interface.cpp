// user interface common code
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

#include <iostream>
#include <sstream>
#include <iomanip>
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
#include "vector3.h"
#include "coastmap.h"
#include "ocean_wave_generator.h"
#include "widget.h"
using namespace std;

#define MAX_PANEL_SIZE 256

#define WATER_BUMPMAP_CYCLE_TIME 10.0

// some more interesting values: phase 256, waveperaxis: ask your gfx card, facesperwave 64+,
// wavelength 256+,
#define WAVE_PHASES 256		// no. of phases for wave animation
#define WAVES_PER_AXIS 8	// no. of waves along x or y axis
#define FACES_PER_WAVE 32	// resolution of wave model in x/y dir.
#define FACES_PER_AXIS (WAVES_PER_AXIS*FACES_PER_WAVE)
#define WAVE_LENGTH 128.0	// in meters, total length of one wave (one sine function)
#define TIDECYCLE_TIME 10.0
#define FOAM_VANISH_FACTOR 0.1	// 1/second until foam goes from 1 to 0.
#define FOAM_SPAWN_FACTOR 0.2	// 1/second until full foam reached. maybe should be equal to vanish factor
// obsolete:
#define WAVE_HEIGHT 4.0		// half of difference top/bottom of wave -> fixme, depends on weather

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
	sake. The effect shouldn't be noticeable.
*/

user_interface::user_interface(sea_object* player, game& gm) :
	pause(false), time_scale(1), player_object ( player ),
	panel_visible(true), bearing(0), elevation(0),
	viewmode(4), target(0), zoom_scope(false), mapzoom(0.1),
	mycoastmap("default.map"), freeviewsideang(0), freeviewupang(-90), freeviewpos()
{
	clouds = 0;
	init ();
	panel = new widget(0, 768-128, 1024, 128, "", 0, panelbackgroundimg);
	panel_messages = new widget_list(8, 8, 512, 128 - 2*8);
	panel->add_child(panel_messages);
	panel->add_child(new widget_text(528, 8, 0, 0, texts::get(1)));
	panel->add_child(new widget_text(528, 8+24+5, 0, 0, texts::get(4)));
	panel->add_child(new widget_text(528, 8+48+10, 0, 0, texts::get(5)));
	panel->add_child(new widget_text(528, 8+72+15, 0, 0, texts::get(2)));
	panel->add_child(new widget_text(528+160, 8, 0, 0, texts::get(98)));
	panel_valuetexts[0] = new widget_text(528+100, 8, 0, 0, "000");
	panel_valuetexts[1] = new widget_text(528+100, 8+24+5, 0, 0, "000");
	panel_valuetexts[2] = new widget_text(528+100, 8+48+10, 0, 0, "000");
	panel_valuetexts[3] = new widget_text(528+100, 8+72+15, 0, 0, "000");
	panel_valuetexts[4] = new widget_text(528+160+100, 8, 0, 0, "000");
	for (unsigned i = 0; i < 5; ++i)
		panel->add_child(panel_valuetexts[i]);
}

user_interface::~user_interface ()
{
	delete panel;
	deinit();
}

void user_interface::update_foam(double deltat)
{
	float foamvanish = deltat * FOAM_VANISH_FACTOR;
	for (unsigned k = 0; k < FACES_PER_AXIS*FACES_PER_AXIS; ++k) {
		float& foam = wavefoam[k];
		foam -= foamvanish;
		if (foam < 0.0f) foam = 0.0f;
		wavefoamtexdata[k] = Uint8(255*foam);
	}
	glBindTexture(GL_TEXTURE_2D, wavefoamtex);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, FACES_PER_AXIS, FACES_PER_AXIS, GL_COLOR_INDEX, GL_UNSIGNED_BYTE, &wavefoamtexdata[0]);
}

void user_interface::spawn_foam(const vector2& pos)
{
	// compute texel position from pos here
	unsigned texel = FACES_PER_AXIS*FACES_PER_AXIS/2+FACES_PER_AXIS/2;
	float& f = wavefoam[texel];
	f += 0.1;
	if (f > 1.0f) f = 1.0f;
	wavefoamtexdata[texel] = Uint8(255*f);
}

void user_interface::init ()
{
	// if the constructors of these classes may ever fail, we should use C++ exceptions.
	captains_logbook = new captains_logbook_display;
	ships_sunk_disp = new ships_sunk_display;

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
	wavetileh.resize(WAVE_PHASES);
	wavetilen.resize(WAVE_PHASES);
	wavedisplaylists = glGenLists(WAVE_PHASES);
	system::sys()->myassert(wavedisplaylists != 0, "no more display list indices available");

	// init foam
	wavefoam.resize(FACES_PER_AXIS*FACES_PER_AXIS);
	wavefoamtexdata.resize(FACES_PER_AXIS*FACES_PER_AXIS);
for (unsigned k = 0; k < FACES_PER_AXIS*FACES_PER_AXIS; ++k) {
wavefoam[k]=rnd();wavefoamtexdata[k]=Uint8(255.0f*wavefoam[k]);}
	glGenTextures(1, &wavefoamtex);
	glBindTexture(GL_TEXTURE_2D, wavefoamtex);
	vector<Uint8> wavefoampalette(3*256);
	color wavefoam0(51,88,124), wavefoam1(255,255,255);
	for (unsigned k = 0; k < 256; ++k) {
		color c(wavefoam0, wavefoam1, k/255.0f);
		wavefoampalette[k*3+0] = c.r;
		wavefoampalette[k*3+1] = c.g;
		wavefoampalette[k*3+2] = c.b;
	}
	glColorTable(GL_TEXTURE_2D, GL_RGB, 256, GL_RGB, GL_UNSIGNED_BYTE, &(wavefoampalette[0]));
	glTexImage2D(GL_TEXTURE_2D, 0, GL_COLOR_INDEX8_EXT, FACES_PER_AXIS, FACES_PER_AXIS, 0, GL_COLOR_INDEX, GL_UNSIGNED_BYTE, &wavefoamtexdata[0]);
	//gluBuild2DMipmaps(GL_TEXTURE_2D, GL_COLOR_INDEX8_EXT, FACES_PER_AXIS, FACES_PER_AXIS, GL_COLOR_INDEX, GL_UNSIGNED_BYTE, &wavefoamtexdata[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	// connectivity data is the same for all meshes and thus is reused
	vector<unsigned> waveindices;
	waveindices.reserve(FACES_PER_WAVE*FACES_PER_WAVE*4);
	for (unsigned y = 0; y < FACES_PER_WAVE; ++y) {
		unsigned y2 = y+1;
		for (unsigned x = 0; x < FACES_PER_WAVE; ++x) {
			unsigned x2 = x+1;
			waveindices.push_back(x +y *(FACES_PER_WAVE+1));
			waveindices.push_back(x2+y *(FACES_PER_WAVE+1));
			waveindices.push_back(x2+y2*(FACES_PER_WAVE+1));
			waveindices.push_back(x +y2*(FACES_PER_WAVE+1));
		}
	}

	// fixme: is transformation from global space to tangent space correct?	
	// what about the glRotafef(90, ?, ?, ?) to swap coordinate space?
	// tangent space is computed by the following assumptions:
	// the water lies in the x,y plane, z points up (to the sky). fixme remove glrotate(90,...) to swap coordinates
	// this doesn't affect the lighting now, since we use a 1,1,1 light vector
	vector3f lightvec = vector3f(1,1,1).normal();	// fixme: direction to light source (directional light)

	ocean_wave_generator<float> owg(FACES_PER_WAVE, vector2f(0,1), 20, 0.000005, WAVE_LENGTH, TIDECYCLE_TIME);
	for (unsigned i = 0; i < WAVE_PHASES; ++i) {
		owg.set_time(i*TIDECYCLE_TIME/WAVE_PHASES);
		wavetileh[i] = owg.compute_heights();
		wavetilen[i] = owg.compute_normals();
		vector<float>& h = wavetileh[i];
		vector<vector3f>& n = wavetilen[i];

		vector<vector2f> d = owg.compute_displacements();

#if 0		// compute normals by finite data, just a test
		vector<vector3f> n2 = n;
			// normals computed by heights.
		for (unsigned y = 0; y < FACES_PER_WAVE; ++y) {
			unsigned y1 = (y+FACES_PER_WAVE-1)%FACES_PER_WAVE;
			unsigned y2 = (y+1)%FACES_PER_WAVE;
			for (unsigned x = 0; x < FACES_PER_WAVE; ++x) {
				unsigned x1 = (x+FACES_PER_WAVE-1)%FACES_PER_WAVE;
				unsigned x2 = (x+1)%FACES_PER_WAVE;
				n[y*FACES_PER_WAVE+x] = vector3f(h[y*FACES_PER_WAVE+x1]-h[y*FACES_PER_WAVE+x2],h[y1*FACES_PER_WAVE+x]-h[y2*FACES_PER_WAVE+x],1).normal();
			}
		}
#endif		
		
		glNewList(wavedisplaylists+i, GL_COMPILE);

		// create and use temporary arrays for texture coords (units 0,1), colors and vertices
//		vector<GLfloat> tex0coords;
//		vector<GLfloat> tex1coords;
		vector<GLubyte/*GLfloat*/> colors;
		vector<GLfloat> coords;
//		tex0coords.reserve((FACES_PER_WAVE+1)*(FACES_PER_WAVE+1)*2);
//		tex1coords.reserve((FACES_PER_WAVE+1)*(FACES_PER_WAVE+1)*2);
		colors.reserve((FACES_PER_WAVE+1)*(FACES_PER_WAVE+1)*3);
		coords.reserve((FACES_PER_WAVE+1)*(FACES_PER_WAVE+1)*3);

		float add = 1.0f/FACES_PER_WAVE;
		float fy = 0;
		float texscale0 = 8;//32;	// 128m/4m
		float texscale1 = 2;
		for (unsigned y = 0; y <= FACES_PER_WAVE; ++y) {
			float fx = 0;
			unsigned cy = y%FACES_PER_WAVE;
			for (unsigned x = 0; x <= FACES_PER_WAVE; ++x) {
				unsigned cx = x%FACES_PER_WAVE;
				unsigned ptr = cy*FACES_PER_WAVE+cx;
				// fixme: displacement is ignored here
				// compute transformation matrix R: light (global) space to tangent space
				// R = (R0, R1, R2), R2 = n[ptr], R1 = R2 x (1,0,0), R0 = R1 x R2
				vector3f R1 = vector3f(0, n[ptr].z, -n[ptr].y).normal();
				vector3f R0 = R1.cross(n[ptr]);
				// multiply inverse (transposed) of R with light vector
				vector3f nl = vector3f(R0 * lightvec, R1 * lightvec, n[ptr] * lightvec);
				nl = nl * 0.5 + vector3f(0.5, 0.5, 0.5);
//				tex0coords.push_back(fx*texscale0);	// use OpenGL automatic texture coord generation to save memory (256*65*65*2*2*4= ~17mb)
//				tex0coords.push_back(fy*texscale0);
//				tex1coords.push_back(fx*texscale1);
//				tex1coords.push_back(fy*texscale1);
				colors.push_back(GLubyte(255*nl.x));//nl.x);			// store it as ubyte to save space (256*65*65*3*(4-1)= ~9.5mb)
				colors.push_back(GLubyte(255*nl.y));//nl.y);
				colors.push_back(GLubyte(255*nl.z));//nl.z);
				coords.push_back(fx*WAVE_LENGTH+d[ptr].x);
				coords.push_back(fy*WAVE_LENGTH+d[ptr].y);
				coords.push_back(h[ptr]);
				fx += add;
			}
			fy += add;
		}
		
		// now set pointers, enable arrays and draw elements, finally disable pointers
		glEnableClientState(GL_COLOR_ARRAY);
		glColorPointer(3, GL_UNSIGNED_BYTE /*GL_FLOAT*/, 0, &colors[0]);
		
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(3, GL_FLOAT, 0, &coords[0]);
		
		glDisableClientState(GL_NORMAL_ARRAY);
		
//		glClientActiveTexture(GL_TEXTURE0);
//		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
//		glTexCoordPointer(2, GL_FLOAT, 0, &tex0coords[0]);
		
//		glClientActiveTexture(GL_TEXTURE1);
//		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
//		glTexCoordPointer(2, GL_FLOAT, 0, &tex1coords[0]);
		
		glActiveTexture(GL_TEXTURE0);
		glDrawElements(GL_QUADS, waveindices.size(), GL_UNSIGNED_INT, &waveindices[0]);
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);
		glClientActiveTexture(GL_TEXTURE1);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glClientActiveTexture(GL_TEXTURE0);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);

#if 0		// draw finite and fft normals to compare them, just a test
		glColor3f(1,0,0);
		glBegin(GL_LINES);
		fy = 0;
		for (unsigned y = 0; y < FACES_PER_WAVE; ++y) {
			float fx = 0;
			for (unsigned x = 0; x < FACES_PER_WAVE; ++x) {
				glVertex3f(fx*WAVE_LENGTH, fy*WAVE_LENGTH, h[y*FACES_PER_WAVE+x]);
				glVertex3f(fx*WAVE_LENGTH+4*n[y*FACES_PER_WAVE+x].x, fy*WAVE_LENGTH+4*n[y*FACES_PER_WAVE+x].y, h[y*FACES_PER_WAVE+x]+4*n[y*FACES_PER_WAVE+x].z);
				fx += add;
			}
			fy += add;
		}
		glColor3f(0,1,0);
		fy = 0;
		for (unsigned y = 0; y < FACES_PER_WAVE; ++y) {
			float fx = 0;
			for (unsigned x = 0; x < FACES_PER_WAVE; ++x) {
				glVertex3f(fx*WAVE_LENGTH, fy*WAVE_LENGTH, h[y*FACES_PER_WAVE+x]);
				glVertex3f(fx*WAVE_LENGTH+4*n2[y*FACES_PER_WAVE+x].x, fy*WAVE_LENGTH+4*n2[y*FACES_PER_WAVE+x].y, h[y*FACES_PER_WAVE+x]+4*n2[y*FACES_PER_WAVE+x].z);
				fx += add;
			}
			fy += add;
		}
		glEnd();
#endif

		glEndList();
	}
	
	// create water bump maps
	ocean_wave_generator<float> owgb(64, vector2f(1,1), 1, 0.01, 4.0, WATER_BUMPMAP_CYCLE_TIME);
	for (int i = 0; i < WATER_BUMP_FRAMES; ++i) {
		owgb.set_time(i*WATER_BUMPMAP_CYCLE_TIME/WATER_BUMP_FRAMES);
		vector<vector3f> n = owgb.compute_normals();
		vector<Uint8> un(n.size()*3);
		for (unsigned j = 0; j < n.size(); ++j) {
			float s = 127.5;
			un[3*j+0] = Uint8(s + n[j].x * s);
			un[3*j+1] = Uint8(s + n[j].y * s);
			un[3*j+2] = Uint8(s + n[j].z * s);
		}
		water_bumpmaps[i] = new texture(&un[0], 64, 64, GL_RGB, GL_RGB,
			GL_UNSIGNED_BYTE, GL_LINEAR_MIPMAP_LINEAR, GL_REPEAT);
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

	// create sky hemisphere display list
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

void user_interface::deinit ()
{
	delete captains_logbook;
	delete ships_sunk_disp;

	glDeleteTextures(1, &wavefoamtex);
	
	delete clouds;
	glDeleteLists(clouds_dl, 1);

	for (int i = 0; i < WATER_BUMP_FRAMES; ++i)
		delete water_bumpmaps[i];
		
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
			fullmap[fullmapptr++] = 255;
			fullmap[fullmapptr++] = Uint8(v * unsigned(cloud_alpha[y*256+x]) / 255);
		}
	}

	delete clouds;	
	clouds = new texture(&fullmap[0], 256, 256, GL_LUMINANCE_ALPHA, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE,
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
		smooth_and_equalize_bytemap(mapsize2, noisemaps[i]);
	}
	return noisemaps;
}

Uint8 user_interface::get_value_from_bytemap(unsigned x, unsigned y, unsigned level,
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

void user_interface::smooth_and_equalize_bytemap(unsigned s, vector<Uint8>& map1)
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
	vector3f rz = get_water_normal(pos.xy(), timefac, rollfac);
	vector3f rx = vector3f(1, 0, -rz.x).normal();
	vector3f ry = vector3f(0, 1, -rz.y).normal();
	if (inverse) {
		float mat[16] = {
			rx.x, ry.x, rz.x, 0,
			rx.y, ry.y, rz.y, 0,
			rx.z, ry.z, rz.z, 0,
			0,    0,    0,    1 };
		glMultMatrixf(&mat[0]);
	} else {
		float mat[16] = {
			rx.x, rx.y, rx.z, 0,
			ry.x, ry.y, ry.z, 0,
			rz.x, rz.y, rz.z, 0,
			0,    0,    0,    1 };
		glMultMatrixf(&mat[0]);
	}
}

float user_interface::get_water_height(const vector2& pos, double t) const
{
	system::sys()->myassert(0 <= t && t < 1, "get water height, t wrong");
	int wavephase = int(WAVE_PHASES*t);
	float ffac = FACES_PER_WAVE/WAVE_LENGTH;
	float x = float(myfmod(pos.x, WAVE_LENGTH)) * ffac;
	float y = float(myfmod(pos.y, WAVE_LENGTH)) * ffac;
	int ix = int(floor(x));
	int iy = int(floor(y));
	int ix2 = (ix+1)%FACES_PER_WAVE;
	int iy2 = (iy+1)%FACES_PER_WAVE;
	float fracx = x - ix;
	float fracy = y - iy;
	float a = wavetileh[wavephase][ix+iy*FACES_PER_WAVE];
	float b = wavetileh[wavephase][ix2+iy*FACES_PER_WAVE];
	float c = wavetileh[wavephase][ix+iy2*FACES_PER_WAVE];
	float d = wavetileh[wavephase][ix2+iy2*FACES_PER_WAVE];
	float e = a * (1.0f-fracx) + b * fracx;
	float f = c * (1.0f-fracx) + d * fracx;
	return (1.0f-fracy) * e + fracy * f;
}

vector3f user_interface::get_water_normal(const vector2& pos, double t, double f) const
{
	system::sys()->myassert(0 <= t && t < 1, "get water normal, t wrong");
	int wavephase = int(WAVE_PHASES*t);
	float ffac = FACES_PER_WAVE/WAVE_LENGTH;
	float x = float(myfmod(pos.x, WAVE_LENGTH)) * ffac;
	float y = float(myfmod(pos.y, WAVE_LENGTH)) * ffac;
	int ix = int(floor(x));
	int iy = int(floor(y));
	int ix2 = (ix+1)%FACES_PER_WAVE;
	int iy2 = (iy+1)%FACES_PER_WAVE;
	float fracx = x - ix;
	float fracy = y - iy;
	vector3f a = wavetilen[wavephase][ix+iy*FACES_PER_WAVE];
	vector3f b = wavetilen[wavephase][ix2+iy*FACES_PER_WAVE];
	vector3f c = wavetilen[wavephase][ix+iy2*FACES_PER_WAVE];
	vector3f d = wavetilen[wavephase][ix2+iy2*FACES_PER_WAVE];
	vector3f e = a * (1.0f-fracx) + b * fracx;
	vector3f g = c * (1.0f-fracx) + d * fracx;
	vector3f h = e * (1.0f-fracy) + g * fracy;
	h.z *= (1.0f/f);
	return h.normal();
}

void user_interface::draw_water(const vector3& viewpos, angle dir, double t,
	double max_view_dist) const
{

	// the origin of the coordinate system is the bottom left corner of the tile
	// that the viewer position projected to xy plane is inside.
	glPushMatrix();
	glTranslatef(-myfmod(viewpos.x, WAVE_LENGTH), -myfmod(viewpos.y, WAVE_LENGTH), -viewpos.z);

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

	// set Color to light vector transformed to object space
	// (we need the inverse of the modelview matrix for that, at least the 3x3 upper left
	// part if we have directional light only)
	vector3f lightvec = vector3f(1,0,1).normal();
	lightvec = lightvec * 0.5 + vector3f(0.5, 0.5, 0.5);
	glColor3f(lightvec.x, lightvec.y, lightvec.z);
	
	// set texture unit to combine primary color with texture color via dot3
	float framepart = myfmod(t, WATER_BUMPMAP_CYCLE_TIME)/WATER_BUMPMAP_CYCLE_TIME;
	glPushAttrib(GL_ALL_ATTRIB_BITS);

	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, water_bumpmaps[unsigned(framepart*WATER_BUMP_FRAMES)]->get_opengl_name());

	// GL_DOT3_RGB doesn't work, why?!?!?!
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
	glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_DOT3_RGB); 
	glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_PRIMARY_COLOR);
	glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
	glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_TEXTURE);
	glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);

	GLfloat scalefac0 = 8.0f/WAVE_LENGTH;
	GLfloat plane_s0[4] = { scalefac0, 0.0f, 0.0f, 0.0f };
	GLfloat plane_t0[4] = { 0.0f, scalefac0, 0.0f, 0.0f };
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	glTexGenfv(GL_S, GL_OBJECT_PLANE, plane_s0);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	glTexGenfv(GL_T, GL_OBJECT_PLANE, plane_t0);
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);

	glActiveTexture(GL_TEXTURE1);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, water->get_opengl_name());
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
	glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
	glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_PREVIOUS);
	glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
	glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_TEXTURE);
	glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);

	GLfloat scalefac1 = 2.0f/WAVE_LENGTH;
	GLfloat plane_s1[4] = { 0.0f/*scalefac1*/, 0.0f, 0.0f, 0.0f };
	GLfloat plane_t1[4] = { 0.0f, 0.0f/*scalefac1*/, 0.0f, 0.0f };
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	glTexGenfv(GL_S, GL_OBJECT_PLANE, plane_s1);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	glTexGenfv(GL_T, GL_OBJECT_PLANE, plane_t1);
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);

	glDisable(GL_LIGHTING);
	glBegin(GL_TRIANGLE_STRIP);
	//glMultiTexCoord2f(GL_TEXTURE0,t0,t3);
	//glMultiTexCoord2f(GL_TEXTURE1,t0,t3);
	glVertex3f(c0,c3,0);
	//glMultiTexCoord2f(GL_TEXTURE0,t1,t2);
	//glMultiTexCoord2f(GL_TEXTURE1,t1,t2);
	glVertex3f(c1,c2,wz);
	//glMultiTexCoord2f(GL_TEXTURE0,t3,t3);
	//glMultiTexCoord2f(GL_TEXTURE1,t3,t3);
	glVertex3f(c3,c3,0);
	//glMultiTexCoord2f(GL_TEXTURE0,t2,t2);
	//glMultiTexCoord2f(GL_TEXTURE1,t2,t2);
	glVertex3f(c2,c2,wz);
	//glMultiTexCoord2f(GL_TEXTURE0,t3,t0);
	//glMultiTexCoord2f(GL_TEXTURE1,t3,t0);
	glVertex3f(c3,c0,0);
	//glMultiTexCoord2f(GL_TEXTURE0,t2,t1);
	//glMultiTexCoord2f(GL_TEXTURE1,t2,t1);
	glVertex3f(c2,c1,wz);
	//glMultiTexCoord2f(GL_TEXTURE0,t0,t0);
	//glMultiTexCoord2f(GL_TEXTURE1,t0,t0);
	glVertex3f(c0,c0,0);
	//glMultiTexCoord2f(GL_TEXTURE0,t1,t1);
	//glMultiTexCoord2f(GL_TEXTURE1,t1,t1);
	glVertex3f(c1,c1,wz);
	//glMultiTexCoord2f(GL_TEXTURE0,t0,t3);
	//glMultiTexCoord2f(GL_TEXTURE1,t0,t3);
	glVertex3f(c0,c3,0);
	//glMultiTexCoord2f(GL_TEXTURE0,t1,t2);
	//glMultiTexCoord2f(GL_TEXTURE1,t1,t2);
	glVertex3f(c1,c2,wz);
	glEnd();
	
	// draw waves
	double timefac = myfmod(t, TIDECYCLE_TIME)/TIDECYCLE_TIME;

	// fixme: use LOD (fft with less resolution) for distance waves
	// until about 10km to the horizon

	// compute the rasterization of the triangle p0,p1,p2
	// with p0 = viewer's pos., p1,2 = p0 + viewrange * direction(brearing +,- fov/2)

	vector2 p0(myfmod(viewpos.x, WAVE_LENGTH)/WAVE_LENGTH, myfmod(viewpos.y, WAVE_LENGTH)/WAVE_LENGTH);
	vector2 p1 = p0 + angle(90/*bearing*/+45/*fov/2*/).direction() * 8 /* view dist */;
	vector2 p2 = p0 + angle(90/*bearing*/-45/*fov/2*/).direction() * 8 /* view dist */;
/*
	vector2 *top = &p0, *middle = &p0, *bottom = &p0;
	if (p1.y < top->y) top = &p1;
	if (p2.y < top->y) top = &p2;
	if (p1.y > bottom->y) bottom = &p1;
	if (p2.y > bottom->y) bottom = &p2;
	if (p1.y > top->y && p1.y < bottom->y) middle = &p1;
	if (p2.y > top->y && p2.y < bottom->y) middle = &p2;
//cout << "p0 1 2 y "<<p0.y<<","<<p1.y<<","<<p2.y<<" t m b y "<<top->y<<","<<middle->y<<","<<bottom->y<<"\n";
	vector2f d0x = (p1.x - p0.x)/(p1.y - p0.y);
	vector2f d1x = (p2.x - p0.x)/(p2.y - p0.y);
	float h0 = ceil(p0.x) - p0.x;
	float p0x = p0.x + h0 * d0x;
	float p1x = p0.x + h0 * d1x;
	while (y < bottom->y) {
		if (y >= middle->y) {
			float hmid = ceil(middle->y) - middle->y;
			if (middle_is_left) {
				d0x = (p2.x - p1.x)/(p2.y - p1.y);
				p0x = middle->x + hmid * d0x;
			} else {
				d1x = (p1.x - p2.x)/(p1.y - p2.y);
				p1x = middle->x + hmid * d1x;
			}
		}
		// rasterline
		for (int i = int(floor(p0x)); i < int(ceil(p1x)); ++i) {
			// draw tile
		}
		p0x += d0x;
		p1x += d1x;
		++y;
	}
*/

	glActiveTexture(GL_TEXTURE1);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, wavefoamtex);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
	glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
	glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_PREVIOUS);
	glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
	glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_TEXTURE);
	glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);

	GLfloat scalefac2 = 2.0f/WAVE_LENGTH;	// fixme: adapt
	GLfloat plane_s2[4] = { scalefac2, 0.0f, 0.0f, 0.0f };
	GLfloat plane_t2[4] = { 0.0f, scalefac2, 0.0f, 0.0f };
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	glTexGenfv(GL_S, GL_OBJECT_PLANE, plane_s2);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	glTexGenfv(GL_T, GL_OBJECT_PLANE, plane_t2);
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);

	unsigned dl = wavedisplaylists + int(WAVE_PHASES*timefac);
	for (int y = 0; y < WAVES_PER_AXIS; ++y) {
		for (int x = 0; x < WAVES_PER_AXIS; ++x) {
			glPushMatrix();
			glTranslatef((-WAVES_PER_AXIS/2+x)*WAVE_LENGTH, (-WAVES_PER_AXIS/2+y)*WAVE_LENGTH, 0);
			glCallList(dl);
			glPopMatrix();
		}
	}

	glPopAttrib();

	glPopMatrix();
	glColor3f(1,1,1);
}

void user_interface::draw_terrain(const vector3& viewpos, angle dir,
	double max_view_dist) const
{
	glPushMatrix();
	glTranslatef(0, 0, -viewpos.z);
	terraintex->set_gl_texture();
	mycoastmap.render(viewpos.x, viewpos.y);
	glPopMatrix();
}

void user_interface::draw_view(class system& sys, class game& gm, const vector3& viewpos,
	angle dir, angle elev, bool aboard, bool drawbridge, bool withunderwaterweapons)
{
	//fixme: vessels are flickering in the water, zbuffer seems to be a bit random
	//maybe this is because of large values for translate (up to 11000000 meters)
	//that are stored/treated with less accuracy than needed in opengl

	double max_view_dist = gm.get_max_view_distance();

	sea_object* player = get_player();

	// fixme: make use of game::job interface, 3600/256 = 14.25 secs job period
	float cf = myfmod(gm.get_time(), CLOUD_ANIMATION_CYCLE_TIME)/CLOUD_ANIMATION_CYCLE_TIME - cloud_animphase;
	if (fabs(cf) < (1.0/(3600.0*256.0))) cf = 0;
	if (cf < 0) cf += 1.0;
	advance_cloud_animation(cf);

	double timefac = myfmod(gm.get_time(), TIDECYCLE_TIME)/TIDECYCLE_TIME;
	
	//fixme: get rid of this
	glRotatef(-90,1,0,0);
	// if we're aboard the player's vessel move the world instead of the ship
	if (aboard) {
		//fixme: use player height correctly here
		glTranslatef(0, 0, -player->get_pos().z - get_water_height(player->get_pos().xy(), timefac));
		double rollfac = (dynamic_cast<ship*>(player))->get_roll_factor();
		rotate_by_pos_and_wave(player->get_pos(), timefac, rollfac, true);
	}

	glRotatef(-elev.value(),1,0,0);

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
	GLfloat lambient[4] = {0,0,0,1};//{0.2, 0.2, 0.2, 1};//lightcol.r/255.0/2.0, lightcol.g/255.0/2.0, lightcol.b/255.0/2.0, 1};
	GLfloat ldiffuse[4] = {lightcol.r/255.0, lightcol.g/255.0, lightcol.b/255.0, 1};
	GLfloat lposition[4] = {0,1,1,0};	//fixed for now. fixme
	glLightfv(GL_LIGHT1, GL_AMBIENT, lambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, ldiffuse);
	glLightfv(GL_LIGHT1, GL_POSITION, lposition);

	glDisable(GL_LIGHTING);
	float tmp = 1.0/30000.0;
	glScalef(tmp, tmp, tmp);	// sky hemisphere is stored as 30km in radius
	skycol2.set_gl_color();
	glBindTexture(GL_TEXTURE_2D, 0);
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
	const double EARTH_RADIUS = 6.378e6;
	const double SUN_RADIUS = 696e6;
	const double MOON_RADIUS = 1.738e6;
	const double EARTH_SUN_DISTANCE = 149600e6;
	const double MOON_EARTH_DISTANCE = 384.4e6;
	const double EARTH_ROT_AXIS_ANGLE = 23.45;
	const double MOON_ORBIT_TIME = 29.5306 * 86400.0;
	const double EARTH_ROTATION_TIME = 86400.0;
	const double EARTH_PERIMETER = 2.0 * M_PI * EARTH_RADIUS;
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
	glScalef(clsc, clsc, 3000);	// bottom of cloud layer has altitude of 3km.
	clouds->set_gl_texture();
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
	update_foam(1.0/25.0);  //fixme: deltat needed here
//	spawn_foam(vector2(myfmod(gm.get_time(),256.0),0));
	draw_water(viewpos, dir, gm.get_time(), max_view_dist);
	

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
	// fixme: z translate according to water height here
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
	// fixme: z translate according to water height here
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
		glRotatef(-elev.value()-90,1,0,0);
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
	if (panel_visible) {
		ostringstream os0;
		os0 << setw(3) << left << get_player()->get_heading().ui_value();
		panel_valuetexts[0]->set_text(os0.str());
		ostringstream os1;
		os1 << setw(3) << left << unsigned(fabs(round(sea_object::ms2kts(get_player()->get_speed()))));
		panel_valuetexts[1]->set_text(os1.str());
		ostringstream os2;
		os2 << setw(3) << left << unsigned(round(-get_player()->get_pos().z));
		panel_valuetexts[2]->set_text(os2.str());
		ostringstream os3;
		os3 << setw(3) << left << bearing.ui_value();
		panel_valuetexts[3]->set_text(os3.str());
		ostringstream os4;
		os4 << setw(3) << left << time_scale;
		panel_valuetexts[4]->set_text(os4.str());

		panel->draw();
		panel->process_input(true);
	}
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
	draw_view(sys, gm, viewpos, player->get_heading()+bearing, elevation, true, true, false);

	sys.prepare_2d_drawing();
	draw_infopanel(sys, gm);
	sys.unprepare_2d_drawing();

	int mmx, mmy;
	sys.get_mouse_motion(mmx, mmy);	
	if (sys.get_mouse_buttons() & system::right_button) {
		SDL_ShowCursor(SDL_DISABLE);
		bearing += angle(float(mmx)/4);
		float e = elevation.value_pm180() + float(mmy)/4;
		if (e < 0) e = 0;
		if (e > 90) e = 90;
		elevation = angle(e);
	} else {
		SDL_ShowCursor(SDL_ENABLE);
	}

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
		// vector2 r = player_object->get_pos ().xy () - p.pos;
		vector2 p1 = (p.pos + offset)*mapzoom;
		vector2 p2 = p1 + (p.dir + p.ping_angle).direction() * p.range * mapzoom;
		vector2 p3 = p1 + (p.dir - p.ping_angle).direction() * p.range * mapzoom;
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
	double max_view_dist, const vector2& offset)
{
	// draw sound contacts
	list<ship*> ships;
	gm.sonar_ships(ships, player);
	for (list<ship*>::iterator it = ships.begin(); it != ships.end(); ++it) {
		vector2 ldir = (*it)->get_pos().xy() - player->get_pos().xy();
		ldir = ldir.normal() * 0.666666 * max_view_dist*mapzoom;
		vector2 pos = (player_object->get_pos().xy() + offset) * mapzoom;
		if ((*it)->is_merchant())
			glColor3f(0,0,0);
		else if ((*it)->is_warship())
			glColor3f(0,0.5,0);
		else if ((*it)->is_escort())
			glColor3f(1,0,0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glBegin(GL_LINES);
		glVertex2f(512+pos.x, 384-pos.y);
		glVertex2f(512+pos.x+ldir.x, 384-pos.y-ldir.y);
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

	int mmx, mmy;
	sys.get_mouse_motion(mmx, mmy);
	if (mb & sys.middle_button) {
		mapoffset.x += mmx / mapzoom;
		mapoffset.y += -mmy / mapzoom;
	}

	sea_object* player = get_player ();
	bool is_day_mode = gm.is_day_mode ();

	if ( is_day_mode )
		glClearColor ( 0.0f, 0.0f, 1.0f, 1.0f );
	else
		glClearColor ( 0.0f, 0.0f, 0.75f, 1.0f );
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	double max_view_dist = gm.get_max_view_distance();

	vector2 offset = player->get_pos().xy() + mapoffset;
unsigned detl = 0xffffff;	// fixme: remove, lod test hack
if (mb&2) detl = my*10/384;

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

	glColor3f(1,1,1);
	glPushMatrix();
	glTranslatef(512, 384, 0);
	glScalef(mapzoom, mapzoom, 1);
	glScalef(1,-1,1);
	glTranslatef(-offset.x, -offset.y, 0);
	mycoastmap.draw_as_map(offset, mapzoom, detl);
	glPopMatrix();
	sys.no_tex();

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
	glBegin(GL_LINE_LOOP);
	for (int i = 0; i < 512; ++i) {
		float a = i*2*M_PI/512;
		glVertex2f(512+(sin(a)*max_view_dist-mapoffset.x)*mapzoom, 384-(cos(a)*max_view_dist-mapoffset.y)*mapzoom);
	}
	glEnd();
	glColor3f(1,1,1);

	// draw vessel symbols (or noise contacts)
	submarine* sub_player = dynamic_cast<submarine*> ( player );
	if (sub_player && sub_player->is_submerged ()) {
		// draw pings
		draw_pings(gm, -offset);

		// draw sound contacts
		draw_sound_contact(gm, sub_player, max_view_dist, -offset);

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

	// draw notepad sheet giving target distance, speed and course
	if (target) {
		int nx = 768, ny = 512;
		notepadsheet->draw(nx, ny);
		ostringstream os0, os1, os2;
		// fixme: use estimated values from target/tdc estimation here, make functions for that
		os0 << texts::get(3) << ": " << unsigned(target->get_pos().xy().distance(player->get_pos().xy())) << texts::get(206);
		os1 << texts::get(4) << ": " << unsigned(sea_object::ms2kts(target->get_speed())) << texts::get(208);
		os2 << texts::get(1) << ": " << unsigned(target->get_heading().value()) << texts::get(207);
		font_arial->print(nx+16, ny+40, os0.str(), color(0,0,128));
		font_arial->print(nx+16, ny+60, os1.str(), color(0,0,128));
		font_arial->print(nx+16, ny+80, os2.str(), color(0,0,128));
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
				case SDLK_PLUS : if (mapzoom < 1) mapzoom *= 2; break;
				case SDLK_MINUS : if (mapzoom > 1.0/16384) mapzoom /= 2; break;
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

//	SDL_ShowCursor(SDL_DISABLE);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glRotatef(freeviewupang,1,0,0);
	glRotatef(freeviewsideang,0,1,0);
	float viewmatrix[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, viewmatrix);
	vector3 sidestep(viewmatrix[0], viewmatrix[4], viewmatrix[8]);
	vector3 upward(viewmatrix[1], viewmatrix[5], viewmatrix[9]);
	vector3 forward(viewmatrix[2], viewmatrix[6], viewmatrix[10]);
	glTranslatef(-freeviewpos.x, -freeviewpos.y, -freeviewpos.z);

	// draw everything
	draw_view(sys, gm, player_object->get_pos() + freeviewpos, 0, 0, false, false, true);

	int mx, my;
	sys.get_mouse_motion(mx, my);
	freeviewsideang += mx*0.5;
	freeviewupang -= my*0.5;

	sys.prepare_2d_drawing();
	draw_infopanel(sys, gm);
	sys.unprepare_2d_drawing();

	// keyboard processing
	int key = sys.get_key().sym;
	while (key != 0) {
		if (!keyboard_common(key, sys, gm)) {
			// specific keyboard processing
			switch(key) {
				case SDLK_KP8: freeviewpos -= forward * 5; break;
				case SDLK_KP2: freeviewpos += forward * 5; break;
				case SDLK_KP4: freeviewpos -= sidestep * 5; break;
				case SDLK_KP6: freeviewpos += sidestep * 5; break;
				case SDLK_KP1: freeviewpos -= upward * 5; break;
				case SDLK_KP3: freeviewpos += upward * 5; break;
			}
		}
		key = sys.get_key().sym;
	}
}

void user_interface::add_message(const string& s)
{
	panel_messages->append_entry(s);
	if (panel_messages->get_listsize() > panel_messages->get_nr_of_visible_entries())
		panel_messages->delete_entry(0);
/*
	panel_texts.push_back(s);
	if (panel_texts.size() > 1+MAX_PANEL_SIZE/font_panel->get_height())
		panel_texts.pop_front();
*/		
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
	draw_view(sys, gm, viewpos, player->get_heading()+bearing, 0, false/*true*/, false, false);

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
