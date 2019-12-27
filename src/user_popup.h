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

// Base interface for user screen popups.
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef USER_POPUP_H
#define USER_POPUP_H

#include <list>
#include <SDL.h>

class user_popup
{
private:
	// no empty construction, no copy
	user_popup();
	user_popup(user_popup& );
	user_popup& operator= (const user_popup& );

protected:
	// display position. (fixme: could vary for each display - maybe store there)
	unsigned x, y;

	// the display needs to know its parent (user_interface) to access common data
	class user_interface& ui;

	user_popup(class user_interface& ui_) : ui(ui_) {}

public:
	// needed for correct destruction of heirs.
	virtual ~user_popup() {}
	// very basic. Just draw display and handle input.
	virtual void display(class game& gm) const = 0;
	// returns true if event was used
	virtual bool process_input(class game& gm, const SDL_Event& event) = 0;
	virtual void process_input(class game& gm, std::list<SDL_Event>& events)
	{
		std::list<SDL_Event>::iterator it = events.begin();
		while (it != events.end()) {
			if (process_input(gm, *it))
				it = events.erase(it);
			else
				++it;
		}
	}
};

#endif /* USER_POPUP_H */
