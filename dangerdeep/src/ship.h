// ships
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef SHIP_H
#define SHIP_H

#include "sea_object.h"
#include "global_data.h"

#define SINK_SPEED 0.5  // m/sec, fixme move to ship. include in local drag.
#define MAXPREVPOS 20	//move to ship!

#define NO_AMMO_REMAINING	0	// used by deck gun
#define TARGET_OUT_OF_RANGE	-1	// used by deck gun
#define GUN_FIRED 1
#define RELOADING 2
#define GUN_NOT_MANNED 3
#define GUN_TARGET_IN_BLINDSPOT 4

class game;

class ship : public sea_object
{
	friend class convoy;
	friend class game;	// for initialization

public:
	// give negative values for fixed speeds, positive values for knots.
	enum throttle_status { reverse=-7, aheadlisten=-6, aheadsonar=-5, aheadslow=-4,
		aheadhalf=-3, aheadfull=-2, aheadflank=-1, stop=0  };

	enum rudder_status { rudderfullleft=-2, rudderleft, ruddermidships, rudderright,
		rudderfullright };

protected:
	class ai* myai;
	unsigned tonnage;	// in BRT, created after values from spec file (but maybe with some randomness), must get stored!

	int throttle;		// if < 0: throttle_state, if > 0: knots
	//double max_acceleration;	// read from spec file

	double rudder_pos;	// in degrees, do not use class angle here, we need explicit positive and negative values.
	int rudder_to;		// symbolic pos (full left, left, mid, right, full right), fixme: should be angle too...
	double max_rudder_angle;// maximum turn, e.g. 30 degr.
	double max_rudder_turn_speed;
	double max_angular_velocity;	// depends on turn rate.
	bool head_to_fixed;	// if true, change rudder_to so that heading moves to head_to
	angle head_to;
	
	double turn_rate;	// in angle/time (at max. speed/throttle), read from spec file

	double max_accel_forward;	// stored. can get computed from engine_torque, screw diameter and ship's mass.
	double max_speed_forward;	// stored.
	double max_speed_reverse;	// stored.

	// fixme: replace by finer model: -> damage editor!
	damage_status stern_damage, midship_damage, bow_damage;

	// Fuel percentage: 0 = empty, 1 = full.
	double fuel_level;
	double fuel_value_a;	// read from spec file
	double fuel_value_t;	// read from spec file
	unsigned fuel_capacity;	// read from spec file

	list<vector2> previous_positions;

	unsigned shipclass;	// read from spec file, e.g. warship/merchant/escort/...

	virtual vector3 get_acceleration(void) const;		// drag must be already included!
	virtual double get_turn_acceleration(void) const;	// drag must be already included!

	ship();
	ship(const ship& other);
	ship& operator= (const ship& other);

	/**
		This method calculates the hourly fuel consumption. An
		exponential is used as a model basing on some fuel consumption values.
		@return double hourly percentage fuel consumption value
	*/
	virtual double get_fuel_consumption_rate () const
	{ return fuel_value_a * ( exp ( get_throttle_speed () / fuel_value_t ) - 1.0f ); }

	/**
		This method is called by the simulate method to recalculate the
		actual fuel status.
		@param delta_time time advance since last call
	*/
	virtual void calculate_fuel_factor ( double delta_time );

	unsigned smoke_type;	// 0 - none, 1-x particle type
	vector3 smokerelpos;	// read from spec file

	// pointer to fire particle (0 means ship is not burning)
	class particle* myfire;
	
	// common constructor. set attributes to sane values.
	void init(void);

	virtual bool use_simple_turning_model(void) const { return false; }
	
	virtual bool causes_spray(void) const { return true; }
	
	// some experience values of the crews to fire a grenade with right angle at any
	// target. This depends on canon type (shot speed, min/max angles etc.) so we need
	// several ai classes later.
	static map<double, map<double, double> > dist_angle_relation;
	static void fill_dist_angle_relation_map(const double initial_velocity);
	
	// deck gun
	struct gun_barrel
	{
		double load_time_remaining;
		angle last_elevation;
		angle last_azimuth;
		
		gun_barrel()
		{
			load_time_remaining = 0.0;
			last_elevation = 0;
			last_azimuth = 0;
		}
	};
	
	struct gun_turret
	{
		int num_shells_remaining;
		int shell_capacity;
		double initial_velocity;		
		int max_declination;
		int max_inclination;
		double time_to_man;
		double time_to_unman;
		bool is_gun_manned;
		double manning_time;
		double shell_damage;	
		int start_of_exclusion_radius;
		int end_of_exclusion_radius;
		double calibre;
		
		list<struct gun_barrel> gun_barrels;
		
		gun_turret()
		{
			num_shells_remaining = 0;
			shell_capacity = 0;
			initial_velocity = 0.0;			
			max_declination = 0;
			max_inclination = 0;
			time_to_man = 0.0;
			time_to_unman = 0.0;
			is_gun_manned = false;
			manning_time = 0.0;
			shell_damage = 0.0;	
			start_of_exclusion_radius = 0;
			end_of_exclusion_radius = 0;
			calibre = 0.0;		
		}
	};
	bool gun_manning_is_changing;
	list<struct gun_turret> gun_turrets;
	typedef list<struct gun_turret>::iterator gun_turret_itr;
	typedef list<struct gun_turret>::const_iterator const_gun_turret_itr;
	typedef list<struct gun_barrel>::iterator gun_barrel_itr;
	typedef list<struct gun_barrel>::const_iterator const_gun_barrel_itr;	
	double maximum_gun_range;
	
	bool is_target_in_blindspot(const struct gun_turret *gun, angle bearingToTarget);
	bool calculate_gun_angle(const double distance, angle &elevation, const double initial_velocity);
	void calc_max_gun_range(double initial_velocity);

public:
	enum shipclasses {
		WARSHIP,
		ESCORT,
		MERCHANT,
		SUBMARINE
	};
	
	// create empty object from specification xml file
	ship(class TiXmlDocument* specfile, const char* topnodename = "dftd-ship");
	
	virtual ~ship();

	virtual void load(istream& in, game& g);
	virtual void save(ostream& out, const game& g) const;

	virtual void parse_attributes(class TiXmlElement* parent);

	virtual unsigned get_class(void) const { return shipclass; }

	virtual void simulate(game& gm, double delta_time);

	virtual void sink(void);

	virtual void ignite(game& gm);
	bool is_burning(void) const { return myfire != 0; }

	// command interface
	virtual void fire_shell_at(const vector2& pos);
	virtual void head_to_ang(const angle& a, bool left_or_right);	// true == left
	virtual void change_rudder(int to);	// give -2..2, fixme not yet used as command
	//virtual void set_rudder(angle ang);	// move rudder to this angle
	virtual void rudder_left(void);
	virtual void rudder_right(void);
	virtual void rudder_hard_left(void);
	virtual void rudder_hard_right(void);
	virtual void rudder_midships(void);
	virtual void set_throttle(throttle_status thr);

	virtual void remember_position(void);
	virtual list<vector2> get_previous_positions(void) const { return previous_positions; }

	virtual bool has_smoke(void) const { return smoke_type != 0; }

	virtual bool damage(const vector3& fromwhere, unsigned strength);
	virtual unsigned calc_damage(void) const;	// returns damage in percent (100 means dead)
	virtual class ai* get_ai(void) { return myai; }
	// this depends on ship's tonnage, type, draught and depth (subs/sinking ships)
	virtual double get_roll_factor(void) const;
	virtual unsigned get_tonnage(void) const { return tonnage; }
	virtual double get_fuel_level () const { return fuel_level; }
	virtual angle get_head_to(void) const { return head_to_fixed ? head_to : heading; };
	virtual angle get_turn_rate(void) const { return turn_rate; };
	virtual double get_max_speed(void) const { return max_speed_forward; };
	virtual double get_throttle_speed(void) const;
	virtual double get_throttle_accel(void) const;	// returns acceleration caused by current throttle
	virtual double get_rudder_pos(void) const { return rudder_pos; }
	virtual int get_rudder_to (void) const { return rudder_to; }
	virtual double get_noise_factor (void) const;
	virtual int fire_shell_at(class game& gm, const sea_object& s);

	// needed for launching torpedoes
	pair<angle, double> bearing_and_range_to(const sea_object* other) const;
	angle estimate_angle_on_the_bow(angle target_bearing, angle target_heading) const;
	
	// gun
	virtual bool toggle_gun_manning();
	virtual bool is_gun_manned();
	virtual void gun_manning_changed(bool isGunManned);	
	virtual long num_shells_remaining();
	virtual double max_gun_range() { return maximum_gun_range; };
};

#endif
