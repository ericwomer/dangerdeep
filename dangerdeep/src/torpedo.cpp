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



void torpedo::init(const xml_elem& parent)
{
}



torpedo::torpedo(game& gm, std::string specfilename, date dt)
	: ship(gm)	// maybe call also ship loading c'tor...
//fixme: torpedoes have no pointer and could be loaded via direct c'tor
//but torp heir from ship and sea_object and THAT has pointers...
//so one c'tor for reading spec file
//and one load(xml_elem)
//fixme...
{
	xml_doc doc(get_torpedo_dir() + specfilename + ".xml");
	doc.load();
	xml_elem hdftdtorp = doc.child(topnodename);
	xml_elem eclass = hdftdtorp.child("classification");
	// country = eclass.attr("country");
	// modelname = eclass.attr("modelname");
	mass = hdftdtorp.child("weight").attr();
	untertrieb = hdftdtorp.child("untertrieb").attrf();
	xml_elem ewarhead = hdftdtorp.child("warhead");
	warhead_weight = ewarhead.attrf("weight");
	string warhead_charge = ewarhead.attr("charge");
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
		throw xml_error(string("unknown charge type ")+charge, doc.get_filename());
	}
	xml_elem earming = hdftdtorp.charge("arming");
	xml_elem efuse = hdftdtorp.charge("fuse");
	xml_elem emotion = hdftdtorp.charge("motion");
	xml_elem epower = hdftdtorp.charge("power");
	xml_elem eavailability = hdftdtorp.charge("availability");

	for (xml_elem::iterator it = earming.iterate("period"); !it.end(); it.next()) {
		xml_elem eperiod = it.elem();
		eperiod.attru("from");
		eperiod.attru("until");
		runlength = eperiod.attru("runlength");
	}


	double arming_distance;	// meters
	std::list<fuse> fuses;
	double range_normal;
	double speed_normal;
	double range_preheated;	// reading with some parsing
	double speed_preheated;
	steering_devices steering_device;	// string with switch
	double hp;		// horse power of engine
	propulsion_types propulsion_type;	// string with switch
}

	
torpedo::torpedo(game& gm_) : ship(gm_)
{
}



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

torpedo::~torpedo()
{
}

void torpedo::load(istream& in)
{
	sea_object::load(in);
	run_length = read_double(in);
	max_run_length = read_double(in);
	type = (torpedo::types)(read_u8(in));
	influencefuse = read_bool(in);
	primaryrange = read_u16(in);
	secondaryrange = read_u16(in);
	initialturn = read_u8(in);
	turnangle = read_u8(in);
}

void torpedo::save(ostream& out) const
{
	sea_object::save(out);
	write_double(out, run_length);
	write_double(out, max_run_length);
	write_u8(out, unsigned(type));
	write_bool(out, influencefuse);
	write_u16(out, primaryrange);
	write_u16(out, secondaryrange);
	write_u8(out, initialturn);
	write_u8(out, turnangle);
}

void torpedo::load(istream& in)
{
	sea_object::load(in);
	run_length = read_double(in);
	max_run_length = read_double(in);
	type = (torpedo::types)(read_u8(in));
	influencefuse = read_bool(in);
	primaryrange = read_u16(in);
	secondaryrange = read_u16(in);
	initialturn = read_u8(in);
	turnangle = read_u8(in);

	xml_elem stg = parent.child("settings");
	primaryrange = stg.child("primaryrange").attru();
	secondaryrange = stg.child("secondaryrange").attru();
	initialturn = stg.child("initialturn").attru();
	turnangle = stg.child("turnangle").attru();
	torpspeed = stg.child("torpspeed").attru();
	rundepth = stg.child("rundepth").attru();
	temperature = parent.child("temperature").attrf();
}

void torpedo::save(ostream& out) const
{
	sea_object::save(out);
	write_double(out, run_length);
	write_double(out, max_run_length);
	write_u8(out, unsigned(type));
	write_bool(out, influencefuse);
	write_u16(out, primaryrange);
	write_u16(out, secondaryrange);
	write_u8(out, initialturn);
	write_u8(out, turnangle);
}




void torpedo::save(xml_elem& parent)
{
	sea_object::save(parent);
	xml_elem stg = parent.add_child("settings");
	stg.set_attr(primaryrange, "primaryrange");
	stg.set_attr(secondaryrange, "secondaryrange");
	stg.set_attr(unsigned(initialturn_left), "initialturn_left");
	stg.set_attr(turnangle, "turnangle");
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
	if (run_length > max_run_length) {
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

void torpedo::display(void) const
{
	// fixme: read from xml spec file.
	torpedo_g7->display();
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

unsigned torpedo::get_hit_points () const	// awful, useless, replace, fixme
{
	return G7A_HITPOINTS;
}
