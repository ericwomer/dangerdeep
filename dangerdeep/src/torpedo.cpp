// torpedoes
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "torpedo.h"
#include "global_data.h"
#include "model.h"
#include "system.h"
#include "game.h"
#include "sensors.h"

// fixme: type T XI has same image as empty tube -> bug!

torpedo::torpedo(sea_object* parent, unsigned type_, bool usebowtubes,
	unsigned pr, unsigned sr, unsigned it, unsigned sp) : sea_object()
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
	head_to = heading;
	turn_rate = 1.0f;	// most submarine simulations seem to ignore this
			// launching a torpedo will cause it to run in target direction
			// immidiately instead of turning there from the sub's heading
			// fixme historic values??
	length = 7;
	width = 1.066;
	run_length = 0;
	switch (type_) {
		case T1:
			max_speed = speed = kts2ms(30);
			max_rev_speed = 0;
			max_run_length = 14000;
			influencefuse = false;
			break;
		case T2:
			max_speed = speed = kts2ms(30);
			max_rev_speed = 0;
			max_run_length = 5000;
			influencefuse = false;
			break;
		case T3:
			max_speed = speed = kts2ms(30);
			max_rev_speed = 0;
			max_run_length = 5000;
			influencefuse = true;
			break;
		case T3a:
			max_speed = speed = kts2ms(30);
			max_rev_speed = 0;
			max_run_length = 7500;
			influencefuse = true;
			break;
		case T4:
			max_speed = speed = kts2ms(20);
			max_rev_speed = 0;
			max_run_length = 7500;
			// fixme: Falke sensor
			set_sensor ( passive_sonar_system, new passive_sonar_sensor (
				passive_sonar_sensor::passive_sonar_type_tt_t5 ) );
			influencefuse = true;
			break;
		case T5:
			max_speed = speed = kts2ms(24);
			max_rev_speed = 0;
			max_run_length = 5700;
			set_sensor ( passive_sonar_system, new passive_sonar_sensor (
				passive_sonar_sensor::passive_sonar_type_tt_t5 ) );
			influencefuse = true;
			break;
		case T11:
			max_speed = speed = kts2ms(24);
			max_rev_speed = 0;
			max_run_length = 5700;
			set_sensor ( passive_sonar_system, new passive_sonar_sensor (
				passive_sonar_sensor::passive_sonar_type_tt_t11 ) );
			influencefuse = true;
			break;
		case T1FAT:
			max_speed = speed = kts2ms(30);
			max_rev_speed = 0;
			max_run_length = 14000;
			influencefuse = false;
			break;
		case T3FAT:
			max_speed = speed = kts2ms(30);
			max_rev_speed = 0;
			max_run_length = 7500;
			influencefuse = true;
			break;
		case T6LUT:
			max_speed = speed = kts2ms(30);
			max_rev_speed = 0;
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
	type = read_u8(in);
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
	write_u8(out, type);
	write_bool(out, influencefuse);
	write_u16(out, primaryrange);
	write_u16(out, secondaryrange);
	write_u8(out, initialturn);
	write_u8(out, searchpattern);
}

void torpedo::simulate(game& gm, double delta_time)
{
	sea_object::simulate(gm, delta_time);

	if (is_defunct() || is_dead())
		return;

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
	run_length += speed * delta_time;
	if (run_length > max_run_length) {
		kill();
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
			kill();
	}
}

void torpedo::display(void) const
{
	torpedo_g7->display();
}

pair<angle, bool> torpedo::lead_angle(double target_speed, angle angle_on_the_bow) const
{
	double sla = target_speed*angle_on_the_bow.sin()/speed;
	if (fabs(sla) >= 1.0) return make_pair(angle(), false);
	return make_pair(angle::from_rad(asin(sla)), true);
}

double torpedo::expected_run_time(angle lead_angle,
	angle angle_on_the_bow, double target_range) const
{
	angle ang = angle(180) - angle_on_the_bow - lead_angle;
	return (angle_on_the_bow.sin() * target_range) / (speed * ang.sin());
}

//#include <sstream>
bool torpedo::adjust_head_to(const sea_object* parent, const sea_object* target, bool usebowtubes,
	const angle& manual_lead_angle)
{
	pair<angle, double> br = parent->bearing_and_range_to(target);
	angle ab = parent->estimate_angle_on_the_bow(br.first, target->get_heading());
	pair<angle, bool> la = lead_angle(target->get_speed(), ab);
	if (la.second) {
		angle gyro_angle = br.first - parent->get_heading() + la.first +
			manual_lead_angle;
		angle headto = parent->get_heading() + gyro_angle;
//ostringstream os;
//	os << "lead angle " << la.first.value() << " gyro angle " << gyro_angle.value() << ", trp head to " << headto.value()
//	<< ", expected run time " << expected_run_time(la.first, ab, br.second);
//	system::sys().add_console(os.str());
		double fga = fabs(gyro_angle.value_pm180());
		if (usebowtubes) {
			if (fga <= 90) {
				head_to_ang(headto, gyro_angle.value_pm180() < 0);
				return true;
			}
		} else {	// stern tubes
			if (fga >= 90) {
				head_to_ang(headto, gyro_angle.value_pm180() > 0);
				return true;
			}
		}
	}
	return false;
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

unsigned torpedo::get_hit_points () const	// awful, useless, replace, fixme
{
	return G7A_HITPOINTS;
}
