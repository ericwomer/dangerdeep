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
#include "date.h"
#include "ship_mediummerchant.h"
#include "ship_mediumtroopship.h"
#include "ship_destroyertribal.h"
#include "ship_battleshipmalaya.h"
#include "ship_carrierbogue.h"
#include "submarine_VIIc.h"
#include "submarine_XXI.h"
#include <iostream>
#include <sstream>
#include "config.h"

class system* sys;
int res_x, res_y;

vector<string> missions;

void draw_background_and_logo(void)
{
	sys->draw_image(0, 0, 512, 512, titel[0]);
	sys->draw_image(512, 0, 512, 512, titel[1]);
	sys->draw_image(0, 512, 512, 256, titel[2]);
	sys->draw_image(512, 512, 512, 256, titel[3]);
	font_logo->print_hc(1024, 150, "Danger from the Deep", color(255,255,255), true);
}

void start_mission(int nr)
{
	string filename = get_data_dir() + "missions/" + missions[nr] + ".mis";
	parser p(filename);
	game* gm = new game(p);
	gm->main_playloop(*sys);
	delete gm;
}

void create_and_run_custom_mission(int subtype, int cvsize, int cvesc, int tod)
{
/****************************************************************
	custom mission generation:
	As first find a random date and time, using time of day (tod).
	Whe have to calculate time of sunrise and sunfall for that, with some time
	until this day of time expires (5:59am is not really "night" when sunrise is at
	6:00am).
	Also weather computation is neccessary.
	Then we calculate size and structure of the convoy (to allow calculation of its
	map area).
	Then we have to calculate maximum viewing distance to know the distance of the
	sub relative to the convoy. We have to find a probable convoy position in the atlantic
	(convoy routes, enough space for convoy and sub).
	Then we place the convoy with probable course and path there.
	To do this we need a simulation of convoys in the atlantic.
	Then we place the sub somewhere randomly around the convoy with maximum viewing distance.
***********************************************************************/	
	

/*
	submarine* playersub = new submarine(subtype == 0 ? submarine::typeVIIc : submarine::typeXXI, vector3(2000, 1000, -30), 270);
//	submarine* playersub = new submarine(subtype == 0 ?
//			{
			
				unsigned subtype = m.get_switch_nr(0);
				// just a test, fixme
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
*/				
}

void menu_convoy_battle(void)
{
	menu m;
	menu::item mi(TXT_Selectsubtype[language]);
	mi.add_switch(TXT_subVIIc[language]);
	mi.add_switch(TXT_subXXI[language]);
	mi.set_switch_nr(0);
	m.add_item(mi);
	menu::item mi2(TXT_Selectconvoysize[language]);
	mi2.add_switch(TXT_small[language]);
	mi2.add_switch(TXT_medium[language]);
	mi2.add_switch(TXT_large[language]);
	m.add_item(mi2);
	menu::item mi3(TXT_Selectconvoyescort[language]);
	mi3.add_switch(TXT_none[language]);
	mi3.add_switch(TXT_small[language]);
	mi3.add_switch(TXT_medium[language]);
	mi3.add_switch(TXT_large[language]);
	m.add_item(mi3);
	menu::item mi4(TXT_Selecttimeofday[language]);
	mi4.add_switch(TXT_night[language]);
	mi4.add_switch(TXT_morning[language]);
	mi4.add_switch(TXT_midday[language]);
	mi4.add_switch(TXT_evening[language]);
	m.add_item(mi4);
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
		if (mmsel == 5) break;
		if ((mmsel & 0xffff) == 4) {
			create_and_run_custom_mission(
				m.get_switch_nr(0),
				m.get_switch_nr(1),
				m.get_switch_nr(2),
				m.get_switch_nr(3) );
		}
	}
}

void menu_historical_mission(void)
{
	// read missions
	missions.clear();
	ifstream ifs((get_data_dir() + "missions/list").c_str());
	string s;
	unsigned nr_missions = 0;
	while (ifs >> s) {
		missions.push_back(s);
		++nr_missions;
	}
	
	menu m;
	for (int i = 0; i < nr_missions; ++i)
		m.add_item(missions[i]);
	m.add_item(texts::get(20));

	while (true) {
		sys->prepare_2d_drawing();
		draw_background_and_logo();
		
		sys->poll_event_queue();
		int key = sys->get_key();
		m.draw(1024, 768, font_tahoma);
		int mmsel = m.input(key, 0, 0, 0) & 0xffff;
		sys->unprepare_2d_drawing();
		sys->swap_buffers();
		if (mmsel == nr_missions) break;
		if (mmsel >= 0 && mmsel < nr_missions) {
			start_mission(mmsel);
		}
	}
}

void menu_single_mission(void)
{
	menu m;
	m.add_item(texts::get(8));
	m.add_item(texts::get(9));
	m.add_item(texts::get(10));
	m.add_item(texts::get(11));

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
			case 2: menu_historical_mission(); break;
		}
	}
}

struct ip
{
	unsigned char a, b, c, d;
	ip() : a(0), b(0), c(0), d(0) {}
	ip(unsigned char a_, unsigned char b_, unsigned char c_, unsigned char d_) :
		a(a_), b(b_), c(c_), d(d_) {}
	bool is_valid(void) const { return (a!=0) || (b!=0) || (c!=0) || (d!=0); }
};

unsigned short string2port(const string& s)
{
	istringstream is(s);
	unsigned short p;
	is >> p;
	return p;
}

ip string2ip(const string& s)
{
	unsigned a, b, c, d;
	sscanf(s.c_str(), "%u.%u.%u.%u", &a, &b, &c, &d);
	if (a > 255 || b > 255 || c > 255 || d > 255)
		return ip();
	return ip(a, b, c, d);
}

void create_network_game(unsigned short port)
{
	printf("create network game %u\n",port);
}

void join_network_game(ip serverip, unsigned short port)
{
	printf("join network game %u.%u.%u.%u:%u\n",serverip.a,serverip.b,serverip.c,serverip.d,port);
}

void menu_multiplayer(void)
{
	menu m;	// just a test
	m.add_item(TXT_Createnetworkgame[language]);
	m.add_item(menu::item(TXT_Joinnetworkgame[language], "192.168.0.0"));
	m.add_item(menu::item(TXT_Enternetworkportnr[language], "7896"));
	m.add_item(TXT_Returntomainmenu[language]);
	
	unsigned short port;
	ip serverip;

	while (true) {
		sys->prepare_2d_drawing();
		draw_background_and_logo();
		m.draw(1024, 768, font_tahoma);

		sys->poll_event_queue();
		int key = sys->get_key();
		int mmsel = m.input(key, 0, 0, 0) & 0xffff;

		port = string2port(m.get_item(2).get_input_text());
		serverip = string2ip(m.get_item(1).get_input_text());

		sys->unprepare_2d_drawing();
		sys->swap_buffers();
		if (mmsel == 3) break;
		switch (mmsel) {
			case 0: if (port != 0)
				create_network_game(port);
				break;
			case 1:	if (port != 0 && serverip.is_valid())
				join_network_game(serverip, port);
				break;
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
	int lastvessel = -1;
	sea_object* s = 0;	// pointer to current vessel
	while (true) {
		if (vessel != lastvessel) {
			if (vessel < 0) vessel = 6;
			if (vessel > 6) vessel = 0;
			delete s;
			lastvessel = vessel;
			switch(vessel) {
				case 0: s = new ship_mediummerchant(); break;
				case 1: s = new ship_mediumtroopship(); break;
				case 2: s = new ship_battleshipmalaya(); break;
				case 3: s = new ship_destroyertribal(); break;
				case 4: s = new ship_carrierbogue(); break;
				case 5: s = new submarine_XXI(); break;
				case 6: s = new submarine_VIIc(); break;
			}
 		}

		glClear(GL_DEPTH_BUFFER_BIT);
		sys->prepare_2d_drawing();
		sys->draw_image(0, 0, 512, 512, threesubs[0]);
		sys->draw_image(512, 0, 512, 512, threesubs[1]);
		sys->draw_image(0, 512, 512, 256, threesubs[2]);
		sys->draw_image(512, 512, 512, 256, threesubs[3]);
		font_tahoma->print_hc(1024, 650, s->get_description(2).c_str());
		font_tahoma->print_hc(1024, 768 - font_tahoma->get_height(),
			TXT_Showvesselinstructions[language]);
		sys->unprepare_2d_drawing();

		glLoadIdentity();
		glTranslatef(0, 0, -2.5);
		glRotatef(-80, 1, 0, 0);
		glRotatef(zangle, 0, 0, 1);
		glRotatef(xangle, 1, 0, 0);
		double sc = 2.0/vector2(s->get_length(), s->get_width()).length();
		glScalef(sc, sc, sc);
		s->display();
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
	delete s;
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

	// set language to default
	texts::set_language();

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
		m.add_item(texts::get(21));
		m.add_item(texts::get(22));
		m.add_item(texts::get(23));
		m.add_item(texts::get(24));
		m.add_item(texts::get(25));
		menu::item mi(texts::get(26));
		mi.add_switch(texts::get(27));
		mi.add_switch(texts::get(28));
		mi.set_switch_nr(language);
		m.add_item(mi);
		m.add_item(texts::get(29));
		m.add_item(texts::get(30));
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
				case 5: language = mmsel / 0x10000; 
					// very ugly.fixme
					texts::set_language((mmsel / 0x10000 == 0) ? texts::english : texts::german);
					rebuildmenu = true; break;
				case 6: break;
			}
		}
	}

	deinit_global_data();
	delete sys;

	return 0;
}
