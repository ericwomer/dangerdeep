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

// torpedoes
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef TORPEDO_H
#define TORPEDO_H

#include "ship.h"

/*
description and info is per language and class-wide.
read from xml and store somewhere.
for a normal torpedo:
parse xml and give time/fusetypes/type
a function will create a torpedo object from it

per torpedo data:
double weight	(may differ, but is this important? maybe for buoyancy sim)
double diameter (21" on all german torpedoes)
double length   (same for all torps)
double untertrieb (percent of weight?)
double warhead_weight
int warhead_explosive	(varois types, hardcoded)
double arming_distance
ptrlist<fuse>
-> per fuse: int type
=> list<type>
double range_normal    (compute from battery values? no, store also)
double speed_normal
double range_preheated
double speed_preheated
double accel, turnrate (sea-object)
bool has_fat, has_fatII?
bool has_lut, has_lut2?
list<sensor> or list<sensor-type> or just sensor*
double hp
int powertype (battery type?)
date availability

double dud_probability... (for each fuse and for what?) depends on fuse, do not store in xml

implement classes per fuse:
double fail propab, depends on time period
double behaviour on impact etc.

causes of failure:
TZ: disturbed by magnetic influence of the Norway Fjords
G7e: depth keeping equipment failed, so torpedo ran 6 feet to deep (2m), problem with impact pistols
     detected 01/30/1942, official 09/02/1942. the longer the boat is submerged the more probable
     is the failure. 1940 (norwegian campain) there were 30-35% torpedo malfunctions
Pi: angle of impact? unsure
*/

///\brief Represents a torpedo with simulation of it.
/** Different types of prupulsion or warheads are possible.
    Torpedo attributes are defined via specification XML file. */
class torpedo : public ship
{
 public:
	/// data about a torpedo fuse
	class fuse
	{
	public:
		enum types { NONE, IMPACT, INFLUENCE, INERTIAL };
		types type;
		float failure_probability;	// in [0...1]

		fuse() : type(NONE), failure_probability(1.0f) {}
		fuse(const xml_elem& parent, date equipdate);

		// this function computes if the fuse ignites or fails, call it once
		//bool handle_impact(angle impactangle) const;
	};

	/// data about setup of a torpedo while it is still in the tube
	class setup
	{
	public:
		setup();
		unsigned primaryrange;	// 1600...3200, [SAVE]
		unsigned secondaryrange;// 800/1600, [SAVE]
		bool initialturn_left;	// initital turn is left (true) or right (false), [SAVE]
		angle turnangle;	// (0...180 degrees, for LUT, FAT has 180), [SAVE]
		unsigned torpspeed;	// torpspeed (0-2 slow-fast, only for G7a torps), [SAVE]
		double rundepth;	// depth the torpedo should run at, [SAVE]

		void load(const xml_elem& parent);
		void save(xml_elem& parent) const;
	};

	enum warhead_types { Ka, Kb, Kc, Kd, Ke, Kf };

	enum steering_devices { STRAIGHT, FaT, LuTI, LuTII };

	enum propulsion_types { STEAM, ELECTRIC, WALTER };

	// do not change these numbers! must be 0-2 for slow...fast
	enum speedrange_types { SLOW = 0, PREHEATED = 0, MEDIUM = 1, FAST = 2, NORMAL = 2, NR_SPEEDRANGE_TYPES = 3 };

 private:
	torpedo();
	torpedo& operator=(const torpedo& other);
	torpedo(const torpedo& other);

 protected:

	friend class sub_torpsetup_display;	// to set up values... maybe add get/set functions for them

	// -------- computed at creation of object ------------------
	double untertrieb;	// negative buoyancy
	double warhead_weight;	// in kg
	warhead_types warhead_type;
	double arming_distance;	// meters
	fuse contact_fuse, magnetic_fuse;
	double range[NR_SPEEDRANGE_TYPES];
	double speed[NR_SPEEDRANGE_TYPES];
	steering_devices steering_device;
	double hp;		// horse power of engine
	propulsion_types propulsion_type;
	double sensor_activation_distance;	// meters. unused if torp has no sensors.

	// ------------- configured by the player ------------------
	setup mysetup;		// [SAVE]

	// ------------ changes over time by simulation
	double temperature;	// only useful for electric torpedoes, [SAVE]
	double probability_of_rundepth_failure;	// basically high before mid 1942, [SAVE]
	double run_length;	// how long the torpedo has run, [SAVE]

	generic_rudder dive_planes;

	// specific damage here:
//	virtual void create_sensor_array ( types t );
	
	void compute_force_and_torque(vector3& F, vector3& T) const;
	void steering_logic();
	void depth_steering_logic();
	double get_turn_accel_factor() const { return 50.0; } // rudder area etc.
	double get_turn_drag_area() const;
	double get_turn_drag_coeff() const { return 10.0; }
	double get_throttle_speed() const;

	virtual bool causes_spray() const { return false; }//causes wake, only true for steam torpedoes and maybe for Walter engine

	bool detect_other_sea_objects() const { return false; }

public:
	// create empty object from specification xml file
	// create from spec file, select values by date. date is taken from game. fixme: avoid random values here!
	// fixme: avoid that a game startet at date x but played until date y takes torpedo settings
	// from date y instead of x for loading! use a special game::get_equipment_date() function for that...
	torpedo(game& gm_, const xml_elem& parent);

	virtual void load(const xml_elem& parent);
	virtual void save(xml_elem& parent) const;

	virtual void simulate(double delta_time);

	// sets speed to initial speed, sets position
	virtual void launch(const vector3& launchpos, angle parenthdg);

	// depends on warhead, will change with newer damage simulation
	virtual unsigned get_hit_points () const;

	virtual double get_range() const;
	double get_torp_speed() const;

	/// fire fuse and test if it works. depends also on angle to target, to be added later as parameter.
	bool test_contact_fuse() const;

	/// fire fuse and test if it works. depends on distance/angle to target, to be added later as parameter.
	bool test_magnetic_fuse() const;
};

#endif
