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

void ship_interface::add_message(const string& s)
{
	panel_texts.push_back(s);
	if (panel_texts.size() > 4)	// (128-8)/24-1 ;-)
		panel_texts.pop_front();
}

ship_interface::ship_interface(ship* player_ship) : user_interface(),
	zoom_glasses(false), mapzoom(0.1), viewsideang(0), viewupang(-90),
	viewpos(0, 0, 10), bearing(0), viewmode(4),
	player(player_ship), target(0), last_trail_time(0), sunken_ship_tonnage(0)
{
}

ship_interface::~ship_interface()
{
}

bool ship_interface::keyboard_common(int keycode, class system& sys, class game& gm)
{
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
		case SDLK_SPACE: target = gm.sub_in_direction_from_pos(player->get_pos().xy(), player->get_heading()+bearing);
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
	
void ship_interface::draw_infopanel(class system& sys) const
{
	glBindTexture(GL_TEXTURE_2D, panelbackgr->get_opengl_name());
	glBegin(GL_QUADS);
	glTexCoord2i(0,0);
	glVertex2i(0,640);
	glTexCoord2i(0,1);
	glVertex2i(0,768);
	glTexCoord2i(8,1);
	glVertex2i(1024,768);
	glTexCoord2i(8,0);
	glVertex2i(1024,640);
	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);
	ostringstream os;
	os << TXT_Heading[language] << ": " << player->get_heading().ui_value()
		<< "   " << TXT_Speed[language] << ": "
		<< unsigned(fabs(round(sea_object::ms2kts(player->get_speed()))))
		<< "   " << TXT_Bearing[language] << ": "
		<< bearing.ui_value();
	font_panel->print(0, 648, os.str().c_str());
	int y = 768 - 24;
	for (list<string>::const_reverse_iterator it = panel_texts.rbegin(); it != panel_texts.rend(); ++it) {
		font_panel->print(0, y, it->c_str());
		y -= 24;	// font_panel's height is 24.
	}
}

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

void ship_interface::draw_trail(const vector2& pos, const list<vector2>& l, const vector2& offset)
{
	glColor4f(1,1,1,1);
	glBegin(GL_LINE_STRIP);
	vector2 p = (pos + offset)*mapzoom;
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
		case 6: display_damagecontrol(sys, gm); break;
		case 7: display_logbook(sys, gm); break;
		case 8: display_successes(sys, gm); break;
		default: display_freeview(sys, gm); break;
	}

	// trail recording
	bool record = (gm.get_time() > last_trail_time + TRAILTIME);
	if (record) {
		last_trail_time += TRAILTIME;
	}
	for (map<sea_object*, list<vector2> >::iterator it = trails.begin(); it != trails.end(); ) {
		map<sea_object*, list<vector2> >::iterator it2 = it; ++it;
		if (it2->first->is_alive() /* && it2->first->is_visible() fixme */ ) {
			if (record) {
				it2->second.push_front(it2->first->get_pos().xy());
				if (it2->second.size() > MAXPREVPOS)
			                it2->second.pop_back();
			}
		} else {
			trails.erase(it2);
		}
	}
}

void ship_interface::display_gauges(class system& sys, game& gm)
{
	sys.prepare_2d_drawing();
	for (int y = 0; y < 3; ++y)	// fixme: replace with gauges
		for (int x = 0; x < 4; ++x)
			sys.draw_image(x*256, y*256, 256, 256, psbackgr);
	angle player_speed = player->get_speed()*360.0/sea_object::kts2ms(36);
	angle player_depth = -player->get_pos().z;
	draw_gauge(sys, 1, 0, 0, 256, player->get_heading(), TXT_Heading[language]);
	draw_gauge(sys, 2, 256, 0, 256, player_speed, TXT_Speed[language]);

	draw_infopanel(sys);
	sys.unprepare_2d_drawing();

	// mouse handling
	int mx, my, mb = sys.get_mouse_buttons();
	sys.get_mouse_position(mx, my);

	if (mb & 1) {
		int marea = (my/256)*4+(mx/256);
		int mareax = (mx/256)*256+128;
		int mareay = (my/256)*256+128;
		angle mang(vector2(mx - mareax, mareay - my));
		switch (marea) {
			case 0:	// change heading
				player->head_to_ang(mang, mang.is_cw_nearer(
					player->get_heading()));
				break;
			case 1:	// change speed
				break;
		}
	}

	// keyboard processing
	int key = sys.get_key();
	while (key != 0) {
		if (!keyboard_common(key, sys, gm)) {
			// specific keyboard processing
		}
		key = sys.get_key();
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
	draw_infopanel(sys);
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

void ship_interface::display_bridge(class system& sys, game& gm)
{
	//fixme adapt
	glClear(GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	vector2 phd = player->get_heading().direction();
	vector3 viewpos = player->get_pos() + vector3(0, 0, 6) + phd.xy0();
	// no torpedoes, no DCs, with player
	draw_view(sys, gm, viewpos, player->get_heading()+bearing, true, false);

	sys.prepare_2d_drawing();
	draw_infopanel(sys);
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

void ship_interface::display_map(class system& sys, game& gm)
{
	glClearColor(0, 0, 1, 1);	// fixme
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	vector2 offset = -player->get_pos().xy();

	sys.prepare_2d_drawing();

	float delta = MAPGRIDSIZE*mapzoom;
	float sx = myfmod(512, delta)-myfmod(-offset.x, MAPGRIDSIZE)*mapzoom;
	float sy = 768.0 - (myfmod(384.0, delta)-myfmod(-offset.y, MAPGRIDSIZE)*mapzoom);
	int lx = int(1024/delta)+2, ly = int(768/delta)+2;

	// draw grid
	glColor3f(0.5, 0.5, 1);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBegin(GL_LINES);
	for (int i = 0; i < lx; ++i) {
		glVertex2f(sx, 0);
		glVertex2f(sx, 768);
		sx += delta;
	}
	for (int i = 0; i < ly; ++i) {
		glVertex2f(0, sy);
		glVertex2f(1024, sy);
		sy -= delta;
	}
	glEnd();
	glColor3f(1,1,1);

	// draw view range
	glColor3f(1,0,0);
	float range = gm.get_max_view_distance()*mapzoom;
	glBegin(GL_LINE_LOOP);
	for (int i = 0; i < range/4; ++i) {
		float a = i*8*M_PI/range;
		glVertex2f(512+sin(a)*range, 384-cos(a)*range);
	}
	glEnd();
	glColor3f(1,1,1);

	// draw pings
	const list<game::ping>& pings = gm.get_pings();
	for (list<game::ping>::const_iterator it = pings.begin(); it != pings.end(); ++it) {
		const game::ping& p = *it;
		vector2 p1 = (p.pos + offset)*mapzoom;
		vector2 p2 = p1 + (p.dir + angle(PINGANGLE/2)).direction() * PINGLENGTH * mapzoom;
		vector2 p3 = p1 + (p.dir - angle(PINGANGLE/2)).direction() * PINGLENGTH * mapzoom;
		glBegin(GL_TRIANGLES);
		glColor4f(0.5,0.5,0.5,1);
		glVertex2f(512+p1.x, 384-p1.y);
		glColor4f(0.5,0.5,0.5,0);
		glVertex2f(512+p2.x, 384-p2.y);
		glVertex2f(512+p3.x, 384-p3.y);
		glEnd();
		glColor4f(1,1,1,1);
	}
	
	// draw trails
	for (map<sea_object*, list<vector2> >::iterator it = trails.begin(); it != trails.end(); ++it) {
		if (it->second.size() > 0)
			draw_trail(it->first->get_pos().xy(), it->second, offset);
	}

	// draw vessel symbols
	list<ship*> ships = gm.visible_ships(player->get_pos());
	list<submarine*> submarines = gm.visible_submarines(player->get_pos());
	list<airplane*> airplanes = gm.visible_airplanes(player->get_pos());
	list<torpedo*> torpedoes = gm.visible_torpedoes(player->get_pos());
	bool record = (gm.get_time() > last_trail_time + TRAILTIME);
	for (list<ship*>::iterator it = ships.begin(); it != ships.end(); ++it) {
		draw_vessel_symbol(sys, offset, *it, color(192,255,192));
		if (record) trails.insert(make_pair(*it, list<vector2>()));
	}
	for (list<submarine*>::iterator it = submarines.begin(); it != submarines.end(); ++it) {
		draw_vessel_symbol(sys, offset, *it, color(255,255,128));
		if (record) trails.insert(make_pair(*it, list<vector2>()));
	}
	for (list<airplane*>::iterator it = airplanes.begin(); it != airplanes.end(); ++it) {
		draw_vessel_symbol(sys, offset, *it, color(0,0,64));
		if (record) trails.insert(make_pair(*it, list<vector2>()));
	}
	for (list<torpedo*>::iterator it = torpedoes.begin(); it != torpedoes.end(); ++it) {
		draw_vessel_symbol(sys, offset, *it, color(255,0,0));
		if (record) trails.insert(make_pair(*it, list<vector2>()));
	}
	
	draw_infopanel(sys);
	sys.unprepare_2d_drawing();
	
	// keyboard processing
	int key = sys.get_key();
	while (key != 0) {
		if (!keyboard_common(key, sys, gm)) {
			// specific keyboard processing
			switch(key) {
				case SDLK_PLUS : if (mapzoom < 1) mapzoom *= 1.5; break;
				case SDLK_MINUS : if (mapzoom > 0.01) mapzoom /= 1.5; break;
			}
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
	draw_infopanel(sys);
	sys.unprepare_2d_drawing();

	// mouse handling
	int mx, my, mb = sys.get_mouse_buttons();
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

void ship_interface::display_damagecontrol(class system& sys, game& gm)
{
	glClearColor(0.25, 0.25, 0.25, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	sys.prepare_2d_drawing();
	font_arial2->print(0, 0, "damage control - fixme");
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

void ship_interface::display_logbook(class system& sys, game& gm)
{
	glClearColor(0.25, 0.25, 0.25, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	sys.prepare_2d_drawing();
	font_arial2->print(0, 0, "logbook - fixme");
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

void ship_interface::display_successes(class system& sys, game& gm)
{
	glClearColor(0.25, 0.25, 0.25, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	sys.prepare_2d_drawing();
	font_arial2->print(0, 0, "success records - fixme");
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

void ship_interface::display_freeview(class system& sys, game& gm)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glRotatef(viewupang,1,0,0);
	glRotatef(viewsideang,0,1,0);
	float viewmatrix[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, viewmatrix);
	vector3 sidestep(viewmatrix[0], viewmatrix[4], viewmatrix[8]);
	vector3 upward(viewmatrix[1], viewmatrix[5], viewmatrix[9]);
	vector3 forward(viewmatrix[2], viewmatrix[6], viewmatrix[10]);
	glTranslatef(-viewpos.x, -viewpos.y, -viewpos.z);

	// draw everything
	draw_view(sys, gm, viewpos, player->get_heading()+bearing, true, true);

	int mx, my;
	sys.get_mouse_motion(mx, my);
	viewsideang += mx*0.5;
	viewupang -= my*0.5;

	sys.prepare_2d_drawing();
	draw_infopanel(sys);
	sys.unprepare_2d_drawing();

	// keyboard processing
	int key = sys.get_key();
	while (key != 0) {
		if (!keyboard_common(key, sys, gm)) {
			// specific keyboard processing
			switch(key) {
				case SDLK_w: viewpos -= forward * 5; break;
				case SDLK_x: viewpos += forward * 5; break;
				case SDLK_a: viewpos -= sidestep * 5; break;
				case SDLK_d: viewpos += sidestep * 5; break;
				case SDLK_q: viewpos -= upward * 5; break;
				case SDLK_e: viewpos += upward * 5; break;
			}
		}
		key = sys.get_key();
	}
}
