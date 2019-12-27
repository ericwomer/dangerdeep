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

// Object to display the damage status of a submarine.
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef SUB_DAMAGE_DISPLAY_H
#define SUB_DAMAGE_DISPLAY_H

#include "user_display.h"
#include "texture.h"
#include "vector3.h"
#include "submarine.h"

class sub_damage_display : public user_display
{
	int mx, my;	// last mouse position, needed for popup display
	std::auto_ptr<image> damage_screen_background;
	std::auto_ptr<image> sub_damage_scheme_all;
	texture::ptr repairlight, repairmedium, repairheavy, repaircritical, repairwrecked;
	objcachet<texture>::reference notepadsheet;
public:
	sub_damage_display(class user_interface& ui_);

	virtual void display_popup (int x, int y, const std::string& text, bool atleft, bool atbottom) const;

	virtual void display(class game& gm) const;
	virtual void process_input(class game& gm, const SDL_Event& event);

	void enter(bool is_day);
	void leave();
};

#endif /* SUB_DAMAGE_DISPLAY_H */
