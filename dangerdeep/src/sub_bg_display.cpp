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

// user display: submarine's bg hearing device
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "sub_bg_display.h"
#include "system.h"
#include "image.h"
#include "texture.h"
#include "game.h"
#include "submarine_interface.h"
#include "submarine.h"
#include "keys.h"
#include "cfg.h"
#include "global_data.h"
#include <sstream>
using namespace std;

static const double TK_ANGFAC = 360.0/512.0;
static const unsigned TK_PHASES = 6;

sub_bg_display::sub_bg_display(user_interface& ui_)
	: user_display(ui_), turnknobdrag(TK_NONE), turnknobang(TK_NR)
{
	daylight.background.reset(new image(get_image_dir() + "BG_daylight_background.jpg"));
	redlight.background.reset(new image(get_image_dir() + "BG_redlight_background.jpg"));
	daylight.direction_ptr.set("BG_daylight_pointer.png", 341, 153, 373, 346);
	redlight.direction_ptr.set("BG_redlight_pointer.png", 341, 153, 373, 346);

	for (unsigned i = 0; i < TK_PHASES; ++i) {
		ostringstream osn;
		osn << (i+1) ;
		daylight.turn_wheel[i].set("BG_daylight_knob_pos" + osn.str() + ".png", 110, 641);
		redlight.turn_wheel[i].set("BG_redlight_knob_pos" + osn.str() + ".png", 110, 641);
	}
}



void sub_bg_display::process_input(class game& gm, const SDL_Event& event)
{
	bool is_day = gm.is_day_mode();
	int mx, my, mb;

	const scheme& s = (is_day) ? daylight : redlight;

	switch (event.type) {
	case SDL_MOUSEBUTTONDOWN:
		mx = event.button.x;
		my = event.button.y;
		// check if mouse is over turn knobs
		turnknobdrag = TK_NONE;
		if (s.turn_wheel[0].is_mouse_over(mx, my, 128)) {
			turnknobdrag = TK_DIRECTION;
		}
		break;
	case SDL_MOUSEMOTION:
		mx = event.motion.xrel;
		my = event.motion.yrel;
		mb = event.motion.state;
		if (event.motion.state & SDL_BUTTON_LMASK) {
			if (turnknobdrag != TK_NONE) {
				float& ang = turnknobang[unsigned(turnknobdrag)];
				ang += mx * TK_ANGFAC;
				switch (turnknobdrag) {
				case TK_DIRECTION:
					// bring to 0...360 degree value
					ang = myfmod(ang, 720.0f);
					//sub->set_bg_direction(ang); // fixme: set angle of player
					break;
				default:	// can never happen
					break;
				}
			}
		}
		break;
	case SDL_MOUSEBUTTONUP:
		mx = event.button.x;
		my = event.button.y;
		turnknobdrag = TK_NONE;
		break;
	default:
		break;
	}
}



void sub_bg_display::display(class game& gm) const
{
	sys().prepare_2d_drawing();
	glColor3f(1,1,1);

	// determine time of day
	bool is_day = gm.is_day_mode();

	// get hearing device angle from submarine, if it has one

	const scheme& s = (is_day) ? daylight : redlight;

	s.background->draw(0, 0);
	s.turn_wheel[unsigned(myfmod(-turnknobang[TK_DIRECTION] * 2.0f, 90.0f)) * TK_PHASES / 90].draw();
	s.direction_ptr.draw(turnknobang[TK_DIRECTION] * 0.5f /* fixme: get angle from player*/);

	ui.draw_infopanel();

	sys().unprepare_2d_drawing();
}
