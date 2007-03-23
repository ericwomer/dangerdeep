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
#endif

#include <gl.h>
#include <glu.h>
#include <SDL.h>

#include "system.h"
#include "vector3.h"
#include "datadirs.h"
#include "font.h"
#include "model.h"
#include "texture.h"
#include <iostream>
#include <sstream>
#include "image.h"
#define VIEWMODEL

class system* mysys;
int res_x, res_y;
font* font_arial = 0;

#define MODEL_DIR "models/"
#define FONT_DIR "fonts/"

#define SCALE_FACTOR 0.35f

using namespace std;

static void draw_model(model* mdl)
{
	double sc = 1.0/(SCALE_FACTOR*mdl->get_boundbox_size()).length();

	// Side view.
	glLoadIdentity();
	glTranslatef(-0.75f, 1.0f, -2.5f);
	glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
	glRotatef(-90.0f, 0.0f, 0.0f, 1.0f);
	glScalef(0.0f, sc, sc);
	glColor3f(1.0f, 1.0f, 1.0f);
	mdl->display();

	glColor3f(1.0f, 0.0f, 0.0f);
	glBegin(GL_LINES);
	glVertex3f(0.0f, -1.5f/sc, 0.0f);
	glVertex3f(0.0f, 1.5f/sc, 0.0f);
	glEnd();

	// Front view.
	glLoadIdentity();
	glColor3f(1.0f, 1.0f, 1.0f);
	glTranslatef(1.5f, 1.0f, -2.5f);
	glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
	glRotatef(180.0f, 0.0f, 0.0f, 1.0f);
	glScalef(sc, 0.0f, sc);
	mdl->display();

	glColor3f(1.0f, 0.0f, 0.0f);
	glBegin(GL_LINES);
	glVertex3f(-0.5f/sc, 0.0f, 0.0f);
	glVertex3f(0.5f/sc, 0.0f, 0.0f);
	glEnd();

	// Top view.
	glLoadIdentity();
	glColor3f(1.0f, 1.0f, 1.0f);
	glTranslatef(-0.75f, 0.0f, -2.5f);
	glRotatef(-90.0f, 0.0f, 0.0f, 1.0f);
	glScalef(sc, sc, 0.0f);
	mdl->display();
}

static void print_model_datas(model* mdl)
{
	ostringstream os;
	os << "Length: " << mdl->get_length() << " m" << endl
	   << "Width : " << mdl->get_width() << " m" << endl
	   << "Height: " << mdl->get_height() << " m";

	mysys->prepare_2d_drawing();
	font_arial->print(10, 600, os.str());
	mysys->unprepare_2d_drawing();
}

static void view_model(const string& modelfilename)
{
	// vector3 viewangles;
	vector3 pos;
	model* mdl = new model(get_model_dir() + modelfilename);
	bool quit = false;

	while (!quit) {
		glMatrixMode(GL_MODELVIEW);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

		// Draw unit model.
		draw_model(mdl);
		print_model_datas(mdl);

		list<SDL_Event> events = mysys->poll_event_queue();
		for (list<SDL_Event>::iterator it = events.begin(); it != events.end(); ++it) {
			if (it->type == SDL_KEYDOWN) {
				quit = true;
			} else if (it->type == SDL_MOUSEBUTTONUP) {
				quit = true;
			}
		}

		// mysys->prepare_2d_drawing();

		// mysys->unprepare_2d_drawing();
		mysys->swap_buffers();
	}
}

#ifdef WIN32
int WinMain(HINSTANCE, HINSTANCE, LPSTR cmdline, int)
{
    string mycmdline(cmdline);
#else
int main(int argc, char** argv)
{
#endif

	// command line argument parsing
	res_x = 1024;
	bool fullscreen = true;
	list<string> args;
	string programname;

#ifdef WIN32
    // parse mycmdline
    while (mycmdline != "") {
        string::size_type st = mycmdline.find(" ");
        args.push_back(mycmdline.substr(0, st));
        if (st == string::npos) break;
        mycmdline = mycmdline.substr(st+1);
    }
	programname = "damagemodel.exe";
#else
     //parse argc, argv
     while (--argc > 0) args.push_front(string(argv[argc]));
	 programname = argv[0];
#endif

	string modelfilename;
	for (list<string>::iterator it = args.begin(); it != args.end(); ++it) {
		if (*it == "--help") {
			cout << programname << ", usage:\n--help\t\tshow this\n"
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

	mysys = new class system(1.0, 1000.0, res_x, res_y, fullscreen);
	mysys->set_res_2d(1024, 768);
	
	mysys->add_console("A simple model viewer for DftD-.mdl files");
	mysys->add_console("copyright and written 2003 by Thorsten Jordan");

	glMatrixMode(GL_PROJECTION);
	gluOrtho2D(0,1024,0,768);//parameters are left,right,bottom,top
	GLfloat lambient[4] = {0.5,0.5,0.5,1};
	GLfloat ldiffuse[4] = {1,1,1,1};
	GLfloat lposition[4] = {0,0,1,0};
	glLightfv(GL_LIGHT0, GL_AMBIENT, lambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, ldiffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, lposition);
	glEnable(GL_LIGHT0);

	font_arial = new font(get_font_dir() + "font_arial");
	mysys->draw_console_with(font_arial, 0);

	view_model(modelfilename);	

	mysys->write_console(true);
	delete font_arial;
	delete mysys;

	return 0;
}
