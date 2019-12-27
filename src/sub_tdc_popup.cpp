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

// Submarine tdc popup.
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "sub_tdc_popup.h"
#include "system.h"
#include "datadirs.h"
#include "game.h"
#include "keys.h"
#include "cfg.h"



sub_tdc_popup::sub_tdc_popup(user_interface& ui_) : user_popup(ui_)
{
	x = 9;
	y = 151;
	background_daylight.reset(new image(get_image_dir() + "popup_control_daylight.jpg|png"));
	background_nightlight.reset(new image(get_image_dir() + "popup_control_redlight.jpg|png"));
}



sub_tdc_popup::~sub_tdc_popup()
{
}



bool sub_tdc_popup::process_input(class game& gm, const SDL_Event& event)
{
	switch (event.type) {
	default: return false;
	}
	return false;
}



void sub_tdc_popup::display(class game& gm) const
{
	sys().prepare_2d_drawing();

	bool is_day = gm.is_day_mode();
	if (is_day)
		background_daylight->draw(x, y);
	else
		background_nightlight->draw(x, y);

	sys().unprepare_2d_drawing();
}
