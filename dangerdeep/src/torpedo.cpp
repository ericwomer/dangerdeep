// torpedoes
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "torpedo.h"
#include "global_data.h"
#include "model.h"
#include "system.h"
#include "game.h"
#include "sensors.h"



torpedo::torpedo(sea_object* parent, torpedo::types type_, bool usebowtubes, angle headto_,
	unsigned pr, unsigned sr, unsigned it, unsigned sp) : ship()
{
	type = type_;
	primaryrange = (pr <= 16) ? 1600+pr*100 : 1600;
	secondaryrange = (sr & 1) ? 1600 : 800;
	initialturn = it;
	searchpattern = sp;
	position = parent->get_pos();
	heading = parent->get_heading();
	if (!usebowtubes) heading += angle(180);
	vector2 dp = heading.direction() * (parent->get_length()/2 + 3.5);
	position.x += dp.x;
	position.y += dp.y;
	head_to_ang(headto_, !heading.is_cw_nearer(headto_));
	turn_rate = 1.0f;	// most submarine simulations seem to ignore this
			// launching a torpedo will cause it to run in target direction
			// immidiately instead of turning there from the sub's heading
			// fixme historic values??
	size3d = vector3f(0.533, 7, 0.533);	// diameter 53.3cm (21inch), length ~ 7m
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
	throttle = aheadfull;
	system::sys().add_console("torpedo created");
}

void torpedo::load(istream& in, class game& g)
{
	sea_object::load(in, g);
	run_length = read_double(in);
	max_run_length = read_double(in);
	type = (torpedo::types)(read_u8(in));
	influencefuse = read_bool(in);
	primaryrange = read_u16(in);
	secondaryrange = read_u16(in);
	initialturn = read_u8(in);
	searchpattern = read_u8(in);
}

void torpedo::save(ostream& out, const class game& g) const
{
	sea_object::save(out, g);
	write_double(out, run_length);
	write_double(out, max_run_length);
	write_u8(out, unsigned(type));
	write_bool(out, influencefuse);
	write_u16(out, primaryrange);
	write_u16(out, secondaryrange);
	write_u8(out, initialturn);
	write_u8(out, searchpattern);
}

void torpedo::simulate(game& gm, double delta_time)
{
	sea_object::simulate(gm, delta_time);

	// Torpedo starts to search for a target when the minimum save
	// distance for the warhead is passed.
	if ((type == T4 || type == T5 || type == T11) && run_length >= TORPEDO_SAVE_DISTANCE)
	{
		ship* target = gm.sonar_acoustical_torpedo_target ( this );

		if (target)
		{
			angle targetang(target->get_engine_noise_source() - get_pos().xy());
			bool turnright = get_heading().is_cw_nearer(targetang);
			head_to_ang(targetang, !turnright);
		}
	}

	double old_run_length = run_length;
	run_length += get_speed() * delta_time;
	if (run_length > max_run_length) {
		destroy();
		return;
	}

	if (type == T1FAT || type == T3FAT || type == T6LUT) { // FAT and LUT
		angle turnang(180);
		if (type == T6LUT && searchpattern == 1) turnang = (initialturn == 0) ? angle(-90) : angle(90);
		if (old_run_length < primaryrange && run_length >= primaryrange) {
			head_to_ang(get_heading()+turnang, initialturn == 0);
		} else if (old_run_length >= primaryrange) {
			unsigned phase = 0;
			while (primaryrange + phase*secondaryrange < old_run_length) ++phase;
			double tmp = primaryrange + phase*secondaryrange;
			if (old_run_length < tmp && run_length >= tmp) {
				if (type == T6LUT && searchpattern == 1) phase = 0;	// always turn in same direction
				bool turnsleft = ((initialturn + phase) & 1) == 0;
				head_to_ang(get_heading() + turnang, turnsleft);
			}
		}
	}
	
	// check for collisions with other subs or ships
	if (run_length > 10) {	// avoid collision with parent after initial creation
		bool runlengthfailure = (run_length < TORPEDO_SAVE_DISTANCE);
		bool failure = false;	// calculate additional probability of torpedo failure
		if (gm.check_torpedo_hit(this, runlengthfailure, failure))
			destroy();
	}
}

void torpedo::display(void) const
{
	torpedo_g7->display();
}

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

double torpedo::get_speed_by_type(types t)
{
	switch (t) {
		case T1:
		case T2:
		case T3:
		case T3a:
		case T1FAT:
		case T3FAT:
		case T6LUT:
			return kts2ms(30);
		case T4:
			return kts2ms(20);
		case T5:
		case T11:
			return kts2ms(24);
	}
	return 0;
}

unsigned torpedo::get_hit_points () const	// awful, useless, replace, fixme
{
	return G7A_HITPOINTS;
}
