// sea objects
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef SEA_OBJECT_H
#define SEA_OBJECT_H

#include "vector3.h"
#include "angle.h"
#include <list>
using namespace std;

#define SINK_SPEED 0.5  // m/sec
#define THROTTLE_SLOW 0.333
#define THROTTLE_HALF 0.5
#define THROTTLE_FULL 0.75
#define THROTTLE_FLANK 1.0
#define MAXPREVPOS 20

class sea_object
{
public:
	enum alive_status { defunct, dead, sinking, alive };
	enum throttle_status { reverse, stop, aheadlisten, aheadsonar, aheadslow,
		aheadhalf, aheadfull, aheadflank };
	enum damage_status { nodamage, lightdamage, mediumdamage, heavydamage, wrecked };
		
	// some useful functions needed for sea_objects

	static double kts2ms(double knots) { return knots*1852.0/3600.0; }
	static double ms2kts(double meters) { return meters*3600.0/1852.0; }
	static double kmh2ms(double kmh) { return kmh/3.6; }
	static double ms2kmh(double meters) { return meters*3.6; }

protected:
	vector3 position;
	angle heading;	// angles 0-359
	double speed, max_speed, max_rev_speed;	// m/sec
	throttle_status throttle;
	double acceleration;
	
	// -1 <= head_chg <= 1
	// if head_chg is != 0 the object is turning with head_chg * turn_rate angles/sec.
	// until heading == head_to or permanently.
	// if head_chg is == 0, nothing happens.
	bool permanent_turn;
	double head_chg;
	angle head_to;
	angle turn_rate;	// in angle/(time*speed) = angle/m
				// this means angle change per forward
				// movement in meters
	double length, width;

	// this should be adapted by heirs.
	damage_status stern_damage, midship_damage, bow_damage;
	// an object is alive until it is sunk or killed.
	// it will sink to some depth then it is killed.
	// it will be dead for exactly one simulation cycle, to give other
	// objects a chance to erase their pointers to this dead object, in the next
	// cycle it will be defunct and erased by its owner.
	alive_status alive_stat;
	
	list<vector2> previous_positions;

	void init_empty(void) {	// init data that is at most constant in the beginning
		speed = 0;
		throttle = stop, acceleration = 0;
		permanent_turn = false;
		head_chg = 0;
		alive_stat = alive;
		stern_damage = midship_damage = bow_damage = nodamage;
	};

	sea_object() {};
	sea_object& operator=(const sea_object& other);
	sea_object(const sea_object& other);
public:
	virtual ~sea_object() {}
	
	virtual void simulate(class game& gm, double delta_time);
	virtual bool is_collision(const sea_object* other);
	// the strength is proportional to damage_status, 0-none, 1-light, 2-medium...
	virtual void damage(const vector3& fromwhere, unsigned strength);
	virtual unsigned calc_damage(void) const;	// returns damage in percent (0 means dead)
	virtual void sink(void);
	virtual void kill(void);
	virtual bool is_defunct(void) const { return alive_stat == defunct; };
	virtual bool is_dead(void) const { return alive_stat == dead; };
	virtual bool is_sinking(void) const { return alive_stat == sinking; };
	virtual bool is_alive(void) const { return alive_stat == alive; };
	
	// command interface
	virtual void head_to_ang(const angle& a, bool left_or_right);	// true == left
	virtual void rudder_left(double amount);	// 0 <= amount <= 1
	virtual void rudder_right(double amount);	// 0 <= amount <= 1
	virtual void rudder_midships(void);
	virtual void set_throttle(throttle_status thr);	// 0 <= amount <= 1
	
	virtual void remember_position(void);
	virtual list<vector2> get_previous_positions(void) const { return previous_positions; };

	vector3 get_pos(void) const { return position; };
	angle get_heading(void) const { return heading; };
	angle get_turn_rate(void) const { return turn_rate; };
	double get_length(void) const { return length; };
	double get_width(void) const { return width; };
	double get_speed(void) const { return speed; };
	virtual double get_max_speed(void) const { return max_speed; };
	virtual double get_throttle_speed(void) const;

	// needed for launching torpedoes
	pair<angle, double> bearing_and_range_to(const sea_object* other) const;
	angle estimate_angle_on_the_bow(angle target_bearing, angle target_heading) const;
	
	virtual void display(void) const {};
};

#endif
