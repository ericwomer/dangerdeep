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

// user display: submarine's ghg (Gruppenhorchgerät) hearing device
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "sub_ghg_display.h"
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

sub_ghg_display::sub_ghg_display(user_interface& ui_)
	: user_display(ui_)
{
	daylight.background.reset(new image(get_image_dir() + "GHG_daylight_background.jpg|png"));
	redlight.background.reset(new image(get_image_dir() + "GHG_redlight_background.jpg|png"));

	daylight.direction_ptr.set("GHG_daylight_pointer.png", 242, 89, 376, 372);
	redlight.direction_ptr.set("GHG_redlight_pointer.png", 242, 89, 376, 372);
	daylight.direction_knob.set("GHG_daylight_knob.png", 306, 516, 373, 583);
	redlight.direction_knob.set("GHG_redlight_knob.png", 306, 516, 373, 583);
	daylight.volume_dial.set("GHG_daylight_volume_dial.png", 711, 470, 849, 605);
	redlight.volume_dial.set("GHG_redlight_volume_dial.png", 711, 470, 849, 605);
	daylight.volume_knob.set("GHG_daylight_volumeknob.png", 783, 550);
	redlight.volume_knob.set("GHG_redlight_volumeknob.png", 783, 550);
}



void sub_ghg_display::process_input(class game& gm, const SDL_Event& event)
{
	submarine* sub = dynamic_cast<submarine*>(gm.get_player());
	bool is_day = gm.is_day_mode();
	int mx, my;
	submarine_interface& si = dynamic_cast<submarine_interface&>(ui);

	const scheme& s = (is_day) ? daylight : redlight;

	switch (event.type) {
	case SDL_MOUSEBUTTONDOWN:
		{
			mx = event.button.x;
			my = event.button.y;
		}
		break;
	case SDL_MOUSEMOTION:
		if (event.motion.state & SDL_BUTTON_LMASK) {
			mx = event.button.x;
			my = event.button.y;
		}
		break;
	default:
		break;
	}

/*
	switch (event.type) {
	case SDL_KEYDOWN:
		//fixme
	default: break;
	}
*/

/*
	if (event.type == SDL_KEYDOWN) {
		if (cfg::instance().getkey(KEY_TOGGLE_POPUP).equal(event.key.keysym)) {
			show_screen1 = !show_screen1;
		}
	}
*/
}



void sub_ghg_display::display(class game& gm) const
{
	submarine* player = dynamic_cast<submarine*>(gm.get_player());

	sys().prepare_2d_drawing();
	glColor3f(1,1,1);

	// determine time of day
	bool is_day = gm.is_day_mode();

	// get hearing device angle from submarine, if it has one

	const scheme& s = (is_day) ? daylight : redlight;

	s.volume_dial.draw(0 /* fixme angle for volume*/);
	s.background->draw(0, 0);
	s.volume_knob.draw();
	s.direction_ptr.draw(0 /* fixme angle*/);
	s.direction_knob.draw(0 /* fixme angle*/);

	sys().unprepare_2d_drawing();
}
