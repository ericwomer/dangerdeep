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
#include "make_mesh.h"
#include "xml.h"
#define VIEWMODEL

#include "mymain.cpp"

class system* mysys;
int res_x, res_y;
font* font_arial = 0;

vector4t<GLfloat> lposition(0,0,0,1);

#define LIGHT_ANG_PER_SEC 30


int scalelength(int i)
{
	return (i % 50 == 0) ? 5 : ((i % 10 == 0) ? 4 : ((i % 5 == 0) ? 2 : 1));
}


void view_model(const string& modelfilename, const string& datafilename)
{
	model::enable_shaders = true;

	//fixme ^ make chooseable via command line!

	model* mdl = new model(get_model_dir() + modelfilename);

	mdl->write_to_dftd_model_file("test.xml");

	mdl->get_mesh(0).write_off_file("test.off");

        // rendering mode

        int wireframe = 0;

        // Smoke variables

        bool smoke = false;
        int smoke_type = 0;
        vector3 smoke_pos;
        bool smoke_display = false;

        // Read xml file if supplied
        
        if (!datafilename.empty()) {

                        xml_doc dataxml(get_ship_dir() + datafilename);
                        dataxml.load();
                        xml_elem root = dataxml.child("dftd-ship").child("smoke");
                        smoke_type = root.attri("type");
                        float smokex = root.attrf("x");
                        float smokey = root.attrf("y");
                        float smokez = root.attrf("z");

                        smoke_pos = vector3(smokex, smokey, smokez);
                        smoke = true;
        }

/*
	// fixme test hack
	delete mdl;
	mdl = new model("test.xml", true, false);
	mdl->write_to_dftd_model_file("test2.xml");
	delete mdl;
	mdl = new model("test2.xml", true, false);
	mdl->write_to_dftd_model_file("test3.xml");
*/

	double sc = (mdl->get_boundbox_size()*0.5).length();
	vector3 viewangles;
	// place viewer along positive z-axis
	vector3 pos(0, 0, sc);
	bool lightmove = true;
	bool coordinatesystem = false;

	vector<Uint8> pixels(32*32*3, 64);
	for (unsigned i = 0; i < 32*32; ++i) { pixels[3*i+2] = (((i/32) + (i % 32)) & 1) * 127 + 128; }
	vector<Uint8> bumps(32*32);
	for (unsigned i = 0; i < 32*32; ++i) { bumps[i] = 0; } //(((i/32)%8)<7 && ((i%32)%8)<7)?255:0; }
	model::material::map* dmap = new model::material::map();
	model::material::map* bmap = new model::material::map();
	dmap->mytexture.reset(new texture(pixels, 32, 32, GL_RGB, texture::NEAREST, texture::CLAMP_TO_EDGE));
	bmap->mytexture.reset(new texture(bumps, 32, 32, GL_LUMINANCE, texture::LINEAR, texture::CLAMP_TO_EDGE, true));
	model::material* mat = new model::material();
	mat->specular = color::white();
	mat->colormap.reset(dmap);
	mat->normalmap.reset(bmap);
	mdl->add_material(mat);
	model::mesh* msh = make_mesh::cube(3*sc, 3*sc, 3*sc, 1.0f, 1.0f, false);

//	model::mesh* msh = make_mesh::sphere(1.5*sc, 3*sc, 16, 16, 1.0f, 1.0f, false);
//	model::mesh* msh = make_mesh::cylinder(1.5*sc, 3*sc, 16, 1.0f, 1.0f, true, false);
	msh->mymaterial = mat;
	//mdl->add_mesh(msh);
        msh->compile();
	mdl->compile();

	unsigned time1 = sys().millisec();
	double ang = 0;

//	model::mesh* lightsphere = make_mesh::sphere(5.0f, 5.0f, 4, 4, 1, 1, true, "sun");
	model::mesh* lightsphere = make_mesh::cube(5.0f, 5.0f, 5.0f, 1, 1, true, "sun");
	lightsphere->mymaterial = new model::material();
	lightsphere->mymaterial->diffuse = color(255, 255, 128);
	lightsphere->mymaterial->specular = color(255, 255, 128);
	lightsphere->compile();

	while (true) {

                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                
		// rotate light
		unsigned time2 = sys().millisec();
		if (lightmove && time2 > time1) {
			ang += LIGHT_ANG_PER_SEC*(time2-time1)/1000.0;
			if (ang > 360) ang -= 360;
			time1 = time2;
			lposition.x = 1.4*sc*cos(3.14159*ang/180);
			lposition.z = 1.4*sc*sin(3.14159*ang/180);
		}

		// test: rotate object 1+2
		mdl->set_object_angle(1, ang);
		mdl->set_object_angle(2, ang);
		mdl->set_object_translation(4, ang*50/360);
	
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		glColor3f(1, 1, 1);
		glBindTexture(GL_TEXTURE_2D, 0);

		glLoadIdentity();
		glTranslated(-pos.x, -pos.y, -pos.z);
		glRotatef(viewangles.z, 0, 0, 1);
		glRotatef(viewangles.y, 0, 1, 0);
		glRotatef(viewangles.x, 1, 0, 0);

		glPushMatrix();
		glTranslatef(lposition.x, lposition.y, lposition.z);
		vector4t<GLfloat> null(0,0,0,1);
		glLightfv(GL_LIGHT0, GL_POSITION, &null.x);
		lightsphere->display();
		glPopMatrix();

		glBegin(GL_LINES);
		glVertex3f(0,0,0);
		glVertex3fv(&lposition.x);
		glEnd();

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

		if (coordinatesystem) {
			glDisable(GL_LIGHTING);
			glBegin(GL_LINES);
			glColor4f(1,0,0,1);
			vector3f min = mdl->get_min();
			vector3f max = mdl->get_max();
			float h = max.z;
			float w = max.x;
			glVertex3f(-30, 0, h);
			glVertex3f(30, 0, h);
			glVertex3f(-30, 0, -h);
			glVertex3f(30, 0, -h);
			for (int i = 0; i <= 30; ++i) {
				int l = scalelength(i);
				glVertex3f(i, 0, h);
				glVertex3f(i, 0, h+l);
				glVertex3f(-i, 0, h);
				glVertex3f(-i, 0, h+l);
				glVertex3f(i, 0, -h);
				glVertex3f(i, 0, -h+l);
				glVertex3f(-i, 0, -h);
				glVertex3f(-i, 0, -h+l);
			}
			glColor4f(0,1,0,1);
			glVertex3f(0, -150, h);
			glVertex3f(0, 150, h);
			glVertex3f(0, -150, -h);
			glVertex3f(0, 150, -h);
			for (int i = 0; i <= 150; ++i) {
				int l = scalelength(i);
				glVertex3f(0, i, h);
				glVertex3f(0, i, h+l);
				glVertex3f(0, -i, h);
				glVertex3f(0, -i, h+l);
				glVertex3f(0, i, -h);
				glVertex3f(0, i, -h+l);
				glVertex3f(0, -i, -h);
				glVertex3f(0, -i, -h+l);
			}
			glColor4f(1,1,0,1);
			glVertex3f(w, 0, 30);
			glVertex3f(w, 0, -30);
			glVertex3f(-w, 0, 30);
			glVertex3f(-w, 0, -30);
			for (int i = 0; i <= 30; ++i) {
				int l = scalelength(i);
				glVertex3f(w, 0, i);
				glVertex3f(w+l, 0, i);
				glVertex3f(w, 0, -i);
				glVertex3f(w+l, 0, -i);
				glVertex3f(-w, 0, i);
				glVertex3f(-w+l, 0, i);
				glVertex3f(-w, 0, -i);
				glVertex3f(-w+l, 0, -i);
			}
			glEnd();
			glColor4f(1,1,1,0.5);
			glEnable(GL_LIGHTING);
		}

                msh->display();

                if (wireframe==1) {
                        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                }

		matrix4 mvp = matrix4::get_gl(GL_PROJECTION_MATRIX) * matrix4::get_gl(GL_MODELVIEW_MATRIX);
		mdl->display();
		// test
		//mdl->display_mirror_clip();

                // display smoke origin
                if (smoke && smoke_display) {
                        glDisable(GL_LIGHTING);
                        glDisable(GL_DEPTH_TEST);
                        glBegin(GL_LINES);
                        glColor3f(1,0,0);
                        glVertex3f(smoke_pos.x-1, smoke_pos.y, smoke_pos.z);
                        glVertex3f(smoke_pos.x+1, smoke_pos.y, smoke_pos.z);
                        glVertex3f(smoke_pos.x, smoke_pos.y-1, smoke_pos.z);
                        glVertex3f(smoke_pos.x, smoke_pos.y+1, smoke_pos.z);
                        glVertex3f(smoke_pos.x, smoke_pos.y, smoke_pos.z-1);
                        glVertex3f(smoke_pos.x, smoke_pos.y, smoke_pos.z+1);
                        glEnd();
                        glEnable(GL_DEPTH_TEST);
                        glEnable(GL_LIGHTING);
                }



                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		// draw scales if requested
		if (coordinatesystem) {
			glColor4f(1,1,1,1);
		}

		list<SDL_Event> events = mysys->poll_event_queue();
		for (list<SDL_Event>::iterator it = events.begin(); it != events.end(); ++it) {
			SDL_Event& event = *it;
			if (event.type == SDL_KEYDOWN) {
				switch (event.key.keysym.sym) {
				case SDLK_ESCAPE:
					return;
				case SDLK_KP4: pos.x -= 1.0; break;
				case SDLK_KP6: pos.x += 1.0; break;
				case SDLK_KP8: pos.y -= 1.0; break;
				case SDLK_KP2: pos.y += 1.0; break;
				case SDLK_KP1: pos.z -= 1.0; break;
				case SDLK_KP3: pos.z += 1.0; break;
				case SDLK_l: lightmove = !lightmove; break;
				case SDLK_s:
					{
						pair<model::mesh*, model::mesh*> parts = mdl->get_mesh(0).split(vector3f(0,1,0), -1);
						parts.first->transform(matrix4f::trans(0, 30, 50));
						parts.second->transform(matrix4f::trans(0, -30, 50));
						mdl->add_mesh(parts.first);
						mdl->add_mesh(parts.second);
					}
					break;
				case SDLK_c: coordinatesystem = !coordinatesystem; break;
                                case SDLK_p: smoke_display = !smoke_display; break;
                                case SDLK_w: if (++wireframe>1) wireframe = 0; break;
				default: break;
				}
			} else if (event.type == SDL_MOUSEMOTION) {
				if (event.motion.state & SDL_BUTTON_LMASK) {
					viewangles.y += event.motion.xrel * 0.5;
					viewangles.x += event.motion.yrel * 0.5;
				} else if (event.motion.state & SDL_BUTTON_RMASK) {
					viewangles.z += event.motion.xrel * 0.5;
					viewangles.x += event.motion.yrel * 0.5;
				} else if (event.motion.state & SDL_BUTTON_MMASK) {
					pos.x += event.motion.xrel * 0.5;
					pos.y += event.motion.yrel * 0.5;
				}
			} else if (event.type == SDL_MOUSEBUTTONDOWN) {
				if (event.button.button == SDL_BUTTON_WHEELUP) {
					pos.z -= 2;
				} else if (event.button.button == SDL_BUTTON_WHEELDOWN) {
					pos.z += 2;
				}
			}
		}
		mysys->prepare_2d_drawing();
		vector3f minp = mdl->get_min(), maxp = mdl->get_max();
		ostringstream os;
		os << "A simple model viewer for Danger from the Deep.\n"
			<< "Press any key to exit.\n"
			<< "Press left mouse button and move mouse to rotate x/y.\n"
			<< "Press right mouse button and move mouse to rotate z.\n"
                        << "Press 'p' to toggle smoke origin.\n"
                        << "Press 'w' to toggle wireframe mode.\n"
			<< "Rotation " << viewangles.x << ", " << viewangles.y << ", " << viewangles.z <<
			"\nTranslation " << pos.x << ", " << pos.y << ", " << pos.z <<
			"\nmin " << minp.x << ", " << minp.y << ", " << minp.z <<
			"\nmax " << maxp.x << ", " << maxp.y << ", " << maxp.z <<
			"\n";
		
                if (smoke) {	
                        os << "Smoke: " << ((smoke_display)? "On" : "Off") << "\n";
                        os << "Smoke origin " << smoke_pos.x << ", " << smoke_pos.y << ", " << smoke_pos.z << "\n";
                } else {
                        os << "Smoke: Off (no data xml provided).\n";
                }
                        
		font_arial->print(0, 0, os.str());

		// print scale descriptions
		if (coordinatesystem) {
			matrix4 xf = matrix4::trans(res_x/2, res_y/2, 0) * matrix4::diagonal(res_x/2, -res_y/2, 1) * mvp;
			vector3f min = mdl->get_min();
			vector3f max = mdl->get_max();
			float h = max.z;
			float w = max.x;
			
			glColor4f(1,0,0,1);
			for (int i = 0; i <= 30; i += 5) {
				vector3 a(i, 0, h), b(i, 0, -h);
				vector2 c = (xf * a).xy();
				vector2 d = (xf * b).xy();
				char tmp[20];
				sprintf(tmp, "%i", i);
				font_arial->print(c.x, c.y, tmp);
				font_arial->print(d.x, d.y, tmp);
			}
			glColor4f(0,1,0,1);
			for (int i = 0; i <= 150; i += 5) {
				vector3 a(0, i, h), b(0, i, -h);
				vector2 c = (xf * a).xy();
				vector2 d = (xf * b).xy();
				char tmp[20];
				sprintf(tmp, "%i", i);
				font_arial->print(c.x, c.y, tmp);
				font_arial->print(d.x, d.y, tmp);
			}
			glColor4f(1,1,0,1);
			for (int i = 0; i <= 30; i += 5) {
				vector3 a(w, 0, i), b(-w, 0, i);
				vector2 c = (xf * a).xy();
				vector2 d = (xf * b).xy();
				char tmp[20];
				sprintf(tmp, "%i", i);
				font_arial->print(c.x, c.y, tmp);
				font_arial->print(d.x, d.y, tmp);
			}
		}

		mysys->unprepare_2d_drawing();
		mysys->swap_buffers();
	}

        delete msh;
	delete mdl;
}



int mymain(list<string>& args)
{
	// command line argument parsing
	res_x = 1024;
	bool fullscreen = true;

	string modelfilename;
        string datafilename;
	for (list<string>::iterator it = args.begin(); it != args.end(); ++it) {
		if (*it == "--help") {
			cout << "DftD viewmodel, usage:\n--help\t\tshow this\n"
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
                } else if (*it == "--dataxml") {
                        ++it;
                        datafilename = it==args.end()? "" : it->c_str();
		} else {
			modelfilename = *it;
		}
	}

	res_y = res_x*3/4;

	mysys = new class system(1.0, 1000.0, res_x, fullscreen);
	mysys->set_res_2d(1024, 768);
	
	mysys->add_console("A simple model viewer for DftD-.mdl files");
	mysys->add_console("copyright and written 2003 by Thorsten Jordan");

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
	mysys->draw_console_with(font_arial, 0);
	
	view_model(modelfilename,datafilename);	

	mysys->write_console(true);
	delete font_arial;
	delete mysys;

	return 0;
}
