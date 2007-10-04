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

// gun shells
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef GUN_SHELL_H
#define GUN_SHELL_H

#include "sea_object.h"

#define AIR_RESISTANCE 0.05	// factor of velocity that gets subtracted
				// from it to slow the shell down

///\brief Represents a gun shell with simulation of it.
class gun_shell : public sea_object
{
 private:
	gun_shell();
	gun_shell& operator=(const gun_shell& other);
	gun_shell(const gun_shell& other);
 protected:
	vector3 oldpos;		// position at last iteration (for collision detection)
	double damage_amount;

	void check_collision();

 public:
	gun_shell(game& gm_);	// for loading
	gun_shell(game& gm_, const vector3& pos, angle direction, angle elevation,
		double initial_velocity, double damage);	// for creation

	virtual void load(const xml_elem& parent);
	virtual void save(xml_elem& parent) const;

	virtual void simulate(double delta_time);
	virtual void display() const;
	virtual float surface_visibility(const vector2& watcher) const;
	// acceleration is only gravity and already handled by sea_object
	virtual double damage() const { return damage_amount; }
};

#endif
