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

// tri-tri intersect test
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "system.h"
#include "triangle_collision.h"
#include "log.h"
#include "cfg.h"
#include <SDL.h>
#include "oglext/OglExt.h"
#include "shader.h"
#include "mymain.cpp"

using std::vector;



inline double rnd() { return double(rand())/RAND_MAX; }

int mymain(list<string>& args)
{
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

	system::create_instance(new class system(1.0, 1000.0, 1024, 768, false));
	sys().set_res_2d(1024, 768);
	sys().set_max_fps(60);
	
	srand(time(0));
	vector3f triab[6];
	for (int i = 0; i < 6; ++i) {
		triab[i].x = rnd()*2-1;
		triab[i].y = rnd()*2-1;
		triab[i].z = rnd()*2-1;
	}
	bool intersects = false;
	unsigned tria_or_b = 0;
	unsigned vn = 0;
	unsigned moveaxis = 0;

	vector3f pos(0, 0, 2);
	vector3f viewangles(0, 0, 0);

	glDisable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glsl_shader_setup::default_col->use();

	// hier laufen lassen
	for (bool doquit = false; !doquit; ) {
		list<SDL_Event> events = sys().poll_event_queue();
		for (list<SDL_Event>::iterator it = events.begin(); it != events.end(); ++it) {
			SDL_Event& event = *it;
			if (event.type == SDL_KEYDOWN) {
				switch (event.key.keysym.sym) {
				case SDLK_ESCAPE:
					doquit = true;
					break;
				case SDLK_a:
					tria_or_b = 0;
					break;
				case SDLK_b:
					tria_or_b = 3;
					break;
				case SDLK_1:
					vn = 0;
					break;
				case SDLK_2:
					vn = 1;
					break;
				case SDLK_3:
					vn = 2;
					break;
				case SDLK_x:
					moveaxis = 0;
					break;
				case SDLK_y:
					moveaxis = 1;
					break;
				case SDLK_z:
					moveaxis = 2;
					break;
				default: break;
				}
			} else if (event.type == SDL_MOUSEMOTION) {
				if (event.motion.state & SDL_BUTTON_RMASK) {
					vector3f& v = triab[tria_or_b + vn];
					float& f0 = (&v.x)[moveaxis];
					float& f1 = (&v.x)[(moveaxis+1)%3];
					f0 += event.motion.xrel * 0.01f;
					f1 += event.motion.yrel * 0.01f;
					intersects = triangle_collision_t<float>::compute(triab[0], triab[1], triab[2], triab[3], triab[4], triab[5]);
				} else if (event.motion.state & SDL_BUTTON_LMASK) {
					viewangles.x += event.motion.xrel;
					viewangles.y += event.motion.yrel;
				} else if (event.motion.state & SDL_BUTTON_MMASK) {
					viewangles.y += event.motion.xrel;
					viewangles.z += event.motion.yrel;
				}
			}
		}

		if (intersects)
			glClearColor(1.0, 0.2, 0.2, 0.0);
		else
			glClearColor(0.2, 0.2, 1.0, 0.0);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		glLoadIdentity();
		glTranslated(-pos.x, -pos.y, -pos.z);
		glRotatef(viewangles.z, 0, 0, 1);
		glRotatef(viewangles.y, 0, 1, 0);
		glRotatef(viewangles.x, 1, 0, 0);
		float tmp[6*8];
		for (unsigned i = 0; i < 6; ++i) {
			tmp[i*8+0] = triab[i].x;
			tmp[i*8+1] = triab[i].y;
			tmp[i*8+2] = triab[i].z;
		}
		colorf(0, 1, 0).store_rgba(tmp + 0*8 + 4);
		colorf(0, 1, 0.5).store_rgba(tmp + 1*8 + 4);
		colorf(0, 1, 1).store_rgba(tmp + 2*8 + 4);
		colorf(1, 1, 0).store_rgba(tmp + 3*8 + 4);
		colorf(1, 0.75, 0).store_rgba(tmp + 4*8 + 4);
		colorf(1, 0.5, 0).store_rgba(tmp + 5*8 + 4);
		glsl_shader_setup::default_col->use();
		glVertexPointer(3, GL_FLOAT, sizeof(float)*8, tmp);
		glVertexAttribPointer(glsl_shader_setup::idx_c_color, 4, GL_FLOAT, GL_FALSE, sizeof(float)*8, tmp+4);
		glEnableVertexAttribArray(glsl_shader_setup::idx_c_color);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(glsl_shader_setup::idx_c_color);
		sys().swap_buffers();
	}

	system::destroy_instance();

	return 0;
}
