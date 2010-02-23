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
	int res_x, res_y;
	font* font_arial = 0;

	res_x = 640;
	res_y = res_x*3/4;

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
	glEnable(GL_LIGHTING);
	glEnable(GL_NORMALIZE);

	font_arial = new font(get_font_dir() + "font_arial");
	sys().draw_console_with(font_arial, 0);

	{
		sys().prepare_2d_drawing();

		primitives::quad( vector2f( 20, 20 ), vector2f( 40, 40 ), colorf( 1, 0, 0 ) ).render();
		primitives::quad( vector2f( 40, 20 ), vector2f( 60, 40 ), colorf( 0, 1, 0 ) ).render();
		primitives::quad( vector2f( 60, 20 ), vector2f( 80, 40 ), colorf( 0, 0, 1 ) ).render();

		sys().unprepare_2d_drawing();

		sys().swap_buffers();

		SDL_Delay( 5000 );
	}

	delete font_arial;

	system::destroy_instance();

	return 0;
}
