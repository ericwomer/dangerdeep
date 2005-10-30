// Simulation of the Torpedo Data Computer (TDC)
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef TDC_H
#define TDC_H

#include "vector3.h"
#include "angle.h"
#include <iostream>

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
	angle additional_leadangle;	// set by the crew, compensation for turning or for spread fire

	// results / internal data
	angle lead_angle;		// fire solution, absolute angle, not relative to course
	double torpedo_runtime;		// time that the torpedo runs before impact
	bool valid_solution;		// only true when fire solution is valid

	void compute_aob(angle br);	// compute and set AoB from target course and bearing br

public:
	tdc();
	void load(std::istream& in);
	void save(std::ostream& out) const;

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
	void set_additional_leadangle(angle ala);

	bool solution_valid() const { return valid_solution; }
	angle get_lead_angle() const { return lead_angle; }
	angle get_bearing() const { return bearing_dial; }
	angle get_angle_on_the_bow() const { return angleonthebow; }
	angle get_target_course() const;	// computed from current Angle on the Bow
	double get_target_distance() const { return target_distance; }
	double get_target_speed() const { return target_speed; }
	double get_torpedo_speed() const { return torpedo_speed; }
	double get_torpedo_runtime() const { return torpedo_runtime; }
	angle get_additional_leadangle() const { return additional_leadangle; }
};

#endif
