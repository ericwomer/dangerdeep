// user interface for controlling a ship
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL/SDL.h>
#include <sstream>
#include "ship_interface.h"
#include "system.h"
#include "game.h"
#include "texts.h"

ship_interface::ship_interface(ship* player_ship) :
	user_interface( player_ship )
{
}

ship_interface::~ship_interface()
{}

bool ship_interface::keyboard_common(int keycode, class system& sys, class game& gm)
{
	ship* player = dynamic_cast<ship*> ( get_player() );
    
	// handle common keys (fixme: make configureable?)
	switch (keycode) {
		// viewmode switching
		case SDLK_F1: viewmode = 0; break;
		case SDLK_F2: viewmode = 1; break;
		case SDLK_F3: viewmode = 2; break;
		case SDLK_F4: viewmode = 3; break;
		case SDLK_F5: viewmode = 4; break;
		case SDLK_F6: viewmode = 5; break;
		case SDLK_F7: viewmode = 6; break;
		case SDLK_F8: viewmode = 7; break;
		case SDLK_F9: viewmode = 8; break;
		case SDLK_F10: viewmode = 9; break;

		// time scaling fixme: too simple
		case SDLK_F11: if (time_scale < 100) { ++time_scale; add_message(TXT_Timescaleup[language]); } break;
		case SDLK_F12: if (time_scale > 1) { --time_scale; add_message(TXT_Timescaledown[language]); } break;

		// control
		case SDLK_LEFT: player->rudder_left(); add_message(TXT_Rudderleft[language]); break;
		case SDLK_RIGHT: player->rudder_right(); add_message(TXT_Rudderright[language]); break;
		case SDLK_RETURN : player->rudder_midships(); add_message(TXT_Ruddermidships[language]); break;
		case SDLK_1: player->set_throttle(sea_object::aheadslow); add_message(TXT_Aheadslow[language]); break;
		case SDLK_2: player->set_throttle(sea_object::aheadhalf); add_message(TXT_Aheadhalf[language]); break;
		case SDLK_3: player->set_throttle(sea_object::aheadfull); add_message(TXT_Aheadfull[language]); break;
		case SDLK_4: player->set_throttle(sea_object::aheadflank); add_message(TXT_Aheadflank[language]); break;//flank/full change?
		case SDLK_5: player->set_throttle(sea_object::stop); add_message(TXT_Enginestop[language]); break;
		case SDLK_6: player->set_throttle(sea_object::reverse); add_message(TXT_Enginereverse[language]); break;

		// view
		case SDLK_COMMA : bearing -= angle(sys.key_shift() ? 10 : 1); break;
		case SDLK_PERIOD : bearing += angle(sys.key_shift() ? 10 : 1); break;

		// weapons, fixme
		case SDLK_t: {
#if 0
			bool bow = true;
			if (target != 0) {
				angle a(target->get_pos().xy() - player->get_pos().xy());
				if (a.ui_abs_value180() > 90)
					bow = false;
			}
			if (player->fire_torpedo(gm, bow, -1/*fixme*/, target))
				add_message(TXT_Torpedofired[language]);
			break;
#endif			
		}
		case SDLK_SPACE: target = gm.sub_in_direction_from_pos(player, player->get_heading()+bearing);
			if (target) add_message(TXT_Newtargetselected[language]);
			else add_message(TXT_Notargetindirection[language]);
			break;

		// quit, screenshot, pause etc.
		case SDLK_ESCAPE: quit = true; break;
		case SDLK_i: sys.screenshot(); sys.add_console("screenshot taken."); break;
		case SDLK_PAUSE: pause = !pause;
			if (pause) add_message(TXT_Gamepaused[language]);
			else add_message(TXT_Gameunpaused[language]);
			break;
		default: return false;		
	}
	return true;
}

/*
texture* ship_interface::torptex(unsigned type)
{
	switch (type) {
		case torpedo::T1: return torpt1;
		case torpedo::T3: return torpt3;
		case torpedo::T5: return torpt5;
		case torpedo::T3FAT: return torpt3fat;
		case torpedo::T6LUT: return torpt6lut;
		case torpedo::T11: return torpt11;
	}
	return torpempty;
}
*/
	
void ship_interface::draw_gauge(class system& sys, unsigned nr, int x, int y,
	unsigned wh, angle a, const char* text) const
{
	switch (nr) {
		case 1:	sys.draw_image(x, y, wh, wh, gauge1); break;
		case 2:	sys.draw_image(x, y, wh, wh, gauge2); break;
		case 3:	sys.draw_image(x, y, wh, wh, gauge3); break;
		case 4:	sys.draw_image(x, y, wh, wh, gauge4); break;
		default: return;
	}
	vector2 d = a.direction();
	int xx = x+wh/2, yy = y+wh/2;
	pair<unsigned, unsigned> twh = font_arial2->get_size(text);
	font_arial2->print(xx-twh.first/2, yy-twh.second/2, text);
	glColor3f(1,0,0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBegin(GL_TRIANGLES);
	glVertex2i(xx - int(d.y*4),yy - int(d.x*4));
	glVertex2i(xx + int(d.y*4),yy + int(d.x*4));
	glVertex2i(xx + int(d.x*wh*3/8),yy - int(d.y*wh*3/8));
	glEnd();
	glColor3f(1,1,1);
}

void ship_interface::draw_vessel_symbol(class system& sys,
	const vector2& offset, const sea_object* so, color c) const
{
	vector2 d = so->get_heading().direction();
	float w = so->get_width()*mapzoom/2, l = so->get_length()*mapzoom/2;
	vector2 p = (so->get_pos().xy() + offset) * mapzoom;
	p.x += 512;
	p.y = 384 - p.y;
	c.set_gl_color();
	glBindTexture(GL_TEXTURE_2D, 0);
	glBegin(GL_QUADS);
	glVertex2f(p.x - d.y*w, p.y - d.x*w);
	glVertex2f(p.x - d.x*l, p.y + d.y*l);
	glVertex2f(p.x + d.y*w, p.y + d.x*w);
	glVertex2f(p.x + d.x*l, p.y - d.y*l);
	glEnd();
	glBegin(GL_LINES);
	glVertex2f(p.x - d.x*l, p.y + d.y*l);
	glVertex2f(p.x + d.x*l, p.y - d.y*l);
	glEnd();
	glColor3f(1,1,1);
}

void ship_interface::draw_trail(sea_object* so, const vector2& offset)
{
	list<vector2> l = so->get_previous_positions();
	glColor4f(1,1,1,1);
	glBegin(GL_LINE_STRIP);
	vector2 p = (so->get_pos().xy() + offset)*mapzoom;
	glVertex2f(512+p.x, 384-p.y);
	float la = 1.0/float(l.size()), lc = 0;
	for (list<vector2>::const_iterator it = l.begin(); it != l.end(); ++it) {
		glColor4f(1,1,1,1-lc);
		vector2 p = (*it + offset)*mapzoom;
		glVertex2f(512+p.x, 384-p.y);
		lc += la;
	}
	glEnd();
	glColor4f(1,1,1,1);
}

/*
void ship_interface::draw_torpedo(class system& sys, bool usebow, int x, int y,
	const ship::stored_torpedo& st)
{
	if (usebow) {
		if (st.status == 0) {	// empty
			sys.draw_image(x, y, 256, 32, torpempty);
		} else {
			sys.draw_image(x, y, 256, 32, torptex(st.type));
			if (st.status == 1) // reloading
				sys.draw_image(x, y, 256, 32, torpreload);
			else if (st.status == 2) // unloading
				sys.draw_image(x, y, 256, 32, torpunload);
		}
	} else {
		if (st.status == 0) {	// empty
			sys.draw_hm_image(x, y, 256, 32, torpempty);
		} else {
			sys.draw_hm_image(x, y, 256, 32, torptex(st.type));
			if (st.status == 1) // reloading
				sys.draw_hm_image(x, y, 256, 32, torpreload);
			else if (st.status == 2) // unloading
				sys.draw_hm_image(x, y, 256, 32, torpunload);
		}
	}
}
*/
	
void ship_interface::display(class system& sys, game& gm)
{
	if (target != 0 && target->is_dead()) target = 0;

	switch (viewmode) {
		case 0: display_gauges(sys, gm); break;
		case 1: display_sonar(sys, gm); break;
		case 2: display_glasses(sys, gm); break;
		case 3: display_bridge(sys, gm); break;
		case 4: display_map(sys, gm); break;
		case 5: display_dc_throwers(sys, gm); break;
		case 6: display_damagestatus(sys, gm); break;
		case 7: display_logbook(sys, gm); break;
		case 8: display_successes(sys, gm); break;
		default: display_freeview(sys, gm); break;
	}
}

void ship_interface::display_sonar(class system& sys, game& gm)
{
	glClear(GL_DEPTH_BUFFER_BIT);

#if 0
	unsigned res_x = sys.get_res_x(), res_y = sys.get_res_y();

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluPerspective (20.0, 1.0/1.0, 2.0, gm.get_max_view_distance());
	glViewport(res_x/2, res_y/3, res_x/2, res_x/2);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	vector3 viewpos = player->get_pos() + vector3(0, 0, 12+3);//fixme: +3 to be above waves
	draw_view(sys, gm, viewpos, false);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glViewport(0, 0, res_x, res_y);
	glMatrixMode(GL_MODELVIEW);
	
	sys.prepare_2d_drawing();
	for (int x = 0; x < 3; ++x)
		sys.draw_image(x*256, 512, 256, 256, psbackgr);
	sys.draw_image(2*256, 0, 256, 256, periscope[0]);
	sys.draw_image(3*256, 0, 256, 256, periscope[1]);
	sys.draw_image(2*256, 256, 256, 256, periscope[2]);
	sys.draw_image(3*256, 256, 256, 256, periscope[3]);
	angle targetbearing;
	angle targetaob;
	angle targetrange;
	angle targetspeed;
	angle targetheading;
	if (target) {
		pair<angle, double> br = player->bearing_and_range_to(target);
		targetbearing = br.first;
		targetaob = player->estimate_angle_on_the_bow(br.first, target->get_heading());
		unsigned r = unsigned(round(br.second));
		if (r > 9999) r = 9999;
		targetrange = r*360.0/9000.0;
		targetspeed = target->get_speed()*360.0/sea_object::kts2ms(36);
		targetheading = target->get_heading();
	}
	draw_gauge(sys, 1, 0, 0, 256, targetbearing, TXT_Targetbearing[language]);
	draw_gauge(sys, 3, 256, 0, 256, targetrange, TXT_Targetrange[language]);
	draw_gauge(sys, 2, 0, 256, 256, targetspeed, TXT_Targetspeed[language]);
	draw_gauge(sys, 1, 256, 256, 256, targetheading, TXT_Targetcourse[language]);
	sys.draw_image(768, 512, 256, 256, addleadangle);
	const vector<ship::stored_torpedo>& torpedoes = player->get_torpedoes();
	pair<unsigned, unsigned> bow_tube_indices = player->get_bow_tube_indices();
	pair<unsigned, unsigned> stern_tube_indices = player->get_stern_tube_indices();
	for (unsigned i = bow_tube_indices.first; i < bow_tube_indices.second; ++i) {
		int j = i-bow_tube_indices.first;
		draw_torpedo(sys, true, (j/4)*256, 512+(j%4)*32, torpedoes[i]);
	}
	for (unsigned i = stern_tube_indices.first; i < stern_tube_indices.second; ++i) {
		draw_torpedo(sys, false, 512, 512+(i-stern_tube_indices.first)*32, torpedoes[i]);
	}
	glColor3f(1,1,1);
	draw_infopanel(sys);
	sys.unprepare_2d_drawing();
#endif	

	// keyboard processing
	int key = sys.get_key();
	while (key != 0) {
		if (!keyboard_common(key, sys, gm)) {
			// specific keyboard processing
		}
		key = sys.get_key();
	}
}

void ship_interface::display_glasses(class system& sys, game& gm)
{
	ship* player = dynamic_cast<ship*> ( get_player() );

	//fixme ugly hack
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	unsigned res_x = sys.get_res_x(), res_y = sys.get_res_y();

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluPerspective (30.0, 2.0/1.0, 2.0, gm.get_max_view_distance());
	glViewport(0, res_y/3, res_x, res_x/2);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	vector3 viewpos = player->get_pos() + vector3(0, 0, 6);
	// no torpedoes, no DCs, no player
	draw_view(sys, gm, viewpos, player->get_heading()+bearing, false, false);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glViewport(0, 0, res_x, res_y);
	glMatrixMode(GL_MODELVIEW);
	
	sys.prepare_2d_drawing();
	sys.draw_image(0, 0, 512, 512, uzo);
	sys.draw_hm_image(512, 0, 512, 512, uzo);
	draw_infopanel(sys, gm);
	sys.unprepare_2d_drawing();

	// keyboard processing
	int key = sys.get_key();
	while (key != 0) {
		if (!keyboard_common(key, sys, gm)) {
			// specific keyboard processing
		}
		key = sys.get_key();
	}
}

void ship_interface::display_dc_throwers(class system& sys, game& gm)
{
	sys.prepare_2d_drawing();
	// fixme adapt
#if 0	
	glBindTexture(GL_TEXTURE_2D, background->get_opengl_name());
	glBegin(GL_QUADS);
	glTexCoord2i(0,0);
	glVertex2i(0,0);
	glTexCoord2i(0,6);
	glVertex2i(0,768);
	glTexCoord2i(8,6);
	glVertex2i(1024,768);
	glTexCoord2i(8,0);
	glVertex2i(1024,0);
	glEnd();
	glClear(GL_DEPTH_BUFFER_BIT);
	
	glPushMatrix();
	glTranslatef(512, 192, 1);
	glScalef(1024/80.0, 1024/80.0, 0.001);
	glRotatef(90, 0, 0, 1);
	glRotatef(-90, 0, 1, 0);
	player->display();
	glPopMatrix();
	
	// draw tubes
	const vector<ship::stored_torpedo>& torpedoes = player->get_torpedoes();
	pair<unsigned, unsigned> bow_tube_indices = player->get_bow_tube_indices();
	pair<unsigned, unsigned> stern_tube_indices = player->get_stern_tube_indices();
	pair<unsigned, unsigned> bow_storage_indices = player->get_bow_storage_indices();
	pair<unsigned, unsigned> stern_storage_indices = player->get_stern_storage_indices();
	pair<unsigned, unsigned> bow_top_storage_indices = player->get_bow_top_storage_indices();
	pair<unsigned, unsigned> stern_top_storage_indices = player->get_stern_top_storage_indices();
	for (unsigned i = bow_tube_indices.first; i < bow_tube_indices.second; ++i) {
		draw_torpedo(sys, true, 0, 256+i*32, torpedoes[i]);
	}
	for (unsigned i = bow_storage_indices.first; i < bow_storage_indices.second; ++i) {
		unsigned j = i - bow_storage_indices.first;
		draw_torpedo(sys, true, (1+j/6)*256, 256+(j%6)*32, torpedoes[i]);
	}
	for (unsigned i = bow_top_storage_indices.first; i < bow_top_storage_indices.second; ++i) {
		unsigned j = i - bow_top_storage_indices.first;
		draw_torpedo(sys, true, 0, j*32, torpedoes[i]);
	}
	for (unsigned i = stern_tube_indices.first; i < stern_tube_indices.second; ++i) {
		unsigned j = i - stern_tube_indices.first;
		draw_torpedo(sys, false, 768, 256+j*32, torpedoes[i]);
	}
	for (unsigned i = stern_storage_indices.first; i < stern_storage_indices.second; ++i) {
		unsigned j = i - stern_storage_indices.first;
		draw_torpedo(sys, false, 512, 256+j*32, torpedoes[i]);
	}
	for (unsigned i = stern_top_storage_indices.first; i < stern_top_storage_indices.second; ++i) {
		unsigned j = i - stern_top_storage_indices.first;
		draw_torpedo(sys, false, 768, j*32, torpedoes[i]);
	}

#endif
	draw_infopanel(sys, gm);
	sys.unprepare_2d_drawing();

	// mouse handling
	int mx, my; // mb = sys.get_mouse_buttons(); Unused variable
	sys.get_mouse_position(mx, my);

	// keyboard processing
	int key = sys.get_key();
	while (key != 0) {
		if (!keyboard_common(key, sys, gm)) {
			// specific keyboard processing
		}
		key = sys.get_key();
	}
}

void ship_interface::display_damagestatus(class system& sys, game& gm)
{
	sys.prepare_2d_drawing();
	draw_infopanel(sys, gm);
	sys.unprepare_2d_drawing();

	// mouse handling
	int mx, my; // mb = sys.get_mouse_buttons(); Unused variable
	sys.get_mouse_position(mx, my);

	// keyboard processing
	int key = sys.get_key();
	while (key != 0) {
		if (!keyboard_common(key, sys, gm)) {
			// specific keyboard processing
		}
		key = sys.get_key();
	}
}
