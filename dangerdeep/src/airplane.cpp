// airplanes
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "airplane.h"
#include "model.h"
#include "tokencodes.h"

airplane::airplane(unsigned type_, const vector3& pos, double heading) : sea_object()
{
	type = type_;
	position = pos;
	this->heading = heading;
	head_to = heading;
	rotation = quaternion::neutral_rot();
	rollfac = pitchfac = 0.0;
//	turn_rate = deg2rad(5);
//	length = 7;
//	width = 1;
	switch (type_) {
/*
		case 3:
			speed = 8;
			max_speed = 17.6;
			max_rev_speed = 5;
			acceleration = 0.8;
			turn_rate = 1;
			break;
*/			
	}
	
	throttle = aheadfull;
}

void airplane::load(istream& in, class game& g)
{
	sea_object::load(in, g);
	type = read_u32(in);
	rotation.s = read_double(in);
	rotation.v.x = read_double(in);
	rotation.v.y = read_double(in);
	rotation.v.z = read_double(in);
	velocity.x = read_double(in);
	velocity.y = read_double(in);
	velocity.z = read_double(in);
	rollfac = read_double(in);
	pitchfac = read_double(in);
}

void airplane::save(ostream& out, const class game& g) const
{
	sea_object::save(out, g);
	write_u32(out, type);
	write_double(out, rotation.s);
	write_double(out, rotation.v.x);
	write_double(out, rotation.v.y);
	write_double(out, rotation.v.z);
	write_double(out, velocity.x);
	write_double(out, velocity.y);
	write_double(out, velocity.z);
	write_double(out, rollfac);
	write_double(out, pitchfac);
}


airplane* airplane::create(istream& in)
{
	return new airplane();
/*
	unsigned type = read_u16(in);
	switch (type) {
		case largemerchant: return new airplane_largemerchant();
		case mediummerchant: return new airplane_mediummerchant();
		case smallmerchant: return new airplane_smallmerchant();
		case mediumtroopairplane: return new airplane_mediumtroopairplane();
		case destroyertribal: return new airplane_destroyertribal();
		case battleairplanemalaya: return new airplane_battleairplanemalaya();
		case carrierbogue: return new airplane_carrierbogue();
		case corvette: return new airplane_corvette();
		case largefreighter: return new airplane_largefreighter();
		case mediumfreighter: return new airplane_mediumfreighter();
		case smalltanker: return new airplane_smalltanker();
	}
	return 0;
*/
}

airplane* airplane::create(airplane::types type_)
{
	return new airplane();
/*	
	switch (type_) {
		case largemerchant: return new airplane_largemerchant();
		case mediummerchant: return new airplane_mediummerchant();
		case smallmerchant: return new airplane_smallmerchant();
		case mediumtroopairplane: return new airplane_mediumtroopairplane();
		case destroyertribal: return new airplane_destroyertribal();
		case battleairplanemalaya: return new airplane_battleairplanemalaya();
		case carrierbogue: return new airplane_carrierbogue();
		case corvette: return new airplane_corvette();
		case largefreighter: return new airplane_largefreighter();
		case mediumfreighter: return new airplane_mediumfreighter();
		case smalltanker: return new airplane_smalltanker();
	}
	return 0;
*/ 
}

airplane* airplane::create(parser& p)
{
	p.parse(TKN_AIRPLANE);
	int t = p.type();
//string s = p.text();
//	p.consume();
	airplane* a = new airplane();

	// parse changeable values	
	p.parse(TKN_SLPARAN);
	while (p.type() != TKN_SRPARAN) {
		bool ok = a->parse_attribute(p);
		if (!ok) {
			p.error("Expected airplane ??? attribute");
		}
	}
	p.consume();

/*
	// fix some values	
	double c0 = heading.cos(), s0 = -heading.sin();
	double c1 = pitch.cos(), s1 = -pitch.sin();
	double c2 = roll.cos(), s2 = -roll.sin();
	vector3 R0(c0*c2-s0*s1*s2, s0*c2+c0*s1*s2, -c1*s2);
	vector3 R1(-s0*c1, c0*c1, s1);
	vector3 R2(c0*s2+s0*s1*c2, s0*s2-c0*s1*c2, c1*c2);
	velocity = vector3(0, speed, 0).matrixmul(R0, R1, R2);
*/	

//	speed = get_throttle_speed();
//	head_to = heading;

/*	
	switch (t) {
		case TKN_LARGEMERCHANT: return new airplane_largemerchant(p);
		case TKN_MEDIUMMERCHANT: return new airplane_mediummerchant(p);
		case TKN_SMALLMERCHANT: return new airplane_smallmerchant(p);
		case TKN_MEDIUMTROOPSHIP: return new airplane_mediumtroopairplane(p);
		case TKN_DESTROYERTRIBAL: return new airplane_destroyertribal(p);
		case TKN_BATTLESHIPMALAYA: return new airplane_battleairplanemalaya(p);
		case TKN_CARRIERBOGUE: return new airplane_carrierbogue(p);
		case TKN_CORVETTE: return new airplane_corvette(p);
		case TKN_LARGEFREIGHTER: return new airplane_largefreighter(p);
		case TKN_MEDIUMFREIGHTER: return new airplane_mediumfreighter(p);
		case TKN_SMALLTANKER: return new airplane_smalltanker(p);
	}
*/	
//cerr << "token " << s << " unknown.\n";	
	return a;
}



void airplane::simulate(class game& gm, double delta_time)
{
	if (is_defunct()) {
		return;
	} else if (is_dead()) {
		alive_stat = defunct;
		return;
	}

	quaternion invrot = rotation.conj();
	vector3 localvelocity = invrot.rotate(velocity);
		
	speed = localvelocity.y;
	double speed_sq = speed * speed;

	vector3 wingarea(get_wing_width(), get_wing_length(), 0);
	vector3 locx = rotation.rotate(1, 0, 0);
	vector3 locy = rotation.rotate(0, 1, 0);
	vector3 locz = rotation.rotate(0, 0, 1);
	vector3 rotwingarea = rotation.rotate(wingarea);
	double areafac = rotwingarea.x * rotwingarea.y;
//	if (locz.z < 0) locz = -locz;

	// forces:
	// thrust: engine thrust along local y-axis
	// lift: wing lift along local z-axis, proportional to square of speed
	// gravity*mass: along negative global z-axis

	// fixme: simulate stall: if speed drops below a specific quantum, the plane's nose
	// drops down. This avoids negative values for speed
//	if (speed < 0) {speed = 0; speed_sq = 0;}

	// avoid negative speeds/forces fixme

	// compute forces	
	// propulsion by engine (thrust)
	double drag_damping = 1.0 - localvelocity.y * localvelocity.y * get_drag_factor();
	if (drag_damping < 0.0) drag_damping = 0.0;
	vector3 thrust = get_engine_thrust() * drag_damping * locy;
	// lift by wings (fixme: works also if plane is upside down or nearly!)
	double lift_damping = (localvelocity.z < 0) ? 1.0 : 1.0 - localvelocity.z * localvelocity.z * get_antilift_factor();
	if (lift_damping < 0.0) lift_damping = 0.0;
	vector3 lift = (areafac * speed_sq * get_lift_factor() * lift_damping) * locz;
	// gravity, fixme: depends on plane's rotation!
	double gravity_damping = (velocity.z >= 0) ? 1.0 : 1.0 - velocity.z * velocity.z * get_antilift_factor();
	if (gravity_damping < 0.0) gravity_damping = 0.0;
	vector3 gravity = vector3(0, 0, get_mass() * gravity_damping * -GRAVITY);
	// slide damping
	double slide_damping = 1.0 - localvelocity.x * localvelocity.x * get_slide_factor();
	if (slide_damping < 0) slide_damping = 0.0;
	vector3 slide = (get_mass() * -slide_damping) * locx;

/*	
cout << "global velocity " << velocity << "\n";
cout << "local velocity  " << localvelocity << "\n";
cout << "thrust          " << thrust << "\n";
cout << "lift            " << lift << "\n";
cout << "slide           " << slide << "\n";
cout << "gravity         " << gravity << "\n";
*/

	// update position and speed
	vector3 accel = (thrust + lift + gravity + slide) * (1.0/get_mass());
	position += velocity * delta_time + accel * (0.5 * delta_time * delta_time);
	velocity += accel * delta_time;

	quaternion qpitch = quaternion::rot(pitchfac * get_pitch_deg_per_sec() * delta_time, 1, 0, 0); // fixme: also depends on speed
	quaternion qroll = quaternion::rot(rollfac * get_roll_deg_per_sec() * delta_time, 0, 1, 0); // fixme: also depends on speed
	rotation *= qpitch * qroll;

//	if ( myai )
//		myai->act(gm, delta_time);

//	// Adjust fuel_level.
//	calculate_fuel_factor ( delta_time );
}

void airplane::roll_left(void)
{
	rollfac = -1;
}

void airplane::roll_right(void)
{
	rollfac = 1;
}

void airplane::roll_zero(void)
{
	rollfac = 0;
}

void airplane::pitch_down(void)
{
	pitchfac = -1;
}

void airplane::pitch_up(void)
{
	pitchfac = 1;
}

void airplane::pitch_zero(void)
{
	pitchfac = 0;
}


const model* airplane::get_model(void) const
{
	switch(type) {
		default:
			return std_plane;
	}
}
