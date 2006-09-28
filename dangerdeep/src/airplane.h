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

// airplanes
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef AIRPLANE_H
#define AIRPLANE_H

#include "sea_object.h"
//#include "global_data.h"
#include "quaternion.h"

///\brief Represents an airplane with simulation of it.
///\todo improve the interface
///\todo finish a working implementation
///\todo add airplanes to the simulation
///\todo add steering AI and control AI for airplanes.
class airplane : public sea_object
{
 private:
	airplane();
	airplane& operator=(const airplane& other);
	airplane(const airplane& other);
	
 protected:
	double rollfac, pitchfac;	// rudder state, pitch/roll factor per time.

	bool detect_other_sea_objects() const { return true; }

 public:
	// create empty object from specification xml file
	airplane(game& gm_, const xml_elem& parent);

	virtual void load(const xml_elem& parent);
	virtual void save(xml_elem& parent) const;

	virtual void simulate(double delta_time);

	virtual double get_mass() const { return 4000.0; }	// 4 tons.
	virtual double get_engine_thrust() const { return 20000.0; }
	virtual double get_drag_factor() const { return 0.00005184; }
	virtual double get_antislide_factor() const { return 0.0025; }
	virtual double get_antilift_factor() const { return 0.04; }
	virtual double get_lift_factor() const { return 0.75; }
	virtual double get_roll_deg_per_sec() const { return 90.0; }
	virtual double get_pitch_deg_per_sec() const { return 45.0; }

	// command interface for airplanes
	virtual void roll_left();
	virtual void roll_right();
	virtual void roll_zero();
	virtual void pitch_down();
	virtual void pitch_up();
	virtual void pitch_zero();
};

#endif
