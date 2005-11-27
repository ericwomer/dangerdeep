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

class torpedo : public ship
{
 public:
	struct fuse {
		enum models { Pi1, Pi2, Pi3, Pi4a, Pi4b, Pi4c, Pi6, TZ3, TZ5, TZ6 };
		enum types { IMPACT, INFLUENCE, INERTIAL };
		models model;
		types type;
		float failure_probability;	// in [0...1]
		// this function computes if the fuse ignites or fails, call it once
		bool handle_impact(angle impactangle) const;
	};

	enum warhead_types { Ka, Kb, Kc, Kd, Ke, Kf };

	enum steering_devices { STRAIGHT, FaT, LuTI, LuTII };

	enum propulsion_types { STEAM, ELECTRIC, WALTER };

 private:
	torpedo();
	torpedo& operator=(const torpedo& other);
	torpedo(const torpedo& other);

 protected:

	friend class sub_torpsetup_display;	// to set up values... maybe add get/set functions for them

	// -------- computed at creation of object ------------------
	double mass;		// in kg
	double untertrieb;	// negative buoyancy
	double warhead_weight;	// in kg
	warhead_types warhead_type;
	double arming_distance;	// meters
	std::list<fuse> fuses;
	double range_normal;
	double speed_normal;
	double range_preheated;
	double speed_preheated;
	steering_devices steering_device;
	double hp;		// horse power of engine
	propulsion_types propulsion_type;
	double sensor_activation_distance;	// meters. unused if torp has no sensors.

	// ------------- configured by the player ------------------
	unsigned primaryrange;	// 1600...3200, [SAVE]
	unsigned secondaryrange;// 800/1600, [SAVE]
	bool initialturn_left;	// initital turn is left (true) or right (false), [SAVE]
	angle turnangle;	// (0...180 degrees, for LUT, FAT has 180), [SAVE]
	unsigned torpspeed;	// torpspeed (0-2 slow-fast, only for G7a torps), [SAVE]
	double rundepth;	// depth the torpedo should run at, [SAVE]

	// ------------ changes over time by simulation
	double temperature;	// only useful for electric torpedoes, [SAVE]
	double probability_of_rundepth_failure;	// basically high before mid 1942, [SAVE]
	double run_length;	// how long the torpedo has run, [SAVE]


	// specific damage here:
//	virtual void create_sensor_array ( types t );
	
	bool use_simple_turning_model() const { return true; }

	// torpedoes have constant speed... for now
	virtual vector3 get_acceleration() const { return vector3(); }

	virtual bool causes_spray() const { return false; }//causes wake, only true for steam torpedoes and maybe for Walter engine

public:
	// create empty object from specification xml file
	// create from spec file, select values by date. date is taken from game. fixme: avoid random values here!
	// fixme: avoid that a game startet at date x but played until date y takes torpedo settings
	// from date y instead of x for loading! use a special game::get_equipment_date() function for that...
	torpedo(game& gm_, const xml_elem& parent);

	virtual void load(const xml_elem& parent);
	virtual void save(xml_elem& parent) const;

	// additional FAT/LUT values as indices (0-16,0-1,0-1,0-180,0-2,0-25) fixme
	void set_steering_values(unsigned primrg, unsigned secrg, bool initurnleft, angle turnang,
				 unsigned tspeedsel, double rdepth);
	
	virtual void simulate(double delta_time);

	// sets speed to initial speed, sets position
	virtual void launch(const vector3& launchpos, angle parenthdg);

#if 0 // obsolete!!!!!!!!!!
	// compute gyro lead angle and expected run time of torpedo
	// obsolete, done by TDC now
	static pair<angle, bool> lead_angle(types torptype, double target_speed,
		angle angle_on_the_bow);
	static double expected_run_time(types torptype, angle lead_angle,
	        angle angle_on_the_bow, double target_range);

	// can torpedo gyroscope be adjusted to this target?
	// note that parent is a ship, no sea_object. airplanes can also launch torpedoes
	// but target computing would be done in a different way.
	// fixme: this function assumes that torpedoes are launched from bow or stern
	// what is not always right for ships.
	// obsolete should be computed by TDC sim
	static pair<angle, bool> compute_launch_data(types torptype, const ship* parent,
		const sea_object* target, bool usebowtubes, const angle& manual_lead_angle);
#endif

	// depends on warhead, will change with newer damage simulation
	virtual unsigned get_hit_points () const;

	virtual double get_range() const;
	virtual double get_speed() const;
};

#endif
