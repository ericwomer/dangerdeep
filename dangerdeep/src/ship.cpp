// ships
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "ship.h"
#include "model.h"
#include "game.h"
#include "date.h"
#include "tokencodes.h"
#include "sensors.h"
#include "ai.h"
#include "system.h"
#include "particle.h"
#include "tinyxml/tinyxml.h"


//fixme: redefine display, call base display, add spray at bow (2 quads with spray texture, maybe animated,
//parallel to bow hull sides ~ 30degrees or so)


// empty c'tor is needed by heirs
ship::ship() : myai(0), smoke_type(0)
{
	init();
}



void ship::init(void)
{
	heading = 0;
	throttle = 0;
	head_to_fixed = false;
	rudder_pos = 0;
	rudder_to = 0;
	max_rudder_angle = 40;
	max_rudder_turn_speed = 10;
	max_angular_velocity = 2;
}



ship::ship(TiXmlDocument* specfile, const char* topnodename) : sea_object(specfile, topnodename)
{
	init();
	TiXmlHandle hspec(specfile);
	TiXmlHandle hdftdship = hspec.FirstChild(topnodename);
	TiXmlElement* eclassification = hdftdship.FirstChildElement("classification").Element();
	system::sys().myassert(eclassification != 0, string("ship: classification node missing in ")+specfilename);
	string typestr = XmlAttrib(eclassification, "type");
	if (typestr == "warship") shipclass = WARSHIP;
	else if (typestr == "escort") shipclass = ESCORT;
	else if (typestr == "merchant") shipclass = MERCHANT;
	else if (typestr == "submarine") shipclass = SUBMARINE;
	else system::sys().myassert(false, string("illegal ship type in ") + specfilename);
	TiXmlElement* etonnage = hdftdship.FirstChildElement("tonnage").Element();
	system::sys().myassert(etonnage != 0, string("tonnage node missing in ")+specfilename);
	unsigned minton = XmlAttribu(etonnage, "min");
	unsigned maxton = XmlAttribu(etonnage, "max");
	tonnage = minton + rnd(maxton - minton + 1);
	TiXmlElement* emotion = hdftdship.FirstChildElement("motion").Element();
	system::sys().myassert(emotion != 0, string("motion node missing in ")+specfilename);
	double tmp = 0;
	if (emotion->Attribute("maxspeed", &tmp))
		max_speed_forward = kts2ms(tmp);
	tmp = 0;
	if (emotion->Attribute("maxrevspeed", &tmp))
		max_speed_reverse = kts2ms(tmp);
	emotion->Attribute("acceleration", &max_accel_forward);
	tmp = 0;
	if (emotion->Attribute("turnrate", &tmp))
		turn_rate = tmp;
	TiXmlElement* esmoke = hdftdship.FirstChildElement("smoke").Element();
	smoke_type = 0;
	if (esmoke) {
		int smtype = 0;
		esmoke->Attribute("type", &smtype);
		if (smtype > 0) {
			TiXmlElement* esmpos = esmoke->FirstChildElement("position");
			system::sys().myassert(esmpos != 0, string("no smoke position given in ")+specfilename);
			esmpos->Attribute("x", &smokerelpos.x);
			esmpos->Attribute("y", &smokerelpos.y);
			esmpos->Attribute("z", &smokerelpos.z);
			smoke_type = 1;
		}
	}
	TiXmlElement* eai = hdftdship.FirstChildElement("ai").Element();
	system::sys().myassert(eai != 0, string("ai node missing in ")+specfilename);
	string aitype = XmlAttrib(eai, "type");
	if (aitype == "dumb") myai = new ai(this, ai::dumb);
	else if (aitype == "escort") myai = new ai(this, ai::escort);
	else if (aitype == "none") myai = 0;
	else system::sys().myassert(false, string("illegal AI type in ") + specfilename);
	TiXmlElement* efuel = hdftdship.FirstChildElement("fuel").Element();
	system::sys().myassert(efuel != 0, string("fuel node missing in ")+specfilename);
	fuel_capacity = XmlAttribu(efuel, "capacity");
	efuel->Attribute("consumption_a", &fuel_value_a);
	efuel->Attribute("consumption_t", &fuel_value_t);
}



ship::~ship()
{
	delete myai;
}



void ship::sink(void)
{
	sea_object::kill();
}



void ship::change_rudder(int to)
{
	if (to >= rudderfullleft && to <= rudderfullright)
		rudder_to = to;
	else
		rudder_to = ruddermidships;
	head_to_fixed = false;
}



void ship::rudder_left(void)
{
	if (rudder_to > rudderfullleft)
		--rudder_to;
	head_to_fixed = false;
}



void ship::rudder_right(void)
{
	if (rudder_to < rudderfullright)
		++rudder_to;
	head_to_fixed = false;
}



void ship::rudder_hard_left(void)
{
	rudder_to = rudderfullleft;
	head_to_fixed = false;
}



void ship::rudder_hard_right(void)
{
	rudder_to = rudderfullright;
	head_to_fixed = false;
}



void ship::rudder_midships(void)
{
	rudder_to = ruddermidships;
	head_to_fixed = false;
}



void ship::set_throttle(throttle_status thr)
{
	throttle = thr;
}



void ship::remember_position(void)
{
	previous_positions.push_front(get_pos().xy());
	if (previous_positions.size() > MAXPREVPOS)
		previous_positions.pop_back();
}	



double ship::get_throttle_speed(void) const
{
	double ms = get_max_speed();
	if (throttle <= 0) {
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
	} else {
		double sp = kts2ms(throttle);
		if (sp > ms) sp = ms;
		return sp;
	}
	return 0;
}



double ship::get_throttle_accel(void) const
{
	// Beware: a throttle of 1/3 doesn't mean 1/3 of engine acceleration
	// This is because drag raises quadratically.
	// we have: max_accel_forward / max_speed_forward^2 = drag_factor
	// and: drag = drag_factor * speed^2
	// get acceleration for constant throttled speed: accel = drag
	// solve:
	// accel = drag_factor * speed^2 = max_accel_forward * speed^2 / max_speed_forward^2
	// fixme: 2004/07/18: throttle to some speed would mean maximum acceleration until
	// we get close to this speed... but we don't set speed here but engine throttle...
	// fixme: reverse throttle doesn't work. obvious why...
	double speed_fac = get_throttle_speed() / max_speed_forward;
	return max_accel_forward * (speed_fac * speed_fac);
}



pair<angle, double> ship::bearing_and_range_to(const sea_object* other) const
{
	vector2 diff = other->get_pos().xy() - position.xy();
	return make_pair(angle(diff), diff.length());
}



angle ship::estimate_angle_on_the_bow(angle target_bearing, angle target_heading) const
{
	return (angle(180) + target_bearing - target_heading).value_pm180();
}



void ship::parse_attributes(TiXmlElement* parent)
{
	sea_object::parse_attributes(parent);
	
	TiXmlElement* emotion = TiXmlHandle(parent).FirstChildElement("motion").Element();
	if (emotion) {
		double tmp = 0;
		if (emotion->Attribute("heading", &tmp))
			heading = angle(tmp);
		tmp = 0;
		if (emotion->Attribute("speed", &tmp))
			velocity.y = kts2ms(tmp);
		string thr = XmlAttrib(emotion, "throttle");
		if (thr == "stop") throttle = stop;
		else if (thr == "reverse") throttle = reverse;
		else if (thr == "aheadlisten") throttle = aheadlisten;
		else if (thr == "aheadsonar") throttle = aheadsonar;
		else if (thr == "aheadslow") throttle = aheadslow;
		else if (thr == "aheadhalf") throttle = aheadhalf;
		else if (thr == "aheadfull") throttle = aheadfull;
		else if (thr == "aheadflank") throttle = aheadflank;
		else throttle = atoi(thr.c_str());
	}
	// fixme: parse permanent_turn,head_chg,head_to,rudder  maybe also alive_stat,previous_positions
	// parse tonnage, fuel level, damage status, fixme
}



void ship::load(istream& in, game& g)
{
	sea_object::load(in, g);

	if (read_bool(in))
		myai = new ai(in, g);
	
	tonnage = read_u32(in);
	stern_damage = damage_status(read_u8(in));
	midship_damage = damage_status(read_u8(in));
	bow_damage = damage_status(read_u8(in));
	fuel_level = read_double(in);
}

void ship::save(ostream& out, const game& g) const
{
	sea_object::save(out, g);

	write_bool(out, (myai != 0));
	if (myai)
		myai->save(out, g);
	
	write_u32(out, tonnage);
	write_u8(out, stern_damage);
	write_u8(out, midship_damage);
	write_u8(out, bow_damage);
	write_double(out, fuel_level);
}



void ship::simulate(game& gm, double delta_time)
{
	sea_object::simulate(gm, delta_time);
	if ( myai )
		myai->act(gm, delta_time);

	// calculate sinking
	if (is_dead()) {
		position.z -= delta_time * SINK_SPEED;
		if (position.z < -50)	// used for ships.
			destroy();
		throttle = stop;
		rudder_midships();
		return;
	}

	// Adjust fuel_level.
	calculate_fuel_factor ( delta_time );
	
	// place smoke particle generation logic here fixme
	if (is_alive() && smoke_type != 0) {//replace by has_particle
		double produce_time = smoke_particle::get_produce_time();
		double t = myfmod(gm.get_time(), produce_time);
		if (t < produce_time && t + delta_time >= produce_time) {
			gm.spawn_particle(new smoke_particle(position + smokerelpos));//fixme: maybe add some random offset
		}
	}

	// steering logic, adjust rudder pos so that heading matches head_to
	if (head_to_fixed) {
		// check if we should turn left or right
		bool turn_rather_right = (heading.is_cw_nearer(head_to));
//cout<<this<<" logic: heading " << heading.value() << " head_to " << head_to.value() << " trr " << turn_rather_right << " rudder_to " << rudder_to << " rudder_pos " << rudder_pos << " \n";
		rudder_to = (turn_rather_right) ? rudderfullright : rudderfullleft;
//cout <<this<<" logic2 rudder_to " << rudder_to << " turn velo " << turn_velocity << "\n";
		// check if we approach head_to (brake by turning rudder to the opposite)
		// if time need to set the rudder to midships is smaller than the time until heading
		// passes over head_to, we have to brake.
		double angledist = fabs((heading - head_to).value_pm180());
		double time_to_pass = (fabs(turn_velocity) < 0.01) ? 1e30 : angledist / fabs(turn_velocity);
		double time_to_midships = fabs(rudder_pos) / max_rudder_turn_speed;
//cout <<this<<" logic3 angledist " << angledist << " timetopass " << time_to_pass << " time_to_ms " << time_to_midships << "\n";
		double damping_factor = 0.5;	// set to > 0 to brake earlier, fixme set some value
		if (time_to_pass < time_to_midships + damping_factor) {
			rudder_to = (turn_rather_right) ? rudderfullleft : rudderfullright;
//cout <<this<<" near target! " << rudder_to << "\n";
		}
		// check for final rudder midships, fixme adapt values...
		if (angledist < 0.5 && fabs(rudder_pos) < 1.0) {
			rudder_to = ruddermidships;
//cout <<this<<" dest reached " << angledist << "," << fabs(rudder_pos) << "\n";
			head_to_fixed = false;
		}
	}
	
	// Adjust rudder
	// rudder_to with max_rudder_angle gives set rudder angle.
	double rudder_angle_set = max_rudder_angle * rudder_to / 2;
	// current angle is rudder_pos. rudder moves with constant speed to set pos (or 0).
	double max_rudder_turn_dist = max_rudder_turn_speed * delta_time;
	double rudder_d = rudder_angle_set - rudder_pos;
	if (fabs(rudder_d) <= max_rudder_turn_dist) {	// if rudder_d is 0, nothing happens.
		rudder_pos = rudder_angle_set;
	} else {
		if (rudder_d < 0) {
			rudder_pos -= max_rudder_turn_dist;
		} else {
			rudder_pos += max_rudder_turn_dist;
		}
	}
}



void ship::fire_shell_at(const vector2& pos)
{
	// fixme!!!!!!
}



void ship::head_to_ang(const angle& a, bool left_or_right)	// true == left
{
	head_to = a;
	//fixme: very crude... or use rudderleft/rudderright here (not full rudder?)
	//not crude with steering logic somewhere else... in simulate
	rudder_to = (left_or_right) ? rudderfullleft : rudderfullright;
	head_to_fixed = true;
}



bool ship::damage(const vector3& fromwhere, unsigned strength)
{
	damage_status& where = midship_damage;//fixme
	int dmg = int(where) + strength;
	if (dmg > wrecked) where = wrecked; else where = damage_status(dmg);
	// fixme:
	stern_damage = midship_damage = bow_damage = wrecked;
	sink();
	return true;
}



unsigned ship::calc_damage(void) const
{
	if (bow_damage == wrecked || midship_damage == wrecked || stern_damage == wrecked)
		return 100;
	unsigned dmg = unsigned(round(15*(bow_damage + midship_damage + stern_damage)));
	return dmg > 100 ? 100 : dmg;
}



double ship::get_roll_factor(void) const
{
	return 400.0 / (get_tonnage() + 6000.0);	// fixme: rather simple yet. should be overloaded by each ship
}



double ship::get_noise_factor (void) const
{
    return get_throttle_speed () / max_speed_forward;
}



//fixme: deceleration is to low at low speeds, causing the sub the turn a LONG time after
//rudder is midships/screws stopped. Is fixed by setting drag to linear at speeds < 1.0
//fixme: drag can go nuts when time is scaled causing NaN in double...
//this is because damping becomes to crude at high time scale
vector3 ship::get_acceleration(void) const		// drag must be already included!
{
	// acceleration of ship depends on rudder.
	// forward acceleration is max_accel_forward * cos(rudder_ang)
	//fixme 2004/07/18: the drag is too small. engine stop -> the ship slows down but
	//to slowly, especially on low speeds. it would take a LONG time until it comes
	//to an halt.
	//compute max_accel_forward from mass and engine power to have a rough guess.
	//in general: Power/(rpm * screw_radius * mass) = accel
	//Power: engine Power (kWatts), rpm (screw turns per second), mass (ship's mass)
	//SubVIIc: ~3500kW, rad=0.5m, rpm=2 (?), mass=750000kg -> acc=4,666. a bit much...
	double speed = get_speed();
	double speed2 = speed*speed;
	if (fabs(speed) < 1.0) speed2 = fabs(speed)*max_speed_forward;
	double drag_factor = (speed2) * max_accel_forward / (max_speed_forward*max_speed_forward);
	double acceleration = get_throttle_accel() * cos(rudder_pos * M_PI / 180.0);
	if (speed > 0) drag_factor = -drag_factor;
	return vector3(0, acceleration + drag_factor, 0);
}



double ship::get_turn_acceleration(void) const	// drag must be already included!
{
	// acceleration of ship depends on rudder state and actual forward speed (linear).
	// angular acceleration (turning) is speed * sin(rudder_ang) * factor
	// this is acceleration around local z-axis.
	// the factor depends on rudder area etc.
	//fixme: do we have to multiply in some factor? we have angular values here not linear...
	//double drag_factor = some_factor * current_turn_speed^2
	//some_factor is given by turn rate
	double speed = get_speed();
	double tv2 = turn_velocity*turn_velocity;
	if (fabs(turn_velocity) < 1.0) tv2 = fabs(turn_velocity) * max_angular_velocity;
	double accel_factor = 1.0;	// given by turn rate, influenced by rudder area...
	double max_turn_accel = accel_factor * max_speed_forward * sin(max_rudder_angle * M_PI / 180.0);
	double drag_factor = (tv2) * max_turn_accel / (max_angular_velocity*max_angular_velocity);
	double acceleration = accel_factor * speed * sin(rudder_pos * M_PI / 180.0);
	if (turn_velocity > 0) drag_factor = -drag_factor;
//cout << "TURNING: accel " << acceleration << " drag " << drag_factor << " max_turn_accel " << max_turn_accel << " turn_velo " << turn_velocity << " heading " << heading.value() << " tv2 " << tv2 << "\n";
//cout << "get_rot_accel for " << this << " rudder_pos " << rudder_pos << " sin " << sin(rudder_pos * M_PI / 180.0) << " max_turn_accel " << max_turn_accel << "\n";
	return acceleration + drag_factor;
}



void ship::calculate_fuel_factor ( double delta_time )
{
	fuel_level -= delta_time * get_fuel_consumption_rate ();
}
