// torpedoes
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "torpedo.h"
#include "global_data.h"
#include "model.h"
#include "system.h"
#include "game.h"
#include "sensors.h"
#include "submarine.h"
using std::string;



torpedo::torpedo(game& gm, const xml_elem& parent)
	: ship(gm, parent),
	  primaryrange(1600),
	  secondaryrange(800),
	  initialturn_left(true),
	  turnangle(180),
	  torpspeed(0),	// 0-2 slow-fast, only for G7a torps
	  rundepth(3),
	  temperature(15),	// degrees C
	  probability_of_rundepth_failure(0.2),	// basically high before mid 1942, fixme
	  run_length(0)
{
	mass = parent.child("weight").attrf();
	untertrieb = parent.child("untertrieb").attrf();
	xml_elem ewarhead = parent.child("warhead");
	warhead_weight = ewarhead.attrf("weight");
	string charge = ewarhead.attr("charge");
	if (charge == "Ka") {
		warhead_type = Ka;
	} else if (charge == "Kb") {
		warhead_type = Kb;
	} else if (charge == "Kc") {
		warhead_type = Kc;
	} else if (charge == "Kd") {
		warhead_type = Kd;
	} else if (charge == "Ke") {
		warhead_type = Ke;
	} else if (charge == "Kf") {
		warhead_type = Kf;
	} else {
		// fixme: charges are atm numbers, should be replaced later...
		warhead_type = Ka;
		//throw error(string("unknown charge type ")+charge);
	}
	xml_elem earming = parent.child("arming");
	xml_elem efuse = parent.child("fuse");
	xml_elem emotion = parent.child("motion");
	xml_elem epower = parent.child("power");
	xml_elem eavailability = parent.child("availability");

	// fixme: finish loading code!

	for (xml_elem::iterator it = earming.iterate("period"); !it.end(); it.next()) {
		xml_elem eperiod = it.elem();
		eperiod.attru("from");
		eperiod.attru("until");
		run_length = eperiod.attru("runlength");
	}
}

	
#if 0 // fixme: to fire a torp, let the TDC set the values while torpedo is in tube (stored!)
//and then spawn it in game, so that torp::simulate() is called...
torpedo::torpedo(game& gm_, sea_object* parent, torpedo::types type_, bool usebowtubes, angle headto_,
		 const tubesetup& stp) : ship(gm_), temperature(20.0 /* fixme*/ )
{
	type = type_;
	primaryrange = (stp.primaryrange <= 16) ? 1600+stp.primaryrange*100 : 1600;
	secondaryrange = (stp.secondaryrange & 1) ? 1600 : 800;
	initialturn = stp.initialturn;
	turnangle = stp.turnangle;
	torpspeed = stp.torpspeed;
	rundepth = stp.rundepth;

	position = parent->get_pos();
	heading = parent->get_heading();
	if (!usebowtubes) heading += angle(180);
	vector2 dp = heading.direction() * (parent->get_length()/2 + 3.5);
	position.x += dp.x;
	position.y += dp.y;
	head_to_ang(headto_, !heading.is_cw_nearer(headto_));
	// fixme: simulate variable speeds of T1?
	// fixme: simulate effect of magnetic influence fuse (much greater damage)
	turn_rate = 1.0f;	// most submarine simulations seem to ignore this
			// launching a torpedo will cause it to run in target direction
			// immidiately instead of turning there from the sub's heading
			// fixme historic values??
	size3d = vector3f(0.533, 7, 0.533);	// diameter 53.3cm (21inch), length ~ 7m
		//fixme: retrieve from model file referenced in xml file
		// preheated torpedoes hat longer ranges... this needs to be simulated, too
		// look at: http://www.uboat.net/technical/torpedoes.htm
		// and: http://www.uboat.net/history/torpedo_crisis.htm
		// and: http://www.u-boot-greywolf.de/utorpedo.htm
	run_length = 0;
	max_speed_forward = velocity.y = get_speed_by_type(type_);
	max_speed_reverse = 0;
	switch (type_) {
		case T1:
			max_run_length = 14000;
			influencefuse = false;
			break;
		case T2:
			max_run_length = 5000;
			influencefuse = false;
			break;
		case T3:
			max_run_length = 5000;
			influencefuse = true;
			break;
		case T3a:
			max_run_length = 7500;
			influencefuse = true;
			break;
		case T4:
			max_run_length = 7500;
			// fixme: Falke sensor
			set_sensor ( passive_sonar_system, new passive_sonar_sensor (
				passive_sonar_sensor::passive_sonar_type_tt_t5 ) );
			influencefuse = true;
			break;
		case T5:
			max_run_length = 5700;
			set_sensor ( passive_sonar_system, new passive_sonar_sensor (
				passive_sonar_sensor::passive_sonar_type_tt_t5 ) );
			influencefuse = true;
			break;
		case T11:
			max_run_length = 5700;
			set_sensor ( passive_sonar_system, new passive_sonar_sensor (
				passive_sonar_sensor::passive_sonar_type_tt_t11 ) );
			influencefuse = true;
			break;
		case T1FAT:
			max_run_length = 14000;
			influencefuse = false;
			break;
		case T3FAT:
			max_run_length = 7500;
			influencefuse = true;
			break;
		case T6LUT:
			max_run_length = 7500;
			influencefuse = true;
			break;
	};
	throttle = aheadflank;

	// set ship turning values
	max_rudder_angle = 40;
	max_rudder_turn_speed = 200;//20;	// with smaller values torpedo course oscillates. damping too high?! steering to crude?! fixme
	max_angular_velocity = 18;	// ~ 5 seconds for 90 degree turn (50m radius circle with 30 knots)
	turn_rate = 1; // ? is this needed somewhere?!
	max_accel_forward = 1;

	sys().add_console("torpedo created");
}
#endif


void torpedo::load(const xml_elem& parent)
{
	sea_object::load(parent);
	xml_elem stg = parent.child("settings");
	primaryrange = stg.attru("primaryrange");
	secondaryrange = stg.attru("secondaryrange");
	initialturn_left = stg.attrb("initialturn_left");
	turnangle = angle(stg.attrf("turnangle"));
	torpspeed = stg.attru("torpspeed");
	rundepth = stg.attrf("rundepth");
	temperature = parent.child("temperature").attrf();
	probability_of_rundepth_failure = parent.child("probability_of_rundepth_failure").attrf();
	run_length = parent.child("run_length").attrf();
}



void torpedo::save(xml_elem& parent) const
{
	sea_object::save(parent);
	xml_elem stg = parent.add_child("settings");
	stg.set_attr(primaryrange, "primaryrange");
	stg.set_attr(secondaryrange, "secondaryrange");
	stg.set_attr(initialturn_left, "initialturn_left");
	stg.set_attr(turnangle.value(), "turnangle");
	stg.set_attr(torpspeed, "torpspeed");
	stg.set_attr(rundepth, "rundepth");
	parent.add_child("temperature").set_attr(temperature);
	parent.add_child("probability_of_rundepth_failure").set_attr(probability_of_rundepth_failure);
	parent.add_child("run_length").set_attr(run_length);
}



void torpedo::set_steering_values(unsigned primrg, unsigned secrg,
				  bool initurnleft, angle turnang,
				  unsigned tspeedsel, double rdepth)
{
	primaryrange = primrg;	// fixme: multiply? ...
	secondaryrange = secrg;
	initialturn_left = initurnleft;
	turnangle = turnang;
	torpspeed = tspeedsel;
	rundepth = rdepth;
}



void torpedo::simulate(double delta_time)
{
//	cout << "torpedo " << this << " heading " << heading.value() << " should head to " << head_to.value() << " turn speed " << turn_velocity << "\n";

	ship::simulate(delta_time);
	if (is_defunct() || is_dead()) return;

	double old_run_length = run_length;
	run_length += get_speed() * delta_time;
	if (run_length > get_range()) {
		// later: simulate slow sinking to the ground...
		destroy();
		return;
	}

	// Torpedo starts to search for a target when the minimum save
	// distance for the warhead is passed.
	if (!sensors.empty() && run_length >= sensor_activation_distance) {
		ship* target = gm.sonar_acoustical_torpedo_target ( this );
		if (target) {
			angle targetang(target->get_engine_noise_source() - get_pos().xy());
			bool turnright = get_heading().is_cw_nearer(targetang);
			head_to_ang(targetang, !turnright);
		}
	}

	if (steering_device != STRAIGHT) {
		unsigned old_phase = unsigned(floor((old_run_length < primaryrange) ? old_run_length/primaryrange : 1.0+(old_run_length - primaryrange)/secondaryrange));
		unsigned phase = unsigned(floor((run_length < primaryrange) ? run_length/primaryrange : 1.0+(run_length - primaryrange)/secondaryrange));
		if (phase > old_phase) {
			// phase change.
			if (phase == 1) {
				// first turn. Differences between LuT and FaT,
				// FaT always 180 degrees, LuT variable. Angle is stored, use that
				angle turn = initialturn_left ? -turnangle : turnangle;
				head_to_ang(get_heading() + turn, initialturn_left);
			} else {
				// further turns, always 180 degrees.
				bool turn_is_left = initialturn_left ? ((phase & 1) != 0) : ((phase & 1) == 0);
				angle turn = turn_is_left ? -180 : 180;
				head_to_ang(get_heading() + turn, initialturn_left);
			}
		}
	}

	// check for collisions with other subs or ships
	if (run_length > 10) {	// avoid collision with parent after initial creation
		bool runlengthfailure = (run_length < arming_distance);
		bool failure = false;	// calculate additional probability of torpedo failure
		//fixme: depends on fuse. compute this once and not for every test...
		if (gm.check_torpedo_hit(this, runlengthfailure, failure))
			destroy();
	}
}



#if 0 // obsolete!!!!!!!!!!!!!!!!!!!!!!!!
pair<angle, bool> torpedo::lead_angle(torpedo::types torptype, double target_speed, angle angle_on_the_bow)
{
	double sla = target_speed*angle_on_the_bow.sin()/get_speed_by_type(torptype);
	if (fabs(sla) >= 1.0) return make_pair(angle(), false);
	return make_pair(angle::from_rad(asin(sla)), true);
}

double torpedo::expected_run_time(torpedo::types torptype, angle lead_angle,
	angle angle_on_the_bow, double target_range)
{
	angle ang = angle(180) - angle_on_the_bow - lead_angle;
	return (angle_on_the_bow.sin() * target_range) / (get_speed_by_type(torptype) * ang.sin());
}

pair<angle, bool> torpedo::compute_launch_data(torpedo::types torptype, const ship* parent,
	const sea_object* target, bool usebowtubes, const angle& manual_lead_angle)
{
	pair<angle, double> br = parent->bearing_and_range_to(target);
	angle ab = parent->estimate_angle_on_the_bow(br.first, target->get_heading());
	pair<angle, bool> la = lead_angle(torptype, target->get_speed(), ab);
	if (la.second) {
		angle gyro_angle = br.first - parent->get_heading() + la.first +
			manual_lead_angle;
		angle headto = parent->get_heading() + gyro_angle;
		double fga = fabs(gyro_angle.value_pm180());
		if (usebowtubes) {
			if (fga <= 90) {
				return make_pair(headto, true);
			}
		} else {	// stern tubes
			if (fga >= 90) {
				return make_pair(headto, true);
			}
		}
	}
	return make_pair(0, false);
}
#endif

#if 0
//fixme beim laden mitbenutzen
void torpedo::create_sensor_array ( types t )
{
	switch ( t )
	{
		case T4:	// fixme
			set_sensor ( passive_sonar_system, new passive_sonar_sensor (
				passive_sonar_sensor::passive_sonar_type_tt_t5 ) );
			break;
		case T5:
			set_sensor ( passive_sonar_system, new passive_sonar_sensor (
				passive_sonar_sensor::passive_sonar_type_tt_t5 ) );
			break;
		case T11:
			set_sensor ( passive_sonar_system, new passive_sonar_sensor (
				passive_sonar_sensor::passive_sonar_type_tt_t11 ) );
			break;
	}
}
#endif

unsigned torpedo::get_hit_points () const	// awful, useless, replace, fixme
{
	return 100;//G7A_HITPOINTS;//fixme
}



double torpedo::get_range() const
{
	double s = 0;
	if (temperature > 15) {
		if (temperature > 30)
			s = 1;
		else
			s = (temperature - 15)/15;
	}
	return range_normal * (1-s) + range_preheated * s;
}
