// main program
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#define VERSION "win32"
#include <windows.h>
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
#include "game.h"
#include "menu.h"
#include "global_data.h"
#include "texts.h"
#include "date.h"
#include "sound.h"
#include "ship_largemerchant.h"
#include "ship_mediummerchant.h"
#include "ship_smallmerchant.h"
#include "ship_mediumtroopship.h"
#include "ship_destroyertribal.h"
#include "ship_battleshipmalaya.h"
#include "ship_carrierbogue.h"
#include "ship_corvette.h"
#include "ship_largefreighter.h"
#include "ship_mediumfreighter.h"
#include "ship_smalltanker.h"
#include "submarine_VIIc.h"
#include "submarine_IXc40.h"
#include "submarine_XXI.h"
#include <iostream>
#include <sstream>
#include "image.h"
#include "widget.h"

class system* sys;
int res_x, res_y;

// a dirty hack
void menu_notimplemented(void)
{
	menu m(110, titlebackgrimg);
	m.add_item(20, 0);
	m.run();
}



//
// show results after a game ended
//
void show_results_for_game(const game* gm)
{
	widget w(0, 0, 1024, 768, texts::get(124), 0, sunkendestroyerimg);
	widget_list* wl = new widget_list(64, 64, 1024-64-64, 768-64-64);
	w.add_child(wl);
	w.add_child(new widget_caller_arg_button<widget, void (widget::*)(int), int>(&w, &widget::close, 1, (1024-128)/2, 768-32-16, 128, 32, texts::get(105)));
	unsigned totaltons = 0;
	for (list<game::sink_record>::const_iterator it = gm->sunken_ships.begin(); it != gm->sunken_ships.end(); ++it) {
		ostringstream oss;
		oss << it->dat.get_value(date::year) << "/"
			<< it->dat.get_value(date::month) << "/"
			<< it->dat.get_value(date::day) << "\t"
			<< it->descr << "\t"
			<< it->tons << " BRT";
		totaltons += it->tons;
		wl->append_entry(oss.str());
	}
	ostringstream os;
	os << "total: " << totaltons;
	wl->append_entry(os.str());
	w.run();
}


//
// save a game to disk
//
void save_game(const game* gm)
{
	gm->save("TESTSAVEGAME", "DO NOT USE THIS");
}

//
// start and run a game, handle load/save (game menu), show results after game's end, delete game
//
void run_game(game* gm)
{
	while (true) {
		unsigned state = gm->exec();
		//if (state == 2) break;
		//SDL_ShowCursor(SDL_ENABLE);
		if (state == 1) {
			//widget* w = widget::create_dialogue_ok_cancel("Quit game?");
			//int q = w->run();
			//delete w;
			//if (q == 1)
				break;
		} else {
			widget woptions(0, 0, 1024, 768, texts::get(29), 0, depthchargeimg);
			widget_menu* wmn = new widget_menu(312, 260, 400, 40, false);
			woptions.add_child(wmn);
			wmn->add_entry(texts::get(118), new widget_func_button<void (*)(void)>(menu_notimplemented));
			wmn->add_entry(texts::get(119), new widget_func_arg_button<void (*)(const game*), const game*>(save_game, gm));
			wmn->add_entry(texts::get(120), new widget_caller_arg_button<widget, void (widget::*)(int), int>(&woptions, &widget::close, 1));
			wmn->add_entry(texts::get(121), new widget_caller_arg_button<widget, void (widget::*)(int), int>(&woptions, &widget::close, 2));
			unsigned sel = woptions.run();
			if (sel == 1)
				break;
		}
		//SDL_ShowCursor(SDL_DISABLE);
	}
	show_results_for_game(gm);
	delete gm;
}


//
// create a custom convoy mission
//
void create_convoy_mission(void)
{
	widget w(0, 0, 1024, 768, texts::get(9), 0, scopewatcherimg);
	w.add_child(new widget_text(40, 60, 0, 0, texts::get(16)));
	widget_list* wsubtype = new widget_list(40, 90, 200, 200);
	w.add_child(wsubtype);
	w.add_child(new widget_text(280, 60, 0, 0, texts::get(84)));
	widget_list* wcvsize = new widget_list(280, 90, 200, 200);
	w.add_child(wcvsize);
	w.add_child(new widget_text(520, 60, 0, 0, texts::get(88)));
	widget_list* wescortsize = new widget_list(520, 90, 200, 200);
	w.add_child(wescortsize);
	w.add_child(new widget_text(760, 60, 0, 0, texts::get(90)));
	widget_list* wtimeofday = new widget_list(760, 90, 200, 200);
	w.add_child(wtimeofday);
	wsubtype->append_entry(texts::get(17));
	wsubtype->append_entry(texts::get(174));
	wsubtype->append_entry(texts::get(18));
	wcvsize->append_entry(texts::get(85));
	wcvsize->append_entry(texts::get(86));
	wcvsize->append_entry(texts::get(87));
	wescortsize->append_entry(texts::get(89));
	wescortsize->append_entry(texts::get(85));
	wescortsize->append_entry(texts::get(86));
	wescortsize->append_entry(texts::get(87));
	wtimeofday->append_entry(texts::get(91));
	wtimeofday->append_entry(texts::get(92));
	wtimeofday->append_entry(texts::get(93));
	wtimeofday->append_entry(texts::get(94));

	w.add_child(new widget_caller_arg_button<widget, void (widget::*)(int), int>(&w, &widget::close, 1, 70, 700, 400, 40, texts::get(20)));
	w.add_child(new widget_caller_arg_button<widget, void (widget::*)(int), int>(&w, &widget::close, 2, 540, 700, 400, 40, texts::get(19)));
	int result = w.run();
	if (result == 2) {	// start game
		submarine::types st = submarine::typeVIIc;
		switch (wsubtype->get_selected()) {
			case 0: st = submarine::typeVIIc; break;
			case 1: st = submarine::typeIXc40; break;
			case 2: st = submarine::typeXXI; break;
		}
		run_game(new game(st, wcvsize->get_selected(), wescortsize->get_selected(), wtimeofday->get_selected()));
	}
}


//
// choose a historical mission
//
void choose_historical_mission(void)
{
	vector<string> missions;
	
	// read missions
	ifstream ifs((get_mission_dir() + "list").c_str());
	string s;
	unsigned nr_missions = 0;
	while (ifs >> s) {
		missions.push_back(s);
		++nr_missions;
	}

	// set up window
	widget w(0, 0, 1024, 768, texts::get(10), 0, sunderlandimg);
	widget_list* wmission = new widget_list(40, 60, 1024-80, 620);
	w.add_child(wmission);
	for (unsigned i = 0; i < nr_missions; ++i)
		wmission->append_entry(texts::get(500+i));
	
	w.add_child(new widget_caller_arg_button<widget, void (widget::*)(int), int>(&w, &widget::close, 1, 70, 700, 400, 40, texts::get(20)));
	w.add_child(new widget_caller_arg_button<widget, void (widget::*)(int), int>(&w, &widget::close, 2, 540, 700, 400, 40, texts::get(19)));
	int result = w.run();
	if (result == 2) {	// start game
		string filename = get_mission_dir() + missions[wmission->get_selected()] + ".mis";
		parser p(filename);
		run_game(new game(p));
	}
}




// old menus are used from here on
void menu_single_mission(void)
{
	menu m(21, titlebackgrimg);
	m.add_item(8, menu_notimplemented);
	m.add_item(9, create_convoy_mission);
	m.add_item(10, choose_historical_mission);
	m.add_item(11, 0);
	m.run();
}

void menu_select_language(void)
{
	menu m(26, titlebackgrimg);
	m.add_item(27, 0);
	m.add_item(28, 0);
	m.add_item(11, 0);
	unsigned sel = m.run();
	if (sel < 2) {
		texts::set_language(texts::languages(sel));
	}
}

//
// options:
// - set resolution
// - enable bump mapping
// - set fullscreen
//
void menu_resolution(void)
{
	menu m(106, titlebackgrimg);
	m.add_item(107, menu_notimplemented);
	m.add_item(108, menu_notimplemented);
	m.add_item(109, menu_notimplemented);
	m.add_item(20, 0);
	unsigned sel = m.run();
	// fixme: change resolution
}

void menu_options(void)
{
	menu m(29, titlebackgrimg);
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
	m.add_item(menu::item(TXT_Enternetworkportnr[language], "7896"));//57117=0xdf1d
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
	glDisable(GL_LIGHTING);
	glLoadIdentity();
	glTranslatef(0, 0, -2.5);
	glRotatef(-80, 1, 0, 0);
	glRotatef(vessel_zangle, 0, 0, 1);
	glRotatef(vessel_xangle, 1, 0, 0);
	double sc = 3.0/(vessel->get_model()->get_boundbox_size().length());
	glScalef(sc, sc, sc);
	glColor4f(0, 0, 0, 1);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBegin(GL_LINES);
	glVertex3f(-10.0, 0.0, -10.0);
	glVertex3f( 10.0, 0.0, -10.0);
	glVertex3f( 0.0,-50.0, -10.0);
	glVertex3f( 0.0, 50.0, -10.0);
	glEnd();
	glColor3f(1, 1, 1);
	vessel->display();
	sys->prepare_2d_drawing();
	font_nimbusrom->print_hc(512, 128, vessel->get_description(2), color::white(), true);
	sys->unprepare_2d_drawing();
	glEnable(GL_LIGHTING);
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
	vessel = new ship_largemerchant();
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
			if (current_vessel < 0) current_vessel = 13;
			if (current_vessel > 13) current_vessel = 0;
			delete vessel;
			lastvessel = current_vessel;
			switch(current_vessel) {
				case  0: vessel = new ship_largemerchant(); break;
				case  1: vessel = new ship_mediummerchant(); break;
				case  2: vessel = new ship_smallmerchant(); break;
				case  3: vessel = new ship_mediumtroopship(); break;
				case  4: vessel = new ship_battleshipmalaya(); break;
				case  5: vessel = new ship_destroyertribal(); break;
				case  6: vessel = new ship_carrierbogue(); break;
				case  7: vessel = new ship_corvette(); break;
				case  8: vessel = new ship_largefreighter(); break;
				case  9: vessel = new ship_mediumfreighter(); break;
				case 10: vessel = new ship_smalltanker(); break;
				case 11: vessel = new submarine_VIIc(); break;
				case 12: vessel = new submarine_IXc40(); break;
				case 13: vessel = new submarine_XXI(); break;
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
			<< "--nofullscreen\tdon't use fullscreen\n"
			<< "--nosound\tdon't use sound\n";
			return 0;
		} else if (*it == "--nofullscreen") {
			fullscreen = false;
		} else if (*it == "--nosound") {
			sound::use_sound = false;
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
	
	widget::set_theme(new widget::theme("widgetelements_menu.png", "widgeticons_menu.png",
		font_nimbusrom, color::white(), color(255, 64, 64)));

	sys->draw_console_with(font_arial, background);
	
	// main menu
	menu m(104, titlebackgrimg);
	m.add_item(21, menu_single_mission);
	m.add_item(22, menu_notimplemented);//menu_multiplayer);
	m.add_item(23, menu_notimplemented);
	m.add_item(24, menu_show_vessels);
	m.add_item(25, menu_notimplemented);
	m.add_item(26, menu_select_language);
	m.add_item(29, menu_notimplemented /*menu_options*/);
	m.add_item(30, 0);


	m.run();

	sys->write_console(true);

	deinit_global_data();
	delete sys;

	return 0;
}
