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
#include "filehelper.h"

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
// save game directory and helper functions
//
string savegamedirectory =
#ifdef WIN32
	"./save/";
#else	
	string(getenv("HOME"))+"/."+PACKAGE + "/";
#endif	

string get_savegame_name_for(const string& descr, map<string, string>& savegames)
{
	unsigned num = 1;
	for (map<string, string>::iterator it = savegames.begin(); it != savegames.end(); ++it) {
		if (it->second == descr)
			return savegamedirectory + it->first;
		unsigned num2 = unsigned(atoi((it->first.substr(5, 4)).c_str()));
		if (num2 >= num) num = num2+1;
	}
	char tmp[20];
	sprintf(tmp, "save_%04u.dftd", num);
	return savegamedirectory + tmp;
}

bool is_savegame_name(const string& s)
{
	if (s.length() != 14) return false;
	if (s.substr(0, 5) != "save_") return false;
	if (s.substr(9, 7) != ".dftd") return false;
	for (int i = 5; i < 9; ++i)
		if (s[i] < '0' || s[i] > '9') return false;
	return true;
}



//
// the game options menu, loading, saving games
//
class loadsavequit_dialogue : public widget
{
	widget_edit* gamename;
	widget_list* gamelist;
	widget_button *btnload, *btnsave, *btndel, *btnquit, *btncancel;
	game* mygame;
	bool gamesaved;
	map<string, string> savegames;
	void load(void);
	void save(void);
	void erase(void);
	void quit(void);
	void cancel(void) { close(0); }	// return 0 for cancel/return, 1 for quit (if saving is enabled), 2 for loaded
	void update_list(void);

public:	
	game* get_game(void) const { return mygame; }
	widget_edit* get_gamename(void) const { return gamename; }
	loadsavequit_dialogue(game* g);	// give 0 to disable saving
	~loadsavequit_dialogue() {};
};

loadsavequit_dialogue::loadsavequit_dialogue(game *g) : widget(0, 0, 1024, 768, texts::get(177), 0, depthchargeimg), mygame(g), gamesaved(false)
{
	add_child(new widget_text(40, 40, 0, 0, texts::get(178)));
	gamename = new widget_edit(200, 40, 684, 40, "");
	add_child(gamename);
	widget_menu* wm = new widget_menu(40, 700, 180, 40, true);
	add_child(wm);
	btnload = wm->add_entry(texts::get(118), new widget_caller_button<loadsavequit_dialogue, void (loadsavequit_dialogue::*)(void)>(this, &loadsavequit_dialogue::load));
	if (mygame)
		btnsave = wm->add_entry(texts::get(119), new widget_caller_button<loadsavequit_dialogue, void (loadsavequit_dialogue::*)(void)>(this, &loadsavequit_dialogue::save));
	btndel = wm->add_entry(texts::get(179), new widget_caller_button<loadsavequit_dialogue, void (loadsavequit_dialogue::*)(void)>(this, &loadsavequit_dialogue::erase));
	if (mygame)
		btnquit = wm->add_entry(texts::get(120), new widget_caller_button<loadsavequit_dialogue, void (loadsavequit_dialogue::*)(void)>(this, &loadsavequit_dialogue::quit));
	btncancel = wm->add_entry(texts::get(mygame ? 121 : 20), new widget_caller_button<loadsavequit_dialogue, void (loadsavequit_dialogue::*)(void)>(this, &loadsavequit_dialogue::cancel));
	wm->adjust_buttons(944);
	struct lsqlist : public widget_list
	{
		void on_sel_change(void) {
			dynamic_cast<loadsavequit_dialogue*>(parent)->get_gamename()->set_text(get_selected_entry());
		}
		lsqlist(int x, int y, int w, int h) : widget_list(x, y, w, h) {}
		~lsqlist() {}
	};
	gamelist = new lsqlist(40, 100, 944, 580);
	add_child(gamelist);

	update_list();
	
	gamename->set_text(gamelist->get_selected_entry());
}

void loadsavequit_dialogue::load(void)
{
	//fixme: ask: replace this game?
	delete mygame;
	mygame = new game(get_savegame_name_for(gamename->get_text(), savegames));
	widget* w = create_dialogue_ok(texts::get(185), texts::get(180) + gamename->get_text() + texts::get(181));
	w->run();
	delete w;
	close(2);	// load and close
}

void loadsavequit_dialogue::save(void)
{
	string fn = get_savegame_name_for(gamename->get_text(), savegames);
	FILE* f = fopen(fn.c_str(), "rb");
	if (f) {
		fclose(f);
		widget* w = create_dialogue_ok_cancel(texts::get(182), texts::get(183) + gamename->get_text() + texts::get(184));
		int ok = w->run();
		delete w;
		if (!ok) return;
	}
	gamesaved = true;
	mygame->save(fn, gamename->get_text());
	widget* w = create_dialogue_ok(texts::get(186), texts::get(180) + gamename->get_text() + texts::get(187));
	w->run();
	delete w;
	update_list();
}

void loadsavequit_dialogue::erase(void)
{
	widget* w = create_dialogue_ok_cancel(texts::get(182), texts::get(188) + gamename->get_text() + texts::get(189));
	int ok = w->run();
	delete w;
	if (ok) {
		string fn = get_savegame_name_for(gamename->get_text(), savegames);
		remove(fn.c_str());
		int s = gamelist->get_selected() - 1;
		update_list();
		if (s < 0) s = 0;
		gamelist->set_selected(s);
		gamename->set_text(gamelist->get_selected_entry());
	}
}

void loadsavequit_dialogue::quit(void)
{
	if (!gamesaved) {
		widget* w = create_dialogue_ok_cancel(texts::get(182), texts::get(190));
		int q = w->run();
		if (q) close(1);
	} else {
		close(1);
	}
}

void loadsavequit_dialogue::update_list(void)
{
	savegames.clear();

	// read save games in directory
	directory savegamedir = open_dir(savegamedirectory);
	system::sys()->myassert(savegamedir != 0, "game: could not open save game directory");
	while (true) {
		string e = read_dir(savegamedir);
		if (e.length() == 0) break;
		if (is_savegame_name(e)) {
			string descr = mygame->read_description_of_savegame(savegamedirectory+e);
			savegames[e] = descr;
		}
	}
	close_dir(savegamedir);
	
	gamelist->clear();

	unsigned sel = 0;		
	for (map<string, string>::iterator it = savegames.begin(); it != savegames.end(); ++it) {
		gamelist->append_entry(it->second);
		if (it->second == gamename->get_text())
			gamelist->set_selected(sel);
		++sel;
	}

	if (savegames.size() == 0) {
		btnload->disable();
		btndel->disable();
	} else {
		btnload->enable();
		btndel->enable();
	}
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
// start and run a game, handle load/save (game menu), show results after game's end, delete game
//
void run_game(game* gm)
{
	while (true) {
		unsigned state = gm->exec();
		//if (state == 2) break;
		//SDL_ShowCursor(SDL_ENABLE);
		if (state == 1) {
			//widget* w = widget::create_dialogue_ok_cancel("Quit game?", "");
			//int q = w->run();
			//delete w;
			//if (q == 1)
				break;
		} else {
			loadsavequit_dialogue dlg(gm);
			int q = dlg.run();
			if (q == 1)
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

	widget_menu* wm = new widget_menu(40, 700, 0, 40, true);
	w.add_child(wm);
	wm->add_entry(texts::get(20), new widget_caller_arg_button<widget, void (widget::*)(int), int>(&w, &widget::close, 1, 70, 700, 400, 40));
	wm->add_entry(texts::get(19), new widget_caller_arg_button<widget, void (widget::*)(int), int>(&w, &widget::close, 2, 540, 700, 400, 40));
	wm->adjust_buttons(944);
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

	widget_menu* wm = new widget_menu(40, 700, 0, 40, true);
	w.add_child(wm);
	wm->add_entry(texts::get(20), new widget_caller_arg_button<widget, void (widget::*)(int), int>(&w, &widget::close, 1, 70, 700, 400, 40));
	wm->add_entry(texts::get(19), new widget_caller_arg_button<widget, void (widget::*)(int), int>(&w, &widget::close, 2, 70, 700, 400, 40));
	wm->adjust_buttons(944);
	int result = w.run();
	if (result == 2) {	// start game
		string filename = get_mission_dir() + missions[wmission->get_selected()] + ".mis";
		parser p(filename);
		run_game(new game(p));
	}
}


//
// choose a saved game
//
void choose_saved_game(void)
{
	loadsavequit_dialogue dlg(0);
	int q = dlg.run();
	if (q == 0) return;
	if (q == 2) {
		run_game(dlg.get_game());
	}
}




// old menus are used from here on
void menu_single_mission(void)
{
	menu m(21, titlebackgrimg);
	m.add_item(8, menu_notimplemented);
	m.add_item(9, create_convoy_mission);
	m.add_item(10, choose_historical_mission);
	m.add_item(118, choose_saved_game);
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
		font_nimbusrom, color::white(), color(255, 64, 64), color(64,64,32)));

	sys->draw_console_with(font_arial, background);
	

	// init config and save game dir
	directory savegamedir = open_dir(savegamedirectory);
	if (savegamedir == 0) {
		bool ok = make_dir(savegamedirectory);
		if (!ok) {
			system::sys()->myassert(false, "could not create save game directory.");
		}
	}


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
