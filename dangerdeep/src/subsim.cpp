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
#include <SDL_net.h>

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
#include "network.h"
#include <iostream>
#include <sstream>
#include "image.h"
#include "widget.h"
#include "filehelper.h"
#include "highscorelist.h"
#include "user_interface.h"
#include "cfg.h"
#include "tinyxml/tinyxml.h"

class system* sys;
int res_x, res_y;

highscorelist hsl_mission, hsl_career;
#define HSL_MISSION_NAME "mission.hsc"
#define HSL_CAREER_NAME "career.hsc"


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
// loading, saving games
//
class loadsavequit_dialogue : public widget
{
	widget_edit* gamename;
	widget_list* gamelist;
	widget_button *btnload, *btnsave, *btndel, *btnquit, *btncancel;
	const game* mygame;
	bool gamesaved;
	map<string, string> savegames;
	string gamefilename_to_load;
	void load(void);
	void save(void);
	void erase(void);
	void quit(void);
	void cancel(void) { close(0); }	// return 0 for cancel/return, 1 for quit (if saving is enabled), 2 for loaded
	void update_list(void);

public:	
	string get_gamefilename_to_load(void) const { return gamefilename_to_load; }
	widget_edit* get_gamename(void) const { return gamename; }
	loadsavequit_dialogue(const game* g);	// give 0 to disable saving
	~loadsavequit_dialogue() {};
};

loadsavequit_dialogue::loadsavequit_dialogue(const game *g) : widget(0, 0, 1024, 768, texts::get(177), 0, depthchargeimg), mygame(g), gamesaved(false)
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
	gamefilename_to_load = get_savegame_name_for(gamename->get_text(), savegames);
	//fixme: ask: replace this game?
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
	system::sys().myassert(savegamedir.is_valid(), "game: could not open save game directory");
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
// show hall of fame
//
void show_halloffame(const highscorelist& hsl)
{
	widget w(0, 0, 1024, 768, texts::get(197), 0, kruppdocksimg);
	w.add_child(new widget_caller_arg_button<widget, void (widget::*)(int), int>(&w, &widget::close, 1, (1024-128)/2, 768-32-16, 128, 32, texts::get(105)));
	hsl.show(&w);
	w.run();
}
void show_halloffame_mission(void) { show_halloffame(hsl_mission); }
void show_halloffame_career(void) { show_halloffame(hsl_career); }


//
// check if a game is good enough for the high score list
//
void check_for_highscore(const game* gm)
{
	unsigned totaltons = 0;
	for (list<game::sink_record>::const_iterator it = gm->sunken_ships.begin(); it != gm->sunken_ships.end(); ++it) {
		totaltons += it->tons;
	}
	highscorelist& hsl = (/* check if game is career or mission fixme */ true) ? hsl_mission : hsl_career;
	unsigned points = totaltons /* compute points from tons etc here fixme */;

	widget w(0, 0, 1024, 768, texts::get(197), 0, kruppdocksimg);
	w.add_child(new widget_caller_arg_button<widget, void (widget::*)(int), int>(&w, &widget::close, 1, (1024-128)/2, 768-32-16, 128, 32, texts::get(105)));
	unsigned pos = hsl.get_listpos_for(points);
	if (hsl.is_good_enough(points)) {
		w.add_child(new widget_text(200, 200, 0,0, texts::get(199)));
		if (pos == 0)
			w.add_child(new widget_text(200, 240, 0,0, texts::get(201)));
		w.add_child(new widget_text(400, 280, 0,0, texts::get(200)));
		widget_edit* wname = new widget_edit(300, 320, 424, 32, "");
		w.add_child(wname);
		w.run();
		string playername = wname->get_text();
		if (playername.length() == 0)
			playername = "INCOGNITO";
		hsl.record(points, playername);
	} else {
		w.add_child(new widget_text(400, 200, 0,0, texts::get(198)));
	}
	w.run();
	show_halloffame(hsl);
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
		oss << texts::numeric_from_date(it->dat) << "\t"
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
// show the credits
//
#include "credits.h"
void show_credits(void)
{
	glClearColor(0.1,0.25,0.4,0);

	model* mdlgear = modelcache.ref("gear.3ds");
	
//	mdlgear->get_mesh(0).mymaterial->col = color(255,255,255);

	int lineheight = font_arial->get_height();
	int lines_per_page = (768+lineheight-1)/lineheight;
	int textpos = -lines_per_page;
	int textlines = 0;
	for ( ; credits[textlines] != 0; ++textlines);
	int textendpos = textlines;
	float lineoffset = 0.0f;

	bool quit = false;
	float ang = 0.0f, ang_per_sec = 10.0f, r = 78, lines_per_sec = 2;
	unsigned tm = system::sys().millisec();
	while (!quit) {
		system::sys().poll_event_queue();
		if (system::sys().get_key().sym == SDLK_ESCAPE) quit = true;

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glPushMatrix();
		glRotatef(ang/10, 0, 0, 1);
		glTranslatef(-r,r,-150);
		//glScalef(12.8,12.8,0.1);
		glRotatef(ang, 0, 0, 1);
		mdlgear->display();
		glPopMatrix();

		glPushMatrix();
		glRotatef(ang/10, 0, 0, 1);
		glTranslatef(r,-r,-150);
		//glScalef(12.8,12.8,0.1);
		glRotatef(4.05-ang, 0, 0, 1);
		mdlgear->display();
		glPopMatrix();

		system::sys().prepare_2d_drawing();
		for (int i = textpos; i <= textpos+lines_per_page; ++i) {
			if (i >= 0 && i < textlines) {
				font_arial->print_hc(512, (i-textpos)*lineheight+int(-lineoffset*lineheight), credits[i], color::white(), true);
			}
		}
		system::sys().unprepare_2d_drawing();

		unsigned tm2 = system::sys().millisec();
		lineoffset += lines_per_sec*(tm2-tm)/1000.0f;
		int tmp = int(lineoffset);
		lineoffset -= tmp;
		textpos += tmp;
		if (textpos >= textendpos) textpos = -lines_per_page;
		ang += ang_per_sec*(tm2-tm)/1000.0f;
		tm = tm2;
		system::sys().swap_buffers();
	}
	
	modelcache.unref("gear.3ds");
	
	glClearColor(0, 0, 1, 0);
}



//
// start and run a game, handle load/save (game menu), show results after game's end, delete game
//
void run_game(game* gm)
{
	widget::theme* gametheme =
		new widget::theme("widgetelements_game.png", "widgeticons_game.png",
		font_panel /* font_arial */, color(224,224,224), color(180, 180, 255), color(64,64,64));
	user_interface* ui = user_interface::create(*gm);
	while (true) {
		widget::theme* tmp = widget::replace_theme(gametheme);
		gm->set_user_interface(ui);
		unsigned state = gm->exec();
		widget::replace_theme(tmp);
		
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
			// replace game and ui if new game was loaded
			if (q == 2) {
				delete gm;
				delete ui;
				gm = new game(dlg.get_gamefilename_to_load());
				ui = user_interface::create(*gm);
			}
			//replace ui after loading!!!!
			if (q == 1)
				break;
		}
		//SDL_ShowCursor(SDL_DISABLE);
	}
	delete gametheme;
	show_results_for_game(gm);
	check_for_highscore(gm);
	gm->set_user_interface(0);
	delete ui;
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
	w.add_child(new widget_text(40, 310, 0, 0, texts::get(62)));
	widget_list* wtimeperiod = new widget_list(40, 340, 440, 200);
	w.add_child(wtimeperiod);
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
	wtimeperiod->append_entry(texts::get(63));
	wtimeperiod->append_entry(texts::get(64));
	wtimeperiod->append_entry(texts::get(65));
	wtimeperiod->append_entry(texts::get(66));
	wtimeperiod->append_entry(texts::get(67));
	wtimeperiod->append_entry(texts::get(68));
	wtimeperiod->append_entry(texts::get(69));
	wtimeperiod->append_entry(texts::get(70));

	widget_menu* wm = new widget_menu(40, 700, 0, 40, true);
	w.add_child(wm);
	wm->add_entry(texts::get(20), new widget_caller_arg_button<widget, void (widget::*)(int), int>(&w, &widget::close, 1, 70, 700, 400, 40));
	wm->add_entry(texts::get(19), new widget_caller_arg_button<widget, void (widget::*)(int), int>(&w, &widget::close, 2, 540, 700, 400, 40));
	wm->adjust_buttons(944);
	int result = w.run();
	if (result == 2) {	// start game
		string st;
		switch (wsubtype->get_selected()) {
			case 0: st = "submarine_VIIc"; break;
			case 1: st = "submarine_IXc40"; break;
			case 2: st = "submarine_XXI"; break;
		}
		run_game(new game(st,
			wcvsize->get_selected(),
			wescortsize->get_selected(),
			wtimeofday->get_selected(),
			wtimeperiod->get_selected()));
	}
}


//
// choose a historical mission
//
void choose_historical_mission(void)
{
	vector<string> missions;
	
	// read missions
	unsigned nr_missions = 0;
	directory missiondir = open_dir(get_mission_dir());
	system::sys().myassert(missiondir.is_valid(), "could not open mission directory");
	while (true) {
		string e = read_dir(missiondir);
		if (e.length() == 0) break;
		if (e.length() > 4 && e.substr(e.length()-4) == ".xml") {
			missions.push_back(e);
			++nr_missions;
		}
	}
	close_dir(missiondir);

	// read descriptions, set up windows
	widget w(0, 0, 1024, 768, texts::get(10), 0, sunderlandimg);
	vector<string> descrs;
	struct msnlist : public widget_list
	{
		const vector<string>& descrs;
		widget_text* wdescr;
		void on_sel_change(void) {
			int sel = get_selected();
			if (sel >= 0 && sel < int(descrs.size()))
				wdescr->set_text(descrs[sel]);
			else
				wdescr->set_text("");
		}
		msnlist(int x, int y, int w, int h, const vector<string>& descrs_, widget_text* wdescr_) : widget_list(x, y, w, h), descrs(descrs_), wdescr(wdescr_) {}
		~msnlist() {}
	};
	widget_text* wdescr = new widget_text(40, 380, 1024-80, 300, "", 0, true);
	widget_list* wmission = new msnlist(40, 60, 1024-80, 300, descrs, wdescr);
	w.add_child(wmission);
	w.add_child(wdescr);
	for (unsigned i = 0; i < nr_missions; ++i) {
		TiXmlDocument doc(get_mission_dir() + missions[i]);
		doc.LoadFile();
		TiXmlHandle hdoc(&doc);
		TiXmlHandle hdftdmission = hdoc.FirstChild("dftd-mission");
		TiXmlHandle hdescription = hdftdmission.FirstChild("description");
		TiXmlElement* eshort = hdescription.FirstChild("short").Element();
		for ( ; eshort != 0; eshort = eshort->NextSiblingElement("short")) {
			if (XmlAttrib(eshort, "lang") == texts::get_language_code()) {
				TiXmlNode* ntext = eshort->FirstChild();
				system::sys().myassert(ntext != 0, string("short description text child node missing in ")+doc.Value());
				wmission->append_entry(ntext->Value());
				break;
			}
		}
		TiXmlElement* elong = hdescription.FirstChild("long").Element();
		for ( ; elong != 0; elong = elong->NextSiblingElement("long")) {
			if (XmlAttrib(elong, "lang") == texts::get_language_code()) {
				TiXmlNode* ntext = elong->FirstChild();
				system::sys().myassert(ntext != 0, string("long description text child node missing in ")+doc.Value());
				descrs.push_back(ntext->Value());
				break;
			}
		}
	}
	wmission->on_sel_change();

	widget_menu* wm = new widget_menu(40, 700, 0, 40, true);
	w.add_child(wm);
	wm->add_entry(texts::get(20), new widget_caller_arg_button<widget, void (widget::*)(int), int>(&w, &widget::close, 1, 70, 700, 400, 40));
	wm->add_entry(texts::get(19), new widget_caller_arg_button<widget, void (widget::*)(int), int>(&w, &widget::close, 2, 70, 700, 400, 40));
	wm->adjust_buttons(944);
	int result = w.run();
	if (result == 2) {	// start game
		TiXmlDocument doc(get_mission_dir() + missions[wmission->get_selected()]);
		doc.LoadFile();
		run_game(new game(&doc));
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
		run_game(new game(dlg.get_gamefilename_to_load()));
	}
}


//
// create a network mission
//

// send message, wait for answer, returns true if answer received
bool send_and_wait(network_connection& nc, const string& sendmsg, const string& waitmsg, unsigned timeout = 0xffffffff)
{
	nc.send_message(sendmsg);
	string answer;
	unsigned waited = 0;
	do {
		system::sys().poll_event_queue();
		SDL_Delay(50);
		waited += 50;
		answer = nc.receive_message();
	} while (waited < timeout && answer.length() == 0);
	return (waited < timeout);
}

void ask_for_offered_games(widget_list* wservers, Uint16 server_port, network_connection& scan)
{
	wservers->clear();
	
	// send a broadcast to all local addresses (assuming subnet mask 255.255.255.0)
	scan.bind("192.168.0.0", server_port);
	scan.send_message(MSG_ask);
	scan.unbind();
}

void listen_for_offered_games(widget_list* wservers, Uint16 server_port, network_connection& scan)
{
	string msg;
	do {
		IPaddress ip;
		msg = scan.receive_message(&ip);
		if (msg == MSG_offer) {
			wservers->append_entry(network_connection::ip2string(ip));
		}
	} while (msg.length() != 0);
	wservers->sort_entries();
	wservers->make_entries_unique();
}

void send_msg_to_all(network_connection& sv, const vector<IPaddress>& clients, const string& msg)
{
	for (vector<IPaddress>::const_iterator it = clients.begin(); it != clients.end(); ++it) {
		sv.unbind();
		sv.bind(*it);
		sv.send_message(msg);
		sv.unbind();
	}
}

// collect players, answer "ask" messages, returns true if game should start, false if cancelled
bool server_wait_for_clients(network_connection& sv, Uint16 server_port, vector<IPaddress>& clients, unsigned nr_of_players)
{
	IPaddress hostip;
	int error = SDLNet_ResolveHost(&hostip, 0, server_port);
	system::sys().myassert(error == 0, "can resolve host ip for this computer");
	
	widget w(0, 0, 1024, 768, texts::get(22), 0, swordfishimg);
	w.add_child(new widget_text(40, 60, 0, 0, texts::get(195)));
	widget_list* wplayers = new widget_list(40, 90, 500, 400);
	w.add_child(wplayers);
	
	widget_menu* wm = new widget_menu(40, 700, 0, 40, true);
	w.add_child(wm);
	wm->add_entry(texts::get(20), new widget_caller_arg_button<widget, void (widget::*)(int), int>(&w, &widget::close, 1, 70, 700, 400, 40));
	widget_button* startgame = new widget_caller_arg_button<widget, void (widget::*)(int), int>(&w, &widget::close, 2, 70, 700, 400, 40);
	startgame->disable();
	wm->add_entry(texts::get(196), startgame);
	wm->adjust_buttons(944);

	while (true) {
		int result = w.run(50);
		if (result == 1) {
			send_msg_to_all(sv, clients, MSG_cancel);
			return false;
		} else if (result == 2) {
			return true;
		}
		IPaddress clientip;
		string msg = sv.receive_message(&clientip);

		// answer "ask" or "join" messages only until maximum number of players is reached
		if (clients.size() + 1 < nr_of_players) {
			startgame->disable();
			if (msg == MSG_ask) {
				sv.unbind();
				sv.bind(clientip);
				sv.send_message(MSG_offer);
				sv.unbind();
			} else if (msg == MSG_join) {
				clients.push_back(clientip);
				wplayers->append_entry(network_connection::ip2string(clientip));
				sv.unbind();
				sv.bind(clientip);
				sv.send_message(MSG_joined);
				sv.unbind();
			}
		} else {
			startgame->enable();
		}
	}
}		

void create_network_game(Uint16 server_port)
{
	// start server
	network_connection serv(server_port);
	
	// create a game
	unsigned nr_of_players = 2;	// fixme
	game* gm = new game("submarine_VIIc", 1, 1, 1, 1);		// fixme
	
	// wait for clients to join, reply to "ask" messages
	vector<IPaddress> clients;
	clients.reserve(nr_of_players-1);
	bool ok = server_wait_for_clients(serv, server_port, clients, nr_of_players);
	
	if (!ok) {
		delete gm;
		return;
	}

	// send all players the INIT message and the game state
	ostringstream oss;
	gm->save_to_stream(oss);
	string gamestatemsg = string(MSG_gamestate) + oss.str();
	send_msg_to_all(serv, clients, MSG_initgame);
	send_msg_to_all(serv, clients, gamestatemsg);
	
	// wait for "ready" messages from all clients
	vector<bool> clientready(clients.size());
	unsigned clientsready = 0;
	while (true) {
		IPaddress ip;
		string msg = serv.receive_message(&ip);
		if (msg == MSG_ready) {
			for (unsigned i = 0; i < clients.size(); ++i) {
				if (ip == clients[i] && !clientready[i]) {
					clientready[i] = true;
					++clientsready;
				}
			}
		}
		// all clients ready?
		if (clientsready == clients.size())
			break;
	}

	// send "start" to all clients
	send_msg_to_all(serv, clients, MSG_start);
	
	// now run the game
	run_game(gm);
}

void join_network_game(const string& servername, Uint16 server_port, network_connection& client)
{
	// join game
	client.unbind();
	client.bind(servername, server_port);
	send_and_wait(client, MSG_join, MSG_joined);

	// wait for server to start game
	while (true) {
		string serv = client.receive_message();
		if (serv == MSG_initgame)
			break;
		if (serv == MSG_cancel) {
			client.unbind();
			return;
		}
		system::sys().poll_event_queue();
		SDL_Delay(50);
	}
	
	// create and init game, wait for game state
	string gamestate;
	while (true) {
		string serv = client.receive_message();
		if (serv.substr(0, MSG_length) == MSG_gamestate) {
			gamestate = serv.substr(MSG_length);
			break;
		}
		if (serv == MSG_cancel) {
			client.unbind();
			return;
		}
		system::sys().poll_event_queue();
		SDL_Delay(50);
	}

	istringstream iss(gamestate);
	game* gm = new game(iss);

	bool ok = send_and_wait(client, MSG_ready, MSG_start, 60000);	// 1 minute timeout

	if (ok)
		run_game(gm);
	else
		delete gm;
}

void play_network_game(void)
{
	IPaddress computer_ip;
	Uint16 server_port = 0xdf7d;
	Uint16 local_port = 0xdf7d;
	
	// initialize network play
	int network_ok = SDLNet_Init();
	system::sys().myassert(network_ok != -1, "failed to initialize SDLnet");
	int error = SDLNet_ResolveHost(&computer_ip, NULL, local_port);
	system::sys().myassert(error == 0, "can't resolve host ip for this computer");

	network_connection client;	// used for scanning and later for playing

	widget w(0, 0, 1024, 768, texts::get(22), 0, swordfishimg);
	w.add_child(new widget_text(40, 60, 0, 0, texts::get(57)));
	widget_edit* wserverip = new widget_edit(40, 90, 200, 40, network_connection::ip2string(computer_ip));
	w.add_child(wserverip);
	w.add_child(new widget_text(280, 60, 0, 0, texts::get(58)));
	ostringstream oss; oss << server_port;
	widget_edit* wportaddr = new widget_edit(280, 90, 200, 40, oss.str());
	w.add_child(wportaddr);
	w.add_child(new widget_text(40, 140, 0, 0, texts::get(193)));

	struct wserver_list : public widget_list
	{
		widget_edit * wip, * wpt;
		void on_sel_change(void) {
			string s = get_selected_entry();
			wip->set_text(s.substr(0, s.find(":")));
			wpt->set_text(s.substr(s.find(":")+1));
		}
		wserver_list(int x, int y, int w, int h, widget_edit* wip_, widget_edit* wpt_) : widget_list(x, y, w, h), wip(wip_), wpt(wpt_) {}
		~wserver_list() {}
	};
	widget_list* wservers = new wserver_list(40, 170, 440, 400, wserverip, wportaddr);
	w.add_child(wservers);
	ostringstream oss2; oss2 << local_port;
	w.add_child(new widget_text(40, 600, 0, 0, texts::get(212)));
	widget_edit* wlocalportaddr = new widget_edit(40, 630, 200, 40, oss2.str());
	w.add_child(wlocalportaddr);
	
	ask_for_offered_games(wservers, server_port, client);
	
	widget_menu* wm = new widget_menu(40, 700, 0, 40, true);
	w.add_child(wm);
	wm->add_entry(texts::get(20), new widget_caller_arg_button<widget, void (widget::*)(int), int>(&w, &widget::close, 1, 70, 700, 400, 40));
	wm->add_entry(texts::get(191), new widget_caller_arg_button<widget, void (widget::*)(int), int>(&w, &widget::close, 2, 70, 700, 400, 40));
	wm->add_entry(texts::get(192), new widget_caller_arg_button<widget, void (widget::*)(int), int>(&w, &widget::close, 3, 540, 700, 400, 40));
	wm->add_entry(texts::get(194), new widget_caller_arg_button<widget, void (widget::*)(int), int>(&w, &widget::close, 4, 540, 700, 400, 40));
	wm->adjust_buttons(944);
	int result = 0;
	while (result != 1) {
		w.set_return_value(-1);
		result = w.run(50);
		listen_for_offered_games(wservers, server_port, client);
		if (result == 2) {
			create_network_game(local_port);	// start server, wait for players
		} else if (result == 3) {
			join_network_game(wserverip->get_text(), server_port, client);
		} else if (result == 4) {
			ask_for_offered_games(wservers, server_port, client);
		}
	}
	
	SDLNet_Quit();
}






// old menus are used from here on
void menu_single_mission(void)
{
/*
	widget w(0, 0, 1024, 768, texts::get(21), 0, titlebackgrimg);
	widget_menu* wm = new widget_menu(312, 0, 400, 40);
	w.add_child(wm);
	wm->add_entry(texts::get(8), new widget_func_button<void (*)(void)>(&menu_notimplemented, 0, 0, 0, 0));
	wm->add_entry(texts::get(9), new widget_func_button<void (*)(void)>(&create_convoy_mission, 0, 0, 0, 0));
	wm->add_entry(texts::get(10), new widget_func_button<void (*)(void)>(&choose_historical_mission, 0, 0, 0, 0));
	wm->add_entry(texts::get(118), new widget_func_button<void (*)(void)>(&choose_saved_game, 0, 0, 0, 0));
	wm->add_entry(texts::get(11), new widget_caller_arg_button<widget, void (widget::*)(int), int>(&w, &widget::close, 0, 0, 0, 0, 0));
	wm->align(0, 0);
	w.run();
*/


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
// - enable wave foam
// - detail for map/terrain/water (wave tile detail, # of wave tiles, global terrain detail, wave bump map detail)
// - set fullscreen
// - invert mouse in view
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


// vessel preview
const char* shipnames[14] = {
"battleship_malaya",
"carrier_bogue",
"corvette",
"destroyer_tribal",
"freighter_large",
"freighter_medium",
"merchant_large",
"merchant_medium",
"merchant_small",
"tanker_small",
"troopship_medium",
"submarine_VIIc",
"submarine_IXc40",
"submarine_XXI",
};
int current_ship = 0;
double ship_zangle = 0;
double ship_xangle = 0;
ship* shp = 0;
void draw_ship(void)
{
	glClear(GL_DEPTH_BUFFER_BIT);
	glDisable(GL_LIGHTING);
	glLoadIdentity();
	glTranslatef(0, 0, -2.5);
	glRotatef(-80, 1, 0, 0);
	glRotatef(ship_zangle, 0, 0, 1);
	glRotatef(ship_xangle, 1, 0, 0);
	double sc = 3.0/(modelcache.find(shp->get_modelname())->get_boundbox_size().length());
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
	shp->display();
	sys->prepare_2d_drawing();
	font_nimbusrom->print_hc(512, 128, shp->get_description(2), color::white(), true);
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
	TiXmlDocument doc(get_ship_dir() + shipnames[current_ship] + ".xml");
	doc.LoadFile();
	shp = new ship(&doc);
	while (true) {
		unsigned sel = m.run(draw_ship);
		if (sel == 6) break;
		int lastship = current_ship;
#define ROTANG 5
		switch (sel) {
			case 0: ship_zangle -= ROTANG; break;
			case 1: ship_zangle += ROTANG; break;
			case 2: ship_xangle -= ROTANG; break;
			case 3: ship_xangle += ROTANG; break;
			case 4: ++current_ship; break;
			case 5: --current_ship; break;
		}
#undef ROTANG		
		if (current_ship != lastship) {
			if (current_ship < 0) current_ship = 13;
			if (current_ship > 13) current_ship = 0;
			delete shp;
			lastship = current_ship;
			if (current_ship < 11) {
				TiXmlDocument doc(get_ship_dir() + shipnames[current_ship] + ".xml");
				doc.LoadFile();
				shp = new ship(&doc);
			} else {
				TiXmlDocument doc(get_submarine_dir() + shipnames[current_ship] + ".xml");
				doc.LoadFile();
				shp = new submarine(&doc);
			}
 		}
	}
	delete shp;
}


bool file_exists(const string& fn)
{
	ifstream in(fn.c_str(), ios::in|ios::binary);
	return in.good();
}


int main(int argc, char** argv)
{
	string highscoredirectory =
#ifdef WIN32
	"./highscores/";
#else
	// fixme: use global /var/games instead
	string(getenv("HOME"))+"/."+PACKAGE + "/";
#endif

	string configdirectory =
#ifdef WIN32
	"./config/";
#else
	string(getenv("HOME"))+"/."+PACKAGE + "/";
#endif

	cfg& mycfg = cfg::instance();
	mycfg.register_option("screen_res_x", 1024);
	mycfg.register_option("screen_res_y", 768);
	mycfg.register_option("fullscreen", true);
	mycfg.register_option("debug", false);
	mycfg.register_option("sound", true);

	// make sure the default values are stored if there is no config file,
	// and make sure all registered values are stored in it
	//fixme: 2004/05/29, this failes if there is no config dir
	//(no .dangerdeep in user's home). This is created after sys
	//has been initialised, but config loading must be done before
	//sys is inited, because it must be done before the command line
	//is read and this must happen before sys is inited...
	FILE* fconfig = fopen((configdirectory + "config").c_str(), "rt");
	if (fconfig) {
		fclose(fconfig);
	} else {
		mycfg.save(configdirectory + "config");
	}
	mycfg.load(configdirectory + "config");

/*
	// 100 times...
	cfg::registeru("key_x", SDLK_x);
	...
	cfg::registeru("key_zoommap", SDLK_PLUS);
	...
	cfg::registeru("key_flankspeed", SDLK_4);
*/

/*
	2004/04/28 idea.
	make an "options" or "config" class, that is initialized by reading
	the config file and reading the command line arguments.
	Every other class can ask the config-class about some values, e.g.
	user_interface could init the water with
	int waterresx = config::ask("water_res_x"); etc...
	this would be very handy.
	fixme: finish class cfg, use it!
*/

	// command line argument parsing
	res_x = 1024;
	bool fullscreen = true;
	string cmdmissionfilename;
	list<string> args;
	while (--argc > 0) args.push_front(string(argv[argc]));
	for (list<string>::iterator it = args.begin(); it != args.end(); ++it) {
		if (*it == "--help") {
			cout << argv[0] << ", usage:\n--help\t\tshow this\n"
			<< "--res n\t\tuse resolution n horizontal,\n\t\tn is 512,640,800,1024 (recommended) or 1280\n"
			<< "--nofullscreen\tdon't use fullscreen\n"
			<< "--debug\t\tdebug mode: no fullscreen, resolution 800\n"
			<< "--mission fn\trun mission from file fn (just the filename in the mission directory)\n"
			<< "--nosound\tdon't use sound\n";
			return 0;
		} else if (*it == "--nofullscreen") {
			fullscreen = false;
		} else if (*it == "--debug") {
			fullscreen = false;
			res_x = 800;
		} else if (*it == "--mission") {
			list<string>::iterator it2 = it; ++it2;
			if (it2 != args.end()) {
				cmdmissionfilename = *it2;
				++it;
			}
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
		} else {
			cout << "unknown " << *it << " parse cfg.\n";
			mycfg.parse_value(*it);//fixme test
		}
	}

//	mycfg.save("./testconf");

	// set language to default
	texts::set_language();

	res_y = res_x*3/4;
	// weather conditions and earth curvature allow 30km sight at maximum.
	sys = new class system(1.0, 30000.0+500.0, res_x, fullscreen);
	sys->set_res_2d(1024, 768);
	sys->set_max_fps(100);
	
	sys->add_console("$ffffffDanger $c0c0c0from the $ffffffDeep");
	sys->add_console("$ffff00copyright and written 2003 by $ff0000Thorsten Jordan");
	sys->add_console(string("$ff8000version ")+VERSION);
	sys->add_console("$80ff80*** welcome ***");

	GLfloat lambient[4] = {0.5,0.5,0.5,1};
	GLfloat ldiffuse[4] = {1,1,1,1};
	GLfloat lposition[4] = {0,0,1,0};
	glLightfv(GL_LIGHT0, GL_AMBIENT, lambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, ldiffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, lposition);
	glEnable(GL_LIGHT0);

	init_global_data();
	
	widget::set_theme(new widget::theme("widgetelements_menu.png", "widgeticons_menu.png",
		font_nimbusrom, color::white(), color(255, 64, 64), color(64,64,32)));

	sys->draw_console_with(font_arial, background);

	
	// init save game dir
	directory savegamedir = open_dir(savegamedirectory);
	if (!savegamedir.is_valid()) {
		bool ok = make_dir(savegamedirectory);
		if (!ok) {
			system::sys().myassert(false, "could not create save game directory.");
		}
	}
	// init config dir
	directory configdir = open_dir(configdirectory);
	if (!configdir.is_valid()) {
		bool ok = make_dir(configdirectory);
		if (!ok) {
			system::sys().myassert(false, "could not create config directory.");
		}
	}
	// init highscore dir
	directory highscoredir = open_dir(highscoredirectory);
	if (!highscoredir.is_valid()) {
		bool ok = make_dir(highscoredirectory);
		if (!ok) {
			system::sys().myassert(false, "could not create highscore directory.");
		}
	}


	// read highscores
	if (!file_exists(highscoredirectory + HSL_MISSION_NAME))
		highscorelist().save(highscoredirectory + HSL_MISSION_NAME);
	if (!file_exists(highscoredirectory + HSL_CAREER_NAME))
		highscorelist().save(highscoredirectory + HSL_CAREER_NAME);
	hsl_mission = highscorelist(highscoredirectory + HSL_MISSION_NAME);
	hsl_career = highscorelist(highscoredirectory + HSL_CAREER_NAME);


	// check if there was a mission given at the command line
	if (cmdmissionfilename.length() > 0) {
		TiXmlDocument doc(get_mission_dir() + cmdmissionfilename);
		doc.LoadFile();
		run_game(new game(&doc));
	} else {
		// main menu
		menu m(104, titlebackgrimg);
		m.add_item(21, menu_single_mission);
		m.add_item(22, play_network_game);
		m.add_item(23, menu_notimplemented);
		m.add_item(24, menu_show_vessels);
		m.add_item(25, show_halloffame_mission);
		m.add_item(213, show_credits);
		m.add_item(26, menu_select_language);
		m.add_item(29, menu_notimplemented /*menu_options*/);
		m.add_item(30, 0);
		m.run();
	}

	sys->write_console(true);

	hsl_mission.save(highscoredirectory + HSL_MISSION_NAME);
	hsl_career.save(highscoredirectory + HSL_CAREER_NAME);

	deinit_global_data();
	delete sys;

	return 0;
}
