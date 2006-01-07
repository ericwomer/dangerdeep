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

// convoys
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef CONVOY_H
#define CONVOY_H

#include "global_data.h"
#include "ai.h"
#include "vector2.h"
#include <new>
#include <list>
class ship;

///\brief Grouping of ships and other objects with central control.
/** This class stores and manages groups of ships and other objects forming a convoy.
    Ships are listed as escorts, merchants or warships.
    Convoy control is handled via special AI.
*/
class convoy
{
 private:
	convoy();
	convoy(const convoy& other);
	convoy& operator= (const convoy& other);

 protected:
	friend class game; // for initialization	

	std::list<std::pair<ship*, vector2> > merchants, warships, escorts;
	std::list<vector2> waypoints;

	std::auto_ptr<ai> myai;	// fixme: maybe one ship should act for the convoy,
	                        // the ship with the convoy commander.
	                        // when it is sunk, convoy is desorganized etc.

	class game& gm;
	double remaining_time;	// time to next thought/situation analysis, fixme move to ai!
	vector2 position;
	vector2 velocity;	// GLOBAL velocity
	// alive_stat?

	// create empty convoy for loading
	convoy(class game& gm_);

 public:
	enum types { small, medium, large, battleship, supportgroup, carrier };
	enum esctypes { etnone, etsmall, etmedium, etlarge };	// escort size

	convoy(class game& gm, types type_, esctypes esct_);	// create custom convoy
	virtual ~convoy() {}

	void load(const xml_elem& parent);
	void save(xml_elem& parent) const;
	
	unsigned get_nr_of_ships(void) const;

	vector2 get_pos() const { return position; }

	virtual class ai* get_ai(void) { return myai.get(); }
	virtual void simulate(double delta_time);
	virtual void display(void) const {}
	virtual void add_contact(const vector3& pos);
};

#endif
