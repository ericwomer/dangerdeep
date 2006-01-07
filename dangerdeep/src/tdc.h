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

// Simulation of the Torpedo Data Computer (TDC)
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef TDC_H
#define TDC_H

#include "xml.h"

///\brief Simulation of the Torpedo Data Computer.
class tdc
{
public:

private:
	tdc& operator=(const tdc& other);
	tdc(const tdc& other);

protected:
	// tracker switches
	bool bearing_tracking;		// enable bearing tracker
	bool angleonthebow_tracking;	// enable AoB tracker
	bool auto_mode;			// is true when TDC is in automatic mode, this means the crew feds the TDC, otherwise the user.

	// data about the target
	double target_speed;		// m/s
	double target_distance;		// meters
	angle target_course;		// set by the crew, AoB is computed from it
	bool target_bow_is_left;	// if the bow of the target is left of the bearing
	angle angleonthebow;		// computed by target's course, updated by tracker (<=180 deg)

	// data about the torpedo
	double torpedo_speed;		// set by the crew, m/s
	double torpedo_runlength;	// meters

	// data about the sub
	angle bearing;			// initially set by crew, updated by tracker, absolute angle
	angle bearing_dial;		// dial angle, the dial follows the real value with only 2.5deg/sec
	angle heading;			// heading of sub, update this to make the bearing tracker work
	angle parallaxangle;		// additional angle to compensate sub length and torpedo turning
	angle additional_parallaxangle;	// extra angle, is added to tdc computed parallax angle.

	// results / internal data
	angle lead_angle;		// fire solution, absolute angle, not relative to course
	double torpedo_runtime;		// time that the torpedo runs before impact
	bool compute_stern_tube;	// computation is done for stern tube
	bool valid_solution;		// only true when fire solution is valid

	void compute_aob(angle br);	// compute and set AoB from target course and bearing br

public:
	tdc();
	void load(const xml_elem& parent);
	void save(xml_elem& parent) const;

	void simulate(double delta_time);

	void enable_bearing_tracker(bool enable);
	void enable_angleonthebow_tracker(bool enable);
	void set_torpedo_data(double speed, double runlength);
	void set_target_speed(double ms);
	void set_target_distance(double m);
	void set_bearing(angle br);
	void set_target_course(angle tc);	// sets (initial) angle on the bow
	void set_heading(angle hd);
	void update_heading(angle hd);
	void compute_for_stern_tube(bool stern) { compute_stern_tube = stern; }
	void set_additional_parallaxangle(angle ala);
	void set_auto_mode(bool enabled) { auto_mode = enabled; }

	bool solution_valid() const { return valid_solution; }
	angle get_lead_angle() const { return lead_angle; }
	angle get_bearing() const { return bearing_dial; }
	angle get_angle_on_the_bow() const { return angleonthebow; }
	angle get_target_course() const;	// computed from current Angle on the Bow
	double get_target_distance() const { return target_distance; }
	double get_target_speed() const { return target_speed; }
	double get_torpedo_speed() const { return torpedo_speed; }
	double get_torpedo_runtime() const { return torpedo_runtime; }
	angle get_additional_parallaxangle() const { return additional_parallaxangle; }
	angle get_parallax_angle() const { return parallaxangle + additional_parallaxangle; }
	bool auto_mode_enabled() const { return auto_mode; }
};

#endif
