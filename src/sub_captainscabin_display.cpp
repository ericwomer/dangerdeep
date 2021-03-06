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

// user display: submarine's torpedo room
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "system.h"
#include "image.h"
#include "texture.h"
#include "game.h"
#include "font.h"
#include "sub_captainscabin_display.h"
#include "texts.h"
#include "torpedo.h"
#include "submarine_interface.h"
#include "global_data.h"
#include <sstream>
#include <fstream>
using namespace std;



void sub_captainscabin_display::goto_successes()
{
	(dynamic_cast<submarine_interface&>(ui)).goto_successes();
}

void sub_captainscabin_display::goto_logbook()
{
	(dynamic_cast<submarine_interface&>(ui)).goto_logbook();
}

void sub_captainscabin_display::goto_torpedoes()
{
	(dynamic_cast<submarine_interface&>(ui)).goto_torpedomanagement();
}

void sub_captainscabin_display::goto_recogmanual()
{
	(dynamic_cast<submarine_interface&>(ui)).goto_recogmanual();
}

sub_captainscabin_display::clickable_area::clickable_area(const vector2i& tl, const vector2i& br,
							  int descr,
							  void (sub_captainscabin_display::*func)(),
							  color dc)
	: topleft(tl), bottomright(br), description(descr), action(func), desc_color(dc)
{
}

bool sub_captainscabin_display::clickable_area::is_mouse_over(int mx, int my) const
{
	return (mx >= topleft.x && mx <= bottomright.x &&
		my >= topleft.y && my <= bottomright.y);
}

void sub_captainscabin_display::clickable_area::do_action(sub_captainscabin_display& obj)
{
	(obj.*action)();
}

sub_captainscabin_display::sub_captainscabin_display(user_interface& ui_) :
	user_display(ui_), mx(0), my(0)
{
	clickable_areas.push_back(clickable_area(vector2i(0, 540), vector2i(292,705), 272, &sub_captainscabin_display::goto_successes, color(255, 224, 224)));
	clickable_areas.push_back(clickable_area(vector2i(415, 495), vector2i(486,520), 255, &sub_captainscabin_display::goto_logbook, color(224, 224, 255)));
	clickable_areas.push_back(clickable_area(vector2i(713, 176), vector2i(862, 575), 253, &sub_captainscabin_display::goto_torpedoes, color(224, 255, 224)));
	clickable_areas.push_back(clickable_area(vector2i(405, 430), vector2i(462,498), 273, &sub_captainscabin_display::goto_recogmanual, color(255, 224, 224)));
}

void sub_captainscabin_display::display(class game& gm) const
{
//	submarine* sub = dynamic_cast<submarine*>(gm.get_player());

	// draw background
	sys().prepare_2d_drawing();

	background->draw(0, 0);

	for (vector<clickable_area>::const_iterator it = clickable_areas.begin();
	     it != clickable_areas.end(); ++it) {
		if (it->is_mouse_over(mx, my)) {
			font_vtremington12->print_hc(mx, my - font_arial->get_height(),
					     texts::get(it->get_description()),
					     it->get_description_color(), true);
			break;
		}
	}
	
	ui.draw_infopanel();
	sys().unprepare_2d_drawing();
}

void sub_captainscabin_display::process_input(class game& gm, const SDL_Event& event)
{
//	submarine* sub = dynamic_cast<submarine*>(gm.get_player());

	switch (event.type) {
	case SDL_MOUSEWHEEL:
		if (event.wheel.y > 0) {
				} else if (event.wheel.y < 0) {
				}
		break;
	case SDL_MOUSEBUTTONDOWN:
		// check if there is a clickable area below the mouse and take some action
		mx = sys().translate_position_x(event);
		my = sys().translate_position_y(event);
		if (event.button.button == SDL_BUTTON_LEFT) {
			// fixme:
		}
		break;
	case SDL_MOUSEBUTTONUP:
		// check if there is a clickable area below the mouse and take some action
		mx = sys().translate_position_x(event);
		my = sys().translate_position_y(event);
		if (event.button.button == SDL_BUTTON_LEFT) {
			for (vector<clickable_area>::iterator it = clickable_areas.begin();
			     it != clickable_areas.end(); ++it) {
				if (it->is_mouse_over(mx, my)) {
					it->do_action(*this);
					break;
				}
			}
		}
		break;
	case SDL_MOUSEMOTION:
		mx = sys().translate_position_x(event);
		my = sys().translate_position_y(event);
		break;
	default:
		break;
	}
}

void sub_captainscabin_display::enter(bool is_day)
{
	background.reset(new image(get_image_dir() + "captainscabin_main_"
				   + (is_day ? "daylight" : "redlight") + ".jpg"));
}

void sub_captainscabin_display::leave()
{
	background.reset();
}
