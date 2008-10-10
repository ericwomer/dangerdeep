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

// user display: general map view
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "system.h"
#include "image.h"
#include "texture.h"
#include "game.h"
#include "game_editor.h"
#include "coastmap.h"
#include "map_display.h"
#include "user_interface.h"
#include "ship.h"
#include "submarine.h"
#include "torpedo.h"
#include "airplane.h"
#include "texts.h"
#include "texture.h"
#include "font.h"
#include "convoy.h"
#include "keys.h"
#include "cfg.h"
#include "xml.h"
#include "filehelper.h"
#include "global_data.h"
#ifdef CVEDIT
#include "color.h"
#include "bspline.h"
#endif
#include <sstream>

using std::pair;
using std::make_pair;
using std::vector;
using std::list;
using std::string;
using std::ostringstream;
using std::auto_ptr;

#define MAPGRIDSIZE 1000	// meters

enum edit_panel_fg_result {
	EPFG_CANCEL,
	EPFG_SHIPADDED,
	EPFG_CHANGEMOTION,
	EPFG_CHANGETIME,
	EPFG_ADDSELTOCV,
	EPFG_MAKENEWCV,
	EPFG_DELETECV,
	EPFG_EDITROUTECV
};


void map_display::draw_vessel_symbol(const vector2& offset, sea_object* so, color c) const
{
//	cout << "draw " << so << " hdg " << so->get_heading().value() << " ort " << so->get_orientation() << " orang " <<
//		angle(so->get_orientation().rotate(0,1,0).xy()).value() << "\n";
	vector2 d = so->get_heading().direction();
	float w = so->get_width()*mapzoom/2, l = so->get_length()*mapzoom/2;
	vector2 p = (so->get_pos().xy() + offset) * mapzoom;
	p.x += 512;
	p.y = 384 - p.y;

	c.set_gl_color();
	glBindTexture(GL_TEXTURE_2D, 0);
	primitive<4> vesselshape(GL_QUADS);
	vesselshape.vertices[0] = vector3f(p.x - d.y*w, p.y - d.x*w, 0);
	vesselshape.vertices[1] = vector3f(p.x - d.x*l, p.y + d.y*l, 0);
	vesselshape.vertices[2] = vector3f(p.x + d.y*w, p.y + d.x*w, 0);
	vesselshape.vertices[3] = vector3f(p.x + d.x*l, p.y - d.y*l, 0);
	vesselshape.render();
	primitives::line(vector2f(p.x - d.x*l, p.y + d.y*l), vector2f(p.x + d.x*l, p.y - d.y*l)).render();
	glColor3f(1,1,1);
}



void map_display::draw_trail(sea_object* so, const vector2& offset) const
{
	// fixme: clean up this mess. maybe merge with function in water.cpp
	// we draw trails in both functions.
	ship* shp = dynamic_cast<ship*>(so);
	if (shp) {
		const list<ship::prev_pos>& l = shp->get_previous_positions();
		if (l.empty()) return;
		glColor4f(1,1,1,1);
		glBindTexture(GL_TEXTURE_2D, 0);
		vector2 p = (shp->get_pos().xy() + offset)*mapzoom;
		primitives tr(GL_LINE_STRIP, false, false, l.size() + 1);
		tr.vertices[0].x = 512+p.x;
		tr.vertices[0].y = 384-p.y;
		float la = 1.0/float(l.size()), lc = 0;
		unsigned trc = 1;
		for (list<ship::prev_pos>::const_iterator it = l.begin(); it != l.end(); ++it) {
			glColor4f(1,1,1,1-lc);
			vector2 p = (it->pos + offset)*mapzoom;
			tr.vertices[trc].x = 512+p.x;
			tr.vertices[trc].y = 384-p.y;
			lc += la;
			++trc;
		}
		tr.render();
		glColor4f(1,1,1,1);
	}
}



void map_display::draw_pings(class game& gm, const vector2& offset) const
{
	// draw pings (just an experiment, you can hear pings, locate their direction
	//	a bit fuzzy but not their origin or exact shape).
	const list<game::ping>& pings = gm.get_pings();
	for (list<game::ping>::const_iterator it = pings.begin(); it != pings.end(); ++it) {
		const game::ping& p = *it;
		// vector2 r = player->get_pos ().xy () - p.pos;
		vector2 p1 = (p.pos + offset)*mapzoom;
		vector2 p2 = p1 + (p.dir + p.ping_angle).direction() * p.range * mapzoom;
		vector2 p3 = p1 + (p.dir - p.ping_angle).direction() * p.range * mapzoom;
		primitive_col<3> tri(GL_TRIANGLES);
		tri.vertices[0].x = 512+p1.x;
		tri.vertices[0].y = 384-p1.y;
		tri.colors[0] = colorf(0.5,0.5,0.5,1).to_uint8();
		tri.vertices[1].x = 512+p2.x;
		tri.vertices[1].y = 384-p2.y;
		tri.colors[1] = colorf(0.5,0.5,0.5,0).to_uint8();
		tri.vertices[2].x = 512+p3.x;
		tri.vertices[2].y = 384-p3.y;
		tri.colors[2] = colorf(0.5,0.5,0.5,0).to_uint8();
		tri.render();
		glColor4f(1,1,1,1);
	}
}



void map_display::draw_sound_contact(class game& gm, const sea_object* player,
	double max_view_dist, const vector2& offset) const
{
	const vector<sonar_contact>& obj = player->get_sonar_objects();
	for (vector<sonar_contact>::const_iterator it = obj.begin(); it != obj.end(); ++it) {
		vector2 ldir = it->pos - player->get_pos().xy();
		ldir = ldir.normal() * 0.666666 * max_view_dist*mapzoom;
		vector2 pos = (player->get_pos().xy() + offset) * mapzoom;
		switch (it->type) {
		case MERCHANT:
			glColor3f(0,0,0);
			break;
		case WARSHIP:
			glColor3f(0,0.5,0);
			break;
		case ESCORT:
			glColor3f(1,0,0);
			break;
		case SUBMARINE:
			glColor3f(1,0,0.5);
			break;
		default:
			// unknown object, not used yet
			glColor3f(0,0.5,0.5);
			break;
		}
		glBindTexture(GL_TEXTURE_2D, 0);
		primitives::line(vector2f(512+pos.x, 384-pos.y), vector2f(512+pos.x+ldir.x, 384-pos.y-ldir.y)).render();
	}
	glColor3f(1,1,1);
}



void map_display::draw_sound_contact(game& gm, const submarine* player,
				     const vector2& offset) const
{
	const std::map<double, sonar_operator::contact>& contacts = player->get_sonarman().get_contacts();
	for (std::map<double, sonar_operator::contact>::const_iterator it = contacts.begin();
	     it != contacts.end(); ++it) {
		// basic length 2km plus 10m per dB, max. 200dB or similar
		double lng = 2000 + it->second.strength_dB * 10;
		vector2 ldir = angle(it->first).direction()
			* lng * mapzoom;
		vector2 pos = (player->get_pos().xy() + offset) * mapzoom;
		switch (it->second.type) {
		case MERCHANT:
			glColor3f(0,0,0);
			break;
		case WARSHIP:
			glColor3f(0,0.5,0);
			break;
		case ESCORT:
			glColor3f(1,0,0);
			break;
		case SUBMARINE:
			glColor3f(1,0,0.5);
			break;
		case NONE:
		default:
			// unknown object, not used yet
			glColor3f(0,0.5,0.5);
			//glColor3f(0.75,0.75,0.2);
			break;
		}
		glBindTexture(GL_TEXTURE_2D, 0);
		primitives::line(vector2f(512+pos.x, 384-pos.y), vector2f(512+pos.x+ldir.x, 384-pos.y-ldir.y)).render();
	}
	glColor3f(1,1,1);
}



void map_display::draw_visual_contacts(class game& gm,
    const sea_object* player, const vector2& offset) const
{
	// draw vessel trails and symbols (since player is submerged, he is drawn too)
	const vector<sea_object*>& objs = player->get_visible_objects();

   	// draw trails
   	for (vector<sea_object*>::const_iterator it = objs.begin(); it != objs.end(); ++it)
   		draw_trail(*it, offset);

   	// draw vessel symbols
   	for (vector<sea_object*>::const_iterator it = objs.begin(); it != objs.end(); ++it) {
		color c;
		if (dynamic_cast<const submarine*>(*it)) c = color(255,255,128);
		else if (dynamic_cast<const torpedo*>(*it)) c = color(255,0,0);
		else if (dynamic_cast<const ship*>(*it)) c = color(192,255,192);
		else if (dynamic_cast<const airplane*>(*it)) c = color(0,0,64);
   		draw_vessel_symbol(offset, *it, c);
	}
}

void map_display::draw_radar_contacts(class game& gm, 
				      const sea_object* player, const vector2& offset) const
{
	const vector<sea_object*>& objs = player->get_radar_objects();

   	// draw trails
   	for (vector<sea_object*>::const_iterator it = objs.begin(); it != objs.end(); ++it)
   		draw_trail(*it, offset);

   	// draw vessel symbols
   	for (vector<sea_object*>::const_iterator it = objs.begin(); it != objs.end(); ++it) {
		color c;
		if (dynamic_cast<const submarine*>(*it)) c = color(255,255,128);
		else if (dynamic_cast<const ship*>(*it)) c = color(192,255,192);
   		draw_vessel_symbol(offset, *it, c);
	}
}



void map_display::draw_square_mark ( class game& gm,
	const vector2& mark_pos, const vector2& offset, const color& c ) const
{
	c.set_gl_color ();
	glBindTexture(GL_TEXTURE_2D, 0);
	vector2 p = ( mark_pos + offset ) * mapzoom;
	int x = int ( round ( p.x ) );
	int y = int ( round ( p.y ) );
	primitives::rectangle(vector2f(512-4+x,384-4-y), vector2f(512+4+x,384+4-y)).render();
}



void map_display::draw_square_mark_special ( class game& gm,
	const vector2& mark_pos, const vector2& offset, const color& c ) const
{
	c.set_gl_color ();
	glBindTexture(GL_TEXTURE_2D, 0);
	vector2 p = ( mark_pos + offset ) * mapzoom;
	int x = int ( round ( p.x ) );
	int y = int ( round ( p.y ) );
	primitives::rectangle(vector2f(512-8+x,384-8-y), vector2f(512+8+x,384+8-y)).render();
	primitives::diamond(vector2f(512+x,384-y), 8).render();
}



map_display::map_display(user_interface& ui_) :
	user_display(ui_), mapzoom(0.1), mx(0), my(0),
	edit_btn_del(0),
	edit_btn_chgmot(0),
	edit_btn_copy(0),
	edit_btn_cvmenu(0),
	edit_panel_fg(0),
	edit_shiplist(0),
	edit_heading(0),
	edit_speed(0),
	edit_throttle(0),
 	edit_timeyear(0),
 	edit_timemonth(0),
 	edit_timeday(0),
 	edit_timehour(0),
 	edit_timeminute(0),
 	edit_timesecond(0),
	edit_cvname(0),
	edit_cvspeed(0),
	edit_cvlist(0),
	mx_down(-1), my_down(-1), shift_key_pressed(0), ctrl_key_pressed(0),
	notepadsheet(texturecache(), "notepadsheet.png")
{
	game& gm = ui_.get_game();

	game_editor* ge = dynamic_cast<game_editor*>(&gm);
	if (ge) {
		game_editor& gme = *ge;

		// create editor main panel
		edit_panel.reset(new widget(0, 0, 1024, 32, "", 0));
		edit_panel->set_background(0);
		edit_panel->add_child(new widget_caller_arg_button<map_display, void (map_display::*)(game_editor&), game_editor&>(this, &map_display::edit_add_obj, gme, 0, 0, 128, 32, texts::get(224)));
		edit_btn_del = new widget_caller_arg_button<map_display, void (map_display::*)(game_editor&), game_editor&>(this, &map_display::edit_del_obj, gme, 128, 0, 128, 32, texts::get(225));
		edit_panel->add_child(edit_btn_del);
		edit_btn_chgmot = new widget_caller_arg_button<map_display, void (map_display::*)(game_editor&), game_editor&>(this, &map_display::edit_change_motion, gme, 256, 0, 128, 32, texts::get(226));
		edit_panel->add_child(edit_btn_chgmot);
		edit_btn_copy = new widget_caller_arg_button<map_display, void (map_display::*)(game_editor&), game_editor&>(this, &map_display::edit_copy_obj, gme, 384, 0, 128, 32, texts::get(227));
		edit_panel->add_child(edit_btn_copy);
		edit_btn_cvmenu = new widget_caller_arg_button<map_display, void (map_display::*)(game_editor&), game_editor&>(this, &map_display::edit_convoy_menu, gme, 512, 0, 128, 32, texts::get(228));
		edit_panel->add_child(edit_btn_cvmenu);
		edit_panel->add_child(new widget_caller_arg_button<map_display, void (map_display::*)(game_editor&), game_editor&>(this, &map_display::edit_time, gme, 640, 0, 128, 32, texts::get(229)));
		edit_panel->add_child(new widget_caller_arg_button<map_display, void (map_display::*)(game_editor&), game_editor&>(this, &map_display::edit_description, gme, 768, 0, 128, 32, texts::get(233)));
		edit_panel->add_child(new widget_caller_arg_button<map_display, void (map_display::*)(game_editor&), game_editor&>(this, &map_display::edit_help, gme, 896, 0, 128, 32, texts::get(230)));

		// create "add ship" window
		edit_panel_add.reset(new widget(0, 32, 1024, 768-2*32, texts::get(224)));
		edit_panel_add->set_background(0);
		edit_shiplist = new widget_list(20, 32, 1024-2*20, 768-2*32-2*32-8);
		edit_panel_add->add_child(edit_shiplist);
		edit_panel_add->add_child(new widget_caller_arg_button<widget, void (widget::*)(int), int>(edit_panel_add.get(), &widget::close, EPFG_SHIPADDED,  20, 768-3*32-8, 512-20, 32, texts::get(224)));
		edit_panel_add->add_child(new widget_caller_arg_button<widget, void (widget::*)(int), int>(edit_panel_add.get(), &widget::close, EPFG_CANCEL, 512, 768-3*32-8, 512-20, 32, texts::get(117)));
		for (list<string>::const_iterator it = data_file().get_ship_list().begin(); it != data_file().get_ship_list().end(); ++it) {
			edit_shiplist->append_entry(*it);
		}

		// create "motion edit" window
		// open widget with text edits: course, speed, throttle
		edit_panel_chgmot.reset(new widget(0, 32, 1024, 768-2*32, texts::get(226)));
		edit_panel_chgmot->set_background(0);
		edit_heading = new widget_slider(20, 128, 1024-40, 80, texts::get(1), 0, 360, 0, 15);
		edit_panel_chgmot->add_child(edit_heading);
		edit_speed = new widget_slider(20, 220, 1024-40, 80, texts::get(4), 0/*minspeed*/, 34/*maxspeed*/, 0, 1);
		edit_panel_chgmot->add_child(edit_speed);
		edit_throttle = new widget_slider(20, 330, 1024-40, 80, texts::get(232), 0/*minspeed*/, 34/*maxspeed*/, 0, 1);
		edit_panel_chgmot->add_child(edit_throttle);
		edit_panel_chgmot->add_child(new widget_caller_arg_button<widget, void (widget::*)(int), int>(edit_panel_chgmot.get(), &widget::close, EPFG_CHANGEMOTION,  20, 768-3*32-8, 512-20, 32, texts::get(226)));
		edit_panel_chgmot->add_child(new widget_caller_arg_button<widget, void (widget::*)(int), int>(edit_panel_chgmot.get(), &widget::close, EPFG_CANCEL, 512, 768-3*32-8, 512-20, 32, texts::get(117)));
		// also edit: target, country, damage status, fuel amount


		// create help window
		edit_panel_help.reset(new widget(0, 32, 1024, 768-2*32, texts::get(230)));
		edit_panel_help->set_background(0);
		edit_panel_help->add_child(new widget_text(20, 32, 1024-2*20, 768-2*32-2*32-8, texts::get(231), 0, true));
		edit_panel_help->add_child(new widget_caller_arg_button<widget, void (widget::*)(int), int>(edit_panel_help.get(), &widget::close, EPFG_CANCEL, 20, 768-3*32-8, 1024-20, 32, texts::get(105)));

		// create edit time window
		edit_panel_time.reset(new widget(0, 32, 1024, 768-2*32, texts::get(229)));
		edit_panel_time->set_background(0);
		edit_panel_time->add_child(new widget_caller_arg_button<widget, void (widget::*)(int), int>(edit_panel_time.get(), &widget::close, EPFG_CHANGETIME,  20, 768-3*32-8, 512-20, 32, texts::get(229)));
		edit_panel_time->add_child(new widget_caller_arg_button<widget, void (widget::*)(int), int>(edit_panel_time.get(), &widget::close, EPFG_CANCEL, 512, 768-3*32-8, 512-20, 32, texts::get(117)));
		edit_timeyear = new widget_slider(20, 128, 1024-40, 80, texts::get(234), 1939, 1945, 0, 1);
		edit_panel_time->add_child(edit_timeyear);
		edit_timemonth = new widget_slider(20, 208, 1024-40, 80, texts::get(235), 1, 12, 0, 1);
		edit_panel_time->add_child(edit_timemonth);
		edit_timeday = new widget_slider(20, 288, 1024-40, 80, texts::get(236), 1, 31, 0, 1);
		edit_panel_time->add_child(edit_timeday);
		edit_timehour = new widget_slider(20, 368, 1024-40, 80, texts::get(237), 0, 23, 0, 1);
		edit_panel_time->add_child(edit_timehour);
		edit_timeminute = new widget_slider(20, 448, 1024-40, 80, texts::get(238), 0, 59, 0, 5);
		edit_panel_time->add_child(edit_timeminute);
		edit_timesecond = new widget_slider(20, 528, 1024-40, 80, texts::get(239), 0, 59, 0, 5);
		edit_panel_time->add_child(edit_timesecond);

		// create convoy menu
		edit_panel_convoy.reset(new widget(0, 32, 1024, 768-2*32, texts::get(228)));
		edit_panel_convoy->set_background(0);
		edit_panel_convoy->add_child(new widget_text(20, 32, 256, 32, texts::get(244)));
		edit_cvname = new widget_edit(256+20, 32, 1024-256-2*20, 32, "-not usable yet, fixme-");
		edit_panel_convoy->add_child(edit_cvname);
		edit_cvspeed = new widget_slider(20, 64, 1024-40, 80, texts::get(245), 0/*minspeed*/, 34/*maxspeed*/, 0, 1);
		edit_panel_convoy->add_child(edit_cvspeed);
		edit_cvlist = new widget_list(20, 144, 1024-2*20, 768-144-3*32-8);
		edit_panel_convoy->add_child(edit_cvlist);
		edit_panel_convoy->add_child(new widget_caller_arg_button<widget, void (widget::*)(int), int>(edit_panel_convoy.get(), &widget::close, EPFG_ADDSELTOCV,  20+0*(1024-40)/5, 768-3*32-8, (1024-40)/5, 32, texts::get(240)));
		edit_panel_convoy->add_child(new widget_caller_arg_button<widget, void (widget::*)(int), int>(edit_panel_convoy.get(), &widget::close, EPFG_MAKENEWCV,   20+1*(1024-40)/5, 768-3*32-8, (1024-40)/5, 32, texts::get(241)));
		edit_panel_convoy->add_child(new widget_caller_arg_button<widget, void (widget::*)(int), int>(edit_panel_convoy.get(), &widget::close, EPFG_DELETECV,    20+2*(1024-40)/5, 768-3*32-8, (1024-40)/5, 32, texts::get(242)));
		edit_panel_convoy->add_child(new widget_caller_arg_button<widget, void (widget::*)(int), int>(edit_panel_convoy.get(), &widget::close, EPFG_EDITROUTECV, 20+3*(1024-40)/5, 768-3*32-8, (1024-40)/5, 32, texts::get(243)));
		edit_panel_convoy->add_child(new widget_caller_arg_button<widget, void (widget::*)(int), int>(edit_panel_convoy.get(), &widget::close, EPFG_CANCEL,      20+4*(1024-40)/5, 768-3*32-8, (1024-40)/5, 32, texts::get(117)));
		//fixme: en/disable some buttons depending on wether we have a selection or not

		check_edit_sel();
	}

#ifdef CVEDIT
	cvridx = -1;
#endif
}



void map_display::edit_add_obj(game_editor& gm)
{
	edit_panel_add->open();
	edit_panel->disable();
	edit_panel_fg = edit_panel_add.get();
}



void map_display::edit_del_obj(game_editor& gm)
{
	// just delete all selected objects, if they are no subs
	for (std::set<sea_object*>::iterator it = selection.begin();
	     it != selection.end(); ++it) {
		if (*it != gm.get_player()) {
			(*it)->kill();
		}
	}
	selection.clear();
	check_edit_sel();
}



void map_display::edit_change_motion(game_editor& gm)
{
	if (selection.size() == 0) return;

	// compute max speed.
	int minspeed = 0, maxspeed = 0;
	for (std::set<sea_object*>::const_iterator it = selection.begin(); it != selection.end(); ++it) {
		const ship* s = dynamic_cast<const ship*>(*it);
		if (s) {
			int sp = int(sea_object::ms2kts(s->get_max_speed()) + 0.5);
			maxspeed = std::max(maxspeed, sp);
		}
	}

	edit_panel_chgmot->open();
	edit_speed->set_values(minspeed, maxspeed, 0, 1);
	edit_throttle->set_values(minspeed, maxspeed, 0, 1);
	edit_panel->disable();
	edit_panel_fg = edit_panel_chgmot.get();
}



void map_display::edit_copy_obj(game_editor& gm)
{
	// just duplicate the objects with some position offset (1km to x/y)
	std::set<sea_object*> new_selection;
	vector3 offset(300, 100, 0);
	for (std::set<sea_object*>::iterator it = selection.begin(); it != selection.end(); ++it) {
		ship* s = dynamic_cast<ship*>(*it);
		submarine* su = dynamic_cast<submarine*>(*it);
		if (s && su == 0) {
			xml_doc spec(data_file().get_filename(s->get_specfilename()));
			spec.load();
			ship* s2 = new ship(gm, spec.first_child());
			s2->set_skin_layout(model::default_layout);
			// set pos and other values etc.
			vector3 pos = s->get_pos() + offset;
			s2->manipulate_position(pos);
			s2->manipulate_speed(s->get_speed());
			s2->manipulate_heading(s->get_heading());
			s2->manipulate_invulnerability(true);
			s2->set_throttle(int(s->get_throttle()));
			gm.spawn_ship(s2);
			new_selection.insert(s2);
		}
	}
	selection = new_selection;
	check_edit_sel();
}



void map_display::edit_convoy_menu(game_editor& gm)
{
	edit_panel_convoy->open();
	edit_panel->disable();
	edit_panel_fg = edit_panel_convoy.get();
	// make convoy from currently selected objects, but without sub
	if (selection.empty()) {
		// fixme: disable
	} else {
		// fixme: enable
	}
	// fill list of convoy names
	edit_cvlist->clear();
	const ptrset<convoy>& convoys = gm.get_convoy_list();
	for (unsigned i = 0; i < convoys.size(); ++i) {
		string nm = convoys[i]->get_name();
		if (nm.length() == 0)
			nm = "???";
		edit_cvlist->append_entry(nm);
	}
	// fill in current cv name and speed
	//...
}



void map_display::edit_time(game_editor& gm)
{
	// open widget with text edits: date/time
	// enter date and time of day
	edit_panel_time->open();
	edit_panel->disable();
	edit_panel_fg = edit_panel_time.get();
}



void map_display::edit_description(game_editor& gm)
{
	// game must store mission description/briefing to make this function work... fixme
	// only store short description here? or take save file name in save dialogue
	// as description? we have no multiline-edit-widget. so we can't really let the user
	// enter long descriptions here.
}



void map_display::edit_help(game_editor& /*gm*/)
{
	edit_panel_help->open();
	edit_panel->disable();
	edit_panel_fg = edit_panel_help.get();
}



void map_display::check_edit_sel()
{
	if (selection.empty()) {
		edit_btn_del->disable();
		edit_btn_chgmot->disable();
		edit_btn_copy->disable();
	} else {
		edit_btn_del->enable();
		edit_btn_chgmot->enable();
		edit_btn_copy->enable();
	}
}



void map_display::display(class game& gm) const
{
	sea_object* player = gm.get_player ();
	bool is_day_mode = gm.is_day_mode ();

	if ( is_day_mode )
		glClearColor ( 0.0f, 0.0f, 1.0f, 1.0f );
	else
		glClearColor ( 0.0f, 0.0f, 0.75f, 1.0f );
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	double max_view_dist = gm.get_max_view_distance();

	vector2 offset = player->get_pos().xy() + mapoffset;

	sys().prepare_2d_drawing();

	float delta = MAPGRIDSIZE*mapzoom;
	float sx = myfmod(512, delta)-myfmod(offset.x, MAPGRIDSIZE)*mapzoom;
	float sy = 768.0 - (myfmod(384.0f, delta)-myfmod(offset.y, MAPGRIDSIZE)*mapzoom);
	int lx = int(1024/delta)+2, ly = int(768/delta)+2;

	// draw grid
	if (mapzoom >= 0.01) {
		glColor3f(0.5, 0.5, 1);
		glBindTexture(GL_TEXTURE_2D, 0);
		for (int i = 0; i < lx; ++i) {
			primitives::line(vector2f(sx, 0), vector2f(sx, 768)).render();
			sx += delta;
		}
		for (int i = 0; i < ly; ++i) {
			primitives::line(vector2f(0, sy), vector2f(1024, sy)).render();
			sy -= delta;
		}
	}

	// draw map
	glColor3f(1,1,1);
	glPushMatrix();
	glTranslatef(512, 384, 0);
	glScalef(mapzoom, mapzoom, 1);
	glScalef(1,-1,1);
	glTranslatef(-offset.x, -offset.y, 0);
	glCullFace(GL_BACK);// we must render the map with front-faced tris
	ui.get_coastmap().draw_as_map(offset, mapzoom);//, detl); // detail should depend on zoom, fixme
	glCullFace(GL_FRONT);//clean up
	glPopMatrix();
	glBindTexture(GL_TEXTURE_2D, 0);

#ifdef CVEDIT
	// draw convoy route points and route
	if (cvroute.size() >= 2) {
		unsigned n = std::min(unsigned(3), cvroute.size()-1);
		bsplinet<vector2> bsp(n, cvroute);
		const unsigned detail = 10000;
		glColor3f(1,0.6,0.2);
		primitives cvr(GL_LINE_STRIP, false, false, detail+1);
		for (unsigned i = 0; i <= detail; ++i) {
			double j = double(i)/detail;
			vector2 p = bsp.value(j);
			p = (p - offset) * mapzoom;
			cvr.vertices[i].x = 512 + p.x;
			cvr.vertices[i].y = 384 - p.y;
		}
		cvr.render();
	}
	for (unsigned i = 0; i < cvroute.size(); ++i) {
		draw_square_mark(gm, cvroute[i], -offset, color(255, 0, 255));
	}
#endif

	// draw city names
	const list<pair<vector2, string> >& cities = ui.get_coastmap().get_city_list();
	for (list<pair<vector2, string> >::const_iterator it = cities.begin(); it != cities.end(); ++it) {
		glBindTexture(GL_TEXTURE_2D, 0);
		draw_square_mark(gm, it->first, -offset, color(255, 0, 0));
		vector2 pos = (it->first - offset) * mapzoom;
		font_vtremington12->print(int(512 + pos.x), int(384 - pos.y), it->second);
	}

	// draw convoy positions	fixme: should be static and fade out after some time
	glColor3f(1,1,1);
	vector<vector2> convoy_pos = gm.convoy_positions();
	for (vector<vector2>::iterator it = convoy_pos.begin(); it != convoy_pos.end(); ++it) {
		draw_square_mark_special ( gm, (*it), -offset, color ( 0, 0, 0 ) );
	}
	glColor3f(1,1,1);

	// draw view range
	glColor3f(1,0,0);
	glBindTexture(GL_TEXTURE_2D, 0);
	primitives::circle(vector2f(512-mapoffset.x*mapzoom,384+mapoffset.y*mapzoom),
			   max_view_dist*mapzoom).render();
	glColor3f(1,1,1);

	sea_object* target = gm.get_player()->get_target();

	// draw vessel symbols (or noise contacts)
	submarine* sub_player = dynamic_cast<submarine*> ( player );
	if (sub_player && sub_player->is_submerged ()) {
		// draw pings
		draw_pings(gm, -offset);

		// draw sound contacts
		//draw_sound_contact(gm, sub_player, max_view_dist, -offset);	// old contacts
		draw_sound_contact(gm, sub_player, -offset);

		// draw player trails and player
		draw_trail(player, -offset);
		draw_vessel_symbol(-offset, sub_player, color(255,255,128));

		// Special handling for submarine player: When the submarine is
		// on periscope depth and the periscope is up the visual contact
		// must be drawn on map.
		if ((sub_player->get_depth() <= sub_player->get_periscope_depth()) &&
			sub_player->is_scope_up())
		{
			draw_visual_contacts(gm, sub_player, -offset);

			// Draw a red box around the selected target.
			if (target)
			{
				draw_square_mark ( gm, target->get_pos ().xy (), -offset,
					color ( 255, 0, 0 ) );
				glColor3f ( 1.0f, 1.0f, 1.0f );
			}
		}
	} 
	else	 	// enable drawing of all object as testing hack by commenting this, fixme
	{
		draw_visual_contacts(gm, player, -offset);
		draw_radar_contacts(gm, player, -offset);

		// Draw a red box around the selected target.
		if (target)
		{
			draw_square_mark ( gm, target->get_pos ().xy (), -offset,
				color ( 255, 0, 0 ) );
			glColor3f ( 1.0f, 1.0f, 1.0f );
		}
	}

#if 1
	// test: draw sonar signals as circles with varying radii
	vector<pair<double, noise> > signal_strengths;
	const unsigned signal_res = 360;
	signal_strengths.resize(signal_res);
	for (unsigned i = 0; i < signal_res; ++i) {
		angle a(360.0*i/signal_res);
		signal_strengths[i] = gm.sonar_listen_ships(sub_player, a);
	}
	// render the strengths as circles with various colors
	glBindTexture(GL_TEXTURE_2D, 0);
	primitives circle(GL_LINE_LOOP, false, false, signal_res);
	for (unsigned j = 0; j < noise::NR_OF_FREQUENCY_BANDS; ++j) {
		float f = 1.0f - float(j)/noise::NR_OF_FREQUENCY_BANDS;
		glColor3f(f,f,f*0.5f);
		for (unsigned i = 0; i < signal_res; ++i) {
			angle a = angle(360.0*i/signal_res) + sub_player->get_heading();
			double r = signal_strengths[i].second.frequencies[j] * 15;
			vector2 p = (sub_player->get_pos().xy() - offset + a.direction() * r) * mapzoom;
			circle.vertices[i] = vector2f(512+p.x, 384-p.y).xy0();
		}
		circle.render();
	}
	// draw total signal strength
	glColor3f(1.0,0.5,0.5);
	for (unsigned i = 0; i < signal_res; ++i) {
		angle a = angle(360.0*i/signal_res) + sub_player->get_heading();
		double r = signal_strengths[i].first * 15;
		vector2 p = (sub_player->get_pos().xy() - offset + a.direction() * r) * mapzoom;
		circle.vertices[i] = vector2f(512+p.x, 384-p.y).xy0();
	}
	circle.render();
	glColor3f(1,1,1);
//	for (int i = 0; i <= 179; ++i) {
//		printf("test[%i]=%f\n",
//		       i,
//		       compute_signal_strength_GHG(angle(45.0), 6000, angle(double(i))));
//	}
#endif

	// draw notepad sheet giving target distance, speed and course
	if (target) {
		int nx = 768, ny = 512;
		notepadsheet.get()->draw(nx, ny);
		ostringstream os0, os1, os2;
		// fixme: use estimated values from target/tdc estimation here, make functions for that
		os0 << texts::get(3) << ": " << unsigned(target->get_pos().xy().distance(player->get_pos().xy())) << texts::get(206);
		os1 << texts::get(4) << ": " << unsigned(fabs(sea_object::ms2kts(target->get_speed()))) << texts::get(208);
		os2 << texts::get(1) << ": " << unsigned(target->get_heading().value()) << texts::get(207);
		font_vtremington12->print(nx+16, ny+40, os0.str(), color(0,0,0));
		font_vtremington12->print(nx+16, ny+60, os1.str(), color(0,0,0));
		font_vtremington12->print(nx+16, ny+80, os2.str(), color(0,0,0));
	}

	// draw world coordinates for mouse
	double mouserealmx = double(mx - 512) / mapzoom + offset.x;
	double mouserealmy = double(384 - my) / mapzoom + offset.y;
	unsigned degrx, degry, minutx, minuty;
	bool west, south;
	sea_object::meters2degrees(mouserealmx, mouserealmy, west, degrx, minutx, south, degry, minuty);
	ostringstream rwcoordss;
	rwcoordss	<< degry << "/" << minuty << (south ? "S" : "N") << ", "
			<< degrx << "/" << minutx << (west ? "W" : "E");
	font_vtremington12->print(0, 0, rwcoordss.str(), color::white(), true);

	// editor specials ------------------------------------------------------------
	if (gm.is_editor()) {
		if (edit_panel_fg) {
			edit_panel_fg->draw();
		} else {
			// selection rectangle
			if (mx_down >= 0 && my_down >= 0) {
				glBindTexture(GL_TEXTURE_2D, 0);
				glColor4f(1,1,0,1);
				int x1 = std::min(mx_down, mx_curr);
				int y1 = std::min(my_down, my_curr);
				int x2 = std::max(mx_down, mx_curr);
				int y2 = std::max(my_down, my_curr);
				primitives::rectangle(vector2f(x1,y1), vector2f(x2,y2)).render();
				glColor3f(1,1,1);
			}
			// selected objects
			for (std::set<sea_object*>::const_iterator it = selection.begin();
			     it != selection.end(); ++it) {
				draw_square_mark(gm, (*it)->get_pos().xy(), -offset,
						 color(255,0,64));
			}
		}
		edit_panel->draw();
	}

	ui.draw_infopanel();
	sys().unprepare_2d_drawing();

}



void map_display::process_input(class game& gm, const SDL_Event& event)
{
	sea_object* player = gm.get_player();

	if (gm.is_editor()) {
		// handle mouse events for edit panel if that exists.
		if (edit_panel->check_for_mouse_event(event))
			return;
		// check if foreground window is open and event should go to it
		if (edit_panel_fg != 0/* && edit_panel_fg->check_for_mouse_event(event)*/) {
			edit_panel_fg->process_input(event);
			// we could compare edit_panel_fg to the pointers of the various panels
			// here instead of using a global enum for all possible return values...
			if (edit_panel_fg->was_closed()) {
				int retval = edit_panel_fg->get_return_value();
				if (retval == EPFG_SHIPADDED) {
					// add ship
					xml_doc spec(data_file().get_filename(edit_shiplist->get_selected_entry()));
					spec.load();
					auto_ptr<ship> shp(new ship(gm, spec.first_child()));
					shp->set_skin_layout(model::default_layout);
					// set pos and other values etc.
					vector2 pos = gm.get_player()->get_pos().xy() + mapoffset;
					shp->manipulate_position(pos.xy0());
					shp->manipulate_invulnerability(true);
					gm.spawn_ship(shp.release());
				} else if (retval == EPFG_CHANGEMOTION) {
					for (std::set<sea_object*>::iterator it = selection.begin(); it != selection.end(); ++it) {
						ship* s = dynamic_cast<ship*>(*it);
						if (s) {
							s->set_throttle(edit_throttle->get_curr_value());
							s->manipulate_heading(angle(edit_heading->get_curr_value()));
							s->manipulate_speed(edit_speed->get_curr_value());
						}
					}
				} else if (retval == EPFG_CHANGETIME) {
					date d(edit_timeyear->get_curr_value(),
					       edit_timemonth->get_curr_value(),
					       edit_timeday->get_curr_value(),
					       edit_timehour->get_curr_value(),
					       edit_timeminute->get_curr_value(),
					       edit_timesecond->get_curr_value());
					double time = d.get_time();
					game_editor& ge = dynamic_cast<game_editor&>(gm);
					ge.manipulate_time(time);
					// construct new date to correct possible wrong date values
					// like 30th february or so...
					ge.manipulate_equipment_date(date(d.get_time()));
				} else if (retval == EPFG_ADDSELTOCV) {
					// compute center of ships
					vector2 center;
					unsigned nrsh = 0;
					for (std::set<sea_object*>::iterator it = selection.begin(); it != selection.end(); ++it) {
						ship* s = dynamic_cast<ship*>(*it);
						if (s) {
							center += s->get_pos().xy();
							++nrsh;
						}
					}
					center = center * (1.0/nrsh);	
					// create convoy object
					auto_ptr<convoy> cv(new convoy(gm, center, edit_cvname->get_text()));
					// add all ships to convoy with relative positions
					nrsh = 0;
					for (std::set<sea_object*>::iterator it = selection.begin(); it != selection.end(); ++it) {
						ship* s = dynamic_cast<ship*>(*it);
						if (s) {
							if (cv->add_ship(s))
								++nrsh;
						}
					}
					// add convoy to class game, if it has ships
					if (nrsh > 0) {
						gm.spawn_convoy(cv.release());
					}
				}
				edit_panel->enable();
				edit_panel_fg = 0;
			}
			return;
		}
		// no panel visible. handle extra edit modes
		if (event.type == SDL_KEYDOWN) {
			if (event.key.keysym.sym == SDLK_LSHIFT) {
				shift_key_pressed |= 1;
				return;
			} else if (event.key.keysym.sym == SDLK_RSHIFT) {
				shift_key_pressed |= 2;
				return;
			} else if (event.key.keysym.sym == SDLK_LCTRL) {
				ctrl_key_pressed |= 1;
				return;
			} else if (event.key.keysym.sym == SDLK_RCTRL) {
				ctrl_key_pressed |= 2;
				return;
			}
		} else if (event.type == SDL_KEYUP) {
			if (event.key.keysym.sym == SDLK_LSHIFT) {
				shift_key_pressed &= ~1;
				return;
			} else if (event.key.keysym.sym == SDLK_RSHIFT) {
				shift_key_pressed &= ~2;
				return;
			} else if (event.key.keysym.sym == SDLK_LCTRL) {
				ctrl_key_pressed &= ~1;
				return;
			} else if (event.key.keysym.sym == SDLK_RCTRL) {
				ctrl_key_pressed &= ~2;
				return;
			}
		} else if (event.type == SDL_MOUSEBUTTONDOWN) {
			if (event.button.button == SDL_BUTTON_LEFT) {
				mx_down = event.button.x;
				my_down = event.button.y;
				return;
			}
		} else if (event.type == SDL_MOUSEBUTTONUP) {
			if (event.button.button == SDL_BUTTON_LEFT) {
				mx_curr = event.button.x;
				my_curr = event.button.y;
				// check for shift / ctrl
				unsigned mode = 0;	// replace selection
				if (shift_key_pressed) mode = 1; // subtract
				if (ctrl_key_pressed) mode = 2; // add.
				if (mx_curr != mx_down || my_curr != my_down) {
					// group select
					int x1 = std::min(mx_down, mx_curr);
					int y1 = std::min(my_down, my_curr);
					int x2 = std::max(mx_down, mx_curr);
					int y2 = std::max(my_down, my_curr);
					// fixme: later all objects
					vector<sea_object*> objs = gm.visible_surface_objects(player);
					if (mode == 0) selection.clear();
					for (vector<sea_object*>::iterator it = objs.begin(); it != objs.end(); ++it) {
						vector2 p = ((*it)->get_pos().xy() -
							     (player->get_pos().xy() + mapoffset)) * mapzoom;
						p.x += 512;
						p.y = 384 - p.y;
						if (p.x >= x1 && p.x <= x2 &&
						    p.y >= y1 && p.y <= y2) {
							if (mode == 1)
								selection.erase(*it);
							else
								selection.insert(*it);
						}
					}
					check_edit_sel();
				} else {
					// select nearest
					vector2 mapclick(event.button.x, event.button.y);
					// fixme: later all objects!
					vector<sea_object*> objs = gm.visible_surface_objects(player);
					double mapclickdist = 1e30;
					sea_object* target = 0;
					if (mode == 0) selection.clear();
					for (vector<sea_object*>::iterator it = objs.begin(); it != objs.end(); ++it) {
						vector2 p = ((*it)->get_pos().xy() -
							     (player->get_pos().xy() + mapoffset)) * mapzoom;
						p.x += 512;
						p.y = 384 - p.y;
						double clickd = mapclick.square_distance(p);
						if (clickd < mapclickdist) {
							target = *it;
							mapclickdist = clickd;
						}
					}
					if (mode == 1)
						selection.erase(target);
					else
						selection.insert(target);
					check_edit_sel();
				}
				mx_down = -1;
				my_down = -1;
				return;
			}
		} else if (event.type == SDL_MOUSEMOTION) {
			mx_curr = event.motion.x;
			my_curr = event.motion.y;
			if ((event.motion.state & SDL_BUTTON_MMASK) && (ctrl_key_pressed != 0)) {
				// move selected objects!
				vector2 drag(event.motion.xrel / mapzoom,
					     -event.motion.yrel / mapzoom);
				for (std::set<sea_object*>::const_iterator it = selection.begin();
				     it != selection.end(); ++it) {
					vector3 p = (*it)->get_pos();
					p.x += drag.x;
					p.y += drag.y;
					(*it)->manipulate_position(p);
				}
				return;
			}
		}
	}

	// non-editor events.
	switch (event.type) {
	case SDL_MOUSEBUTTONDOWN:
		if (event.button.button == SDL_BUTTON_LEFT) {
#ifndef CVEDIT
			// set target. get visible objects and determine which is nearest to
			// mouse position. set target for player object
			vector2 mapclick(event.button.x, event.button.y);
			vector<sea_object*> objs = gm.visible_surface_objects(player);
			double mapclickdist = 1e30;
			sea_object* target = 0;
			for (vector<sea_object*>::iterator it = objs.begin(); it != objs.end(); ++it) {
				if (!(*it)->is_alive()) continue;
				vector2 p = ((*it)->get_pos().xy() -
					     (player->get_pos().xy() + mapoffset)) * mapzoom;
				p.x += 512;
				p.y = 384 - p.y;
				double clickd = mapclick.square_distance(p);
				if (clickd < mapclickdist) {
					target = *it;	// fixme: message?
					mapclickdist = clickd;
				}
			}

			player->set_target(target);
#else
			// move nearest cv point
			if (cvroute.size() > 0) {
				vector2 mapclick(event.button.x - 512, 384 - event.button.y);
				vector2 real = mapclick * (1.0/mapzoom) + mapoffset + player->get_pos().xy();
				double dist = cvroute.front().distance(real);
				cvridx = 0;
				for (unsigned i = 1; i < cvroute.size(); ++i) {
					double d = cvroute[i].distance(real);
					if (d < dist) {
						dist = d;
						cvridx = i;
					}
				}
			}
#endif
		}
#ifdef CVEDIT
		if (event.button.button == SDL_BUTTON_RIGHT) {
			vector2 mapclick(event.button.x - 512, 384 - event.button.y);
			vector2 real = mapclick * (1.0/mapzoom) + mapoffset + player->get_pos().xy();
			cvroute.push_back(real);
		}
#endif

                if (event.button.button == SDL_BUTTON_WHEELUP) {
                        if (mapzoom < 1) mapzoom *=1.25;
                } else if (event.button.button == SDL_BUTTON_WHEELDOWN) {
                        if (mapzoom > 1.0/16384) mapzoom /= 1.25;
                }

	        break;
	case SDL_MOUSEMOTION:
		mx = event.motion.x;
		my = event.motion.y;
		if ((event.motion.state & SDL_BUTTON_MMASK) && (ctrl_key_pressed == 0)) {
			mapoffset.x += event.motion.xrel / mapzoom;
			mapoffset.y += -event.motion.yrel / mapzoom;
		}
#ifdef CVEDIT
		if (event.motion.state & SDL_BUTTON_LMASK && cvridx >= 0) {
			cvroute[cvridx].x += event.motion.xrel / mapzoom;
			cvroute[cvridx].y += -event.motion.yrel / mapzoom;
		}
#endif
	case SDL_KEYDOWN:
		if (cfg::instance().getkey(KEY_ZOOM_MAP).equal(event.key.keysym)) {
			if (mapzoom < 1) mapzoom *= 2;
		} else if (cfg::instance().getkey(KEY_UNZOOM_MAP).equal(event.key.keysym)) {
			if (mapzoom > 1.0/16384) mapzoom /= 2;
		}
#ifdef CVEDIT
		if (event.key.keysym.sym == SDLK_w) {
			cout << "Current cv route:\n\n";
			for (unsigned i = 0; i < cvroute.size(); ++i) {
				cout << "\t<waypoint x=\"" << cvroute[i].x << "\" y=\"" << cvroute[i].y
				     << "\" />\n";
			}
		}
#endif
		break;
	default:
		break;
	}
}
