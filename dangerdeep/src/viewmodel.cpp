// a model viewer
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#define DATADIR "./data/"
#endif

#include <gl.h>
#include <glu.h>
#include <SDL.h>

#include "system.h"
#include "vector3.h"
#include "global_data.h"
#include "model.h"
#include "texture.h"
#include <iostream>
#include <sstream>
#include "image.h"
#define VIEWMODEL

class system* mysys;
int res_x, res_y;

#define MODEL_DIR "models/"
#define FONT_DIR "fonts/"

void view_model(const string& modelfilename)
{
	vector3 viewangles;
	vector3 pos;
	model* mdl = new model(get_model_dir() + modelfilename, true, false);

	while (true) {
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		glLoadIdentity();
		glTranslatef(0, 0, -2.5);
		glColor3f(1, 1, 1);
		glRotatef(viewangles.z, 0, 0, 1);
		glRotatef(viewangles.y, 0, 1, 0);
		glRotatef(viewangles.x, 1, 0, 0);

		glBindTexture(GL_TEXTURE_2D, 0);
		glBegin(GL_LINES);
		glColor3f(1,0,0);
		glVertex3f(0,0,0);
		glVertex3f(1,0,0);
		glColor3f(1,1,0);
		glVertex3f(0,0,0);
		glVertex3f(0,1,0);
		glColor3f(0,1,0);
		glVertex3f(0,0,0);
		glVertex3f(0,0,1);
		glColor3f(1,1,1);
		glEnd();
		
		double sc = 1.0/(mdl->get_boundbox_size()*0.5).length();
		glScalef(sc, sc, sc);
		glTranslatef(pos.x, pos.y, pos.z);
		mdl->display();
		mysys->poll_event_queue();
		int key = mysys->get_key().sym;
		int mb = mysys->get_mouse_buttons();
		if (key == SDLK_ESCAPE) break;
		if (key == SDLK_KP4) pos.x -= 1.0;
		if (key == SDLK_KP6) pos.x += 1.0;
		if (key == SDLK_KP8) pos.y -= 1.0;
		if (key == SDLK_KP2) pos.y += 1.0;
		if (key == SDLK_KP1) pos.z -= 1.0;
		if (key == SDLK_KP3) pos.z += 1.0;
		
		if (key == SDLK_s) {
			pair<model::mesh, model::mesh> parts = mdl->get_mesh(0).split(vector3f(0,1,0), -1);
			parts.first.transform(matrix4f::trans(0, 30, 50));
			parts.second.transform(matrix4f::trans(0, -30, 50));
			mdl->add_mesh(parts.first);
			mdl->add_mesh(parts.second);
		}
		
		int mx, my;
		mysys->get_mouse_motion(mx, my);
		if (mb & 1) {
			viewangles.y += mx * 0.5;
			viewangles.x += my * 0.5;
		}
		if (mb & 2) {
			viewangles.z += mx * 0.5;
		}
		mysys->prepare_2d_drawing();
		vector3f minp = mdl->get_min(), maxp = mdl->get_max();
		ostringstream os;
		os << "A simple model viewer for Danger from the Deep.\n"
			<< "Press any key to exit.\n"
			<< "Press left mouse button and move mouse to rotate x/y.\n"
			<< "Press right mouse button and move mouse to rotate z.\n"
			<< "Rotation " << viewangles.x << ", " << viewangles.y << ", " << viewangles.z <<
			"\nTranslation " << pos.x << ", " << pos.y << ", " << pos.z <<
			"\nmin " << minp.x << ", " << minp.y << ", " << minp.z <<
			"\nmax " << maxp.x << ", " << maxp.y << ", " << maxp.z <<
			"\n";
			
		font_arial->print(0, 0, os.str());
		mysys->unprepare_2d_drawing();
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
	programname = "viewmodel.exe";
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

	mysys = new class system(1.0, 1000.0, res_x, fullscreen);
	mysys->set_res_2d(1024, 768);
	
	mysys->add_console("A simple model viewer for DftD-.mdl files");
	mysys->add_console("copyright and written 2003 by Thorsten Jordan");

	GLfloat lambient[4] = {0.1,0.1,0.1,1};
	GLfloat ldiffuse[4] = {1,1,1,1};
	GLfloat lspecular[4] = {0,0,0,0};
	GLfloat lposition[4] = {100,0,100,0};
	glLightfv(GL_LIGHT0, GL_AMBIENT, lambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, ldiffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, lspecular);
	glLightfv(GL_LIGHT0, GL_POSITION, lposition);
	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);

	font_arial = new font(get_font_dir() + "font_arial");
	mysys->draw_console_with(font_arial, 0);
	
	view_model(modelfilename);	

	mysys->write_console(true);
	delete font_arial;
	delete mysys;

	return 0;
}
