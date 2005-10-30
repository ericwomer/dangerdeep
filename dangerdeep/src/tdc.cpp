// Simulation of the Torpedo Data Computer (TDC)
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "tdc.h"

tdc::tdc()
	: bearing_tracking(true),
	  angleonthebow_tracking(true),
	  target_speed(0),
	  target_distance(0),
	  target_bow_is_left(true),
	  torpedo_speed(0),
	  torpedo_runlength(0),
	  torpedo_runtime(0),
	  valid_solution(false)
{
}



void tdc::load(std::istream& in)
{
	// fixme
}



void tdc::save(std::ostream& out) const
{
	// fixme
}



void tdc::simulate(double delta_t)
{
	// turn bearing dial to set bearing, with max. 2.5 degrees per second
	double bearingturn = 2.5 * delta_t;
	if (bearing_dial.diff(bearing) <= bearingturn) {
		bearing_dial = bearing;
	} else {
		if (!bearing_dial.is_cw_nearer(bearing))
			bearingturn = -bearingturn;
		bearing_dial += angle(bearingturn);
	}

	// AoB tracker: updated by bearing
	if (angleonthebow_tracking) {
		compute_aob(bearing_dial);
	}

	// compute lead angle from data
	// note that the computation does NOT use the target distance.
	// target distance is used only for run time computation
	double sinrelleadangle = target_speed * angleonthebow.sin() / torpedo_speed;
	// note: this value can only be > 1 if target is faster than torpedo and the AoB is bad
	// in that case, just keep old angle, do not "reset" dial to zero (the old analogue
	// technology wouldn't have done that either)
	valid_solution = false;
	if (fabs(sinrelleadangle) <= 1.0) {
		angle relative_lead_angle = asin(sinrelleadangle);
		if (target_bow_is_left) {
			lead_angle = bearing_dial - relative_lead_angle + additional_leadangle;
		} else {
			lead_angle = bearing_dial + relative_lead_angle + additional_leadangle;
		}
		// compute run time from target distance
		angle aob_on_hit = angle(180) - angleonthebow - relative_lead_angle;
		torpedo_runtime = (angleonthebow.sin() * target_distance) / (torpedo_speed * aob_on_hit.sin());
		// check if torpedo can hit target
		if (torpedo_speed * torpedo_runtime <= torpedo_runlength) {
			valid_solution = true;
		}
	}
}



void tdc::enable_bearing_tracker(bool enable)
{
	bearing_tracking = enable;
}



void tdc::enable_angleonthebow_tracker(bool enable)
{
	angleonthebow_tracking = enable;
}



void tdc::set_torpedo_data(double speed, double runlength)
{
	torpedo_speed = speed;
	torpedo_runlength = runlength;
}



void tdc::set_target_speed(double ms)
{
	target_speed = ms;
}



void tdc::set_target_distance(double ms)
{
	target_distance = ms;
}



void tdc::set_bearing(angle br)
{
	bearing = br;
	// Angle on the Bow is now invalid, because bearing has changed.
	// The tracker can correct it though.
}



void tdc::compute_aob(angle br)
{
	angle reverse_bearing = angle(180) + br;
	if (target_course.is_cw_nearer(reverse_bearing)) {
		angleonthebow = reverse_bearing - target_course;
		target_bow_is_left = false;
	} else {
		angleonthebow = target_course - reverse_bearing;
		target_bow_is_left = true;
	}
}



void tdc::set_target_course(angle tc)
{
	target_course = tc;
	// note! if bearing dial is not turned to bearing yet,
	// angle on the bow will be wrong!
	compute_aob(bearing_dial);
}



void tdc::set_heading(angle hd)
{
	heading = hd;
}



void tdc::update_heading(angle hd)
{
	// bearing tracker: updated by submarine's course
	if (bearing_tracking) {
		// add difference to old heading to bearing
		bearing += heading.diff(hd);
	}
	heading = hd;
}



void tdc::set_additional_leadangle(angle ala)
{
	additional_leadangle = ala;
}



angle tdc::get_target_course() const
{
	angle reverse_bearing = angle(180) + bearing_dial;
	if (target_bow_is_left) {
		return reverse_bearing + angleonthebow;
	} else {
		return reverse_bearing - angleonthebow;
	}
}
