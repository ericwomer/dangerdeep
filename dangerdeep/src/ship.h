// ships
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef SHIP_H
#define SHIP_H

#include "sea_object.h"
#include "global_data.h"

#define SINK_SPEED 0.5  // m/sec, fixme move to ship. include in local drag.
#define MAXPREVPOS 20	//move to ship!

class ship : public sea_object
{
	friend class convoy;
	friend class game;	// for initialization

public:
	// give negative values for fixed speeds, positive values for knots.
	enum throttle_status { reverse=-7, aheadlisten=-6, aheadsonar=-5, aheadslow=-4,
		aheadhalf=-3, aheadfull=-2, aheadflank=-1, stop=0  };

	enum rudder_status { rudderfullleft, rudderleft, ruddermid, rudderright,
		rudderfullright };

protected:
	class ai* myai;
	unsigned tonnage;	// in BRT, created after values from spec file (but maybe with some randomness), must get stored!

	angle heading;		// stored additionally, take (0,1,0), rotate with orientation, project to xy-plane, get angle -> heading
	double speed;		// m/sec, speed along local y axis, can be computed from velocity,orientation.
	//double max_speed, max_rev_speed;// m/sec, read from spec file
	int throttle;
	double max_acceleration;	// read from spec file

	// new model: store rudder angle, store if ship heads to a fixed angle and that angle
	bool head_to_fixed;
	angle rudder_pos;
	int rudder_to;		// symbolic pos (full left, left, mid, right, full right)
	double max_rudder_angle;// maximum turn, e.g. 30 degr.
	double max_angular_velocity;	// depends on turn rate.
	bool permanent_turn;	// if true, ignore head_to and move rudder_pos to rudder_to
	angle head_to;
	
	angle turn_rate;	// in angle/(time*speed) = angle/m, means angle change per forward movement in meters, read from spec file

	double max_accel_forward;	// stored. can get computed from engine_torque, screw diameter and ship's mass.
	double max_speed_forward;	// stored.
	double max_speed_reverse;	// stored.
	double max_turn_speed;		// angular velocity. computed from turn_rate.

	// fixme: replace by finer model: -> damage editor!
	damage_status stern_damage, midship_damage, bow_damage;

	// Fuel percentage: 0 = empty, 1 = full.
	double fuel_level;
	double fuel_value_a;	// read from spec file
	double fuel_value_t;	// read from spec file
	unsigned fuel_capacity;	// read from spec file

	list<vector2> previous_positions;

	unsigned shipclass;	// read from spec file, e.g. warship/merchant/escort/...

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

	virtual void change_rudder (int dir);
	
	class smoke_stream* mysmoke;
	vector3 smokerelpos;	// read from spec file
	
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

	virtual void load(istream& in, class game& g);
	virtual void save(ostream& out, const class game& g) const;

	virtual void parse_attributes(class TiXmlElement* parent);

	virtual unsigned get_class(void) const { return shipclass; }

	virtual void simulate(class game& gm, double delta_time);

	virtual void sink(void);

	// command interface
	virtual void fire_shell_at(const vector2& pos);
	virtual void head_to_ang(const angle& a, bool left_or_right);	// true == left
	virtual void rudder_left(void);
	virtual void rudder_right(void);
	virtual void rudder_hard_left(void);
	virtual void rudder_hard_right(void);
	virtual void rudder_midships(void);
	virtual void set_throttle(throttle_status thr);

	virtual void remember_position(void);
	virtual list<vector2> get_previous_positions(void) const { return previous_positions; }

	virtual bool has_smoke(void) const { return mysmoke != 0; }
	virtual void smoke_display(double degr) const;

	virtual bool damage(const vector3& fromwhere, unsigned strength);
	virtual unsigned calc_damage(void) const;	// returns damage in percent (100 means dead)
	virtual class ai* get_ai(void) { return myai; }
	// this depends on ship's tonnage, type, draught and depth (subs/sinking ships)
	virtual double get_roll_factor(void) const;
	virtual unsigned get_tonnage(void) const { return tonnage; }
	virtual double get_fuel_level () const { return fuel_level; }
	virtual angle get_heading(void) const { return heading; };
	virtual angle get_head_to(void) const { return permanent_turn ? heading : head_to; };
	virtual angle get_turn_rate(void) const { return turn_rate; };
	virtual double get_speed(void) const { return speed; };
	virtual double get_max_speed(void) const { return max_speed_forward; };
	virtual double get_throttle_speed(void) const;
	virtual int get_rudder_to (void) const { return rudder_to; }

	// needed for launching torpedoes
	pair<angle, double> bearing_and_range_to(const sea_object* other) const;
	angle estimate_angle_on_the_bow(angle target_bearing, angle target_heading) const;
};

#endif
