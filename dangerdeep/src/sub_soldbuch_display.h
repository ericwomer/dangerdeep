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

// Object to create and display the number and tonnage of sunk ships.
// subsim (C)+(W) Markus Petermann and Thorsten Jordan. SEE LICENSE

#ifndef SUB_SOLDBUCH_DISPLAY_H
#define SUB_SOLDBUCH_DISPLAY_H

#include "user_display.h"

class sub_soldbuch_display : public user_display
{
protected:
	std::auto_ptr<image> background;
	std::auto_ptr<image> player_photo;

public:
	sub_soldbuch_display(class user_interface& ui_);

	virtual void display(class game& gm) const;
	virtual void process_input(class game& gm, const SDL_Event& event);

	void enter(bool is_day);
	void leave();
};

#endif
