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
#include "image.h"

class system* sys;
int res_x, res_y;

vector<string> missions;

void start_mission(int nr)
{
	string filename = get_data_dir() + "missions/" + missions[nr] + ".mis";
	parser p(filename);
	game* gm = new game(p);
	gm->main_playloop(*sys);
	delete gm;
}

void menu_notimplemented(void)
{
	menu m(110, titlebackgr);
	m.add_item(20, 0);
	m.run();
}

unsigned mission_subtype = 0, mission_cvsize = 0, mission_cvesc = 0, mission_tod = 0;
void start_custom_mission(void)//int subtype, int cvsize, int cvesc, int tod)
{
	menu_notimplemented();
	return;
	
	game* gm = new game((submarine::types)(mission_subtype), mission_cvsize,
		mission_cvesc, mission_tod);
	gm->main_playloop(*sys);
	delete gm;
}

void menu_selectsubtype(void)
{
	menu m(9, scopewatcherimg);
	m.add_item(17, 0);
	m.add_item(18, 0);
	m.add_item(20, 0);
	mission_subtype = m.run();
}

void menu_selectconvoysize(void)
{
	menu m(9, scopewatcherimg);
	m.add_item(85, 0);
	m.add_item(86, 0);
	m.add_item(87, 0);
	m.add_item(20, 0);
	mission_cvsize = m.run();
}

void menu_selectconvoyescort(void)
{
	menu m(9, scopewatcherimg);
	m.add_item(89, 0);
	m.add_item(85, 0);
	m.add_item(86, 0);
	m.add_item(87, 0);
	m.add_item(20, 0);
	mission_cvesc = m.run();
}

void menu_selecttimeofday(void)
{
	menu m(9, scopewatcherimg);
	m.add_item(91, 0);
	m.add_item(92, 0);
	m.add_item(93, 0);
	m.add_item(94, 0);
	m.add_item(20, 0);
	mission_tod = m.run();
}

void menu_convoy_battle(void)
{
	menu m(9, scopewatcherimg);
	m.add_item(16, menu_selectsubtype);
	m.add_item(84, menu_selectconvoysize);
	m.add_item(88, menu_selectconvoyescort);
	m.add_item(90, menu_selecttimeofday);
	m.add_item(19, start_custom_mission);
	m.add_item(20, 0);
	m.run();
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
	
	menu m(10, titlebackgr);
	for (unsigned i = 0; i < nr_missions; ++i)
		m.add_item(500+i, 0);
	m.add_item(20, 0);
	
	while (true) {
		unsigned selected = m.run();
		if (selected == nr_missions) break;
		start_mission(selected);
	}
}

void menu_single_mission(void)
{
	menu m(21, titlebackgr);
	m.add_item(8, menu_notimplemented);
	m.add_item(9, menu_convoy_battle);
	m.add_item(10, menu_historical_mission);
	m.add_item(11, 0);
	m.run();
}

void menu_select_language(void)
{
	menu m(26, titlebackgr);
	m.add_item(27, 0);
	m.add_item(28, 0);
	m.add_item(11, 0);
	unsigned sel = m.run();
	if (sel < 2) {
		texts::set_language(texts::languages(sel));
	}
}

void menu_resolution(void)
{
	menu m(106, titlebackgr);
	m.add_item(107, menu_notimplemented);
	m.add_item(108, menu_notimplemented);
	m.add_item(109, menu_notimplemented);
	m.add_item(20, 0);
	unsigned sel = m.run();
	// fixme: change resolution
}

void menu_options(void)
{
	menu m(29, titlebackgr);
	m.add_item(106, menu_resolution);
	m.add_item(11, 0);
	m.run();
}

//---------------------------- some network stuff, just a try
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
/*
	menu m;	// just a test
	m.add_item(TXT_Createnetworkgame[language]);
	m.add_item(menu::item(TXT_Joinnetworkgame[language], "192.168.0.0"));
	m.add_item(menu::item(TXT_Enternetworkportnr[language], "7896"));
	m.add_item(TXT_Returntomainmenu[language]);
	
	unsigned short port;
	ip serverip;

	while (true) {
		sys->prepare_2d_drawing();
//		draw_background_and_logo();
		m.draw(1024, 768);

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
*/	
}
// -------------------------- end network stuff

int current_vessel = 0;
double vessel_zangle = 0;
double vessel_xangle = 0;
sea_object* vessel = 0;
void draw_vessel(void)
{
	glClear(GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	glTranslatef(0, 0, -2.5);
	glRotatef(-80, 1, 0, 0);
	glRotatef(vessel_zangle, 0, 0, 1);
	glRotatef(vessel_xangle, 1, 0, 0);
	double sc = 2.0/vector2(vessel->get_length(), vessel->get_width()).length();
	glScalef(sc, sc, sc);
	vessel->display();
	sys->prepare_2d_drawing();
	font_tahoma->print_hc(1024, 128, vessel->get_description(2).c_str(), color::white(), true);
	sys->unprepare_2d_drawing();
}

void menu_show_vessels(void)
{
	menu m(24, threesubsimg, true);
	m.add_item(111, 0);
	m.add_item(112, 0);
	m.add_item(113, 0);
	m.add_item(114, 0);
	m.add_item(115, 0);
	m.add_item(116, 0);
	m.add_item(117, 0);
	vessel = new ship_mediummerchant();
	while (true) {
		unsigned sel = m.run(draw_vessel);
		if (sel == 6) break;
		int lastvessel = current_vessel;
#define ROTANG 5
		switch (sel) {
			case 0: vessel_zangle -= ROTANG; break;
			case 1: vessel_zangle += ROTANG; break;
			case 2: vessel_xangle -= ROTANG; break;
			case 3: vessel_xangle += ROTANG; break;
			case 4: ++current_vessel; break;
			case 5: --current_vessel; break;
		}
#undef ROTANG		
		if (current_vessel != lastvessel) {
			if (current_vessel < 0) current_vessel = 6;
			if (current_vessel > 6) current_vessel = 0;
			delete vessel;
			lastvessel = current_vessel;
			switch(current_vessel) {
				case 0: vessel = new ship_mediummerchant(); break;
				case 1: vessel = new ship_mediumtroopship(); break;
				case 2: vessel = new ship_battleshipmalaya(); break;
				case 3: vessel = new ship_destroyertribal(); break;
				case 4: vessel = new ship_carrierbogue(); break;
				case 5: vessel = new submarine_XXI(); break;
				case 6: vessel = new submarine_VIIc(); break;
			}
 		}
	}
	delete vessel;
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
	sys = new class system(1.0, 30000.0+500.0, res_x, fullscreen);
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
	menu m(104, titlebackgr);
	m.add_item(21, menu_single_mission);
	m.add_item(22, menu_notimplemented);//menu_multiplayer);
	m.add_item(23, menu_notimplemented);
	m.add_item(24, menu_show_vessels);
	m.add_item(25, menu_notimplemented);
	m.add_item(26, menu_select_language);
	m.add_item(29, menu_options);
	m.add_item(30, 0);

	m.run();

	deinit_global_data();
	delete sys;

	return 0;
}
