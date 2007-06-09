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
#include "water_splash.h"
#include "model.h"
#include "global_data.h"
#include "user_interface.h"
#include "submarine_interface.h"
#include "airplane_interface.h"
#include "ship_interface.h"
#include "texts.h"
#include "convoy.h"
#include "particle.h"
#include "sensors.h"
#include "network.h"
#include "matrix4.h"
#include "quaternion.h"
using std::pair;
using std::make_pair;
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
	time = date(1939, 9, 1).get_time() + 86400/2;	// 12.00 o'clock
	equipment_date = date(1939, 9, 1);

	// standard sub type, can be changed later
	string subtype = "submarine_VIIc";

	submarine* psub = 0;
	for (unsigned i = 0; i < 1/*nr_of_players*/; ++i) {
		xml_doc doc(data_file().get_filename(subtype));
		doc.load();
		submarine* sub = new submarine(*this, doc.first_child());
		sub->set_skin_layout(model::default_layout);
		sub->init_fill_torpedo_tubes(date(1939, 9, 1) /*currentdate*/);
		sub->manipulate_invulnerability(true);
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



// copied from class game
template<class T> void cleanup(ptrset<T>& s)
{
	for (unsigned i = 0; i < s.size(); ++i) {
		if (s[i] && s[i]->is_defunct()) {
			s.reset(i);
		}
	}
	s.compact();
}



void game_editor::simulate(double delta_t)
{
	// fixme: time should be freezeable, editor should be able to set time to any
	// value between 1939/9/1 and 1945/5/8

	// copied from game
	// check if jobs are to be run
	for (list<pair<double, job*> >::iterator it = jobs.begin(); it != jobs.end(); ++it) {
		it->first += delta_t;
		if (it->first >= it->second->get_period()) {
			it->first -= it->second->get_period();
			it->second->run();
		}
	}

	compute_max_view_dist();
	
	bool record = false;
	if (get_time() >= last_trail_time + TRAIL_TIME) {
		last_trail_time = get_time();
		record = true;
	}

	double nearest_contact = 1e10;

	// fixme: the AI should ignore the player inside the editor...
	// or the game starts and the destroyers start to hunt the player.
	// But this would be realistic on the other hand...

	// pings / trails are not simulated

	// copied from game
	// step 1: check for invalidity of every object and remove
	// defunct objects. do NOT mix simulate() calls with real
	// calls to delete an object.
	cleanup(ships);
	cleanup(submarines);
	cleanup(airplanes);
	cleanup(torpedoes);
	cleanup(depth_charges);
	cleanup(gun_shells);
	cleanup(water_splashes);

	// step 2: simulate all objects, possibly setting state to dead/defunct.
	// ------------------------------ ships ------------------------------
	for (unsigned i = 0; i < ships.size(); ++i) {
		if (ships[i] != player) {
			double dist = ships[i]->get_pos().distance(player->get_pos());
			if (dist < nearest_contact) nearest_contact = dist;
		}
		try {
			ships[i]->simulate(delta_t);
			if (record) ships[i]->remember_position(get_time());
		}
		catch (sea_object::is_dead_exception& ) {
			// nothing to do
		}
	}

	// ------------------------------ submarines ------------------------------
	for (unsigned i = 0; i < submarines.size(); ++i) {
		if (submarines[i] != player) {
			double dist = submarines[i]->get_pos().distance(player->get_pos());
			if (dist < nearest_contact) nearest_contact = dist;
		}
		try {
			submarines[i]->simulate(delta_t);
			if (record) submarines[i]->remember_position(get_time());
		}
		catch (sea_object::is_dead_exception& ) {
			// nothing to do
		}
	}

	// ------------------------------ airplanes ------------------------------
	for (unsigned i = 0; i < airplanes.size(); ++i) {
		if (airplanes[i] != player) {
			double dist = airplanes[i]->get_pos().distance(player->get_pos());
			if (dist < nearest_contact) nearest_contact = dist;
		}
		try {
			airplanes[i]->simulate(delta_t);
		}
		catch (sea_object::is_dead_exception& ) {
			// nothing to do
		}
	}

	// ------------------------------ torpedoes ------------------------------
	for (unsigned i = 0; i < torpedoes.size(); ++i) {
		try {
			torpedoes[i]->simulate(delta_t);
			if (record) torpedoes[i]->remember_position(get_time());
		}
		catch (sea_object::is_dead_exception& ) {
			// nothing to do
		}
	}

	// ------------------------------ depth_charges ------------------------------
	for (unsigned i = 0; i < depth_charges.size(); ++i) {
		try {
			depth_charges[i]->simulate(delta_t);
		}
		catch (sea_object::is_dead_exception& ) {
			// nothing to do
		}
	}

	// ------------------------------ gun_shells ------------------------------
	for (unsigned i = 0; i < gun_shells.size(); ++i) {
		try {
			gun_shells[i]->simulate(delta_t);
		}
		catch (sea_object::is_dead_exception& ) {
			// nothing to do
		}
	}

	// ------------------------------ water_splashes ------------------------------
	for (unsigned i = 0; i < water_splashes.size(); ++i) {
		try {
			water_splashes[i]->simulate(delta_t);
		}
		catch (sea_object::is_dead_exception& ) {
			// nothing to do
		}
	}

	// for convoys/particles it doesn't hurt to mix simulate() with compact().
	// ------------------------------ convoys ------------------------------
	for (unsigned i = 0; i < convoys.size(); ++i) {
		if (!convoys[i]) continue;
		convoys[i]->simulate(delta_t);	// fixme: handle erasing of empty convoys!
	}
	convoys.compact();

	// ------------------------------ particles ------------------------------
	for (unsigned i = 0; i < particles.size(); ++i) {
		if (!particles[i]) continue;
		if (particles[i]->is_defunct()) {
			particles.reset(i);
		} else {
			particles[i]->simulate(*this, delta_t);
		}
	}
	particles.compact();

	// clear pings if there are some
	pings.clear();

	time += delta_t;
}



void game_editor::manipulate_time(double tm)
{
	time = tm;
}



void game_editor::manipulate_equipment_date(date equipdate)
{
	equipment_date = equipdate;
}
