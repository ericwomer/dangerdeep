// a model viewer
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#define DATADIR "./data/"
#else
#include "../config.h"
#endif

#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL.h>

#include "system.h"
#include "vector3.h"
#include "model.h"
#include "texture.h"
#include <iostream>
#include <sstream>
#include "image.h"
#define VIEWMODEL
font* font_arial;

class system* sys;
int res_x, res_y;

#define MODEL_DIR "models/"
#define FONT_DIR "fonts/"

#define SCALE_FACTOR 0.35f

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

	sys->prepare_2d_drawing();
	font_arial->print(10, 600, os.str());
	sys->unprepare_2d_drawing();
}

static void view_model(const string& modelfilename)
{
	// vector3 viewangles;
	vector3 pos;
	model* mdl = new model(string(DATADIR) + MODEL_DIR + modelfilename);

	while (true) {
		glMatrixMode(GL_MODELVIEW);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

		// Draw unit model.
		draw_model(mdl);
		print_model_datas(mdl);

		sys->poll_event_queue();
		int key = sys->get_key().sym;
		// int mb = sys->get_mouse_buttons();
		if (key == SDLK_ESCAPE) break;

		// sys->prepare_2d_drawing();

		// sys->unprepare_2d_drawing();
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

	glMatrixMode(GL_PROJECTION);
	gluOrtho2D(0,0, 1024,768);//fixme: to Markus: parameters are left,right,bottom,top and hence should be 0,1024,0,768.
	GLfloat lambient[4] = {0.5,0.5,0.5,1};
	GLfloat ldiffuse[4] = {1,1,1,1};
	GLfloat lposition[4] = {0,0,1,0};
	glLightfv(GL_LIGHT1, GL_AMBIENT, lambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, ldiffuse);
	glLightfv(GL_LIGHT1, GL_POSITION, lposition);
	glEnable(GL_LIGHT1);

	font_arial = new font(string(DATADIR) + FONT_DIR + "font_arial.png");
	sys->draw_console_with(font_arial, 0);

	view_model(modelfilename);	

	sys->write_console(true);
	delete font_arial;
	delete sys;

	return 0;
}
