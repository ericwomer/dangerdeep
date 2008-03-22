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

// user display: submarine's captain's cabin
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef SUB_CAPTAINSCABIN_DISPLAY_H
#define SUB_CAPTAINSCABIN_DISPLAY_H

#include "user_display.h"
#include "submarine.h"
#include "vector2.h"
#include "texture.h"
#include "objcache.h"
#include "color.h"

class sub_captainscabin_display : public user_display
{
	std::auto_ptr<image> background;

	class clickable_area
	{
	protected:
		vector2i topleft;
		vector2i bottomright;
		int description;
		void (sub_captainscabin_display::*action)();
		color desc_color;
	private:
		clickable_area();
	public:
		clickable_area(const vector2i& tl, const vector2i& br,
			       int descr,
			       void (sub_captainscabin_display::*func)(),
			       color dc);
		bool is_mouse_over(int mx, int my) const;
		int get_description() const { return description; }
		void do_action(sub_captainscabin_display& obj);
		color get_description_color() const { return desc_color; }
	};

	std::vector<clickable_area> clickable_areas;

	void goto_successes();
	void goto_logbook();
	void goto_torpedoes();
	void goto_recogmanual();

	int mx, my;

public:
	sub_captainscabin_display(class user_interface& ui_);

	virtual void display(class game& gm) const;
	virtual void process_input(class game& gm, const SDL_Event& event);

	void enter(bool is_day);
	void leave();
};

#endif
