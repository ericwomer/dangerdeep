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

// game editor
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#ifdef USE_NATIVE_GL
#include <gl.h>
#else
#include "oglext/OglExt.h"
#endif
#include <SDL.h>

#include "system.h"
#include <sstream>
#include <float.h>

#include "game_editor.h"
#include "ship.h"
#include "submarine.h"
#include "airplane.h"
#include "torpedo.h"
#include "depth_charge.h"
#include "gun_shell.h"
#include "model.h"
#include "global_data.h"
#include "user_interface.h"
#include "submarine_interface.h"
#include "airplane_interface.h"
#include "ship_interface.h"
#include "tokencodes.h"
#include "texts.h"
#include "convoy.h"
#include "particle.h"
#include "sensors.h"
#include "network.h"
#include "matrix4.h"
#include "quaternion.h"

using std::list;
using std::vector;
using std::string;

const unsigned SAVEVERSION = 1;
const unsigned GAMETYPE = 0;//fixme, 0-mission , 1-patrol etc.

/***************************************************************************/

game_editor::game_editor()
{
	networktype = 0;
	servercon = 0;
	ui = 0;
	time = date(1939, 9, 1).get_time() + 86400/2;	// 12.00 o'clock
	equipment_date = date(1939, 9, 1);

	// standard sub type, can be changed later
	string subtype = "submarine_VIIc";

	submarine* psub = 0;
	for (unsigned i = 0; i < 1/*nr_of_players*/; ++i) {
		xml_doc doc(get_submarine_dir() + subtype + ".xml");
		doc.load();
		submarine* sub = new submarine(*this, doc.first_child());
		sub->init_fill_torpedo_tubes(date(1939, 9, 1) /*currentdate*/);
		if (i == 0) {
			psub = sub;
			player = psub;
			compute_max_view_dist();
		}

		spawn_submarine(sub);
	}
	player = psub;

	my_run_state = running;
	last_trail_time = time - TRAIL_TIME;
}



// --------------------------------------------------------------------------------
//                        LOAD GAME (SAVEGAME OR MISSION)
// --------------------------------------------------------------------------------
game_editor::game_editor(const string& filename)
	: game(filename)
{
	// nothing special for now
}




/*
game_editor::~game_editor()
{
}
*/



void game_editor::simulate(double delta_t)
{
	// fixme: time should be freezeable, editor should be able to set time to any
	// value between 1939/9/1 and 1945/5/8

	game::simulate(delta_t);

	// reset run state, so that game doesn't end because there is only the player sub or so
	my_run_state = running;
}



// main play loop
// fixme: a bit misplaced here, especially after ui was moved away from game
game::run_state game_editor::exec()
{
	// nothing special for now
	return game::exec();
}
