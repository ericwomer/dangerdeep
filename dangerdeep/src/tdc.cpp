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



void tdc::load(const xml_elem& parent)
{
	xml_elem t = parent.child("TDC");
	bearing_tracking = t.attrb("bearing_tracking");
	angleonthebow_tracking = t.attrb("angleonthebow_tracking");
	target_speed = t.attrf("target_speed");
	target_distance = t.attrf("target_distance");
	target_course = angle(t.attrf("target_course"));
	target_bow_is_left = t.attrb("target_bow_is_left");
	angleonthebow = angle(t.attrf("angleonthebow"));
	torpedo_speed = t.attrf("torpedo_speed");
	torpedo_runlength = t.attrf("torpedo_runlength");
	bearing = angle(t.attrf("bearing"));
	bearing_dial = angle(t.attrf("bearing_dial"));
	heading = angle(t.attrf("heading"));
	additional_leadangle = angle(t.attrf("additional_leadangle"));
	lead_angle = angle(t.attrf("lead_angle"));
	torpedo_runtime = t.attrf("torpedo_runtime");
	valid_solution = t.attrb("valid_solution");
}



void tdc::save(xml_elem& parent) const
{
	xml_elem t = parent.add_child("TDC");
	t.set_attr(bearing_tracking, "bearing_tracking");
	t.set_attr(angleonthebow_tracking, "angleonthebow_tracking");
	t.set_attr(target_speed, "target_speed");
	t.set_attr(target_distance, "target_distance");
	t.set_attr(target_course.value(), "target_course");
	t.set_attr(target_bow_is_left, "target_bow_is_left");
	t.set_attr(angleonthebow.value(), "angleonthebow");
	t.set_attr(torpedo_speed, "torpedo_speed");
	t.set_attr(torpedo_runlength, "torpedo_runlength");
	t.set_attr(bearing.value(), "bearing");
	t.set_attr(bearing_dial.value(), "bearing_dial");
	t.set_attr(heading.value(), "heading");
	t.set_attr(additional_leadangle.value(), "additional_leadangle");
	t.set_attr(lead_angle.value(), "lead_angle");
	t.set_attr(torpedo_runtime, "torpedo_runtime");
	t.set_attr(valid_solution, "valid_solution");
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
		// angle is relative to absolute bearing
		// note! should the angle be > 90 degrees, result is wrong,
		// because asin gives -90...90 degrees. This could happen only when
		// target is super fast or very near.
		angle relative_lead_angle = angle::from_rad(asin(sinrelleadangle));
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
		// compute parallax angle (additional angle to lead angle)
		// compute distance from bow (fixme: also from stern!) to impact
		double trd = torpedo_speed * torpedo_runtime;
		// distance center of boat to bow, crude guess
		// 37m center -> bow, 9.5m of straight running of torpedo
		// 95m for radius of turn (a smaller relative lead angle would mean a smaller
		// radius... was this handled in reality??)
		double c = 37 + 9.5 + 95;
		angle gamma = lead_angle - heading;
		double bowdist = sqrt(trd*trd + c*c - 2*trd*c*gamma.cos());
		parallaxangle = angle::from_rad(asin(gamma.sin() * c / bowdist));
/*
		cout << "tgt dist " << target_distance << " trd " << trd << " c " << c << " bowdist " << bowdist
		     << " parang " << parallaxangle.value_pm180() << " sinrell" << sinrelleadangle
		     << " relll " << relative_lead_angle.value_pm180()
		     << " relll2 " << parallax_corrected_lead_angle.value_pm180()
		     << " tau " << tau.value() << "\n";
*/
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
