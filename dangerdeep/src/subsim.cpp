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
#include <sstream>
#include "config.h"

class system* sys;
int res_x, res_y;

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
		sys->draw_image(0, 0, res_x/2, res_x/2, titel[0]);
		sys->draw_image(res_x/2, 0, res_x/2, res_x/2, titel[1]);
		sys->draw_image(0, res_x/2, res_x/2, res_x/4, titel[2]);
		sys->draw_image(res_x/2, res_x/2, res_x/2, res_x/4, titel[3]);

		sys->poll_event_queue();
		int key = sys->get_key();
		font_logo->print_hc(res_x, res_y/5, "Danger from the Deep", color(255,255,255), true);
		m.draw(res_x, res_y, font_tahoma);
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
				game* test = new game(playersub);
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
		sys->draw_image(0, 0, res_x/2, res_x/2, titel[0]);
		sys->draw_image(res_x/2, 0, res_x/2, res_x/2, titel[1]);
		sys->draw_image(0, res_x/2, res_x/2, res_x/4, titel[2]);
		sys->draw_image(res_x/2, res_x/2, res_x/2, res_x/4, titel[3]);

		sys->poll_event_queue();
		int key = sys->get_key();
		font_logo->print_hc(res_x, res_y/5, "Danger from the Deep", color(255,255,255), true);
		m.draw(res_x, res_y, font_tahoma);
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

void show_vessels(void)
{
	int vessel = 0;
	double zangle = 0;
	double xangle = 0;
	unsigned lasttime = sys->millisec();
	while (true) {
		if (vessel < 0) vessel = 7;
		if (vessel > 7) vessel = 0;
		model* m = 0;
		const char* mname = 0;
		switch(vessel) {
			case 0: m = merchant_medium; mname = "Medium merchant ship"; break;
 			case 1: m = subVII; mname = "Submarine type VII"; break;
 			case 2: m = subXXI; mname = "Submarine type XXI"; break;
 			case 3: m = destroyer_tribal; mname = "Detroyer Tribal class"; break;
 			case 4: m = troopship_medium; mname = "Medium Troopship"; break;
			case 5: m = battleship_malaya; mname = "Battleship Malaya class"; break;
 			case 6: m = torpedo_g7; mname = "German type G7 torpedo"; break;
 			case 7: m = depth_charge_mdl; mname = "Allied depth charge"; break;
 		}

		glClear(GL_DEPTH_BUFFER_BIT);
		sys->prepare_2d_drawing();
		sys->draw_image(0, 0, res_x/2, res_x/2, titel[0]);
		sys->draw_image(res_x/2, 0, res_x/2, res_x/2, titel[1]);
		sys->draw_image(0, res_x/2, res_x/2, res_x/4, titel[2]);
		sys->draw_image(res_x/2, res_x/2, res_x/2, res_x/4, titel[3]);
		font_tahoma->print_hc(res_x, res_y*4/5, mname);
		font_tahoma->print_hc(res_x, res_y - font_tahoma->get_height(),
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
	res_x = 1024, res_y = 768;	// fixme
	// weather conditions and earth curvature allow 30km sight at maximum.
	sys = new class system(1.0, 30000.0, res_x, true);
	
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
			sys->draw_image(0, 0, res_x/2, res_x/2, titel[0]);
			sys->draw_image(res_x/2, 0, res_x/2, res_x/2, titel[1]);
			sys->draw_image(0, res_x/2, res_x/2, res_x/4, titel[2]);
			sys->draw_image(res_x/2, res_x/2, res_x/2, res_x/4, titel[3]);

			sys->poll_event_queue();
			int key = sys->get_key();
			font_logo->print_hc(res_x, res_y/5, "Danger from the Deep", color(255,255,255), true);
			m.draw(res_x, res_y, font_tahoma);
			int mmsel = m.input(key, 0, 0, 0);
			sys->unprepare_2d_drawing();
			sys->swap_buffers();
			if (mmsel == 7) { quitgame = true; break; }
			selected = mmsel & 0xffff;
			switch (selected) {
				case 0:	menu_single_mission(); break;
				case 1: break;//fixme!
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
