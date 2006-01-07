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
	void draw_radar_contacts(class game& gm, 
							 const sea_object* player, const vector2& offset) const;
	void draw_square_mark (game& gm,
		const vector2& mark_pos, const vector2& offset, const color& c ) const;

#ifdef CVEDIT
	vector<vector2> cvroute;
	int cvridx;
#endif

private:
	map_display();

public:
	map_display(class user_interface& ui_);
	virtual ~map_display();

	virtual void display(class game& gm) const;
	virtual void process_input(class game& gm, const SDL_Event& event);
};

#endif
