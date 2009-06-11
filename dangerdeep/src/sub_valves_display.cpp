/*
 * Danger from the Deep - Open source submarine simulation
 * Copyright (C) 2003-2006  Thorsten Jordan, Luis Barrancos and others.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "sub_valves_display.h"

	sub_valves_display::sub_valves_display(class user_interface& ui_) 
		: user_display(ui_)
	{
	}

	void sub_valves_display::display(class game& gm) const
	{

		sys().prepare_2d_drawing();
		background->draw(0, 0);

		ui.draw_infopanel();
		sys().unprepare_2d_drawing();
	}

	void sub_valves_display::process_input(class game& gm, const SDL_Event& event) {
	}

	void sub_valves_display::enter(bool is_day)
	{
		background.reset(new image(get_image_dir() + "valves_screen_"
					   + (is_day ? "daylight" : "redlight") + "_t7cv1.jpg"));
	}

	void sub_valves_display::leave()
	{
		background.reset();
	}

