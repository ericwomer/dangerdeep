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

// interface for controlling an airplane
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef AIRPLANE_INTERFACE_H
#define AIRPLANE_INTERFACE_H

#include <list>
#include <vector>
using namespace std;
#include "airplane.h"
#include "global_data.h"
#include "user_interface.h"
#include "color.h"

///\brief User interface implementation for control of airplanes.
/** This class handles all the input and output to and from the player and the game if the user
    plays a pilot of an airplane. */
///\ingroup interfaces
class airplane_interface : public user_interface
{
protected:
	airplane_interface();
	airplane_interface& operator= (const airplane_interface& other);
	airplane_interface(const airplane_interface& other);
	
	// returns true if processed
	virtual bool keyboard_common(int keycode, class game& gm);

	// Display functions for screens.
	virtual void display_cockpit(class game& gm);

	virtual void display_damagestatus(game& gm) {}

public:	
	airplane_interface(airplane* player_plane, class game& gm);
	virtual ~airplane_interface();

	virtual void display(class game& gm);
};

#endif
