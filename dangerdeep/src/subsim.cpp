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

// main program
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <time.h>
#endif

#ifdef USE_NATIVE_GL
#include <gl.h>
#else
#include "oglext/OglExt.h"
#endif
#include <glu.h>
#include <SDL.h>
#include <SDL_net.h>

#include "system.h"
#include "vector3.h"
#include "model.h"
#include "texture.h"
#include "game.h"
#include "game_editor.h"
#include "ship.h"
#include "global_data.h"
#include "texts.h"
#include "date.h"
#include "network.h"
#include <iostream>
#include <sstream>
#include "image.h"
#include "widget.h"
#include "filehelper.h"
#include "highscorelist.h"
#include "user_interface.h"
#include "cfg.h"
#include "keys.h"
#include "music.h"
#include "faulthandler.h"
#include "datadirs.h"
#include "credits.h"

#include "mymain.cpp"

using std::auto_ptr;


/* fixme: 2006/12/02
   about caches:
   especially for images we need a cache like this:
   - basic functionality like objcache with ref and unref
   - unref'd images are kept as objects
   - when new images are ref'd old objects are occaisonally removed.
   How to decide about removal:
   - We have to limit RAM/Video-Ram usage:
   - Each image has a cost, that is width*height*bpp.
   - Each unref'd image has an age, a timestamp taken when unref'd lastly.
   - Compute total cost with some formula
   - Remove image with most cost, if total ram usage of unref'd images is above limit.
   So we have:
   - global limit for unref'd images, good starting point 4MB.
   - A formula to compute cost, c(s, t)  where s is size in bytes and t is age in milliseconds.
   We should have that few images to just keep a simple list for management.
   What about the cost formula?
   - Size matters
   - time is only to get rid of old, but small images.
   Try this formula:
   cost(s, t) = s + t^2/32.
   So after ca. 10 seconds the cost of an 1x1 image is as high as an fullscreen 3MB image.
   The cache should have a "clean_up" function to instantly remove all cleaned up images.
*/


highscorelist hsl_mission, hsl_career;
#define HSL_MISSION_NAME "mission.hsc"
#define HSL_CAREER_NAME "career.hsc"

// a dirty hack
void menu_notimplemented()
{
	widget w(0, 0, 1024, 768, "", 0, "titlebackgr.jpg");
	widget_menu* wm = new widget_menu(0, 0, 400, 40, texts::get(110));
	w.add_child(wm);
	wm->add_entry(texts::get(20), new widget_caller_arg_button<widget, void (widget::*)(int), int>(&w, &widget::close, 0, 0, 0, 0, 0));
	wm->align(0, 0);
	w.run(0, false);
}


//
// save game directory and helper functions
//
string savegamedirectory =
#ifdef WIN32
	"./save/";
#else	
	string(getenv("HOME"))+"/.dangerdeep/";
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
	void load();
	void save();
	void erase();
	void quit();
	void cancel() { close(0); }	// return 0 for cancel/return, 1 for quit (if saving is enabled), 2 for loaded
	void update_list();

public:	
	string get_gamefilename_to_load() const { return gamefilename_to_load; }
	widget_edit* get_gamename() const { return gamename; }
	loadsavequit_dialogue(const game* g);	// give 0 to disable saving
};

loadsavequit_dialogue::loadsavequit_dialogue(const game *g) : widget(0, 0, 1024, 768, texts::get(177), 0, "depthcharge.jpg"), mygame(g), gamesaved(false)
{
	add_child(new widget_text(40, 40, 0, 0, texts::get(178)));
	gamename = new widget_edit(200, 40, 684, 40, "");
	add_child(gamename);
	widget_menu* wm = new widget_menu(40, 700, 180, 40, "", true);
	add_child(wm);
	btnload = wm->add_entry(texts::get(118), new widget_caller_button<loadsavequit_dialogue, void (loadsavequit_dialogue::*)()>(this, &loadsavequit_dialogue::load));
	if (mygame)
		btnsave = wm->add_entry(texts::get(119), new widget_caller_button<loadsavequit_dialogue, void (loadsavequit_dialogue::*)()>(this, &loadsavequit_dialogue::save));
	btndel = wm->add_entry(texts::get(179), new widget_caller_button<loadsavequit_dialogue, void (loadsavequit_dialogue::*)()>(this, &loadsavequit_dialogue::erase));
	if (mygame)
		btnquit = wm->add_entry(texts::get(120), new widget_caller_button<loadsavequit_dialogue, void (loadsavequit_dialogue::*)()>(this, &loadsavequit_dialogue::quit));
	btncancel = wm->add_entry(texts::get(mygame ? 121 : 20), new widget_caller_button<loadsavequit_dialogue, void (loadsavequit_dialogue::*)()>(this, &loadsavequit_dialogue::cancel));
	wm->adjust_buttons(944);
	struct lsqlist : public widget_list
	{
		void on_sel_change() {
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

void loadsavequit_dialogue::load()
{
	gamefilename_to_load = get_savegame_name_for(gamename->get_text(), savegames);
	//fixme: ask: replace this game?
	auto_ptr<widget> w(create_dialogue_ok(texts::get(185), texts::get(180) + gamename->get_text() + texts::get(181)));
	w->run();
	close(2);	// load and close
}

void loadsavequit_dialogue::save()
{
	string fn = get_savegame_name_for(gamename->get_text(), savegames);
	FILE* f = fopen(fn.c_str(), "rb");
	if (f) {
		fclose(f);
		auto_ptr<widget> w(create_dialogue_ok_cancel(texts::get(182), texts::get(183) + gamename->get_text() + texts::get(184)));
		int ok = w->run();
		w.reset();
		if (!ok) return;
	}
	gamesaved = true;
	mygame->save(fn, gamename->get_text());
	auto_ptr<widget> w(create_dialogue_ok(texts::get(186), texts::get(180) + gamename->get_text() + texts::get(187)));
	w->run();
	update_list();
}

void loadsavequit_dialogue::erase()
{
	auto_ptr<widget> w(create_dialogue_ok_cancel(texts::get(182), texts::get(188) + gamename->get_text() + texts::get(189)));
	int ok = w->run();
	w.reset();
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

void loadsavequit_dialogue::quit()
{
	if (!gamesaved) {
		auto_ptr<widget> w(create_dialogue_ok_cancel(texts::get(182), texts::get(190)));
		int q = w->run();
		if (q) close(1);
	} else {
		close(1);
	}
}



void loadsavequit_dialogue::update_list()
{
	savegames.clear();

	// read save games in directory
	{
		directory savegamedir(savegamedirectory);
		while (true) {
			string e = savegamedir.read();
			if (e.empty()) break;
			if (is_savegame_name(e)) {
				string descr = mygame->read_description_of_savegame(savegamedirectory+e);
				savegames[e] = descr;
			}
		}
	}
	
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
	widget w(0, 0, 1024, 768, texts::get(197), 0, "krupp_docks.jpg");
	w.add_child(new widget(40, 50, 944, 640, string("")));
	w.add_child(new widget_caller_arg_button<widget, void (widget::*)(int), int>(&w, &widget::close, 1, (1024-128)/2, 768-32-16, 128, 32, texts::get(105)));
	hsl.show(&w);
	w.run(0, false);
}
void show_halloffame_mission() { show_halloffame(hsl_mission); }
void show_halloffame_career() { show_halloffame(hsl_career); }


//
// check if a game is good enough for the high score list
//
void check_for_highscore(const game& gm)
{
	unsigned totaltons = 0;
	const list<game::sink_record>& sunken_ships = gm.get_sunken_ships();
	for (list<game::sink_record>::const_iterator it = sunken_ships.begin(); it != sunken_ships.end(); ++it) {
		totaltons += it->tons;
	}
	highscorelist& hsl = (/* check if game is career or mission fixme */ true) ? hsl_mission : hsl_career;
	unsigned points = totaltons /* compute points from tons etc here fixme */;

	widget w(0, 0, 1024, 768, texts::get(197), 0, "krupp_docks.jpg");
	w.add_child(new widget(40, 50, 944, 640, string("")));
	w.add_child(new widget_caller_arg_button<widget, void (widget::*)(int), int>(&w, &widget::close, 1, (1024-128)/2, 768-32-16, 128, 32, texts::get(105)));
	unsigned pos = hsl.get_listpos_for(points);
	if (hsl.is_good_enough(points)) {
		w.add_child(new widget_text(200, 200, 0,0, texts::get(199)));
		if (pos == 0)
			w.add_child(new widget_text(200, 240, 0,0, texts::get(201)));
		w.add_child(new widget_text(400, 280, 0,0, texts::get(200)));
		widget_edit* wname = new widget_edit(300, 320, 424, 32, "");
		w.add_child(wname);
		w.run(0, false);
		string playername = wname->get_text();
		if (playername.length() == 0)
			playername = "INCOGNITO";
		hsl.record(points, playername);
	} else {
		w.add_child(new widget_text(400, 200, 0,0, texts::get(198)));
	}
	w.run(0, false);
	show_halloffame(hsl);
}


//
// show results after a game ended
//
void show_results_for_game(const game& gm)
{
	widget w(0, 0, 1024, 768, texts::get(124), 0, "sunken_destroyer.jpg");
	widget_list* wl = new widget_list(64, 64, 1024-64-64, 768-64-64);
	wl->set_column_width((1024-2*64)/4);
	w.add_child(wl);
	w.add_child(new widget_caller_arg_button<widget, void (widget::*)(int), int>(&w, &widget::close, 1, (1024-128)/2, 768-32-16, 128, 32, texts::get(105)));
	unsigned totaltons = 0;
	const list<game::sink_record>& sunken_ships = gm.get_sunken_ships();
	for (list<game::sink_record>::const_iterator it = sunken_ships.begin(); it != sunken_ships.end(); ++it) {
		ostringstream oss;
		oss << texts::numeric_from_date(it->dat) << "\t"
			<< it->descr << "\t\t"
			<< it->tons << " BRT";
		totaltons += it->tons;
		wl->append_entry(oss.str());
	}
	ostringstream os;
	os << "total: " << totaltons;
	wl->append_entry(os.str());
	w.run(0, false);
}



// main play loop
// fixme: clean this up!!!
game::run_state game__exec(game& gm, user_interface& ui)
{
	// fixme: add special ui heir: playback
	// to record videos.
	// record ship positions or at least commands!
	// and camera path (bspline) etc.
	// used for credits background etc.

	unsigned frames = 1;
	unsigned lasttime = sys().millisec();
	unsigned lastframes = 1;
	double fpstime = 0;
	double totaltime = 0;
	double measuretime = 5;	// seconds

	ui.resume_all_sound();
	
	// draw one initial frame
	ui.display();

	ui.request_abort(false);
	
	while (gm.get_run_state() == game::running && !ui.abort_requested()) {
		list<SDL_Event> events = sys().poll_event_queue();

		// maybe limit input processing to 30 fps
		ui.process_input(events);

		// this time_scaling is bad. hits may get computed wrong when time
		// scaling is too high. fixme
		unsigned thistime = sys().millisec();
		if (gm.get_freezetime_start() > 0)
			throw error("freeze_time() called without unfreeze_time() call");
		lasttime += gm.process_freezetime();
		unsigned time_scale = ui.time_scaling();
		double delta_time = (thistime - lasttime)/1000.0; // * time_scale;
		totaltime += (thistime - lasttime)/1000.0;
		lasttime = thistime;
		
		// next simulation step
		if (!ui.paused()) {
			for (unsigned j = 0; j < time_scale; ++j) {
				gm.simulate(time_scale == 1 ? delta_time : (1.0/30.0));
				// evaluate events of game, because they are cleared
				// by next call of game::simulate and new ones are
				// generated
				const ptrlist<event>& events = gm.get_events();
				for (ptrlist<event>::const_iterator it = events.begin(); it != events.end(); ++it) {
					it->evaluate(ui);
				}
			}
		}

		// fixme: make use of game::job interface, 3600/256 = 14.25 secs job period
		ui.set_time(gm.get_time());
		ui.display();
		++frames;

		// record fps
		if (totaltime - fpstime >= measuretime) {
			fpstime = totaltime;
			ostringstream os;
			os << "$c0fffffps " << (frames - lastframes)/measuretime;
			lastframes = frames;
			sys().add_console(os.str());
		}
		
		sys().swap_buffers();
	}
	
	ui.pause_all_sound();
	
	return gm.get_run_state();	// if player is killed, end game (1), else show menu (0)
}


//
// start and run a game, handle load/save (game menu), show results after game's end, delete game
//
void run_game(auto_ptr<game> gm)
{
	// clear memory of menu widgets
	widget::unref_all_backgrounds();

	auto_ptr<widget::theme> gametheme(new widget::theme("widgetelements_game.png", "widgeticons_game.png",
								 font_typenr,color(182, 146, 137),color(240, 217, 127), color(64,64,64)));
	reset_loading_screen();
	// embrace user interface generation with right theme set!
	auto_ptr<widget::theme> tmp = widget::replace_theme(gametheme);
	auto_ptr<user_interface> ui(user_interface::create(*gm));
	gametheme = widget::replace_theme(tmp);
	while (true) {
		tmp = widget::replace_theme(gametheme);
		game::run_state state = game__exec(*gm, *ui);
		gametheme = widget::replace_theme(tmp);

		//if (state == 2) break;
		//SDL_ShowCursor(SDL_ENABLE);
		if (state != game::running) {
//			if (state == game::mission_complete)

			if (state == game::player_killed) {
				music::inst().play_track(1, 500);
				widget w(0, 0, 1024, 768, "", 0, "killed.jpg");
				widget_menu* wm = new widget_menu(0, 0, 400, 40, texts::get(103));
				w.add_child(wm);
				wm->add_entry(texts::get(105), new widget_caller_arg_button<widget, void (widget::*)(int), int>(&w, &widget::close, 0, 0, 0, 0, 0));
				wm->align(0, 0);
				w.run(0, false);
			}

//			if (state == game::contact_lost)

			//widget* w = widget::create_dialogue_ok_cancel("Quit game?", "");
			//int q = w->run();
			//delete w;
			//if (q == 1)
				break;
		} else {
			music::inst().play_track(1, 500);
			loadsavequit_dialogue dlg(gm.get());
			int q = dlg.run(0, false);
			// replace game and ui if new game was loaded
			if (q == 2) {
				// fixme: ui doesn't need to get replaced, just give pointer to new
				// game to old ui, clear ui values and messages, finished...
				// this safes time to recompute map/water/sky etc.
				// this can only work if old and new game have same type
				// of player (and thus same type of ui)
				gm.reset();
				ui.reset();
				gm.reset(new game(dlg.get_gamefilename_to_load()));
				// embrace user interface generation with right theme set!
				tmp = widget::replace_theme(gametheme);
				ui.reset(user_interface::create(*gm));
				gametheme = widget::replace_theme(tmp);
			}
			//replace ui after loading!!!!
			if (q == 1){
				music::inst().play_track(1, 500);
				break;
			}
			if( q == 0 ){
				//music::inst()._fade_out(1000);
			}
		}
		//SDL_ShowCursor(SDL_DISABLE);
	}
	show_results_for_game(*gm);
	check_for_highscore(*gm);

	// restore menu widgets
	widget::ref_all_backgrounds();
}



//
// start and run a game editor, handle load/save (game menu), delete game
//
void run_game_editor(auto_ptr<game> gm)
{
	// clear memory of menu widgets
	widget::unref_all_backgrounds();

	auto_ptr<widget::theme> gametheme(new widget::theme("widgetelements_game.png", "widgeticons_game.png",
							    font_typenr,color(182, 146, 137),color(240, 217, 127), color(64,64,64)));
	reset_loading_screen();
	// embrace user interface generation with right theme set!
	auto_ptr<widget::theme> tmp = widget::replace_theme(gametheme);
	auto_ptr<user_interface> ui(user_interface::create(*gm));
	gametheme = widget::replace_theme(tmp);
	// game is initially running, so pause it.
	ui->toggle_pause();
	while (true) {
		tmp = widget::replace_theme(gametheme);
		// 2006-12-01 doc1972 we should do some checks of the state if game exits
		/*game::run_state state =*/ game__exec(*gm, *ui);
		gametheme = widget::replace_theme(tmp);

		music::inst().play_track(1, 500);
		loadsavequit_dialogue dlg(gm.get());
		int q = dlg.run(0, false);
		// replace game and ui if new game was loaded
		if (q == 2) {
			// fixme: ui doesn't need to get replaced, just give pointer to new
			// game to old ui, clear ui values and messages, finished...
			// this safes time to recompute map/water/sky etc.
			// as long as class game holds a pointer to ui this is more difficult or
			// won't work.
			gm.reset();
			ui.reset();
			gm.reset(new game_editor(dlg.get_gamefilename_to_load()));
			// embrace user interface generation with right theme set!
			tmp = widget::replace_theme(gametheme);
			ui.reset(user_interface::create(*gm));
			gametheme = widget::replace_theme(tmp);
		}
		//replace ui after loading!!!!
		if (q == 1){
			music::inst().play_track(1, 500);
			break;
		}
		if( q == 0 ){
			//music::inst()._fade_out(1000);
		}
	}

	// restore menu widgets
	widget::ref_all_backgrounds();
}



//
// create a custom convoy mission
//
void create_convoy_mission()
{
	widget w(0, 0, 1024, 768, texts::get(9), 0, "scopewatcher.jpg");
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
	widget_list* wtimeperiod = new widget_list(40, 340, 640, 200);
	w.add_child(wtimeperiod);
	wsubtype->append_entry(texts::get(17));
	wsubtype->append_entry(texts::get(174));
	wsubtype->append_entry(texts::get(18));
	wsubtype->append_entry(texts::get(660));
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

	widget_menu* wm = new widget_menu(40, 700, 0, 40, "", true);
	w.add_child(wm);
	wm->add_entry(texts::get(20), new widget_caller_arg_button<widget, void (widget::*)(int), int>(&w, &widget::close, 1, 70, 700, 400, 40));
	wm->add_entry(texts::get(19), new widget_caller_arg_button<widget, void (widget::*)(int), int>(&w, &widget::close, 2, 540, 700, 400, 40));
	wm->adjust_buttons(944);
	int result = w.run(0, false);
	if (result == 2) {	// start game
		string st;
		switch (wsubtype->get_selected()) {
			case 0: st = "submarine_VIIc"; break;
			case 1: st = "submarine_IXc40"; break;
			case 2: st = "submarine_XXI"; break;
			case 3: st = "submarine_IIa"; break;
		}
		// reset loading screen here to show user we are doing something
		reset_loading_screen();
		run_game(auto_ptr<game>(new game(st,
						 wcvsize->get_selected(),
						 wescortsize->get_selected(),
						 wtimeofday->get_selected(),
						 wtimeperiod->get_selected())));
	}
}


//
// choose a historical mission
//
void choose_historical_mission()
{
	vector<string> missions;
	
	// read missions
	unsigned nr_missions = 0;
	{
		directory missiondir(get_mission_dir());
		while (true) {
			string e = missiondir.read();
			if (e.empty()) break;
			if (e.length() > 4 && e.substr(e.length()-4) == ".xml") {
				missions.push_back(e);
				++nr_missions;
			}
		}
	}

	// read descriptions, set up windows
	widget w(0, 0, 1024, 768, texts::get(10), 0, "sunderland.jpg");
	vector<string> descrs;
	struct msnlist : public widget_list
	{
		const vector<string>& descrs;
		widget_text* wdescr;
		void on_sel_change() {
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
	// Note:
	// Missions have the same format like savegames, except that the head xml node
	// has an additional child node <description> with multi-lingual descriptions of the mission.
	for (unsigned i = 0; i < nr_missions; ++i) {
		xml_doc doc(get_mission_dir() + missions[i]);
		doc.load();
		xml_elem edftdmission = doc.child("dftd-mission");
		xml_elem edescription = edftdmission.child("description");
		for (xml_elem::iterator it = edescription.iterate("short"); !it.end(); it.next()) {
			if (it.elem().attr("lang") == texts::get_language_code()) {
				string desc;
				try {
					desc = it.elem().child_text();
				}
				catch (xml_error& e) {
					desc = "NO DESCRIPTION???";
				}
				wmission->append_entry(desc);
				break;
			}
		}
		for (xml_elem::iterator it = edescription.iterate("long"); !it.end(); it.next()) {
			if (it.elem().attr("lang") == texts::get_language_code()) {
				string desc;
				try {
					desc = it.elem().child_text();
				}
				catch (xml_error& e) {
					desc = "NO DESCRIPTION???";
				}
				descrs.push_back(desc);
				break;
			}
		}
	}
	wmission->on_sel_change();

	widget_menu* wm = new widget_menu(40, 700, 0, 40, "", true);
	w.add_child(wm);
	wm->add_entry(texts::get(20), new widget_caller_arg_button<widget, void (widget::*)(int), int>(&w, &widget::close, 1, 70, 700, 400, 40));
	wm->add_entry(texts::get(19), new widget_caller_arg_button<widget, void (widget::*)(int), int>(&w, &widget::close, 2, 70, 700, 400, 40));
	wm->adjust_buttons(944);
	int result = w.run(0, false);
	if (result == 2) {	// start game
		auto_ptr<game> gm;
		try {
			gm.reset(new game(get_mission_dir() + missions[wmission->get_selected()]));
		}
		catch (error& e) {
			sys().add_console(string("error loading game: ") + e.what());
			// fixme: show dialogue!
			return;
		}
		// reset loading screen here to show user we are doing something
		reset_loading_screen();
		run_game(gm);
	}
}


//
// choose a saved game
//
void choose_saved_game()
{
	loadsavequit_dialogue dlg(0);
	int q = dlg.run(0, false);
	if (q == 0) return;
	if (q == 2) {
		// reset loading screen here to show user we are doing something
		reset_loading_screen();
		run_game(auto_ptr<game>(new game(dlg.get_gamefilename_to_load())));
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
		sys().poll_event_queue();
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
	sys().myassert(error == 0, "can resolve host ip for this computer");
	
	widget w(0, 0, 1024, 768, texts::get(22), 0, "swordfish.jpg");
	w.add_child(new widget_text(40, 60, 0, 0, texts::get(195)));
	widget_list* wplayers = new widget_list(40, 90, 500, 400);
	w.add_child(wplayers);
	
	widget_menu* wm = new widget_menu(40, 700, 0, 40, "", true);
	w.add_child(wm);
	wm->add_entry(texts::get(20), new widget_caller_arg_button<widget, void (widget::*)(int), int>(&w, &widget::close, 1, 70, 700, 400, 40));
	widget_button* startgame = new widget_caller_arg_button<widget, void (widget::*)(int), int>(&w, &widget::close, 2, 70, 700, 400, 40);
	startgame->disable();
	wm->add_entry(texts::get(196), startgame);
	wm->adjust_buttons(944);

	while (true) {
		int result = w.run(50, false);
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
	auto_ptr<game> gm(new game("submarine_VIIc", 1, 1, 1, 1));		// fixme
	
	// wait for clients to join, reply to "ask" messages
	vector<IPaddress> clients;
	clients.reserve(nr_of_players-1);
	bool ok = server_wait_for_clients(serv, server_port, clients, nr_of_players);
	
	if (!ok) {
		return;
	}

	// send all players the INIT message and the game state
	ostringstream oss;
	//gm->save_to_stream(oss);//fixme: now xml
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
	// reset loading screen here to show user we are doing something
	reset_loading_screen();
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
		sys().poll_event_queue();
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
		sys().poll_event_queue();
		SDL_Delay(50);
	}

	istringstream iss(gamestate);
	auto_ptr<game> gm;
	try {
		//fixme
//		gm.reset(new game(iss));
	}
	catch (error& e) {
		sys().add_console(string("error loading game: ") + e.what());
		// fixme: show dialogue!
		return;
	}

	bool ok = send_and_wait(client, MSG_ready, MSG_start, 60000);	// 1 minute timeout

	if (ok) {
		// reset loading screen here to show user we are doing something
		reset_loading_screen();
		run_game(gm);
	}
}

void play_network_game()
{
	IPaddress computer_ip;
	Uint16 server_port = 0xdf7d;
	Uint16 local_port = 0xdf7d;
	
	// initialize network play
	int network_ok = SDLNet_Init();
	sys().myassert(network_ok != -1, "failed to initialize SDLnet");
	int error = SDLNet_ResolveHost(&computer_ip, NULL, local_port);
	sys().myassert(error == 0, "can't resolve host ip for this computer");

	network_connection client;	// used for scanning and later for playing

	widget w(0, 0, 1024, 768, texts::get(22), 0, "swordfish.jpg");
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
		void on_sel_change() {
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
	
	widget_menu* wm = new widget_menu(40, 700, 0, 40, "", true);
	w.add_child(wm);
	wm->add_entry(texts::get(20), new widget_caller_arg_button<widget, void (widget::*)(int), int>(&w, &widget::close, 1, 70, 700, 400, 40));
	wm->add_entry(texts::get(191), new widget_caller_arg_button<widget, void (widget::*)(int), int>(&w, &widget::close, 2, 70, 700, 400, 40));
	wm->add_entry(texts::get(192), new widget_caller_arg_button<widget, void (widget::*)(int), int>(&w, &widget::close, 3, 540, 700, 400, 40));
	wm->add_entry(texts::get(194), new widget_caller_arg_button<widget, void (widget::*)(int), int>(&w, &widget::close, 4, 540, 700, 400, 40));
	wm->adjust_buttons(944);
	int result = 0;
	while (result != 1) {
		w.set_return_value(-1);
		result = w.run(50, false);
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



void menu_single_mission()
{
	widget w(0, 0, 1024, 768, "", 0, "titlebackgr.jpg");
	widget_menu* wm = new widget_menu(0, 0, 400, 40, texts::get(21));
	w.add_child(wm);
	wm->add_entry(texts::get(8), new widget_func_button<void (*)()>(&menu_notimplemented, 0, 0, 0, 0));
	wm->add_entry(texts::get(9), new widget_func_button<void (*)()>(&create_convoy_mission, 0, 0, 0, 0));
	wm->add_entry(texts::get(10), new widget_func_button<void (*)()>(&choose_historical_mission, 0, 0, 0, 0));
	wm->add_entry(texts::get(118), new widget_func_button<void (*)()>(&choose_saved_game, 0, 0, 0, 0));
	wm->add_entry(texts::get(11), new widget_caller_arg_button<widget, void (widget::*)(int), int>(&w, &widget::close, 0, 0, 0, 0, 0));
	wm->align(0, 0);
	w.run(0, false);
}



void menu_mission_editor()
{
	widget w(0, 0, 1024, 768, texts::get(222), 0, "scopewatcher.jpg");
	w.add_child(new widget_text(40, 60, 944, 0, texts::get(223)));

/*
	w.add_child(new widget_text(40, 60, 0, 0, texts::get(16)));
	widget_list* wsubtype = new widget_list(40, 90, 200, 200);
	w.add_child(wsubtype);
	wsubtype->append_entry(texts::get(17));
	wsubtype->append_entry(texts::get(174));
	wsubtype->append_entry(texts::get(18));
*/

	widget_menu* wm = new widget_menu(40, 700, 0, 40, "", true);
	w.add_child(wm);
	wm->add_entry(texts::get(20), new widget_caller_arg_button<widget, void (widget::*)(int), int>(&w, &widget::close, 1, 540, 700, 400, 40));
	wm->add_entry(texts::get(222), new widget_caller_arg_button<widget, void (widget::*)(int), int>(&w, &widget::close, 2, 70, 700, 400, 40));
	wm->adjust_buttons(944);
	int result = w.run(0, false);
	if (result == 2) {	// start editor
/*
		string st;
		switch (wsubtype->get_selected()) {
			case 0: st = "submarine_VIIc"; break;
			case 1: st = "submarine_IXc40"; break;
			case 2: st = "submarine_XXI"; break;
		}
*/
		// reset loading screen here to show user we are doing something
		reset_loading_screen();
		run_game_editor(auto_ptr<game>(new game_editor(/*st*/)));
	}
}



void menu_select_language()
{
	widget w(0, 0, 1024, 768, "", 0, "titlebackgr.jpg");
	widget_menu* wm = new widget_menu(0, 0, 400, 40, texts::get(26));

	struct lgclist : public widget_list
	{
		void on_sel_change() {
			texts::set_language(get_selected());
			cfg::instance().set("language", get_selected());
		}
		lgclist(int x, int y, int w, int h) : widget_list(x, y, w, h) {}
	};

	widget_list* wlg = new lgclist(0, 0, 400, 400);
	unsigned nl = texts::get_nr_of_available_languages();
	for (unsigned i = 0; i < nl; ++i) {
		wlg->append_entry(texts::get(i, texts::languages));
	}
	wlg->set_selected(texts::get_current_language_nr());

	widget_button* wcb = new widget_caller_arg_button<widget, void (widget::*)(int), int>(&w, &widget::close, 0, 0, 0, 400, 40, texts::get(11));

	w.add_child(wm);
	w.add_child(wlg);
	w.add_child(wcb);
	wlg->align(0, 0);
	vector2i wlgp = wlg->get_pos();
	vector2i wlgs = wlg->get_size();
	wm->set_pos(vector2i(wlgp.x, wlgp.y - 60));
	wcb->set_pos(vector2i(wlgp.x, wlgp.y + wlgs.y + 20));

	w.run(0, false);
}

//
// options:
// - set resolution
// - enable bump mapping
// - enable wave foam
// - detail for map/terrain/water (wave tile detail, # of wave tiles, global terrain detail, wave bump map detail)
// - set fullscreen
// - invert mouse in view
// - set keys
//

void apply_mode(widget_list* wlg)
{
	int width, height; 
    
	string wks = wlg->get_selected_entry();  
  
	height = atoi(wks.substr(wks.rfind("x")+1).c_str());
	width = atoi(wks.substr(0,wks.rfind("x")).c_str());  

	// try to set video mode BEFORE writing to config file, so that if video mode
	// is broken, user is not forced to same mode again on restart
	try {
		sys().set_video_mode(width, height, sys().is_fullscreen_mode());
		cfg::instance().set("screen_res_y",height);
		cfg::instance().set("screen_res_x",width);
	}
	catch (exception& e) {
		sys().add_console(string("Video mode setup failed: ") + e.what());
	}
}

void menu_resolution()
{
	const list<vector2i>& available_resolutions = sys().get_available_resolutions();

	widget w(0, 0, 1024, 768, "", 0, "titlebackgr.jpg");
	widget_menu* wm = new widget_menu(0, 0, 400, 40, texts::get(106));
  
	widget_list* wlg = new widget_list(0, 0, 400, 400);
   
	vector2i curr_res(sys().get_res_x(), sys().get_res_y());
	unsigned curr_entry = 0;
	unsigned i = 0;
	for (list<vector2i>::const_iterator it = available_resolutions.begin(); it != available_resolutions.end(); ++it) {
		wlg->append_entry(str(it->x) + "x" + str(it->y));
		if (*it == curr_res)
			curr_entry = i;
		++i;
	}
	wlg->set_selected(curr_entry);
  
	widget_button* wcb = new widget_caller_arg_button<widget, void (widget::*)(int), int>(&w, &widget::close, 0, 0, 0, 400, 40, texts::get(20));  
	w.add_child(new widget_func_arg_button<void (*)(widget_list*), widget_list*>(&apply_mode, wlg, 516, 604, 452, 40, texts::get(106)));  
  
	w.add_child(wm);
	w.add_child(wlg);
	w.add_child(wcb);
	wlg->align(0, 0);
	vector2i wlgp = wlg->get_pos();
	vector2i wlgs = wlg->get_size();
	wm->set_pos(vector2i(wlgp.x, wlgp.y - 60));
	wcb->set_pos(vector2i(wlgp.x - 260, wlgp.y + wlgs.y + 20));  
	w.run(0, false);	
}



void configure_key(widget_list* wkeys)
{
	struct confkey_widget : public widget {
		widget_text* keyname;
		unsigned keynr;
		void on_char(const SDL_keysym& ks) {
			if (ks.sym == SDLK_ESCAPE) {
				close(0);
				return;
			}
			bool ctrl = (ks.mod & (KMOD_LCTRL | KMOD_RCTRL)) != 0;
			bool alt = (ks.mod & (KMOD_LALT | KMOD_RALT | KMOD_MODE /* Alt Gr */)) != 0;
			bool shift = (ks.mod & (KMOD_LSHIFT | KMOD_RSHIFT)) != 0;
			cfg::instance().set_key(keynr, ks.sym, ctrl, alt, shift);
			keyname->set_text(cfg::instance().getkey(keynr).get_name());
			redraw();
		}
		confkey_widget(int x, int y, int w, int h, const string& text_, widget* parent_,
			       const std::string& backgrimg, unsigned sel) :
			widget(x, y, w, h, text_, parent_, backgrimg), keynr(sel)
			{
				keyname = new widget_text(40, 80, 432, 40, cfg::instance().getkey(keynr).get_name());
				add_child(keyname);
				add_child(new widget_text(40, 120, 432, 40, texts::get(217)));
			}
		~confkey_widget() {}
	};
	unsigned sel = wkeys->get_selected();
	confkey_widget w(256, 256, 512, 256, texts::get(216), 0, "", sel);
	string wks = wkeys->get_selected_entry();
	wks = wks.substr(0, wks.find("\t"));
	w.add_child(new widget_text(40, 40, 432, 32, wks));
	w.run(0, true);
	wkeys->set_entry(sel, texts::get(sel+600) + string("\t") + cfg::instance().getkey(sel).get_name());
}



void menu_configure_keys()
{
	widget w(0, 0, 1024, 768, texts::get(214), 0, "titlebackgr.jpg");
	widget_list* wkeys = new widget_list(40, 50, 944, 640);
	wkeys->set_column_width(700);
	w.add_child(wkeys);
	
	for (unsigned i = 600; i < 600 + NR_OF_KEY_IDS; ++i) {
		cfg::key k = cfg::instance().getkey(i-600);
		wkeys->append_entry(texts::get(i) + string("\t") + k.get_name());
	}

	// fixme: handle undefined keys!
	// fixme: check for double keys!

	w.add_child(new widget_caller_arg_button<widget, void (widget::*)(int), int>(&w, &widget::close, 0, 40, 708, 452, 40, texts::get(20)));
	w.add_child(new widget_func_arg_button<void (*)(widget_list*), widget_list*>(&configure_key, wkeys, 532, 708, 452, 40, texts::get(215)));
	w.run(0, false);
}



void menu_options()
{
	widget w(0, 0, 1024, 768, "", 0, "titlebackgr.jpg");
	widget_menu* wm = new widget_menu(0, 0, 400, 40, texts::get(29));
	w.add_child(wm);
	wm->add_entry(texts::get(214), new widget_func_button<void (*)()>(&menu_configure_keys, 0, 0, 0, 0));
	wm->add_entry(texts::get(106), new widget_func_button<void (*)()>(&menu_resolution, 0, 0, 0, 0));
	wm->add_entry(texts::get(11), new widget_caller_arg_button<widget, void (widget::*)(int), int>(&w, &widget::close, 0, 0, 0, 0, 0));
	wm->align(0, 0);
	w.run(0, false);
}


// vessel preview
class vessel_view
{
	list<string> shipnames;
	list<string>::iterator current;
	set<string> modellayouts;
	set<string>::iterator currentlayout;
	// note! this is not destructed by this class...
	widget_3dview* w3d;
	auto_ptr<model> load_model() {
		xml_doc doc(data_file().get_filename(*current));
		doc.load();
		string mdlname = doc.first_child().child("classification").attr("modelname");
		auto_ptr<model> mdl(new model(data_file().get_path(*current) + mdlname));
		// register and set default layout.
		mdl->register_layout();
		mdl->set_layout();
		modellayouts.clear();
		mdl->get_all_layout_names(modellayouts);
		currentlayout = modellayouts.begin();
		return mdl;
	}
public:
	vessel_view()
		: current(shipnames.end()), w3d(0)
	{
		color bgcol(50, 50, 150);
		shipnames = data_file().get_ship_list();
		list<string> tmp = data_file().get_submarine_list();
		shipnames.splice(shipnames.end(), tmp);
		tmp = data_file().get_airplane_list();
		shipnames.splice(shipnames.end(), tmp);
		current = shipnames.begin();
		w3d = new widget_3dview(20, 0, 1024-2*20, 700-32-16, load_model(), bgcol);
		vector3f lightdir = vector3f(angle(143).cos(), angle(143).sin(), angle(49.5).tan()).normal(); 
		w3d->set_light_dir(vector4f(lightdir.x, lightdir.y, lightdir.z, 0));
		w3d->set_light_color(color(233, 221, 171));
	}
	widget_3dview* get_w3d() { return w3d; }
	void next() {
		++current;
		if (current == shipnames.end())
			current = shipnames.begin();
		w3d->set_model(load_model());
		w3d->redraw();
	} 
	void previous() {
		if (current == shipnames.begin())
			current = shipnames.end();
		--current;
		w3d->set_model(load_model());
		w3d->redraw();
	}
	void switchlayout() {
		++currentlayout;
		if (currentlayout == modellayouts.end())
			currentlayout = modellayouts.begin();
		// registering the same layout multiple times does not hurt, no problem
		w3d->get_model()->register_layout(*currentlayout);
		w3d->get_model()->set_layout(*currentlayout);
		w3d->redraw();
	}
};

void menu_show_vessels()
{
	widget w(0, 0, 1024, 768, texts::get(24), 0, "threesubs.jpg");
	widget_menu* wm = new widget_menu(0, 700, 140, 32, ""/*texts::get(110)*/, true);
	w.add_child(wm);
	vessel_view vw;
	w.add_child(vw.get_w3d());

	wm->add_entry(texts::get(115), new widget_caller_button<vessel_view, void (vessel_view::*)()>(&vw, &vessel_view::next));
	wm->add_entry(texts::get(116), new widget_caller_button<vessel_view, void (vessel_view::*)()>(&vw, &vessel_view::previous));
	// fixme: disable butten when there is only one layout
	wm->add_entry(texts::get(246), new widget_caller_button<vessel_view, void (vessel_view::*)()>(&vw, &vessel_view::switchlayout));
	wm->add_entry(texts::get(117), new widget_caller_arg_button<widget, void (widget::*)(int), int>(&w, &widget::close, 0));
	wm->adjust_buttons(984);

	w.run(0, false);
}


bool file_exists(const string& fn)
{
	ifstream in(fn.c_str(), ios::in|ios::binary);
	return in.good();
}



bool set_dir(const string& dir, string& setdir)
{
	// check if it is a directory.
	if (!is_directory(dir)) {
		return false;
	}
	// append separator if needed
	if (dir[dir.length()-1] != '/') {
		setdir = dir + "/";
	} else {
		setdir = dir;
	}
	return true;
}



int mymain(list<string>& args)
{
	// report critical errors (on Unix/Posix systems)
	install_segfault_handler();

	string highscoredirectory =
#ifdef WIN32
	"./highscores/";
#else
	// fixme: use global /var/games instead
	string(getenv("HOME"))+"/.dangerdeep/";
#endif

	string configdirectory =
#ifdef WIN32
	"./config/";
#else
	string(getenv("HOME"))+"/.dangerdeep/";
#endif

	// command line argument parsing
	unsigned res_x = 0, res_y = 0;
	bool fullscreen = true;
	string cmdmissionfilename;
	bool runeditor = false;
	unsigned maxfps = 60;
	bool override_lang = false;
	bool use_sound = true;

	// parse commandline
	for (list<string>::iterator it = args.begin(); it != args.end(); ++it) {
		if (*it == "--help") {
			cout << "*** Danger from the Deep ***\nusage:\n--help\t\tshow this\n"
			     << "--language \tuse the listed language CODEs from the common.cvs file. \"en\" is the default language\n"
			     << "--res X*Y\tuse resolution X horizontal, Y vertical.\n\t\tDefault is 1024*768. If no Y value is given, Y=3/4*X is assumed.\n"
			     << "--nofullscreen\tdon't use fullscreen\n"
			     << "--debug\t\tdebug mode: no fullscreen, resolution 800\n"
			     << "--editor\trun mission editor directly\n"
			     << "--mission fn\trun mission from file fn (just the filename in the mission directory)\n"
			     << "--nosound\tdon't use sound\n"
			     << "--datadir path\tset base directory of data, must point to a directory with subdirs images/ textures/ objects/ and so on. Default on Unix e.g. /usr/local/share/dangerdeep.\n"
			     << "--savegamedir path\tdirectory for savegames, default path depends on platform\n"
			     << "--highscoredir path\tdirectory for highscores, default path depends on platform\n"
			     << "--configdir path\tdirectory for configuration data, default path depends on platform\n"
			     << "--maxfps x\tset maximum fps to x frames per second (default 60). Use x=0 to disable fps limit.\n";
			return 0;
		} else if (*it == "--nofullscreen") {
			fullscreen = false;
		} else if (*it == "--debug") {
			fullscreen = false;
			res_x = 800; res_y = 600;
		} else if (*it == "--mission") {
			list<string>::iterator it2 = it; ++it2;
			if (it2 != args.end()) {
				cmdmissionfilename = *it2;
				++it;
			}
		} else if (*it == "--editor") {
			runeditor = true;
		} else if (*it == "--nosound") {
			use_sound = false;
		} else if (*it == "--res") {
			list<string>::iterator it2 = it; ++it2;
			if (it2 != args.end()) {
				string::size_type st = it2->find("*");
				if (st == string::npos) {
					// no "*" found, use y=3/4*x
					res_x = atoi(it2->c_str());
					res_y = 3*res_x/4;
				} else {
					res_x = atoi(it2->substr(0, st).c_str());
					res_y = atoi(it2->substr(st+1).c_str());
				}
				++it;
			}
		} else if (*it == "--datadir") {
			list<string>::iterator it2 = it; ++it2;
			if (it2 != args.end()) {
				string datadir = *it2;
				// check if it is a directory.
				if (!is_directory(datadir)) {
					cout << "ERROR: data directory is no directory!\n";
					return -1;
				}
				// append separator if needed
				if (datadir[datadir.length()-1] != '/') {
					datadir += "/";
				}
				// check if there are valid files in data directory.
				bool datadirseemsok = true;
				if (!is_directory(datadir + "fonts")) {
					datadirseemsok = false;
				} else if (!is_directory(datadir + "images")) {
					datadirseemsok = false;
				} else if (!is_directory(datadir + "missions")) {
					datadirseemsok = false;
				} else if (!is_directory(datadir + "objects")) {
					datadirseemsok = false;
				} else if (!is_directory(datadir + "shaders")) {
					datadirseemsok = false;
				} else if (!is_directory(datadir + "sounds")) {
					datadirseemsok = false;
				} else if (!is_directory(datadir + "texts")) {
					datadirseemsok = false;
				} else if (!is_directory(datadir + "textures")) {
					datadirseemsok = false;
				}
				if (!datadirseemsok) {
					cout << "ERROR: data directory is missing crucial files!\n";
					return -1;
				}
				set_data_dir(datadir);
				cout << "data directory set to \"" << datadir << "\"\n";
				++it;
			}
		} else if (*it == "--savegamedir") {
			list<string>::iterator it2 = it; ++it2;
			if (it2 != args.end()) {
				if (!set_dir(*it2, savegamedirectory)) {
					cout << "ERROR: savegame directory is no directory!\n";
					return -1;
				}
				++it;
			}
		} else if (*it == "--highscoredir") {
			list<string>::iterator it2 = it; ++it2;
			if (it2 != args.end()) {
				if (!set_dir(*it2, highscoredirectory)) {
					cout << "ERROR: highscore directory is no directory!\n";
					return -1;
				}
				++it;
			}
		} else if (*it == "--configdir") {
			list<string>::iterator it2 = it; ++it2;
			if (it2 != args.end()) {
				if (!set_dir(*it2, configdirectory)) {
					cout << "ERROR: config directory is no directory!\n";
					return -1;
				}
				++it;
			}
		} else if (*it == "--language") { // included 2006/11/14 by doc1972
			list<string>::iterator it2 = it; ++it2;
			if (it2 != args.end()) {
				texts::set_language(*it2);
				override_lang = true;
				++it;
			}
		} else if (*it == "--maxfps") {
			list<string>::iterator it2 = it; ++it2;
			if (it2 != args.end()) {
				int mf = atoi(it2->c_str());
				if (mf >= 0) maxfps = unsigned(mf);
				++it;
			}
		} else {
			cout << "unknown parameter " << *it << ".\n";
		}
	}

	// parse configuration
	cfg& mycfg = cfg::instance();
	mycfg.register_option("screen_res_x", 1024);
	mycfg.register_option("screen_res_y", 768);
	mycfg.register_option("fullscreen", true);
	mycfg.register_option("debug", false);
	mycfg.register_option("sound", true);
	mycfg.register_option("use_shaders", true);
	mycfg.register_option("use_shaders_for_water", true);
	mycfg.register_option("water_detail", 128);
	mycfg.register_option("wave_fft_res", 128);
	mycfg.register_option("wave_phases", 256);
	mycfg.register_option("wavetile_length", 256.0f);
	mycfg.register_option("wave_tidecycle_time", 10.24f);
	mycfg.register_option("usex86sse", true);
	mycfg.register_option("language", 0);
	
	mycfg.register_key(key_names[KEY_ZOOM_MAP].name, SDLK_PLUS, 0, 0, 0);
	mycfg.register_key(key_names[KEY_UNZOOM_MAP].name, SDLK_MINUS, 0, 0, 0);
	mycfg.register_key(key_names[KEY_SHOW_GAUGES_SCREEN].name, SDLK_F1, 0, 0, 0);
	mycfg.register_key(key_names[KEY_SHOW_PERISCOPE_SCREEN].name, SDLK_F2, 0, 0, 0);
	mycfg.register_key(key_names[KEY_SHOW_UZO_SCREEN].name, SDLK_F3, 0, 0, 0);
	mycfg.register_key(key_names[KEY_SHOW_BRIDGE_SCREEN].name, SDLK_F4, 0, 0, 0);
	mycfg.register_key(key_names[KEY_SHOW_MAP_SCREEN].name, SDLK_F5, 0, 0, 0);
	mycfg.register_key(key_names[KEY_SHOW_TORPEDO_SCREEN].name, SDLK_F6, 0, 0, 0);
	mycfg.register_key(key_names[KEY_SHOW_DAMAGE_CONTROL_SCREEN].name, SDLK_F7, 0, 0, 0);
	mycfg.register_key(key_names[KEY_SHOW_LOGBOOK_SCREEN].name, SDLK_F8, 0, 0, 0);
	mycfg.register_key(key_names[KEY_SHOW_SUCCESS_RECORDS_SCREEN].name, SDLK_F9, 0, 0, 0);
	mycfg.register_key(key_names[KEY_SHOW_FREEVIEW_SCREEN].name, SDLK_F10, 0, 0, 0);
	mycfg.register_key(key_names[KEY_RUDDER_LEFT].name, SDLK_LEFT, 0, 0, 0);
	mycfg.register_key(key_names[KEY_RUDDER_HARD_LEFT].name, SDLK_LEFT, 0, 0, 1);
	mycfg.register_key(key_names[KEY_RUDDER_RIGHT].name, SDLK_RIGHT, 0, 0, 0);
	mycfg.register_key(key_names[KEY_RUDDER_HARD_RIGHT].name, SDLK_RIGHT, 0, 0, 1);
	mycfg.register_key(key_names[KEY_RUDDER_UP].name, SDLK_UP, 0, 0, 0);
	mycfg.register_key(key_names[KEY_RUDDER_HARD_UP].name, SDLK_UP, 0, 0, 1);
	mycfg.register_key(key_names[KEY_RUDDER_DOWN].name, SDLK_DOWN, 0, 0, 0);
	mycfg.register_key(key_names[KEY_RUDDER_HARD_DOWN].name, SDLK_DOWN, 0, 0, 1);
	mycfg.register_key(key_names[KEY_CENTER_RUDDERS].name, SDLK_RETURN, 0, 0, 0);
	mycfg.register_key(key_names[KEY_THROTTLE_LISTEN].name, SDLK_1, 0, 0, 0);
	mycfg.register_key(key_names[KEY_THROTTLE_SLOW].name, SDLK_2, 0, 0, 0);
	mycfg.register_key(key_names[KEY_THROTTLE_HALF].name, SDLK_3, 0, 0, 0);
	mycfg.register_key(key_names[KEY_THROTTLE_FULL].name, SDLK_4, 0, 0, 0);
	mycfg.register_key(key_names[KEY_THROTTLE_FLANK].name, SDLK_5, 0, 0, 0);
	mycfg.register_key(key_names[KEY_THROTTLE_STOP].name, SDLK_6, 0, 0, 0);
	mycfg.register_key(key_names[KEY_THROTTLE_REVERSE].name, SDLK_7, 0, 0, 0);
	mycfg.register_key(key_names[KEY_THROTTLE_REVERSEHALF].name, SDLK_8, 0, 0, 0);
	mycfg.register_key(key_names[KEY_THROTTLE_REVERSEFULL].name, SDLK_9, 0, 0, 0);
	mycfg.register_key(key_names[KEY_FIRE_TUBE_1].name, SDLK_1, 0, 0, 1);
	mycfg.register_key(key_names[KEY_FIRE_TUBE_2].name, SDLK_2, 0, 0, 1);
	mycfg.register_key(key_names[KEY_FIRE_TUBE_3].name, SDLK_3, 0, 0, 1);
	mycfg.register_key(key_names[KEY_FIRE_TUBE_4].name, SDLK_4, 0, 0, 1);
	mycfg.register_key(key_names[KEY_FIRE_TUBE_5].name, SDLK_5, 0, 0, 1);
	mycfg.register_key(key_names[KEY_FIRE_TUBE_6].name, SDLK_6, 0, 0, 1);
	mycfg.register_key(key_names[KEY_SELECT_TARGET].name, SDLK_SPACE, 0, 0, 0);
	mycfg.register_key(key_names[KEY_SCOPE_UP_DOWN].name, SDLK_0, 0, 0, 0);
	mycfg.register_key(key_names[KEY_CRASH_DIVE].name, SDLK_c, 0, 0, 0);
	mycfg.register_key(key_names[KEY_GO_TO_SNORKEL_DEPTH].name, SDLK_d, 0, 0, 0);
	mycfg.register_key(key_names[KEY_TOGGLE_SNORKEL].name, SDLK_f, 0, 0, 0);
	mycfg.register_key(key_names[KEY_SET_HEADING_TO_VIEW].name, SDLK_h, 0, 0, 0);
	mycfg.register_key(key_names[KEY_IDENTIFY_TARGET].name, SDLK_i, 0, 0, 0);
	mycfg.register_key(key_names[KEY_GO_TO_PERISCOPE_DEPTH].name, SDLK_p, 0, 0, 0);
	mycfg.register_key(key_names[KEY_GO_TO_SURFACE].name, SDLK_s, 0, 0, 0);
	mycfg.register_key(key_names[KEY_FIRE_TORPEDO].name, SDLK_t, 0, 0, 0);
	mycfg.register_key(key_names[KEY_SET_VIEW_TO_HEADING].name, SDLK_v, 0, 0, 0);
	mycfg.register_key(key_names[KEY_TOGGLE_ZOOM_OF_VIEW].name, SDLK_y, 0, 0, 0);
	mycfg.register_key(key_names[KEY_TURN_VIEW_LEFT].name, SDLK_COMMA, 0, 0, 0);
	mycfg.register_key(key_names[KEY_TURN_VIEW_LEFT_FAST].name, SDLK_COMMA, 0, 0, 1);
	mycfg.register_key(key_names[KEY_TURN_VIEW_RIGHT].name, SDLK_PERIOD, 0, 0, 0);
	mycfg.register_key(key_names[KEY_TURN_VIEW_RIGHT_FAST].name, SDLK_PERIOD, 0, 0, 1);
	mycfg.register_key(key_names[KEY_TIME_SCALE_UP].name, SDLK_KP_PLUS, 0, 0, 0);
	mycfg.register_key(key_names[KEY_TIME_SCALE_DOWN].name, SDLK_KP_MINUS, 0, 0, 0);
	mycfg.register_key(key_names[KEY_FIRE_DECK_GUN].name, SDLK_g, 0, 0, 0);
	mycfg.register_key(key_names[KEY_TOGGLE_RELATIVE_BEARING].name, SDLK_r, 0, 0, 0);
	mycfg.register_key(key_names[KEY_TOGGLE_MAN_DECK_GUN].name, SDLK_g, 0, 0, 1);
	mycfg.register_key(key_names[KEY_SHOW_TDC_SCREEN].name, SDLK_F11, 0, 0, 0);
	mycfg.register_key(key_names[KEY_TOGGLE_POPUP].name, SDLK_TAB, 0, 0, 0);
	mycfg.register_key(key_names[KEY_SHOW_TORPSETUP_SCREEN].name, SDLK_F12, 0, 0, 0);
	mycfg.register_key(key_names[KEY_SHOW_TORPEDO_CAMERA].name, SDLK_k, 0, 0, 0);

	//mycfg.register_option("invert_mouse", false);
	//mycfg.register_option("ocean_res_x", 128);
	//mycfg.register_option("ocean_res_y", 128);
	//mycfg.register_option("", );

	// randomize
	srand(time(0));

	// read data files
	data_file();

	// make sure the default values are stored if there is no config file,
	// and make sure all registered values are stored in it
	if (is_file(configdirectory + "config")) {
		mycfg.load(configdirectory + "config");
	} else {
		if (!is_directory(configdirectory)) {
			make_dir(configdirectory);
		}
		mycfg.save(configdirectory + "config");
	}


//	mycfg.save("./testconf");

	model::enable_shaders = cfg::instance().getb("use_shaders");

	// read screen resolution from config file if no override has been set by command line parameters
	if (res_x == 0) {
		res_x = cfg::instance().geti("screen_res_x");
		res_y = cfg::instance().geti("screen_res_y");
	}
	// Read language from options-file
	if (!override_lang)
		texts::set_language(cfg::instance().geti("language"));
	// fixme: also allow 1280x1024, set up gl viewport for 4:3 display
	// with black borders at top/bottom (height 2*32pixels)
	// weather conditions and earth curvature allow 30km sight at maximum.
	auto_ptr<class system> mysys(new class system(1.0, 30000.0+500.0, res_x, res_y, fullscreen));
	mysys->set_screenshot_directory(savegamedirectory);
	mysys->set_res_2d(1024, 768);
	mysys->set_max_fps(maxfps);
	
	mysys->add_console("$ffffffDanger $c0c0c0from the $ffffffDeep");
	mysys->add_console("$ffff00copyright and written 2003 by $ff0000Thorsten Jordan");
	mysys->add_console(string("$ff8000version ") + get_program_version());
	mysys->add_console("$80ff80*** welcome ***");

	GLfloat lambient[4] = {0.1,0.1,0.1,1};
	GLfloat ldiffuse[4] = {1,1,1,1};
	GLfloat lposition[4] = {0,0,1,0};
	glLightfv(GL_LIGHT0, GL_AMBIENT, lambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, ldiffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, lposition);
	glEnable(GL_LIGHT0);

	// create and start thread for music handling.
	thread::auto_ptr<music> mmusic(new music(use_sound));
	mmusic->start();

	reset_loading_screen();
	// init the global_data object before calling init_global_data
	auto_ptr<global_data> gbd(new global_data());
	init_global_data();
	widget::set_image_cache(&(imagecache()));

	mmusic->append_track("ImInTheMood.ogg");
	mmusic->append_track("Betty_Roche-Trouble_Trouble.ogg");
	mmusic->append_track("theme.ogg");
	mmusic->append_track("Auf_Feindfahrt_fast.ogg");
	mmusic->append_track("outside_underwater.ogg");
	mmusic->append_track("Auf_Feindfahrt_environmental.ogg");
	mmusic->append_track("Auf_Feindfahrt.ogg");
	add_loading_screen("Music list loaded");
	//mmusic->set_playback_mode(music::PBM_SHUFFLE_TRACK);
	mmusic->play();
	
	widget::set_theme(auto_ptr<widget::theme>(new widget::theme("widgetelements_menu.png", "widgeticons_menu.png",
									 font_olympiaworn,
									 color(182, 146, 137),
									 color(240, 217, 127) /*color(222, 208, 195)*/,
									 color(92, 72 ,68))));

	std::auto_ptr<texture> metalbackground(new texture(get_image_dir() + "metalbackground.jpg"));
	mysys->draw_console_with(font_arial, metalbackground.get());


	// try to make directories if they do not exist
	try {
		directory savegamedir(savegamedirectory);
	}
	catch (exception& e) {
		if (!make_dir(savegamedirectory))
			throw error("could not create save game directory.");
	}

	try {
		directory configdir(configdirectory);
	}
	catch (exception& e) {
		if (!make_dir(configdirectory))
			throw error("could not create config directory.");
	}

	try {
		directory highscoredir(highscoredirectory);
	}
	catch (exception& e) {
		if (!make_dir(highscoredirectory))
			throw error("could not create save game directory.");
	}

	// read highscores
	if (!file_exists(highscoredirectory + HSL_MISSION_NAME))
		highscorelist().save(highscoredirectory + HSL_MISSION_NAME);
	if (!file_exists(highscoredirectory + HSL_CAREER_NAME))
		highscorelist().save(highscoredirectory + HSL_CAREER_NAME);
	hsl_mission = highscorelist(highscoredirectory + HSL_MISSION_NAME);
	hsl_career = highscorelist(highscoredirectory + HSL_CAREER_NAME);



	// check if there was a mission given at the command line, or editor more etc.
	if (runeditor) {
		// reset loading screen here to show user we are doing something
		reset_loading_screen();
		run_game_editor(auto_ptr<game>(new game_editor(/*st*/)));
	} else if (cmdmissionfilename.length() > 0) {
		// fixme: check here that the file exists or tinyxml faults with a embarassing error message
		auto_ptr<game> gm;
		bool ok = true;
		try {
			gm.reset(new game(get_mission_dir() + cmdmissionfilename));
		}
		catch (error& e) {
			sys().add_console(string("error loading mission: ")+ e.what());
			// fixme: show dialogue!
			ok = false;
		}
		if (ok) {
			// reset loading screen here to show user we are doing something
			reset_loading_screen();
			run_game(gm);
		}
	} else {
		int retval = 1;
		widget w(0, 0, 1024, 768, "", 0, "titlebackgr.jpg");
		do {	// loop until menu is closed.
			// main menu
			w.remove_children();
			widget_menu* wm = new widget_menu(0, 0, 400, 40, texts::get(104));
			wm->set_entry_spacing(8);
			w.add_child(wm);
			wm->add_entry(texts::get(21), new widget_func_button<void (*)()>(&menu_single_mission, 0, 0, 0, 0));
			//wm->add_entry(texts::get(22), new widget_func_button<void (*)()>(&play_network_game, 0, 0, 0, 0));
			//wm->add_entry(texts::get(23), new widget_func_button<void (*)()>(&menu_notimplemented /* career menu */, 0, 0, 0, 0));
			wm->add_entry(texts::get(222), new widget_func_button<void (*)()>(&menu_mission_editor, 0, 0, 0, 0));
			wm->add_entry(texts::get(24), new widget_func_button<void (*)()>(&menu_show_vessels, 0, 0, 0, 0));
			wm->add_entry(texts::get(25), new widget_func_button<void (*)()>(&show_halloffame_mission, 0, 0, 0, 0));
			wm->add_entry(texts::get(213), new widget_func_button<void (*)()>(&show_credits, 0, 0, 0, 0));
			wm->add_entry(texts::get(26), new widget_caller_arg_button<widget, void (widget::*)(int), int>(&w, &widget::close, 1, 0, 0, 0, 0));
			wm->add_entry(texts::get(29), new widget_func_button<void (*)()>(&menu_options, 0, 0, 0, 0));

			wm->add_entry(texts::get(30), new widget_caller_arg_button<widget, void (widget::*)(int), int>(&w, &widget::close, 0, 0, 0, 0, 0));
			wm->align(0, 0);
			retval = w.run(0, false);
			if (retval == 1)
				menu_select_language();
		} while (retval != 0);
	}

	mmusic->stop(1000);
	mmusic.reset();

	mysys->write_console(true);

	hsl_mission.save(highscoredirectory + HSL_MISSION_NAME);
	hsl_career.save(highscoredirectory + HSL_CAREER_NAME);
	mycfg.save(configdirectory + "config");

	data_file_handler::release_instance();
	cfg::release_instance();
	widget::set_theme(auto_ptr<widget::theme>(0));	// clear allocated theme
	deinit_global_data();
	gbd.reset();

	return 0;
}

 	  	 
