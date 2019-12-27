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

#ifndef SUB_GHG_DISPLAY_H
#define SUB_GHG_DISPLAY_H

#include "user_display.h"
#include "image.h"
#include <vector>

class sub_ghg_display : public user_display
{
	class scheme {
	public:
		std::auto_ptr<image> background;
		rotat_tex direction_ptr;
		rotat_tex direction_knob;
		rotat_tex volume_dial;
		fix_tex volume_knob;
		scheme(bool day);
	protected:
		scheme();
		scheme(const scheme& );
		scheme& operator= (const scheme& );
	};

	enum turnknobtype {
		TK_NONE = -1,
		TK_DIRECTION = 0,
		TK_VOLUME = 1,
		TK_NR = 2
	};

	std::auto_ptr<scheme> myscheme;

	turnknobtype turnknobdrag;
	std::vector<float> turnknobang;

 public:
	sub_ghg_display(class user_interface& ui_);

	virtual void process_input(class game& gm, const SDL_Event& event);
	virtual void display(class game& gm) const;

	void enter(bool is_day);
	void leave();
};

#endif
