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

static const double TK_ANGFAC = 360.0/512.0;

sub_ghg_display::scheme::scheme(bool day)
{
	const string x = day ? "GHG_daylight" : "GHG_redlight";
	background.reset(new image(get_image_dir() + x + "_background.jpg|png"));
	direction_ptr.set(x + "_pointer.png", 242, 89, 376, 372);
	direction_knob.set(x + "_knob.png", 306, 516, 373, 583);
	volume_dial.set(x + "_volume_dial.png", 711, 470, 849, 605);
	volume_knob.set(x + "_volumeknob.png", 783, 550);
}



sub_ghg_display::sub_ghg_display(user_interface& ui_)
	: user_display(ui_), turnknobdrag(TK_NONE), turnknobang(TK_NR)
{
}



void sub_ghg_display::process_input(class game& gm, const SDL_Event& event)
{
	int mx, my, mb;

	// fixme: errors like this are rather bug indicators. they should throw a special
	// exception or rather use an assert like thing etc. same for many other screens.
	if (!myscheme.get()) throw error("sub_ghg_display::process_input without scheme!");
	const scheme& s = *myscheme;

	switch (event.type) {
	case SDL_MOUSEBUTTONDOWN:
		mx = event.button.x;
		my = event.button.y;
		// check if mouse is over turn knobs
		turnknobdrag = TK_NONE;
		if (s.direction_knob.is_mouse_over(mx, my, 64)) {
			turnknobdrag = TK_DIRECTION;
		} else if (s.volume_knob.is_mouse_over(mx, my)) {
			turnknobdrag = TK_VOLUME;
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
					// 0-360*4 degrees match to 0-360
					ang = myclamp(ang, -320.0f, 320.0f);
					//sub->set_ghg_direction(ang*0.5); // fixme: set angle of player
					break;
				case TK_VOLUME:
					// 0-288 degrees match to 5-85 degrees angle
					ang = myclamp(ang, 0.0f, 252.0f);
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



void sub_ghg_display::display(class game& gm) const
{
	sys().prepare_2d_drawing();

	// get hearing device angle from submarine, if it has one

	if (!myscheme.get()) throw error("sub_ghg_display::display without scheme!");
	const scheme& s = *myscheme;

	s.volume_dial.draw(-turnknobang[TK_VOLUME]-18.0f);
	s.background->draw(0, 0);
	s.volume_knob.draw();
	s.direction_ptr.draw(turnknobang[TK_DIRECTION]*0.5 /* fixme: get angle from player*/);
	s.direction_knob.draw(turnknobang[TK_DIRECTION]);

	// test hack: test signal strengths
// 	angle sonar_ang = angle(turnknobang[TK_DIRECTION]*0.5) + player->get_heading();
// 	vector<double> noise_strengths = gm.sonar_listen_ships(player, sonar_ang);
// 	printf("noise strengths, global ang=%f, L=%f M=%f H=%f U=%f\n",
// 	       sonar_ang.value(), noise_strengths[0], noise_strengths[1], noise_strengths[2], noise_strengths[3]);

	ui.draw_infopanel();

	sys().unprepare_2d_drawing();
}



void sub_ghg_display::enter(bool is_day)
{
	myscheme.reset(new scheme(is_day));
}



void sub_ghg_display::leave()
{
	myscheme.reset();
}
