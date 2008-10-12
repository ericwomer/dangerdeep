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

#include "oglext/OglExt.h"
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
#include "primitives.h"
#include "tree_generator.h"

#include <time.h>

int res_x, res_y;

void run();

font* font_arial;

texture* terraintex;

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



void run()
{
	terraintex = new texture(get_texture_dir() + "terrain.jpg", texture::LINEAR_MIPMAP_LINEAR);

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

	tree_generator tgn;
	std::auto_ptr<model> treemdl = tgn.generate();

	vector3f wind_movement;

	while (true) {
		double tm1 = sys().millisec();
		double delta_t = tm1 - tm0;
		tm0 = tm1;

		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

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
		wind_movement.z = cos(tm1/2000.0f * M_PI) * 0.01;

		// render ground plane
		float tc = 600, vc = 3000;
		primitives::textured_quad(vector2f(-vc,-vc), vector2f(vc,vc),
					  *terraintex,
					  vector2f(-tc,-tc), vector2f(tc, tc),
					  colorf(0.5,0.8,0.5,1)).render();

		treemdl->display();
		model::material_glsl& leafmat = dynamic_cast<model::material_glsl&>(treemdl->get_material(1));
		glsl_shader_setup& gss = leafmat.get_shadersetup();
		gss.use();
		gss.set_uniform(gss.get_uniform_location("wind_movement"), wind_movement);
		gss.use_fixed();

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

		// record fps
		float fps = fpsm.account_frame();

		sys().prepare_2d_drawing();
		std::ostringstream oss; oss << "FPS: " << fps << "\n(all time total " << fpsm.get_total_fps() << ")";
		font_arial->print(0, 0, oss.str());
		sys().unprepare_2d_drawing();
		
		sys().swap_buffers();
	}

	delete terraintex;
}
