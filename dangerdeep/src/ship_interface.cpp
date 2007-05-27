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

// user interface for controlling a ship
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL.h>


#include <sstream>
#include "ship_interface.h"
#include "system.h"
#include "game.h"
#include "texts.h"
#include "command.h"

ship_interface::ship_interface(ship* player_ship, game& gm) :
	user_interface( player_ship, gm )
{
}

ship_interface::~ship_interface()
{}

bool ship_interface::keyboard_common(int keycode, class game& gm)
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
		case SDLK_F11: if (time_scale < 100) { ++time_scale; add_message(texts::get(31)); } break;
		case SDLK_F12: if (time_scale > 1) { --time_scale; add_message(texts::get(32)); } break;

		// control
		case SDLK_LEFT: gm.send(new command_rudder_left(player)); add_message(texts::get(33)); break;
		case SDLK_RIGHT: gm.send(new command_rudder_right(player)); add_message(texts::get(34)); break;
		case SDLK_RETURN : gm.send(new command_rudder_midships(player)); add_message(texts::get(42)); break;
		case SDLK_1: gm.send(new command_set_throttle(player, sea_object::aheadslow)); add_message(texts::get(43)); break;
		case SDLK_2: gm.send(new command_set_throttle(player, sea_object::aheadhalf)); add_message(texts::get(44)); break;
		case SDLK_3: gm.send(new command_set_throttle(player, sea_object::aheadfull)); add_message(texts::get(45)); break;
		case SDLK_4: gm.send(new command_set_throttle(player, sea_object::aheadflank)); add_message(texts::get(46)); break;//flank/full change? fixme
		case SDLK_5: gm.send(new command_set_throttle(player, sea_object::stop)); add_message(texts::get(47)); break;
		case SDLK_6: gm.send(new command_set_throttle(player, sea_object::reverse)); add_message(texts::get(48)); break;

		// view
		case SDLK_COMMA : bearing -= angle(sys().key_shift() ? 10 : 1); break;
		case SDLK_PERIOD : bearing += angle(sys().key_shift() ? 10 : 1); break;

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
				add_message(texts::get(49));
			break;
#endif			
		}
		case SDLK_SPACE: target = gm.sub_in_direction_from_pos(player, player->get_heading()+bearing);
			if (!target.is_null()) add_message(texts::get(50));
			else add_message(texts::get(51));
			break;

		// quit, screenshot, pause etc.
		case SDLK_ESCAPE: gm.stop(); break;
		case SDLK_i: sys().screenshot(); sys().add_console("screenshot taken."); break;
		case SDLK_PAUSE: pause = !pause;
			if (pause) add_message(texts::get(52));
			else add_message(texts::get(53));
			break;
		default: return false;		
	}
	return true;
}

void ship_interface::draw_gauge(unsigned nr, int x, int y,
	unsigned wh, angle a, const string& text) const
{
	switch (nr) {
		case 1:	gauge1->draw(x, y, wh, wh); break;
		case 2:	gauge2->draw(x, y, wh, wh); break;
		case 3:	gauge3->draw(x, y, wh, wh); break;
		case 4:	gauge4->draw(x, y, wh, wh); break;
		default: return;
	}
	vector2 d = a.direction();
	int xx = x+wh/2, yy = y+wh/2;
	pair<unsigned, unsigned> twh = font_vtremington12->get_size(text);
	font_vtremington12->print(xx-twh.first/2, yy-twh.second/2, text);
	glColor3f(1,0,0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBegin(GL_TRIANGLES);
	glVertex2i(xx - int(d.y*4),yy - int(d.x*4));
	glVertex2i(xx + int(d.y*4),yy + int(d.x*4));
	glVertex2i(xx + int(d.x*wh*3/8),yy - int(d.y*wh*3/8));
	glEnd();
	glColor3f(1,1,1);
}

void ship_interface::draw_vessel_symbol(const vector2& offset, const sea_object* so, color c) const
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
void ship_interface::draw_torpedo(bool usebow, int x, int y,
	const ship::stored_torpedo& st)
{
	if (usebow) {
		if (st.status == 0) {	// empty
			sys().draw_image(x, y, 256, 32, torpempty);
		} else {
			sys().draw_image(x, y, 256, 32, torptex(st.type));
			if (st.status == 1) // reloading
				sys().draw_image(x, y, 256, 32, torpreload);
			else if (st.status == 2) // unloading
				sys().draw_image(x, y, 256, 32, torpunload);
		}
	} else {
		if (st.status == 0) {	// empty
			sys().draw_hm_image(x, y, 256, 32, torpempty);
		} else {
			sys().draw_hm_image(x, y, 256, 32, torptex(st.type));
			if (st.status == 1) // reloading
				sys().draw_hm_image(x, y, 256, 32, torpreload);
			else if (st.status == 2) // unloading
				sys().draw_hm_image(x, y, 256, 32, torpunload);
		}
	}
}
*/
	
void ship_interface::display(game& gm)
{
	switch (viewmode) {
		case 0: display_gauges(gm); break;
		case 1: display_sonar(gm); break;
		case 2: display_glasses(gm); break;
		case 3: display_bridge(gm); break;
		case 4: display_map(gm); break;
		case 5: display_dc_throwers(gm); break;
		case 6: display_damagestatus(gm); break;
		case 7: display_logbook(gm); break;
		case 8: display_successes(gm); break;
		default: display_freeview(gm); break;
	}
}

void ship_interface::display_sonar(game& gm)
{
	glClear(GL_DEPTH_BUFFER_BIT);

#if 0
	unsigned res_x = sys().get_res_x(), res_y = sys().get_res_y();

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	sys().gl_perspective_fovx (20.0, 1.0/1.0, 2.0, gm.get_max_view_distance());
	glViewport(res_x/2, res_y/3, res_x/2, res_x/2);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glRotatef(-90, 1,0,0);	// swap y and z coordinates (camera looks at +y axis)

	vector3 viewpos = player->get_pos() + vector3(0, 0, 12+3);//fixme: +3 to be above waves
	draw_view(gm, viewpos, res_x/2, res_y/3, res_x/2, res_x/2, 0,0, true, false);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glViewport(0, 0, res_x, res_y);
	glMatrixMode(GL_MODELVIEW);
	
	sys().prepare_2d_drawing();
	for (int x = 0; x < 3; ++x)
		sys().draw_image(x*256, 512, 256, 256, psbackgr);
	sys().draw_image(2*256, 0, 256, 256, periscope[0]);
	sys().draw_image(3*256, 0, 256, 256, periscope[1]);
	sys().draw_image(2*256, 256, 256, 256, periscope[2]);
	sys().draw_image(3*256, 256, 256, 256, periscope[3]);
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
	draw_gauge(1, 0, 0, 256, targetbearing, texts::get(12));
	draw_gauge(3, 256, 0, 256, targetrange, texts::get(13));
	draw_gauge(2, 0, 256, 256, targetspeed, texts::get(14));
	draw_gauge(1, 256, 256, 256, targetheading, texts::get(15));
	sys().draw_image(768, 512, 256, 256, addleadangle);
	const vector<ship::stored_torpedo>& torpedoes = player->get_torpedoes();
	pair<unsigned, unsigned> bow_tube_indices = player->get_bow_tube_indices();
	pair<unsigned, unsigned> stern_tube_indices = player->get_stern_tube_indices();
	for (unsigned i = bow_tube_indices.first; i < bow_tube_indices.second; ++i) {
		int j = i-bow_tube_indices.first;
		draw_torpedo(true, (j/4)*256, 512+(j%4)*32, torpedoes[i]);
	}
	for (unsigned i = stern_tube_indices.first; i < stern_tube_indices.second; ++i) {
		draw_torpedo(false, 512, 512+(i-stern_tube_indices.first)*32, torpedoes[i]);
	}
	glColor3f(1,1,1);
	draw_infopanel(sys);
	sys().unprepare_2d_drawing();
#endif	

	// keyboard processing
	int key = sys().get_key().sym;
	while (key != 0) {
		if (!keyboard_common(key, gm)) {
			// specific keyboard processing
		}
		key = sys().get_key().sym;
	}
}

void ship_interface::display_glasses(game& gm)
{
	ship* player = dynamic_cast<ship*> ( get_player() );

	//fixme ugly hack
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	unsigned res_x = sys().get_res_x(), res_y = sys().get_res_y();

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	sys().gl_perspective_fovx (30.0, 2.0/1.0, 2.0, gm.get_max_view_distance());
	glViewport(0, res_y/3, res_x, res_x/2);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glRotatef(-90, 1,0,0);	// swap y and z coordinates (camera looks at +y axis)

	vector3 viewpos = player->get_pos() + vector3(0, 0, 6);
	// no torpedoes, no DCs, no player
	draw_view(gm, viewpos, 0, res_y/3, res_x, res_x/2, player->get_heading()+bearing, 0, true, false, false);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glViewport(0, 0, res_x, res_y);
	glMatrixMode(GL_MODELVIEW);
	
	sys().prepare_2d_drawing();
	uzo->draw(0, 0, 512, 512);
	uzo->draw_hm(512, 0, 512, 512);
	draw_infopanel(gm);
	sys().unprepare_2d_drawing();

	// keyboard processing
	int key = sys().get_key().sym;
	while (key != 0) {
		if (!keyboard_common(key, gm)) {
			// specific keyboard processing
		}
		key = sys().get_key().sym;
	}
}

void ship_interface::display_dc_throwers(game& gm)
{
	sys().prepare_2d_drawing();
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
		draw_torpedo(true, 0, 256+i*32, torpedoes[i]);
	}
	for (unsigned i = bow_storage_indices.first; i < bow_storage_indices.second; ++i) {
		unsigned j = i - bow_storage_indices.first;
		draw_torpedo(true, (1+j/6)*256, 256+(j%6)*32, torpedoes[i]);
	}
	for (unsigned i = bow_top_storage_indices.first; i < bow_top_storage_indices.second; ++i) {
		unsigned j = i - bow_top_storage_indices.first;
		draw_torpedo(true, 0, j*32, torpedoes[i]);
	}
	for (unsigned i = stern_tube_indices.first; i < stern_tube_indices.second; ++i) {
		unsigned j = i - stern_tube_indices.first;
		draw_torpedo(false, 768, 256+j*32, torpedoes[i]);
	}
	for (unsigned i = stern_storage_indices.first; i < stern_storage_indices.second; ++i) {
		unsigned j = i - stern_storage_indices.first;
		draw_torpedo(false, 512, 256+j*32, torpedoes[i]);
	}
	for (unsigned i = stern_top_storage_indices.first; i < stern_top_storage_indices.second; ++i) {
		unsigned j = i - stern_top_storage_indices.first;
		draw_torpedo(false, 768, j*32, torpedoes[i]);
	}

#endif
	draw_infopanel(gm);
	sys().unprepare_2d_drawing();

	// mouse handling
	int mx, my; // mb = sys().get_mouse_buttons(); Unused variable
	sys().get_mouse_position(mx, my);

	// keyboard processing
	int key = sys().get_key().sym;
	while (key != 0) {
		if (!keyboard_common(key, gm)) {
			// specific keyboard processing
		}
		key = sys().get_key().sym;
	}
}

void ship_interface::display_damagestatus(game& gm)
{
	sys().prepare_2d_drawing();
	draw_infopanel(gm);
	sys().unprepare_2d_drawing();

	// mouse handling
	int mx, my; // mb = sys().get_mouse_buttons(); Unused variable
	sys().get_mouse_position(mx, my);

	// keyboard processing
	int key = sys().get_key().sym;
	while (key != 0) {
		if (!keyboard_common(key, gm)) {
			// specific keyboard processing
		}
		key = sys().get_key().sym;
	}
}
