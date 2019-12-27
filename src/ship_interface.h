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

// user interface for controlling a ship
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef SHIP_INTERFACE_H
#define SHIP_INTERFACE_H

#include <list>
#include <vector>
#include "ship.h"
#include "global_data.h"
#include "user_interface.h"
#include "color.h"

///\brief User interface implementation for control of ships.
/** This class handles all the input and output to and from the player and the game if the user
    plays a captain of a ship. */
///\ingroup interfaces
class ship_interface : public user_interface
{
protected:
	ship_interface();
	ship_interface& operator= (const ship_interface& other);
	ship_interface(const ship_interface& other);
	
	// returns true if processed
	bool keyboard_common(int keycode, class game& gm);

	// 2d drawing must be turned on for them
	void draw_gauge(unsigned nr, int x, int y, unsigned wh, angle a,
			const std::string& text) const;
	void draw_vessel_symbol(const vector2& offset, const sea_object* so, color c) const;
	void draw_trail(sea_object* so, const vector2& offset);

	// Display function for screens.
	void display_sonar(class game& gm); // or better display_guns?
	void display_glasses(class game& gm);
	void display_dc_throwers(class game& gm);
	void display_damagestatus(class game& gm);
	
public:	
	ship_interface(ship* player_ship, class game& gm);
	virtual ~ship_interface();

	virtual void display(class game& gm);
};

#endif
