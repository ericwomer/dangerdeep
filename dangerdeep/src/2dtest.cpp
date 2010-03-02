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

// a model viewer
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#define DATADIR "./data/"
#endif

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "system.h"
#include "vector3.h"
#include "datadirs.h"
#include "font.h"
#include "model.h"
#include "texture.h"
#include "image.h"
#include "make_mesh.h"
#include "xml.h"
#include "filehelper.h"
#include "widget.h"
#include "objcache.h"
#include "log.h"
#include "primitives.h"
#include <glu.h>
#include <SDL.h>

#include "mymain.cpp"

using std::vector;


int mymain(list<string>& args)
{
	int res_x, res_y, res_area_2d_w, res_area_2d_h, res_area_2d_x, res_area_2d_y;
//	font* font_arial = 0;

	res_x = 640;
	res_y = res_x*3/4;

	// compute 2d area and resolution. it must be 4:3 always.
	if (res_x * 3 >= res_y * 4) {
		// screen is wider than high
		res_area_2d_w = res_y * 4 / 3;
		res_area_2d_h = res_y;
		res_area_2d_x = (res_x - res_area_2d_w) / 2;
		res_area_2d_y = 0;
	} else {
		// screen is higher than wide
		res_area_2d_w = res_x;
		res_area_2d_h = res_x * 3 / 4;
		res_area_2d_x = 0;
		res_area_2d_y = (res_y - res_area_2d_h) / 2;
		// maybe limit y to even lines for interlaced displays?
		//res_area_2d_y &= ~1U;
	}

	// parse configuration
	cfg& mycfg = cfg::instance();
	mycfg.register_option("screen_res_x", 1024);
	mycfg.register_option("screen_res_y", 768);
	mycfg.register_option("fullscreen", true);
	mycfg.register_option("debug", false);
	mycfg.register_option("sound", true);
	mycfg.register_option("use_hqsfx", true);
	mycfg.register_option("use_ani_filtering", false);
	mycfg.register_option("anisotropic_level", 1.0f);
	mycfg.register_option("use_compressed_textures", false);
	mycfg.register_option("multisampling_level", 0);
	mycfg.register_option("use_multisampling", false);
	mycfg.register_option("bloom_enabled", false);
	mycfg.register_option("hdr_enabled", false);
	mycfg.register_option("hint_multisampling", 0);
	mycfg.register_option("hint_fog", 0);
	mycfg.register_option("hint_mipmap", 0);
	mycfg.register_option("hint_texture_compression", 0);
	mycfg.register_option("vsync", false);
	mycfg.register_option("water_detail", 128);
	mycfg.register_option("wave_fft_res", 128);
	mycfg.register_option("wave_phases", 256);
	mycfg.register_option("wavetile_length", 256.0f);
	mycfg.register_option("wave_tidecycle_time", 10.24f);
	mycfg.register_option("usex86sse", true);
	mycfg.register_option("language", 0);
	mycfg.register_option("cpucores", 1);
	mycfg.register_option("terrain_texture_resolution", 0.1f);

	system::create_instance(new class system(1.0, 1000.0, res_x, res_y, false));
	sys().set_res_2d(640, 480);
	sys().set_max_fps(25);
	
	log_info("A simple 2D test for ATI cards");

	GLfloat lambient[4] = {0.1,0.1,0.09,1};
	GLfloat ldiffuse[4] = {1,1,0.9,1};
	GLfloat lspecular[4] = {1,1,0.9,1};
	glLightfv(GL_LIGHT0, GL_AMBIENT, lambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, ldiffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, lspecular);
	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);


	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	sdl_image test_img( get_image_dir() + "sv_r2c2_red_pos8.jpg" );
	colorf col(1,1,1,1);
	texture *test_texture = new texture( test_img, 0, 0, test_img->w, test_img->h, texture::NEAREST, texture::CLAMP );
	texture *test_texture2 = new texture( test_img, 0, 0, test_img->w, test_img->h, texture::LINEAR, texture::CLAMP );
	texture *test_texture3 = new texture( test_img, 0, 0, test_img->w, test_img->h, texture::NEAREST_MIPMAP_NEAREST, texture::CLAMP );

	{
		glFlush();
		glViewport(res_area_2d_x, res_area_2d_y, res_area_2d_w, res_area_2d_h);
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		gluOrtho2D(0, res_area_2d_w, 0, res_area_2d_h);
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();
		glTranslatef(0, res_area_2d_h, 0);
		glScalef(1, -1, 1);
		glDisable(GL_DEPTH_TEST);
//		glCullFace(GL_FRONT);
//		glPixelZoom(float(res_area_2d_w)/res_area_2d_w, -float(res_area_2d_h)/res_area_2d_h);	// flip images
		glDisable(GL_LIGHTING);
		glDisable(GL_CULL_FACE);

		test_texture->draw( 0, 0, col );
		test_texture2->draw( 200, 200, col );
		test_texture3->draw( 0, 200, col );

		primitives::quad( vector2f( 20, 20 ), vector2f( 40, 40 ), colorf( 1, 0, 0 ) ).render();
		primitives::quad( vector2f( 40, 20 ), vector2f( 60, 40 ), colorf( 0, 1, 0 ) ).render();
		primitives::quad( vector2f( 60, 20 ), vector2f( 80, 40 ), colorf( 0, 0, 1 ) ).render();

		glFlush();
//		glPixelZoom(1.0f, 1.0f);
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
		glEnable(GL_DEPTH_TEST);
//		glCullFace(GL_BACK);
		glEnable(GL_LIGHTING);

		sys().swap_buffers();

		SDL_Delay( 5000 );
	}

	delete test_texture;

	system::destroy_instance();

	return 0;
}
