/*
Danger from the Deep - Open source submarine simulation
Copyright (C) 2003-2006  Thorsten Jordan, Luis Barrancos and others.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

// ships
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "ship.h"
#include "model.h"
#include "game.h"
#include "date.h"
#include "sensors.h"
#include "ai.h"
#include "system.h"
#include "particle.h"
#include "gun_shell.h"
#include "global_data.h"
#include "physics.h"

using std::vector;
using std::list;
using std::string;
using std::map;
using std::pair;
using std::make_pair;

map<double, map<double, double> > ship::dist_angle_relation;
#define MAX_INCLINATION 45.0
#define MAX_DECLINATION -20.0
#define ANGLE_GAP 0.1
#define GUN_RELOAD_TIME 5.0

//fixme: redefine display, call base display

void ship::fill_dist_angle_relation_map(const double initial_velocity)
{
	if (dist_angle_relation.find(initial_velocity) == dist_angle_relation.end())
	{
		for (double a = 0; a > MAX_DECLINATION+ANGLE_GAP; a -= ANGLE_GAP) {
			angle elevation(a);
			double z = 4;	// meters, initial height above water
			double vz = initial_velocity * elevation.sin();
			double dist = 0;
			double vdist = initial_velocity * elevation.cos();
			
			for (double dt = 0; dt < 120.0; dt += 0.001) {
				dist += vdist * dt;
				z += vz * dt;
				vz += 0 - GRAVITY * dt;
				if (z <= 0) break;
			}
			
			dist_angle_relation[initial_velocity][dist] = a;
		}
		
		for (double a = 0; a < MAX_INCLINATION+ANGLE_GAP; a += ANGLE_GAP) {
			angle elevation(a);
			double z = 4;	// meters, initial height above water
			double vz = initial_velocity * elevation.sin();
			double dist = 0;
			double vdist = initial_velocity * elevation.cos();
			
			for (double dt = 0; dt < 120.0; dt += 0.001) {
				dist += vdist * dt;
				z += vz * dt;
				vz += 0 - GRAVITY * dt;
				if (z <= 0) break;
			}
			
			dist_angle_relation[initial_velocity][dist] = a;
		}
	}
}



ship::ship(game& gm_, const xml_elem& parent)
	: sea_object(gm_, parent),
	  throttle(0),
	  rudder_pos(0),
	  rudder_to(0),
	  max_rudder_angle(40),
	  max_rudder_turn_speed(10),
	  max_angular_velocity(2),
	  head_to_fixed(false),
	  max_accel_forward(1),
	  max_speed_forward(10),
	  max_speed_reverse(0),
	  fuel_level(0),
	  myfire(0),		
	  gun_manning_is_changing(false),
	  maximum_gun_range(0.0)
{
	xml_elem eclassification = parent.child("classification");
	string typestr = eclassification.attr("type");
	if (typestr == "warship") myclass = WARSHIP;
	else if (typestr == "escort") myclass = ESCORT;
	else if (typestr == "merchant") myclass = MERCHANT;
	else if (typestr == "submarine") myclass = SUBMARINE;
	else if (typestr == "torpedo") myclass = TORPEDO;
	else throw error(string("illegal ship type in ") + specfilename);

	if (myclass == TORPEDO) {
		tonnage = 0;
	} else {
		xml_elem etonnage = parent.child("tonnage");
		unsigned minton = etonnage.attru("min");
		unsigned maxton = etonnage.attru("max");
		tonnage = minton + rnd(maxton - minton + 1);
	}
	xml_elem emotion = parent.child("motion");
	if (myclass == TORPEDO) {
		// fixme: not stored yet, but it should be...
		max_speed_forward = 0;
		max_speed_reverse = 0;
	} else {
		max_speed_forward = kts2ms(emotion.attrf("maxspeed"));
		max_speed_reverse = kts2ms(emotion.attrf("maxrevspeed"));
	}
	max_accel_forward = emotion.attrf("acceleration");
	turn_rate = emotion.attrf("turnrate");
	// compute max_angular_velocity from turn_rate:
	// turn_rate = angles/m_forward at max. speed.
	// so max_angular_veloctiy = angles/time * m/s
	max_angular_velocity = turn_rate * max_speed_forward;

	for (xml_elem::iterator it = parent.iterate("smoke"); !it.end(); it.next()) {
		smoke.push_back(make_pair(it.elem().attru("type"), it.elem().attrv3()));
	}

	if (parent.has_child("ai")) {
		xml_elem eai = parent.child("ai");
		string aitype = eai.attr("type");
		if (aitype == "dumb") myai.reset(new ai(this, ai::dumb));
		else if (aitype == "escort") myai.reset(new ai(this, ai::escort));
		else if (aitype == "none") myai.reset();
		else throw error(string("illegal AI type in ") + specfilename);
	}
	if (parent.has_child("fuel")) {
		xml_elem efuel = parent.child("fuel");
		fuel_capacity = efuel.attru("capacity");
		fuel_value_a = efuel.attrf("consumption_a");
		fuel_value_t = efuel.attrf("consumption_t");
	}

	if (parent.has_child("gun_turrets")) {
		xml_elem eturrets = parent.child("gun_turrets");
		for (xml_elem::iterator it = eturrets.iterate("turret"); !it.end(); it.next()) {
			unsigned num_barrels = it.elem().attru("barrels");
			gun_turret new_turret;
			new_turret.shell_capacity = it.elem().attru("shell_capacity");
			new_turret.num_shells_remaining = new_turret.shell_capacity;
			new_turret.initial_velocity = it.elem().attrf("initial_velocity");
			new_turret.max_declination = it.elem().attri("max_declination");
			new_turret.max_inclination = it.elem().attri("max_inclination");
			new_turret.time_to_man = it.elem().attrf("time_to_man");
			new_turret.time_to_unman = it.elem().attrf("time_to_unman");
			new_turret.shell_damage = it.elem().attrf("shell_damage");
			new_turret.start_of_exclusion_radius = it.elem().attri("exclusion_radius_start");
			new_turret.end_of_exclusion_radius = it.elem().attri("exclusion_radius_end");
			new_turret.calibre = it.elem().attrf("calibre");
			new_turret.gun_barrels.resize(num_barrels);
			
			// setup angles map for this initial velocity
			fill_dist_angle_relation_map(new_turret.initial_velocity);
			calc_max_gun_range(new_turret.initial_velocity);
			
			gun_turrets.push_back(new_turret);		
		}
	}

	// set some sensible values for sonar noise (testing)
	for (unsigned i = 0; i < noise::NR_OF_FREQUENCY_BANDS; ++i) {
		noise_sign.band_data[i].basic_noise_level
			= noise_signature::typical_noise_signature[unsigned(myclass)][i];
		// 1 dB per m/s, maybe non-linear (higher speed = more high frequencies?)
		noise_sign.band_data[i].speed_factor = 1.0;
	}
}



void ship::sink()
{
	sea_object::set_inactive();
	if (myfire) {
		myfire->kill();
		myfire = 0;
	}
}



void ship::ignite()
{
	if (myfire) {
		myfire->kill();
		myfire = 0;
	}
	myfire = new fire_particle(get_pos());
	gm.spawn_particle(myfire);
}



void ship::change_rudder(int to)
{
	if (to >= rudderfullleft && to <= rudderfullright)
		rudder_to = to;
	else
		rudder_to = ruddermidships;
	head_to_fixed = false;
}



void ship::rudder_left()
{
	if (rudder_to > rudderfullleft)
		--rudder_to;
	head_to_fixed = false;
}



void ship::rudder_right()
{
	if (rudder_to < rudderfullright)
		++rudder_to;
	head_to_fixed = false;
}



void ship::rudder_hard_left()
{
	rudder_to = rudderfullleft;
	head_to_fixed = false;
}



void ship::rudder_hard_right()
{
	rudder_to = rudderfullright;
	head_to_fixed = false;
}



void ship::rudder_midships()
{
	rudder_to = ruddermidships;
	head_to_fixed = false;
}



void ship::set_throttle(int thr)
{
	throttle = thr;
}



void ship::remember_position(double t)
{
	// store 4 values: x,y position, time, speed.
	// with these we can build the foam trail much better
	// time for decay, and speed for width. width is shipwidth + speedfactor * speed,
	// where factor grows over time in the first seconds, then is constant, like 1-e^-x
	// do NOT remember position if it is closer than 5m to the last position.
	// problem is that for non-moving objects all positions are identical.
	vector2 p = get_pos().xy();
	if (previous_positions.empty() || previous_positions.front().pos.square_distance(p) >= 25.0) {
		previous_positions.push_front(prev_pos(p, get_heading().direction(), t, get_speed()));
		if (previous_positions.size() > TRAIL_LENGTH)
			previous_positions.pop_back();
	}
}	



double ship::get_throttle_speed() const
{
	double ms = get_max_speed();
	if (throttle <= 0) {
		switch (throttle) {
		case reversefull: return -ms*0.5f; // 1/5 back slower than forward 
		case reversehalf: return -ms*0.33333f; // 1/3
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



double ship::get_throttle_accel() const
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
	double speed_fac = get_throttle_speed() / max_speed_forward;
	// fixme: reverse throttle doesn't work. obvious why... hack below is nasty
	int signal = speed_fac > 0 ? 1 : -1;
	return max_accel_forward * (speed_fac * speed_fac) * signal;
}



bool ship::screw_cavitation() const
{
	return get_throttle_speed() >= 0.75 * get_max_speed();
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



void ship::load(const xml_elem& parent)
{
	sea_object::load(parent);
	tonnage = parent.child("tonnage").attru();
	xml_elem st = parent.child("steering");
	throttle = throttle_status(st.attri("throttle"));
	rudder_pos = st.attrf("rudder_pos");
	rudder_to = st.attri("rudder_to");
	head_to_fixed = st.attrb("head_to_fixed");
	head_to = angle(st.attrf("head_to"));
	xml_elem dm = parent.child("damage");
	bow_damage = damage_status(dm.attru("bow"));
	midship_damage = damage_status(dm.attru("midship"));
	stern_damage = damage_status(dm.attru("stern"));
	fuel_level = parent.child("fuel_level").attrf();

	// fixme load that
	//list<prev_pos> previous_positions;
	//class particle* myfire;

	//fixme: load per gun data
	//bool gun_manning_is_changing;

#if 0	
	// gun turrets
	unsigned long num_turrets = read_u32(in);

	gun_turrets.clear();
	for (unsigned long x = 0; x < num_turrets; x++)
	{
		struct gun_turret turret;
		unsigned long num_barrels = 0;
		
		turret.num_shells_remaining = read_u32(in);
		turret.shell_capacity = read_u32(in);
		turret.initial_velocity = read_double(in);		
		turret.max_declination = read_i32(in);
		turret.max_inclination = read_i32(in);
		turret.time_to_man = read_double(in);
		turret.time_to_unman = read_double(in);
		turret.is_gun_manned = read_bool(in);
		turret.manning_time = read_double(in);
		turret.shell_damage = read_double(in);
		turret.start_of_exclusion_radius = read_u32(in);
		turret.end_of_exclusion_radius = read_u32(in);
		turret.calibre = read_double(in);

		num_barrels = read_u32(in);
		
		for (unsigned long x = 0; x < num_barrels; x++)
		{
			struct gun_barrel new_barrel;	
			
			new_barrel.load_time_remaining = read_double(in);
			new_barrel.last_elevation = read_double(in);
			new_barrel.last_azimuth = read_double(in);
			
			turret.gun_barrels.push_back(new_barrel);
		}
		
		// setup angles map for this initial velocity
		fill_dist_angle_relation_map(turret.initial_velocity);
		calc_max_gun_range(turret.initial_velocity);
		
		gun_turrets.push_back(turret);
	}	
#endif
}



void ship::save(xml_elem& parent) const
{
	sea_object::save(parent);
	parent.add_child("tonnage").set_attr(tonnage);
	xml_elem st = parent.add_child("steering");
	st.set_attr(int(throttle), "throttle");
	st.set_attr(rudder_pos, "rudder_pos");
	st.set_attr(rudder_to, "rudder_to");
	st.set_attr(head_to_fixed, "head_to_fixed");
	st.set_attr(head_to.value(), "head_to");
	xml_elem dm = parent.add_child("damage");
	dm.set_attr(unsigned(bow_damage), "bow");
	dm.set_attr(unsigned(midship_damage), "midship");
	dm.set_attr(unsigned(stern_damage), "stern");
	parent.add_child("fuel_level").set_attr(fuel_level);

	// fixme save that
	//list<prev_pos> previous_positions;
	//class particle* myfire;

	//fixme: save per gun data
	//bool gun_manning_is_changing;

#if 0 // old code: check which values are VARIABLE and save only them!
	// gun turrets
	write_u32(out, gun_turrets.size());
	
	const_gun_turret_itr turret = gun_turrets.begin();
	while (turret != gun_turrets.end())
	{
		write_u32(out, turret->num_shells_remaining);
		write_u32(out, turret->shell_capacity);
		write_double(out, turret->initial_velocity);		
		write_i32(out, turret->max_declination);
		write_i32(out, turret->max_inclination);
		write_double(out, turret->time_to_man);
		write_double(out, turret->time_to_unman);
		write_bool(out, turret->is_gun_manned);
		write_double(out, turret->manning_time);
		write_double(out, turret->shell_damage);
		write_u32(out, turret->start_of_exclusion_radius);
		write_u32(out, turret->end_of_exclusion_radius);
		write_double(out, turret->calibre);
		
		write_u32(out, turret->gun_barrels.size());
		
		const_gun_barrel_itr barrel = turret->gun_barrels.begin();
		while (barrel != turret->gun_barrels.begin())
		{		
			write_double(out, barrel->load_time_remaining);
			write_double(out, barrel->last_elevation.value());
			write_double(out, barrel->last_azimuth.value());
			
			barrel++;
		}
								  
		turret++;
	}
#endif
}



void ship::simulate(double delta_time)
{
	sea_object::simulate(delta_time);

	if ( myai.get() )
		myai->act(gm, delta_time);

	// calculate sinking
	if (is_inactive()) {
		position.z -= delta_time * SINK_SPEED;
		if (position.z < -50)	// used for ships.
			kill();
		throttle = stop;
		rudder_midships();
		return;
	}

	// Adjust fuel_level.
	calculate_fuel_factor(delta_time);

	// adjust fire pos if burning
	if (myfire) {
		myfire->set_pos(get_pos() + vector3(0, 0, 12));
	}

	if (causes_spray()) {
		double v = velocity.length();
		if (v > 0.1) {
			double produce_time = 2.0/v;
			double t = myfmod(gm.get_time(), produce_time);
			if (t + delta_time >= produce_time) {
				vector3 forward = global_velocity.normal();
				vector3 sideward = forward.cross(vector3(0, 0, 1)).normal() * 2.0;//speed 2.0 m/s
				vector3 spawnpos = get_pos() + forward * (get_length() * 0.5);
				gm.spawn_particle(new spray_particle(spawnpos, sideward));
				gm.spawn_particle(new spray_particle(spawnpos, -sideward));
			}
		}
	}
	
	// smoke particle generation logic
	if (is_alive()) {
		for (list<pair<unsigned, vector3> >::iterator it = smoke.begin(); it != smoke.end(); ++it) {
			double produce_time = 1e10;
			switch (it->first) {
			case 1: produce_time = smoke_particle::get_produce_time(); break;
			case 2: produce_time = smoke_particle_escort::get_produce_time(); break;
			}
			double t = myfmod(gm.get_time(), produce_time);
			if (t + delta_time >= produce_time) {
				particle* p = 0;
				vector3 ppos = position + it->second;//fixme: maybe add some random offset
				switch (it->first) {
				case 1: p = new smoke_particle(ppos); break;
				case 2: p = new smoke_particle_escort(ppos); break;
				}
				gm.spawn_particle(p);
			}
		}
	}

	// steering logic, adjust rudder pos so that heading matches head_to
	if (head_to_fixed) {
		steering_logic();
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
	
	// gun turrets
	gun_turret_itr gun_turret = gun_turrets.begin();	
	while (gun_turret != gun_turrets.end()) {
		// Note! condition must be greater than zero, so that nothing happens when manning time is zero,
		// like at begin of mission.
		if (gun_turret->manning_time > 0.0) {
			gun_turret->manning_time -= delta_time;
			if (gun_turret->manning_time <= 0.0) {
				gun_turret->is_gun_manned = !gun_turret->is_gun_manned;					
				gun_manning_is_changing = false;
				gun_manning_changed(gun_turret->is_gun_manned);
			}
		}
		
		if (gun_turret->manning_time <= 0.0) {
			gun_barrel_itr gun_barrel = gun_turret->gun_barrels.begin();
			while (gun_barrel != gun_turret->gun_barrels.end()) {		
				if (gun_barrel->load_time_remaining > 0.0)
					gun_barrel->load_time_remaining -= delta_time;
				gun_barrel++;
			}
		}
			
		gun_turret++;
	}	
}



void ship::steering_logic()
{
	/* Helmsman simulation
	   New idea: add turn_velocity * time_to_midships * factor to target heading,
	   so that when turn velocity is high, target course is nearer than set,
	   then react when course is "missed" (factor=1) or near miss (factor=2),
	   this will lead to early braking and hitting target course better.
	   Low velocities -> Low drift.
	   Could be better for torps too.
	   We don't need states with this model.
	*/

	double time_to_midships = fabs(rudder_pos) / max_rudder_turn_speed;
	angle heading2 = heading + angle(-turn_velocity * time_to_midships * 1.5);
	bool turn_rather_right = (heading2.is_cw_nearer(head_to));
	double angledist = fabs((heading2 - head_to).value_pm180());
	std::cout <<this<<" angledist " << angledist << " heading=" << heading.value() << " head_to=" << head_to.value() << " heading2=" << heading2.value() << " time_to_ms " << time_to_midships << "\n";
	if (angledist < 0.25 && fabs(rudder_pos) < 1.0) {
		head_to_fixed = false;
		rudder_to = ruddermidships;
		std::cout << "reached course, diff=" << head_to.value() - heading.value() << " tv=" << turn_velocity << "\n";
	} else {
		// we need to do something
		rudder_to = (turn_rather_right) ? rudderfullright : rudderfullleft;
	}
}



void ship::head_to_ang(const angle& a, bool left_or_right)	// true == left
{
	head_to = a;
	//cout << "head to ang: " << a.value() << " left? " << left_or_right << "\n";
	//fixme: very crude... or use rudderleft/rudderright here (not full rudder?)
	//not crude with steering logic somewhere else... in simulate
	rudder_to = (left_or_right) ? rudderfullleft : rudderfullright;
	//cout << "rudder_to=" << rudder_to << "\n";
	head_to_fixed = true;
}



bool ship::damage(const vector3& fromwhere, unsigned strength)
{
	if (invulnerable)
		return false;

	damage_status& where = midship_damage;//fixme
	int dmg = int(where) + strength;
	if (dmg > wrecked) where = wrecked; else where = damage_status(dmg);
	// fixme:
	if (rand() % 2 == 0) {
		stern_damage = midship_damage = bow_damage = wrecked;
		sink();
		return true;
	} else{
		stern_damage = midship_damage = bow_damage = mediumdamage;
		return false;
	}
}



unsigned ship::calc_damage() const
{
	if (bow_damage == wrecked || midship_damage == wrecked || stern_damage == wrecked)
		return 100;
	unsigned dmg = unsigned(round(15*(bow_damage + midship_damage + stern_damage)));
	return dmg > 100 ? 100 : dmg;
}



double ship::get_roll_factor() const
{
	return 400.0 / (get_tonnage() + 6000.0);	// fixme: rather simple yet. should be overloaded by each ship
}



double ship::get_noise_factor () const
{
    return get_throttle_speed () / max_speed_forward;
}



void ship::compute_force_and_torque(vector3& F, vector3& T) const
{
	// we need to add the force/torque generated from tide.
	// for certain sample points around the hull we compute the draught
	// and from that a lift force. It can be negative because of gravity
	// or positive because of buoyancy.
	// The sum of all these forces is total lift force,
	// the sum of the cross products between these forces and their relative
	// vectors where they act (the points around the hull) give the torque.
	// this is rather easy to code.
	// sample 5-10 points along length axis of ship, compute real world xy
	// coordinate of it and then water height there, compute draught
	// by comparing z-pos of that point (real world!) with water height.
	// for rolling, we need to compute a 2d array of points, e.g. 2*5 or 3*5.
	// costly, but needed for realism.
	// To simulate this we need to "distribute" mass over the sample points
	// and the ship's volume that is under water, too.
	// This can be computed if we distribute ship's area (when seen from above),
	// then draught and area give volume under water.
	// We use an evenly distributed mass/volume as start.
	// 5 points: center, bow, stern, port, starboard (outside points at 2/3 length/width).
	const double buoyancy_factors[5] = { 0.25, 0.25, 0.25, 0.125, 0.125 };
	const vector3 buoyancy_vec[5] = {
		vector3(),
		vector3(0, +size3d.y*(1/3.0), 0),
		vector3(0, -size3d.y*(1/3.0), 0),
		vector3(-size3d.x*(1/3.0), 0, 0),
		vector3(+size3d.x*(1/3.0), 0, 0)
	};
	const vector3 buoyancy_vec_nrml[5] = {
		vector3(),
		vector3(0, +1, 0),
		vector3(0, -1, 0),
		vector3(-1, 0, 0),
		vector3(+1, 0, 0)
	};
	double lift_forces[5];
	double lift_force_sum = 0;
	const double dr_area = size3d.x * size3d.y;
	vector3 dr_torque;
	for (int i = 0; i < 5; ++i) {
		vector3 realworldpos = orientation.rotate(buoyancy_vec[i]) + position;
		double waterheight = gm.compute_water_height(realworldpos.xy());
		double draught = waterheight - realworldpos.z;
		double volume = dr_area * buoyancy_factors[i] * draught;
		double dr_mass = mass * buoyancy_factors[i];
		double f_gravity = dr_mass * GRAVITY;
		double f_lift = volume * 1000.0 * GRAVITY; // 1000kg/m^3
		lift_forces[i] = f_lift - f_gravity;
		lift_force_sum += lift_forces[i];
		vector3 lift_torque = vector3(0, 0, lift_forces[i]).cross(buoyancy_vec_nrml[i]);
		DBGOUT3(i,lift_forces[i],lift_torque);
		dr_torque += lift_torque;
	}
	// fixme: damping!!! without this sub seems to capsize over time.
	//it slows down turning, but doesn't stop it. wtf?!
	dr_torque.y += -roll_velocity*roll_velocity * 1000000.0; // damping
	dr_torque.x += -pitch_velocity*pitch_velocity * 1000000.0; // damping
	DBGOUT2(dr_torque,lift_force_sum);

#if 0
	vector3 bow_buoy_pos = orientation.rotate(0,  size3d.y*(1/3.0), 0) + position;
	vector3 stn_buoy_pos = orientation.rotate(0, -size3d.y*(1/3.0), 0) + position;
	vector3 lef_buoy_pos = orientation.rotate(-size3d.x*(1/3.0), 0, 0) + position;
	vector3 rig_buoy_pos = orientation.rotate( size3d.x*(1/3.0), 0, 0) + position;
	double bow_wh = gm.compute_water_height(bow_buoy_pos.xy());
	double stn_wh = gm.compute_water_height(stn_buoy_pos.xy());
	double lef_wh = gm.compute_water_height(lef_buoy_pos.xy());
	double rig_wh = gm.compute_water_height(rig_buoy_pos.xy());
//	DBGOUT5(lef_buoy_pos, rig_buoy_pos, lef_wh, rig_wh, roll_velocity);
	double mid_wh = gm.compute_water_height(position.xy());
	double bow_deltaz = bow_wh - bow_buoy_pos.z;
	double stn_deltaz = stn_wh - stn_buoy_pos.z;
	double lef_deltaz = lef_wh - lef_buoy_pos.z;
	double rig_deltaz = rig_wh - rig_buoy_pos.z;
	double mid_deltaz = mid_wh - position.z;
	if (pitch_velocity > 0) {
		// stern goes down
		stn_deltaz += pitch_velocity*pitch_velocity;
	} else {
		// bow goes down
		bow_deltaz += pitch_velocity*pitch_velocity;
	}
	if (roll_velocity > 0) {
		// right goes down
		rig_deltaz += roll_velocity*roll_velocity;
	} else {
		// left goes down
		lef_deltaz += roll_velocity*roll_velocity;
	}
	//double lift_gravity_force = bow_deltaz + mid_deltaz + stn_deltaz;
	//fixme: use 5 sample points, bow,stern,left,right,center
	//with mass distribution 1/4,1/4,1/8,1/8,1/4.
	double tide_torque = (bow_deltaz - stn_deltaz) * size3d.y*0.5  * 10000.0;
	double roll_torque = (lef_deltaz - rig_deltaz) * size3d.x*0.5  * 10000.0;
//	DBGOUT4(tide_torque,roll_torque,pitch_velocity,roll_velocity);
#endif

	// fixme: torpedoes MUST NOT be affected by tide.

	// fixme: for this we
	// need to move the water data from user_interface to class game.
	// since water construction is multi threaded, we have to check barriers,
	// the call to finish_construction() in water.
	// if class game is constructed before user_interface, we haven't much
	// for parallelism here... yes, in subsim.cpp, run_game() the ui
	// is created after the game is constructed.
	// we can parallelize water construction by letting 2 levels compute in parallel.

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
	F = vector3(0, acceleration + drag_factor, 0) * mass;
	F.z = lift_force_sum; // buoyancy/gravity



	// torque:
	// there are two forces leading to torque:
	// - water flowing over the rudders that is deflected to the side giving a sidewards
	//   force at the end of the vessel
	// - drag that limits turn velocity, acts on whole body, but can be described
	//   as force acting on one end.
	// drag formula:
	// D = Drag_coefficient * density * velocity^2 * reference_area / 2.
	//     (kg/m^3 * m^2/s^2 * m^2 = kg*m/s^2 = mass * acceleration = force.)
	// And D_torque = D * z, where z is distance to turn center,
	// and z in [-L...L], thus the length of the hull is 2*L.
	// The velocity is not constant over the hull, but variies linearily along
	// the hull from turn center to outmost part.
	// We compute total drag torque with an integral over z.
	// We can get the area from the cross section data.
	// Density of water is 1000kg/m3 (1000) for ease of computation.
	// If the hull turns with x angles/second, and a point on it is z meters away
	// from center of turn, then it moves x/360 (or x/(2*Pi)) parts of a circle
	// per second. Circle diameter is 2*Pi*r where r=z.
	// Thus it moves 2*Pi*z * x/360 or 2*Pi*z * x/(2*Pi) = z * x m/s.
	// Let's compute in radians, its easier.
	// So velocity = z * tvr, where tvr is turn velocity in radians.
	// area per z is area/(2*L), this is needed for the integral.
	// D_torque = Dcoeff * densitiy * velocity^2 * area / 2 * z
	//          = Dcoeff * densitiy * z^2 * tvr^2 * area / 2 * z
	//          = Int z=-L...L   Dcoeff * densitiy * z^3 * tvr^2 * area/(2*L) / 2
	//          = Dcoeff * densitiy * tvr^2 * area / (4*L) * Int z=-L...L   z^3
	//          = Dcoeff * densitiy * tvr^2 * area / (4*L) * (L^4/4 - (-L)^4/4)
	// this would be 0, but drag for stern part of hull doesn't count negative, so:
	//          = Dcoeff * densitiy * tvr^2 * area / (4*L) * 2*L^4/4
	//          = Dcoeff * densitiy * tvr^2 * area * L^3 / 8
	// which makes sense, because outmost velocity depends on L, drag depends
	// on square of that (hence L^2) and torque also on L (hence L^3).
	const double drag_coefficient = get_turn_drag_coeff();
	const double water_density = 1000.0;
	// we take absolute value because we don't need the sign but the absolute value later
	double turn_velocity_rad = fabs(turn_velocity * (M_PI/180.0));
	// modify turn velocity a bit to make sure turning really stops on low turn speeds.
	const double tvf = (turn_velocity_rad < 0.1) ? 1.0 : 0.0;
	double tvr2 = turn_velocity_rad * turn_velocity_rad + turn_velocity_rad * tvf;
	double L = size3d.y * 0.5;
	double drag_torque = drag_coefficient * water_density * tvr2
		* get_turn_drag_area() * L*L*L * 0.125;
//	std::cout << "Turn drag torque=" << drag_torque << " Nm\n";
	if (turn_velocity > 0) drag_torque = -drag_torque;

	// negate rudder_pos here, because turning is mathematical, so rudder left means
	// rudder_pos < 0 and this means ccw turning and this means turn velocity > 0!
	double rudder_torque = L * get_turn_accel_factor() * speed * sin(-rudder_pos * M_PI / 180.0);
//	std::cout << "turn torque=" << acceleration << " Nm, or " << acceleration*2.0/size3d.y << " N\n";
// 	std::cout << "TURNING: accel " << acceleration << " drag " << drag_factor << " max_turn_accel " << max_turn_accel << " turn_velo " << turn_velocity << " heading " << heading.value() << " tv2 " << tv2 << "\n";
// 	std::cout << "get_rot_accel for " << this << " rudder_pos " << rudder_pos << " sin " << sin(rudder_pos * M_PI / 180.0) << " max_turn_accel " << max_turn_accel << "\n";
	rudder_torque += drag_torque;

	// positive torque turns counter clockwise!
#if 0
	vector2 hd = heading.direction();
	T = (hd * roll_torque - hd.orthogonal() * tide_torque).xyz(rudder_torque);
#else
	T = vector3(0, 0, rudder_torque) + dr_torque;
#endif
//	DBGOUT2(hd,T);
}



double ship::get_turn_drag_area() const
{
	// only take cross section that is below water! (roughly 1/2), rather a hack
	return mymodel->get_cross_section(90.0) * 0.5;
}



void ship::calculate_fuel_factor ( double delta_time )
{
	fuel_level -= delta_time * get_fuel_consumption_rate ();
}


ship::gun_status ship::fire_shell_at(const vector2& pos)
{
 if (!has_guns())
    return NO_GUNS;

	gun_status res = GUN_FIRED;
	gun_turret_itr gun_turret = gun_turrets.begin();
	gun_barrel_itr gun_barrel;
	//fixme! move dist_angle relation also,
	//maybe approximate that relation with splines.		
	
	while (gun_turret != gun_turrets.end()) {
		struct gun_turret *gun = &(*gun_turret);
		if (gun->num_shells_remaining > 0) {
			if (gun->is_gun_manned && gun->manning_time <= 0.0) {
				gun_barrel = gun_turret->gun_barrels.begin();
				while (gun_barrel != gun_turret->gun_barrels.end()) {				
					if (gun_barrel->load_time_remaining <= 0.0) {
						vector2 deltapos = pos - get_pos().xy();
						double distance = deltapos.length();
						angle direction(deltapos);

						double max_shooting_distance = (dist_angle_relation[gun_turret->initial_velocity].rbegin())->first;
						if (distance > max_shooting_distance) 
							res = TARGET_OUT_OF_RANGE;	// can't do anything
						
						if (GUN_FIRED == res) {
							if (!is_target_in_blindspot(gun, heading - direction)) {
								// initial angle: estimate distance and fire, remember angle
								// next shots: adjust angle after distance fault:
								//	estimate new distance from old and fault
								//	select new angle, fire.
								//	use an extra bit of correction for wind etc.
								//	to do that, we need to know where the last shot impacted!							
								angle elevation;
								if (true == calculate_gun_angle(distance, elevation, gun_turret->initial_velocity)) {														
									if (elevation.value() > gun->max_inclination) {
										res = TARGET_OUT_OF_RANGE;
									} else if (elevation.value() < gun->max_declination) {
										res = GUN_TARGET_IN_BLINDSPOT;
									} else {
										// fixme: snap angle values to simulate real cannon accuracy.
										gm.spawn_gun_shell(new gun_shell(gm, get_pos(), direction, elevation, gun->initial_velocity, gun->shell_damage), 
												   gun->calibre);
										gun->num_shells_remaining--;
										gun_barrel->load_time_remaining = GUN_RELOAD_TIME;
										gun_barrel->last_elevation = elevation;	
										gun_barrel->last_azimuth = direction;
									}
								} else {
									res = TARGET_OUT_OF_RANGE;	// unsuccesful angle, fixme
								}
							} else {
								res = GUN_TARGET_IN_BLINDSPOT;
							}
						}
					} else {
						res = RELOADING;
					}
					gun_barrel++;
				}
			} else {
				res = GUN_NOT_MANNED;
			}
		} else {
			res = NO_AMMO_REMAINING;
		}
		gun_turret++;
	}
		
	return res;
}



ship::gun_status ship::fire_shell_at(const sea_object& s)
{
	// we should not use current position but rather
	// estimated position at impact!
	// fixme: for a smart ai: try to avoid firing at friendly ships that are in line
	// of fire.
	// fixme: adapt direction & elevation to course and speed of target!
	// fixme: get_pos() is to crude!
	return fire_shell_at(s.get_pos().xy());										
}

bool ship::man_guns()
{
	if (has_guns() && !is_gun_manned()) {
		if (!gun_manning_is_changing) {
			// fixme: man ALL guns
			gun_turrets.begin()->manning_time = gun_turrets.begin()->time_to_man;
			gun_manning_is_changing = true;
			return true;
		}
	}
	return false;
}



bool ship::unman_guns()
{
	if (has_guns() && is_gun_manned()) {
		if (!gun_manning_is_changing) {
			// fixme: unman ALL guns
			gun_turrets.begin()->manning_time = gun_turrets.begin()->time_to_unman;
			gun_manning_is_changing = true;
			return true;
		}
	}
	return false;
}



// This function determines is the target for the gun is within the exclusion radius for 
// the turret. The exclusion radius defines one constant arc where the gun cannot aim (i.e. on a sub
// this would usually be the area directly behind the gun where the conning tower is, you can't shoot 
// through that). 
bool ship::is_target_in_blindspot(const struct gun_turret *gun, angle bearingToTarget)
{
	bool isInBlindSpot = false;
	
	if (gun->start_of_exclusion_radius != gun->end_of_exclusion_radius)
	{
		if (gun->start_of_exclusion_radius < gun->end_of_exclusion_radius)
		{
			if (bearingToTarget.value() >= gun->start_of_exclusion_radius && bearingToTarget.value() <= gun->end_of_exclusion_radius)
				isInBlindSpot = true;
		}
		else
		{
			if (bearingToTarget.value() >= gun->start_of_exclusion_radius || bearingToTarget.value() <= gun->end_of_exclusion_radius)
				isInBlindSpot = true;
		}
	}
	
	return isInBlindSpot;
}

long ship::num_shells_remaining()
{
	long numShells = 0;
	gun_turret_itr gunTurret = gun_turrets.begin();
	
	while (gunTurret != gun_turrets.end())
	{
		numShells += gunTurret->num_shells_remaining;
		gunTurret++;
	}
	
	return numShells;
}

bool ship::is_gun_manned()
{
	return gun_turrets.begin()->is_gun_manned;
}

bool ship::calculate_gun_angle(const double distance, angle &elevation, const double initial_velocity)
{
	bool withinRange = false;

	map<double, double>::iterator it = dist_angle_relation[initial_velocity].lower_bound(distance);
	if (it != dist_angle_relation[initial_velocity].end()) 
	{
		elevation = angle(it->second);
		withinRange = true;
	}

	return withinRange;
}

void ship::calc_max_gun_range(double initial_velocity)
{
	double max_range = dist_angle_relation[initial_velocity].rbegin()->first;
	
	maximum_gun_range = (max_range > maximum_gun_range) ? max_range : maximum_gun_range;
}



void ship::manipulate_heading(angle hdg)
{
	sea_object::manipulate_heading(hdg);
	head_to = hdg;
	head_to_fixed = true;
}
