// torpedoes
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "torpedo.h"
#include "global_data.h"
#include "model.h"
#include "system.h"
#include "game.h"
#include "sensors.h"

torpedo::torpedo(sea_object* parent_, unsigned type_, bool usebowtubes) : sea_object()
{
	parent = parent_;
	type = type_;
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
	length = 7;
	width = 1;
	run_length = 0;
	switch (type_) {
		case T1:		// G7a steam torpedo
			speed = kts2ms(30);
			max_speed = kts2ms(30);
			max_rev_speed = 0;
			max_run_length = 12500;
			vis_cross_section_factor = CROSS_SECTION_VIS_TORPWB;
			break;
		case T3:		// G7e electric torpedo (T2/T3)
			speed = kts2ms(30);
			max_speed = kts2ms(30);
			max_rev_speed = 0;
			max_run_length = 7500;
			vis_cross_section_factor = CROSS_SECTION_VIS_NULL;
			break;
		case T5:		// G7e acustic torpedo
			speed = kts2ms(24);
			max_speed = kts2ms(24);
			max_rev_speed = 0;
			max_run_length = 5000;	// fixme: historical value?
			vis_cross_section_factor = CROSS_SECTION_VIS_NULL;
			set_sensor ( passive_sonar_system, new passive_sonar_sensor (
				passive_sonar_sensor::passive_sonar_type_tt_t5 ) );
			break;
		case T3FAT:		// G7e FAT torpedo
			speed = kts2ms(30);
			max_speed = kts2ms(30);
			max_rev_speed = 0;
			max_run_length = 7500;
			vis_cross_section_factor = CROSS_SECTION_VIS_NULL;
			break;
		case T6LUT:		// G7e LUT torpedo
			speed = kts2ms(30);	//fixme: wrong values
			max_speed = kts2ms(30);
			max_rev_speed = 0;
			max_run_length = 12500;
			vis_cross_section_factor = CROSS_SECTION_VIS_NULL;
			break;
		case T11:		// G7e acustic torpedo (improved T5)
			speed = kts2ms(24);	// fixme: historical value?
			max_speed = kts2ms(24);
			max_rev_speed = 0;
			max_run_length = 5000;	// fixme: historical value?
			vis_cross_section_factor = CROSS_SECTION_VIS_NULL;
			set_sensor ( passive_sonar_system, new passive_sonar_sensor (
				passive_sonar_sensor::passive_sonar_type_tt_t11 ) );
			break;
	};
	throttle = aheadfull;
	system::sys()->add_console("torpedo created");
}

void torpedo::simulate(game& gm, double delta_time)
{
	if (parent != 0 && parent->is_dead())
		parent = 0;

	sea_object::simulate(gm, delta_time);

	if (is_defunct() || is_dead())
		return;

	// Torpedo starts to search for a target when the minimum save
	// distance for the warhead is passed.
	if ((type == T5 || type == T11) && run_length >= TORPEDO_SAVE_DISTANCE)
	{
		ship* target = gm.sonar_acoustical_torpedo_target ( this );

		if (target)
		{
			angle targetang(target->get_engine_noise_source() - get_pos().xy());
			bool turnright = get_heading().is_cw_nearer(targetang);
			head_to_ang(targetang, !turnright);
		}
	}

	if (type == T3FAT) { // FAT, test hack
		if (run_length > 1600) {	// wrong pattern fixme
			rudder_right();
		}
	}
	run_length += speed * delta_time;
	if (run_length > max_run_length) {
		kill();
		return;
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
bool torpedo::adjust_head_to(const sea_object* target, bool usebowtubes)
{
	if (parent == 0) return false;
	pair<angle, double> br = parent->bearing_and_range_to(target);
	angle ab = parent->estimate_angle_on_the_bow(br.first, target->get_heading());
	pair<angle, bool> la = lead_angle(target->get_speed(), ab);
	if (la.second) {
		angle gyro_angle = br.first - parent->get_heading() + la.first;
		angle headto = parent->get_heading() + gyro_angle;
//ostringstream os;
//	os << "lead angle " << la.first.value() << " gyro angle " << gyro_angle.value() << ", trp head to " << headto.value()
//	<< ", expected run time " << expected_run_time(la.first, ab, br.second);
//	system::sys()->add_console(os.str());
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
