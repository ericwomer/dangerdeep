// sea objects
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "sea_object.h"
#include "vector2.h"
#include "tokencodes.h"
#include "sensors.h"
#include "model.h"

sea_object::sea_object() : position(vector3(0.0f, 0.0f, 0.0f)), heading(0.0f),
	speed(0.0f), max_speed(0.0f), max_rev_speed(0.0f), throttle(stop),
	rudder(ruddermid), acceleration(0.0f), permanent_turn(false), head_chg(0.0f),
	head_to(0.0f), turn_rate(0.0f), length(0.0f), width(0.0f), alive_stat(alive),
	vis_cross_section_factor(1.0f)
{
	sensors.resize ( last_sensor_system );
}

sea_object::~sea_object()
{
	int size = sensors.size ();
	for ( int i = 0; i < size; i++ )
	{
		if ( sensors[i] != 0 )
			delete sensors[i];
	}
}

bool sea_object::parse_attribute(parser& p)
{
	switch (p.type()) {
		case TKN_POSITION: {
			p.consume();
			p.parse(TKN_ASSIGN);
			int x = p.parse_number();
			p.parse(TKN_COMMA);
			int y = p.parse_number();
			p.parse(TKN_COMMA);
			int z = p.parse_number();
			p.parse(TKN_SEMICOLON);
			position = vector3(x, y, z);
			break; }
		case TKN_HEADING:
			p.consume();
			p.parse(TKN_ASSIGN);
			heading = angle(p.parse_number());
			p.parse(TKN_SEMICOLON);
			break;
		case TKN_THROTTLE:
			p.consume();
			p.parse(TKN_ASSIGN);
			switch (p.type()) {
				case TKN_STOP: throttle = stop; break;
				case TKN_REVERSE: throttle = reverse; break;
				case TKN_AHEADLISTEN: throttle = aheadlisten; break;
				case TKN_AHEADSONAR: throttle = aheadsonar; break;
				case TKN_AHEADSLOW: throttle = aheadslow; break;
				case TKN_AHEADHALF: throttle = aheadhalf; break;
				case TKN_AHEADFULL: throttle = aheadfull; break;
				case TKN_AHEADFLANK: throttle = aheadflank; break;
				default: p.error("Expected throttle value");
			}
			p.consume();
			p.parse(TKN_SEMICOLON);
			break;
		default: return false;
	}
	return true;
}

void sea_object::simulate(game& gm, double delta_time)
{
	// calculate sinking
	if (is_defunct()) {
		return;
	} else if (is_dead()) {
		alive_stat = defunct;
		return;
	} else if (is_sinking()) {
		position.z -= delta_time * SINK_SPEED;
		if (position.z < -50)	// used for ships.
			kill();
		throttle = stop;
		rudder_midships();
		return;
	}

	// calculate directional speed
	vector2 dir_speed_2d = heading.direction() * speed;
	vector3 dir_speed(dir_speed_2d.x, dir_speed_2d.y, 0);
	
	// calculate new position
	position += dir_speed * delta_time;
	
	// calculate speed change
	double t = get_throttle_speed() - speed;
	double s = acceleration * delta_time;
	if (fabs(t) > s) {
		speed += (t < 0) ? -s : s;
	} else {
		speed = get_throttle_speed();
	}

	// calculate heading change (fixme, this turning is not physically correct)
	angle maxturnangle = turn_rate * (head_chg * delta_time * speed);
	if (permanent_turn) {
		heading += maxturnangle;
	} else {
		double fac = (head_to - heading).value_pm180()
			/ maxturnangle.value_pm180();
		if (0 <= fac && fac <= 1) {
			rudder_midships();
			heading = head_to;
		} else {
			heading += maxturnangle;
		}
	}
}

/*		--- commented out. is never used and superseded by game::is_collision
		--- fixme: object state is not tested there!
		--- maybe this is the reason for double counting torpedo hits.
bool sea_object::is_collision(const sea_object* other)
{
	if (is_defunct() || is_dead() || other->is_defunct() || other->is_dead()) return false;
	// korrekt wäre die beiden Rechtecke von *this und other auf Schnitt zu testen, fixme
	vector2 headdir = heading.direction();
	vector2 other_headdir = other->heading.direction();
	vector2 pos = position.xy();
	vector2 other_pos = other->position.xy();
	double s, t;
	bool solved = (pos - other_pos).solve(-headdir, other_headdir, s, t);
	if (solved) {
		// quick hack. je mehr heading/other.heading unterschiedlich sind, desto besser
		return (fabs(s) <= length/2 + width/2 && fabs(t) <= other->length/2 + other->width/2);
	}
	return false;
}

bool sea_object::is_collision(const vector2& pos)
{
	if (is_defunct() || is_dead()) return false;
	// a bit slower than neccessary. fixme
	vector2 headdir = heading.direction();
	vector2 nheaddir = headdir.orthogonal();
	vector2 mypos = position.xy();
	double s, t;
	bool solved = (mypos - pos).solve(headdir, nheaddir, s, t);
	if (solved) {
		return (fabs(s) <= length/2 && fabs(t) <= width/2);
	}
	return false;
}
*/

bool sea_object::damage(const vector3& fromwhere, unsigned strength)
{
	sink();
	return true;
}

unsigned sea_object::calc_damage(void) const
{
	return alive_stat == sinking ? 100 : 0;
}

void sea_object::sink(void)
{
	alive_stat = sinking;
}

void sea_object::kill(void)
{
	alive_stat = dead;
}

void sea_object::head_to_ang(const angle& a, bool left_or_right)	// true == left
{
	head_to = a;
	head_chg = (left_or_right) ? -1 : 1;
	permanent_turn = false;
}

void sea_object::change_rudder (const int& dir)
{
    // Change rudder state first.
    if ( dir < 0 )
        rudder --;
    else if ( dir > 0 )
        rudder ++;

    // Limit rudder state.
    if ( rudder < rudderfullleft )
        rudder = rudderfullleft;
    else if ( rudder > rudderfullright )
        rudder = rudderfullright;

    // Set head_chg due to rudder state.
    switch ( rudder )
    {
        case rudderfullleft:
            head_chg = -1.0f;
            break;
        case rudderleft:
            head_chg = -0.5f;
            break;
        case rudderright:
            head_chg = 0.5f;
            break;
        case rudderfullright:
            head_chg = 1.0f;
            break;
        default:
            head_chg = 0.0f;
            break;
    }
    
    if ( rudder == ruddermid )
        permanent_turn = false;
    else
        permanent_turn = true;
}

void sea_object::rudder_left(void)
{
    change_rudder ( -1 );
}

void sea_object::rudder_right(void)
{
    change_rudder ( 1 );
}

void sea_object::rudder_hard_left(void)
{
    rudder = rudderfullleft;
	head_chg = -1.0f;
	permanent_turn = true;
}

void sea_object::rudder_hard_right(void)
{
    rudder = rudderfullright;
	head_chg = 1.0f;
	permanent_turn = true;
}

void sea_object::rudder_midships(void)
{
    rudder = ruddermid;
	head_chg = 0.0f;
	head_to = heading;
	permanent_turn = false;
}

void sea_object::set_throttle(throttle_status thr)
{
	throttle = thr;
}

void sea_object::remember_position(void)
{
	previous_positions.push_front(get_pos().xy());
	if (previous_positions.size() > MAXPREVPOS)
		previous_positions.pop_back();
}	

bool sea_object::set_course_to_pos(const vector2& pos)
{
	vector2 d = pos - get_pos().xy();
	vector2 hd = get_heading().direction();
	double a = d.x*hd.x + d.y*hd.y;
	double b = d.x*hd.y - d.y*hd.x;
	// if a is < 0 then target lies behind our pos.
	// if b is < 0 then target is left, else right of our pos.
	double r1 = (b == 0) ? 1e10 : (a*a + b*b)/fabs(2*b);
	double r2 = 1.0/get_turn_rate().rad();
	if (a <= 0) {	// target is behind us
		if (b < 0) {	// target is left
			head_to_ang(get_heading() - angle(180), true);
		} else {
			head_to_ang(get_heading() + angle(180), false);
		}
		return false;
	} else if (r2 > r1) {	// target can not be reached with smallest curve possible
		if (b < 0) {	// target is left
			head_to_ang(get_heading() + angle(180), false);
		} else {
			head_to_ang(get_heading() - angle(180), true);
		}
		return false;
	} else {	// target can be reached, steer curve
		head_to_ang(angle::from_math(atan2(d.y, d.x)), (b < 0));
//	this code computes the curve that hits the target
//	but it is much better to turn fast and then steam straight ahead
/*
		double needed_turn_rate = (r1 == 0) ? 0 : 1.0/r1; //speed/r1;
		double fac = ((180.0*needed_turn_rate)/PI)/fabs(turn_rate.value_pm180());
		head_chg = (b < 0) ? -fac : fac;
*/		
		return true;
	}
}

double sea_object::get_throttle_speed(void) const
{
	double ms = get_max_speed();
	switch (throttle) {
		case reverse: return -ms*0.25f;     // 1/4
		case stop: return 0;
		case aheadlisten: return ms*0.25f;  // 1/4
		case aheadsonar: return ms*0.25f;   // 1/4
		case aheadslow: return ms*0.33333f; // 1/3
		case aheadhalf: return ms*0.5f;     // 1/2
		case aheadfull: return ms*0.75f;    // 3/4
		case aheadflank: return ms;
	}
	return 0;
}

pair<angle, double> sea_object::bearing_and_range_to(const sea_object* other) const
{
	vector2 diff = other->get_pos().xy() - position.xy();
	return make_pair(angle(diff), diff.length());
}

angle sea_object::estimate_angle_on_the_bow(angle target_bearing, angle target_heading) const
{
	return (angle(180) + target_bearing - target_heading).value_pm180();
}

float sea_object::surface_visibility(const vector2& watcher) const
{
	return vis_cross_section_factor * get_profile_factor ( watcher );
}

sensor* sea_object::get_sensor ( sensor_system ss )
{
	if ( ss >= 0 && ss < last_sensor_system )
		return sensors[ss];

	return 0;
}

const sensor* sea_object::get_sensor ( sensor_system ss ) const
{
	if ( ss >= 0 && ss < last_sensor_system )
		return sensors[ss];

	return 0;
}

void sea_object::set_sensor ( sensor_system ss, sensor* s )
{
	if ( ss >= 0 && ss < last_sensor_system )
		sensors[ss] = s;
}

double sea_object::get_profile_factor ( const vector2& d ) const
{
	// Calculate scalar product first and get cosine value.
	vector2 r = get_pos ().xy () - d;
	angle diffAngle = angle ( r ) - heading;

	return ( 0.3f + 0.7f * fabs ( diffAngle.sin () ) );
}

double sea_object::get_noise_factor () const
{
    return get_throttle_speed () / max_speed;
}

vector2 sea_object::get_engine_noise_source () const
{
	return get_pos ().xy () - get_heading ().direction () * 0.3f * length;
}

void sea_object::display(void) const
{
	const model* mdl = get_model ();

	if ( mdl )
		mdl->display ();
}
