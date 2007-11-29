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
#include "sub_soldbuch_display.h"
#include "texts.h"
#include "submarine_interface.h"
#include "global_data.h"
#include <sstream>
#include <fstream>
using namespace std;



sub_soldbuch_display::sub_soldbuch_display(user_interface& ui_) :
	user_display(ui_)
{
}



void sub_soldbuch_display::display(class game& gm) const
{
	// draw display without display color.
	glColor4f(1,1,1,1);
	// draw background
	sys().prepare_2d_drawing();

	background->draw(0, 0);

	// render player info here..
	const game::player_info& pi = gm.get_player_info();
	player_photo->draw(250, 200);
	font_vtremington12->print(300, 600, pi.name, color(20, 20, 30));
	std::string fn = texts::get(164);
	fn.replace(fn.find("#"), 1, str(pi.flotilla));
	font_vtremington12->print(300, 630, fn, color(20, 20, 30));
	font_vtremington12->print(300, 660, pi.submarineid, color(20, 20, 30));
	
	ui.draw_infopanel();
	sys().unprepare_2d_drawing();
}



void sub_soldbuch_display::process_input(class game& gm, const SDL_Event& event)
{
//	submarine* sub = dynamic_cast<submarine*>(gm.get_player());

	switch (event.type) {
	case SDL_MOUSEBUTTONDOWN:
#if 0
		// check if there is a clickable area below the mouse and take some action
		mx = event.button.x;
		my = event.button.y;
		if (event.button.button == SDL_BUTTON_LEFT) {
			// fixme:
		} else if (event.button.button == SDL_BUTTON_WHEELUP) {
		} else if (event.button.button == SDL_BUTTON_WHEELDOWN) {
		}
#endif
		break;
	case SDL_MOUSEBUTTONUP:
#if 0
		// check if there is a clickable area below the mouse and take some action
		mx = event.button.x;
		my = event.button.y;
		if (event.button.button == SDL_BUTTON_LEFT) {
			for (vector<clickable_area>::iterator it = clickable_areas.begin();
			     it != clickable_areas.end(); ++it) {
				if (it->is_mouse_over(mx, my)) {
					it->do_action(*this);
					break;
				}
			}
		}
#endif
		break;
	case SDL_MOUSEMOTION:
#if 0
		mx = event.motion.x;
		my = event.motion.y;
#endif
		break;
	default:
		break;
	}
}



void sub_soldbuch_display::enter(bool is_day)
{
	background.reset(new image(get_image_dir() + "soldbuchscreen_background.jpg"));
	player_photo.reset(new image(get_image_dir() + "photo_stamp" + ui.get_game().get_player_info().photo + ".jpg|png"));
}



void sub_soldbuch_display::leave()
{
	background.reset();
	player_photo.reset();
}
