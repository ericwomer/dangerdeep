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



torpedo::fuse::fuse(const xml_elem& parent, date equipdate)
{
	string modelstr = parent.attr("type");
	if (modelstr == "Pi1") model = Pi1;
	else if (modelstr == "Pi2") model = Pi2;
	else if (modelstr == "Pi3") model = Pi3;
	else if (modelstr == "Pi4a") model = Pi4a;
	else if (modelstr == "Pi4b") model = Pi4b;
	else if (modelstr == "Pi4c") model = Pi4c;
	else if (modelstr == "Pi6") model = Pi6;
	else if (modelstr == "TZ3") model = TZ3;
	else if (modelstr == "TZ5") model = TZ5;
	else if (modelstr == "TZ6") model = TZ6;
	else throw xml_error(string("illegal fuse model given: ") + modelstr, parent.doc_name());
	// fixme: check here if that is correct!!! Pi4 intertial?
	switch (model) {
	case Pi1:
		failure_probability = 0.3f;	// fixme depends on date!
		type = IMPACT;
		break;
	case Pi2:
		failure_probability = 0.2f;	// fixme depends on date!
		type = IMPACT;
		break;
	case Pi3:
		failure_probability = 0.1f;	// fixme depends on date!
		type = IMPACT;
		break;
	case Pi4a:
		failure_probability = 0.1f;	// fixme depends on date!
		type = IMPACT;
		break;
	case Pi4b:
		failure_probability = 0.1f;	// fixme depends on date!
		type = IMPACT;
		break;
	case Pi4c:
		failure_probability = 0.1f;	// fixme depends on date!
		type = IMPACT;
		break;
	case Pi6:
		failure_probability = 0.02f;	// fixme depends on date!
		type = INERTIAL;
		break;
	case TZ3:
		failure_probability = 0.5f;	// fixme depends on date!
		type = INFLUENCE;
		break;
	case TZ5:
		failure_probability = 0.2f;	// fixme depends on date!
		type = INFLUENCE;
		break;
	case TZ6:
		failure_probability = 0.1f;	// fixme depends on date!
		type = INFLUENCE;
		break;
	}
}



bool torpedo::fuse::handle_impact(angle impactangle) const
{
	// compute failure depending on angle, type and probability
	if (rnd() < failure_probability)
		return false;
	return true;
}



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
	date dt = gm.get_equipment_date();
	// ------------ availability, check this first
	xml_elem eavailability = parent.child("availability");
	date availdt = date(eavailability.attr("date"));
	if (dt < availdt) throw xml_error("torpedo type not available at this date!", parent.doc_name());

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
		//throw xml_error(string("unknown charge type ")+charge, parent.doc_name());
	}
	// ------------- arming
	xml_elem earming = parent.child("arming");
	arming_distance = -1;
	for (xml_elem::iterator it = earming.iterate("period"); !it.end(); it.next()) {
		date from = it.elem().attr("from");
		date until = it.elem().attr("until");
		if (from <= dt && dt <= until) {
			arming_distance = it.elem().attrf("runlength");
			break;
		}
	}
	if (arming_distance < 0)
		throw xml_error("no period subtags of arming that match current equipment date!", parent.doc_name());
	// ---------- fuse(s)
	xml_elem efuse = parent.child("fuse");
	for (xml_elem::iterator it = efuse.iterate("period"); !it.end(); it.next()) {
		date from = it.elem().attr("from");
		date until = it.elem().attr("until");
		if (from <= dt && dt <= until) {
			fuses.push_back(fuse(it.elem(), dt));
		}
	}
	if (fuses.empty())
		throw xml_error("no period subtags of fuse that match current equipment date!", parent.doc_name());
	// ----------- motion / steering device
	xml_elem emotion = parent.child("motion");
	unsigned hasfat = emotion.attru("FAT");
	unsigned haslut = emotion.attru("LUT");
	if (hasfat > 0) {
		if (haslut > 0) throw xml_error("steering device must be EITHER LuT OR FaT!", parent.doc_name());
		steering_device = FaT;
	} else if (haslut > 0) {
		steering_device = (haslut == 1) ? LuTI : LuTII;
	} else {
		steering_device = STRAIGHT;
	}
	// ------------ sensors, fixme
	// ------------ ranges
	xml_elem eranges = parent.child("ranges");
	for (xml_elem::iterator it = eranges.iterate("range"); !it.end(); it.next()) {
		if (it.elem().attrb("preheated")) {
			range_preheated = it.elem().attrf("distance");
			speed_preheated = kts2ms(it.elem().attrf("speed"));
		} else {
			range_normal = it.elem().attrf("distance");
			speed_normal = kts2ms(it.elem().attrf("speed"));
		}
	}
	// ------------ power, fixme - not needed yet
	xml_elem epower = parent.child("power");

	// ------------ set ship turning values, fixme: read from files, more a hack...
	max_rudder_angle = 40;
	max_rudder_turn_speed = 200;//20;	// with smaller values torpedo course oscillates. damping too high?! steering to crude?! fixme
	// set turn rate here. With 0.6 a torpedo takes roughly 10 seconds to turn 90 degrees.
	// With that value the torpedo turn radius is ~98m. Maybe a bit too much.
	turn_rate = 0.6;
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
	turn_rate = 0.6f;	// most submarine simulations seem to ignore this
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
/*
	cout << "torpedo  " << this << " heading " << heading.value() << " should head to " << head_to.value() << " turn speed " << turn_velocity << "\n";
	cout << " position " << position << " orientation " << orientation << " run_length " << run_length << "\n";
	cout << " velo " << velocity << " turnvelo " << turn_velocity << " global vel " << global_velocity << "\n";
	cout << " acceleration " << acceleration << " delta t "<< delta_time << "\n";
*/
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



void torpedo::launch(const vector3& launchpos, angle parenthdg)
{
	position = launchpos;
	orientation = quaternion::rot(-parenthdg.value(), 0, 0, 1);
	heading = parenthdg;
	max_speed_forward = get_speed();
	max_angular_velocity = max_speed_forward * turn_rate;
	// velocity is local, object moves only forward.
	velocity = vector3(0, max_speed_forward, 0);
	run_length = 0;
}




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
	// fixme: TI independent on temperature, but depends on torpspeed selector!
	double s = 0;
	if (temperature > 15) {
		if (temperature > 30)
			s = 1;
		else
			s = (temperature - 15)/15;
	}
	return range_normal * (1-s) + range_preheated * s;
}



double torpedo::get_speed() const
{
	// fixme: TI independent on temperature, but depends on torpspeed selector!
	double s = 0;
	if (temperature > 15) {
		if (temperature > 30)
			s = 1;
		else
			s = (temperature - 15)/15;
	}
	return speed_normal * (1-s) + speed_preheated * s;
}
