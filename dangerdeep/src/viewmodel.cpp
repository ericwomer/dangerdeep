// a model viewer
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
#include "system.h"
#include "vector3.h"
#include "model.h"
#include "texture.h"
#include <iostream>
#include <sstream>
#include "config.h"
#include "image.h"

class system* sys;
int res_x, res_y;

string get_data_dir(void) { return DATADIR; }
#define MODEL_DIR "models/"
#define FONT_DIR "fonts/"
font* font_arial;

void view_model(const string& modelfilename)
{
	vector3 viewangles;
	model* mdl = new model((get_data_dir() + MODEL_DIR + modelfilename));

	while (true) {
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		glLoadIdentity();
		glTranslatef(0, 0, -2.5);
		glColor3f(1, 1, 1);
		glRotatef(viewangles.y, 0, 1, 0);
		glRotatef(viewangles.z, 0, 0, 1);
		glRotatef(viewangles.x, 1, 0, 0);
		double sc = 1.0/(mdl->get_boundbox_size()*0.5).length();
		glScalef(sc, sc, sc);
		mdl->display();
		sys->poll_event_queue();
		int key = sys->get_key();
		int mb = sys->get_mouse_buttons();
		if (key != 0) break;
		int mx, my;
		sys->get_mouse_motion(mx, my);
		if (mb & 1) {
			viewangles.x += mx * 0.5;
			viewangles.y += my * 0.5;
		}
		if (mb & 2) {
			viewangles.z += mx * 0.5;
			viewangles.y += my * 0.5;
		}
		if (mb & 4) {
			viewangles.x += mx * 0.5;
			viewangles.z += my * 0.5;
		}
		sys->prepare_2d_drawing();
		font_arial->print(0, 0, "A simple model viewer for Danger from the Deep.");
		font_arial->print(0, 16, "Press any key to exit.");
		font_arial->print(0, 32, "Press left mouse button and move mouse to rotate x/y.");
		font_arial->print(0, 48, "Press right mouse button and move mouse to rotate z/y.");
		font_arial->print(0, 64, "Press middle mouse button and move mouse to rotate x/z.");
		sys->unprepare_2d_drawing();
		sys->swap_buffers();
	}
}

int main(int argc, char** argv)
{
	// command line argument parsing
	res_x = 1024;
	bool fullscreen = true;
	list<string> args;
	while (--argc > 0) args.push_front(string(argv[argc]));
	string modelfilename;
	for (list<string>::iterator it = args.begin(); it != args.end(); ++it) {
		if (*it == "--help") {
			cout << argv[0] << ", usage:\n--help\t\tshow this\n"
			<< "--res n\t\tuse resolution n horizontal,\n\t\tn is 512,640,800,1024 (recommended) or 1280\n"
			<< "--nofullscreen\tdon't use fullscreen\n"
			<< "MODELFILENAME\n";
			return 0;
		} else if (*it == "--nofullscreen") {
			fullscreen = false;
		} else if (*it == "--res") {
			list<string>::iterator it2 = it; ++it2;
			if (it2 != args.end()) {
				int r = atoi(it2->c_str());
				if (r==512||r==640||r==800||r==1024||r==1280)
					res_x = r;
				++it;
			}
		} else {
			modelfilename = *it;
		}
	}

	res_y = res_x*3/4;

	sys = new class system(1.0, 1000.0, res_x, fullscreen);
	sys->set_res_2d(1024, 768);
	
	sys->add_console("A simple model viewer for DftD-.mdl files");
	sys->add_console("copyright and written 2003 by Thorsten Jordan");

	GLfloat lambient[4] = {0.5,0.5,0.5,1};
	GLfloat ldiffuse[4] = {1,1,1,1};
	GLfloat lposition[4] = {0,0,1,0};
	glLightfv(GL_LIGHT1, GL_AMBIENT, lambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, ldiffuse);
	glLightfv(GL_LIGHT1, GL_POSITION, lposition);
	glEnable(GL_LIGHT1);

	font_arial = new font(get_data_dir() + FONT_DIR + "font_arial.png");
	sys->draw_console_with(font_arial, 0);
	
	view_model(modelfilename);	

	sys->write_console(true);
	delete font_arial;
	delete sys;

	return 0;
}
