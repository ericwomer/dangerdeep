// main program
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL/SDL.h>
#include "system.h"
#include "vector3.h"
#include "model.h"
#include "texture.h"
#include "game.h"
#include "menu.h"
#include "global_data.h"
#include "texts.h"
#include <iostream>
#include <sstream>
#include "config.h"

class system* sys;
int res_x, res_y;

void draw_background_and_logo(void)
{
	sys->draw_image(0, 0, 512, 512, titel[0]);
	sys->draw_image(512, 0, 512, 512, titel[1]);
	sys->draw_image(0, 512, 512, 256, titel[2]);
	sys->draw_image(512, 512, 512, 256, titel[3]);
	font_logo->print_hc(1024, 150, "Danger from the Deep", color(255,255,255), true);
}

void menu_convoy_battle(void)
{
	menu m;
	menu::item mi(TXT_Selectsubtype[language]);
	mi.add_switch(TXT_subVIIc[language]);
	mi.add_switch(TXT_subXXI[language]);
	mi.set_switch_nr(0);
	m.add_item(mi);
	m.add_item(TXT_Startmission[language]);
	m.add_item(TXT_Returntopreviousmenu[language]);

	while (true) {
		sys->prepare_2d_drawing();
		draw_background_and_logo();
		
		sys->poll_event_queue();
		int key = sys->get_key();
		m.draw(1024, 768, font_tahoma);
		int mmsel = m.input(key, 0, 0, 0) & 0xffff;
		sys->unprepare_2d_drawing();
		sys->swap_buffers();
		if (mmsel == 2) break;
		switch (mmsel & 0xffff) {
			case 0: break;
			case 1:	{
				unsigned subtype = m.get_switch_nr(0);
				// just a test, fixme
				submarine* playersub = new submarine(subtype == 0 ? submarine::typeVIIc : submarine::typeXXI, vector3(2000, 1000, -30), 270);
//				ship* playership = new ship(2, vector3(2000, 1000, 0), 270);
				game* test = new game(playersub);
//				game* test = new game(playership);
				ship* s = new ship(3, vector3(0,150,0));
				s->get_ai()->add_waypoint(vector2(0,3000));
				s->get_ai()->add_waypoint(vector2(3000,3000));
				s->get_ai()->add_waypoint(vector2(3000,0));
				s->get_ai()->add_waypoint(vector2(0,0));
				s->get_ai()->cycle_waypoints();
				test->spawn_ship(s);
				ship* s2 = new ship(1, vector3(0,-150,0));
				s2->get_ai()->follow(s);
                        	test->spawn_ship(s2);
				s2 = new ship(0, vector3(-200,-150,0));
				s2->get_ai()->follow(s);
                        	test->spawn_ship(s2);
				s2 = new ship(2, vector3(-400,-450,0));
				s2->get_ai()->follow(s);
                        	test->spawn_ship(s2);
				s2 = new ship(2, vector3(-800,-850,0));
				s2->get_ai()->follow(s);
                        	test->spawn_ship(s2);
				test->main_playloop(*sys);
				delete test;
				break;
				}
		}
	}
}

void menu_single_mission(void)
{
	menu m;
	m.add_item(TXT_Warshipengagement[language]);
	m.add_item(TXT_Convoybattle[language]);
	m.add_item(TXT_Historicalmission[language]);
	m.add_item(TXT_Returntomainmenu[language]);

	while (true) {
		sys->prepare_2d_drawing();
		draw_background_and_logo();
		m.draw(1024, 768, font_tahoma);

		sys->poll_event_queue();
		int key = sys->get_key();
		int mmsel = m.input(key, 0, 0, 0) & 0xffff;
		sys->unprepare_2d_drawing();
		sys->swap_buffers();
		if (mmsel == 3) break;
		switch (mmsel) {
			case 0: break;
			case 1:	menu_convoy_battle(); break;
			case 2: break;
		}
	}
}

void menu_multiplayer(void)
{
	menu m;	// just a test
	m.add_item(TXT_Createnetworkgame[language]);
	m.add_item(TXT_Joinnetworkgame[language]);
	m.add_item(menu::item(TXT_Enternetworkportnr[language], "7896"));
	m.add_item(TXT_Returntomainmenu[language]);

	while (true) {
		sys->prepare_2d_drawing();
		draw_background_and_logo();
		m.draw(1024, 768, font_tahoma);

		sys->poll_event_queue();
		int key = sys->get_key();
		int mmsel = m.input(key, 0, 0, 0) & 0xffff;
		sys->unprepare_2d_drawing();
		sys->swap_buffers();
		if (mmsel == 3) break;
		switch (mmsel) {
			case 0: break;
			case 1:	break;
			case 2: break;
		}
	}
}

void show_vessels(void)
{
	int vessel = 0;
	double zangle = 0;
	double xangle = 0;
	unsigned lasttime = sys->millisec();
	while (true) {
		if (vessel < 0) vessel = 5;
		if (vessel > 5) vessel = 0;
		model* m = 0;
		const char* mname = 0;
		switch(vessel) {
			case 0: m = merchant_medium; mname = "Medium merchant ship"; break;
 			case 1: m = subVII; mname = "Submarine type VII"; break;
 			case 2: m = subXXI; mname = "Submarine type XXI"; break;
 			case 3: m = destroyer_tribal; mname = "Detroyer Tribal class"; break;
 			case 4: m = troopship_medium; mname = "Medium Troopship"; break;
			case 5: m = battleship_malaya; mname = "Battleship Malaya class"; break;
 		}

		glClear(GL_DEPTH_BUFFER_BIT);
		sys->prepare_2d_drawing();
		sys->draw_image(0, 0, 512, 512, titel[0]);
		sys->draw_image(512, 0, 512, 512, titel[1]);
		sys->draw_image(0, 512, 512, 256, titel[2]);
		sys->draw_image(512, 512, 512, 256, titel[3]);
		font_tahoma->print_hc(1024, 650, mname);
		font_tahoma->print_hc(1024, 768 - font_tahoma->get_height(),
			"Cursor keys to rotate, Page up/down to cycle, Escape to exit");
		sys->unprepare_2d_drawing();

		glLoadIdentity();
		glTranslatef(0, 0, -2.5);
		glRotatef(-80, 1, 0, 0);
		glRotatef(zangle, 0, 0, 1);
		glRotatef(xangle, 1, 0, 0);
		double s = 2.0/vector2(m->get_length(), m->get_width()).length();
		glScalef(s, s, s);
		m->display();
		unsigned thistime = sys->millisec();
		zangle += (thistime - lasttime) * 5 / 1000.0;
		lasttime = thistime;
	
		sys->poll_event_queue();
		int key = sys->get_key();
		if (key == SDLK_ESCAPE) break;
		if (key == SDLK_PAGEUP) --vessel;
		if (key == SDLK_PAGEDOWN) ++vessel;
		if (key == SDLK_LEFT) zangle += 5;
		if (key == SDLK_RIGHT) zangle -= 5;
		if (key == SDLK_UP) xangle += 5;
		if (key == SDLK_DOWN) xangle -= 5;
		sys->swap_buffers();
	}
}

int main(int argc, char** argv)
{
/*
	//fixme use getenv for $(HOME)
	// move to another function
	// package game options in own class
	// config file reading
	FILE* fcfg = fopen("~/.dangerdeep.rc", "rt");
	if (!fcfg) {
		cout << "no config file found\n";
		fcfg = fopen("~/.dangerdeep.rc", "wt");
		if (!fcfg) {
			cout << "could not write config file\n";
		} else {
			fclose(fcfg);
		}
	} else {
		fclose(fcfg);
	}
*/

	// command line argument parsing
	res_x = 1024;
	bool fullscreen = true;
	list<string> args;
	while (--argc > 0) args.push_front(string(argv[argc]));
	for (list<string>::iterator it = args.begin(); it != args.end(); ++it) {
		if (*it == "--help") {
			cout << argv[0] << ", usage:\n--help\t\tshow this\n"
			<< "--res n\t\tuse resolution n horizontal,\n\t\tn is 512,640,800,1024 (recommended) or 1280\n"
			<< "--nofullscreen\tdon't use fullscreen\n";
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
		}
	}

	res_y = res_x*3/4;
	// weather conditions and earth curvature allow 30km sight at maximum.
	sys = new class system(1.0, 30000.0, res_x, fullscreen);
	sys->set_res_2d(1024, 768);
	
	sys->add_console("$ffffffDanger $c0c0c0from the $ffffffDeep");
	sys->add_console("$ffff00copyright and written 2003 by $ff0000Thorsten Jordan");
	sys->add_console(string("$ff8000version ")+VERSION);
	sys->add_console("$80ff80*** welcome ***");

	GLfloat lambient[4] = {0.5,0.5,0.5,1};
	GLfloat ldiffuse[4] = {1,1,1,1};
	GLfloat lposition[4] = {0,0,1,0};
	glLightfv(GL_LIGHT1, GL_AMBIENT, lambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, ldiffuse);
	glLightfv(GL_LIGHT1, GL_POSITION, lposition);
	glEnable(GL_LIGHT1);

	init_global_data();
	
	sys->draw_console_with(font_arial, background);
	
	// main menu
	bool quitgame = false;
	unsigned selected = 0;
	while (!quitgame) {
		menu m;
		m.add_item(TXT_Playsinglemission[language]);
		m.add_item(TXT_Multiplayermenu[language]);
		m.add_item(TXT_Careermenu[language]);
		m.add_item(TXT_Showvessels[language]);
		m.add_item(TXT_Halloffame[language]);
		menu::item mi(TXT_Selectlanguage[language]);
		mi.add_switch(TXT_English[language]);
		mi.add_switch(TXT_German[language]);
		mi.set_switch_nr(language);
		m.add_item(mi);
		m.add_item(TXT_Options[language]);
		m.add_item(TXT_Quit[language]);
		m.set_selected(selected);

		bool rebuildmenu = false;
		while (!rebuildmenu) {
			sys->prepare_2d_drawing();
			draw_background_and_logo();

			sys->poll_event_queue();
			int key = sys->get_key();
			m.draw(1024, 768, font_tahoma);
			int mmsel = m.input(key, 0, 0, 0);
			sys->unprepare_2d_drawing();
			sys->swap_buffers();
			if (mmsel == 7) { quitgame = true; break; }
			selected = mmsel & 0xffff;
			switch (selected) {
				case 0:	menu_single_mission(); break;
				case 1: menu_multiplayer(); break;
				case 2: break;
				case 3: show_vessels(); break;
				case 4: break;
				case 5: language = mmsel / 0x10000; rebuildmenu = true; break;
				case 6: break;
			}
		}
	}

	deinit_global_data();
	delete sys;

	return 0;
}
