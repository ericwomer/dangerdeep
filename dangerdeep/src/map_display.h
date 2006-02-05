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

#ifndef MAP_DISPLAY_H
#define MAP_DISPLAY_H

#include "user_display.h"
#include "vector2.h"
#include "color.h"
#include "widget.h"
class game;
class sea_object;

class map_display : public user_display
{
protected:
	// map
	float mapzoom;	// factor pixel/meter
	vector2 mapoffset;	// additional offset used for display, relative to player (meters)
	int mx, my;	// last mouse position

	void draw_vessel_symbol(const vector2& offset, sea_object* so, color c) const;
	void draw_trail(sea_object* so, const vector2& offset) const;
	void draw_pings(game& gm, const vector2& offset) const;
	void draw_sound_contact(game& gm, const sea_object* player,
		double max_view_dist, const vector2& offset) const;
	void draw_visual_contacts(game& gm,
		const sea_object* player, const vector2& offset) const;
	void draw_radar_contacts(game& gm, 
				 const sea_object* player, const vector2& offset) const;
	void draw_square_mark (game& gm,
		const vector2& mark_pos, const vector2& offset, const color& c ) const;

	// only used in editor mode
	std::auto_ptr<widget> edit_panel;
	std::auto_ptr<widget> edit_panel_add;
	std::auto_ptr<widget> edit_panel_chgmot;
	std::auto_ptr<widget> edit_panel_time;
	std::auto_ptr<widget> edit_panel_descr;
	std::auto_ptr<widget> edit_panel_help;
	widget* edit_panel_fg;
	widget_list* edit_shiplist;
	widget_slider* edit_heading;
	widget_slider* edit_speed;
	widget_slider* edit_throttle;
	int mx_down, my_down;	// position of mouse when button was pressed
	int mx_curr, my_curr;	// current position of mouse
	std::set<sea_object*> selection;
	unsigned shift_key_pressed;
	unsigned ctrl_key_pressed;
#ifdef CVEDIT
	vector<vector2> cvroute;
	int cvridx;
#endif

	// editor methods
	void edit_add_obj(game& gm);
	void edit_del_obj(game& gm);
	void edit_change_motion(game& gm);
	void edit_copy_obj(game& gm);
	void edit_make_convoy(game& gm);
	void edit_time(game& gm);
	void edit_description(game& gm);
	void edit_help(game& gm);

private:
	map_display();

public:
	map_display(class user_interface& ui_);

	virtual void display(game& gm) const;
	virtual void process_input(game& gm, const SDL_Event& event);
};

#endif
