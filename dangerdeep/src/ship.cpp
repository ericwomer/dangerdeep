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
#include "log.h"
#include "global_constants.h"

using std::vector;
using std::list;
using std::string;
using std::map;
using std::pair;
using std::make_pair;
using std::istringstream;
using std::ostringstream;

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
	  flooding_speed(0),
	  max_flooded_mass(0),
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
		if (etonnage.has_attr("value")) {
		tonnage = parent.child("tonnage").attru();
		} else {
			log_warning("wrong <tonnage> tag in file " << etonnage.doc_name());
			unsigned minton = etonnage.attru("min");
			unsigned maxton = etonnage.attru("max");
			tonnage = minton + rnd(maxton - minton + 1);
		}
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

	if (mymodel) {
		max_flooded_mass = mymodel->get_base_mesh().volume * 1000 /* density of water */;
		flooded_mass.resize(mymodel->get_voxel_data().size());
	}
}



void ship::sink()
{
	flooding_speed += 40000; // 40 tons per second
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
	xml_elem esink = parent.child("sinking");
	flooding_speed = esink.attrf("flooding_speed");
	istringstream fiss(esink.child_text());
	for (unsigned j = 0; j < flooded_mass.size(); ++j)
		fiss >> flooded_mass[j];

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
	xml_elem esink = parent.add_child("sinking");
	esink.set_attr(flooding_speed, "flooding_speed");
	ostringstream foss;
	for (unsigned j = 0; j < flooded_mass.size(); ++j)
		foss << flooded_mass[j] << " ";
	esink.add_child_text(foss.str());

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

	// calculate sinking, fixme replace by buoyancy...
	if (is_inactive()) {
		// compute the set of voxels that are currently being flooded.
		// distribute the per-time-flooding mass to them evenly.
		// if a voxel has been filled up, all of its neighbours
		// are set to "filling" state if they aren't already filling or filled.
		// *important note*
		// we don't store extra flags for every voxel if it's filling or already
		// filled, we rather use its flooded mass as special indicator.
		// < 0.05 means empty, not filling
		// < max-flooded-mass-for-voxel means filling, but not yet full
		// else: already filled.
		// beware of float inaccuracies! so add extra margin before comparing
		const vector<model::voxel>& voxdat = mymodel->get_voxel_data();
		vector<unsigned> flooding_voxels;
		flooding_voxels.reserve(voxdat.size());
		double flooding_volume = 0;
		for (unsigned i = 0; i < voxdat.size(); ++i) {
			if (flooded_mass[i] > 0.05f) {
				// voxel is flooding or full
				// get max. flooded mass for voxel
				double mfm = voxdat[i].relative_volume * max_flooded_mass;
				if (flooded_mass[i] < mfm) {
					// voxel is flooding
					flooding_voxels.push_back(i);
					flooding_volume += voxdat[i].relative_volume;
				} else {
					// voxel has been flooded, check for its neighbours
					// use a bit more so that "< mfm" is always false.
					flooded_mass[i] = mfm * 1.00001f;
					for (int k = 0; k < 6; ++k) {
						int ng = voxdat[i].neighbour_idx[k];
						if (ng >= 0 && flooded_mass[ng] < 0.06f) {
							// has a neighbour that is not flooding nor full
							flooded_mass[ng] = 0.1f;
						}
					}
				}
			}
		}
		double flooding_volume_rcp = 1.0/flooding_volume;
		// add mass to all voxels that are currently flooding.
		double totally_flooded = 0;
		for (unsigned k = 0; k < flooding_voxels.size(); ++k) {
			unsigned i = flooding_voxels[k];
			flooded_mass[i] += delta_time * flooding_speed * voxdat[i].relative_volume * flooding_volume_rcp;
			totally_flooded += flooded_mass[i];
		}
		if (position.z < -200)	// used for ships.
			kill();
		throttle = stop;
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
				vector3 forward = velocity.normal();
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
				// handle orientation here!
				// maybe add some random offset, but it don't seems necessary
				vector3 ppos = position + orientation.rotate(it->second);
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


//#include "global_data.h"
void ship::steering_logic()
{
	/* New helmsman simulation.
	   We have the formula
	   error = a * x + b * y + c * z
	   where x = angle difference between heading and head_to
	         y = turn velocity (with sign)
		 z = rudder_pos
	   and a, b, c are some control factors (constants).
	   c should be much smaller than a and b, normally a > b > c.
	   the error has a sign, according to sign and magnitude of it
	   the rudder_to is set.
	   This system should find the correct course, it only needs
	   tuning of a, b, c. Their values depend on maximum turn speed.
	   The following (experimentally gained) formulas give good results.
	*/
	double anglediff = (head_to - heading).value_pm180();
	double error0 = anglediff;
	double error1 = (max_rudder_angle/max_rudder_turn_speed) * turn_velocity * 1.0;
	double error2 = 0;//-rudder_pos/max_rudder_turn_speed * turn_velocity;
	double error = error0 + error1 + error2;
	//DBGOUT7(anglediff, turn_velocity, rudder_pos, error0, error1, error2, error);
	int rd = 0;
	// in reality we can chose finer rudder positions, would give better results.
	// range is -2...2
	if (fabs(error) > 5) rd = 2;
	else if (fabs(error) > 0.25) rd = 1;
	rudder_to = (error < 0 ? -1 : 1) * rd;
	// when error below a certain limit, set head_to_fixed=false, rudder_to=ruddermidships
	if (fabs(anglediff) <= 0.25 && fabs(rudder_pos) < 1.0) {
		head_to_fixed = false;
		rudder_to = ruddermidships;
		//std::cout << "reached course, diff=" << anglediff << " tv=" << turn_velocity << "\n";
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

	// fromwhere is real-world position of damage source.

	// test code: determine which voxels are within damage diameter
	// use a 10m radius, and torps have atm 100 hitpoints, so radius=strength/10
	vector3 relpos = fromwhere - get_pos();
	// rotate relative position to object space
	vector3f objrelpos = orientation.conj().rotate(relpos);
	//log_debug("DAMAGE! relpos="<<relpos << " objrelpos="<<objrelpos);
	vector<unsigned> voxlist = mymodel->get_voxels_within_sphere(objrelpos, strength/10.0);
	for (unsigned j = 0; j < voxlist.size(); ++j) {
		unsigned i = voxlist[j];
		// set all damaged voxels to flooding state (mass > 0.05f)
		flooded_mass[i] = 0.1f;
	}

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



double ship::get_noise_factor () const
{
    return get_throttle_speed () / max_speed_forward;
}



void ship::compute_force_and_torque(vector3& F, vector3& T) const
{
	/* NEW ALGORITHM:
	   we need to add the force/torque generated from tide.
	   for certain sample points around the hull we compute the draught
	   and from that a lift force. Additionally gravity is acting in
	   downward direction.
	   The sample points are taken from the model's voxel data,
	   iterate the list of voxels for the model, for each voxel
	   transform it according to model's transformation (a combination
	   of the model specific transformation and the current orientation
	   quaternion and position). The resulting point in 3-space gives
	   the center of the voxel. Compute the water height at that xy
	   position and compare it to the voxel's z position. If the voxel
	   is below the water, it is accounted for lift force (scaled by its
	   fraction of volume). Add the per-voxel force (specific for each
	   model, depends on model mass and voxel size) to the total sum
	   of forces, and add the relative voxel position cross lift force
	   to global torque.
	   Finally add global gravity force to sum of forces.
	   fixme: if the voxel is near the water surface, within +- voxel_height
	   we could account parts of its lift force. we have to use a medium
	   voxel "radius" here, because the voxel can be arbitrarily oriented.
	   The voxel radius is the half diameter of a voxel cube.
	   fixme: we need to know per-voxel-lift force. That is voxel volume in
	   cubic meters by 1000kg by 9,81m/s^2, as each cubic meter of water gives
	   9,81kN lift force.
	   It would be much more efficient to store the relative position of the
	   voxel in integer numbers and compute the delta-vectors depending
	   on transformation (orientation included) to avoid a matrix-vector
	   multiplication per voxel. then use linear combination of the
	   delta vectors by relative position to get real word position.
	*/

	// fixme: check that max speed and turn radius is still sensible
	// fixme: if every voxel would have a mass we could simulate
	//    non-uniform distribution of mass easily
	//    and fake changes by modifying the voxels (they would need
	//    to be per vessel then, not per model)
	// fixme: add linear drag with small factor, to hinder small
	//        movement effectivly (older code capped, if speed < 1.0 then
	//        use linear drag, else square drag). with that we can
	//        avoid tiny movements that happen over time by rounding
	//        errors.
	// fixme: re-normalization of rotation quaterionions ("orientation")
	//        should be done frequently...

	double lift_force_sum = 0; // = -GRAVITY * mass;
	vector3 dr_torque;
	const std::vector<model::voxel>& voxel_data = mymodel->get_voxel_data();
	const vector3f& voxel_size = mymodel->get_voxel_size();
	const float voxel_radius = mymodel->get_voxel_radius();
	const float voxel_vol = voxel_size.x * voxel_size.y * voxel_size.z;
	const double voxel_vol_force = voxel_vol * GRAVITY * 1000.0; // 1000kg per cubic meter
	const matrix4f transmat = orientation.rotmat4() * mymodel->get_base_mesh_transformation()
		* matrix4f::diagonal(voxel_size);
	double vol_below_water=0;
	const double gravity_force = mass * -GRAVITY;
	//fixme: split loop to two cores to speed up physics a tiny bit
	for (unsigned i = 0; i < voxel_data.size(); ++i) {
		// instead of a per-voxel matrix-vector multiplication we could
		// transform all other vectors to mesh-vertex-space and skip
		// the matrix multiplication here, then later re-transform the
		// resulting force-vectors back to model space.
		// we know here that transmat only has non-projective part
		// so mul4vec3xlat is sufficient.
		vector3f p = transmat.mul4vec3xlat(voxel_data[i].relative_position);
		//std::cout << "i=" << i << " voxeldata " << voxel_data[i].relative_position << " p=" << p << "\n";
		float wh = gm.compute_water_height(vector2(position.x + p.x, position.y + p.y));
		//std::cout << "i=" << i << " p=" << p << " wh=" << wh << "\n";
		double voxel_below_water = std::max(std::min((p.z + position.z - wh) / voxel_radius, 1.0), -1.0);
		if (voxel_below_water < 1.0 /*p.z + position.z < wh*/) {
			// voxels partly below water must be computed or torque is severely wrong
			double submerged_part = 1.0 - (voxel_below_water + 1.0) * 0.5;
			double lift_force = voxel_data[i].part_of_volume * voxel_vol_force * submerged_part;
			vol_below_water += voxel_data[i].part_of_volume * submerged_part;
			lift_force_sum += lift_force;
			vector3 lift_torque = p.cross(vector3(0, 0, lift_force));
			dr_torque += lift_torque;
			//std::cout << "i=" << i << " subm=" << submerged_part << " vdw=" << voxel_data[i].part_of_volume << " lift_force=" << lift_force << " lift_torque=" << lift_torque << "\n";
		}
		double relative_gravity_force = gravity_force * voxel_data[i].relative_mass;
		// add part because of flooding
		relative_gravity_force += flooded_mass[i] * -GRAVITY;
		lift_force_sum += relative_gravity_force;
		dr_torque += p.cross(vector3(0, 0, relative_gravity_force));
	}
//	std::cout << "mass=" << mass << " lift_force_sum=" << lift_force_sum << " grav=" << -GRAVITY*mass << "\n";
//	std::cout << "vol below water=" << vol_below_water << " of " << voxel_data.size() << "\n";

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
	// force is in world space
	F = orientation.rotate(vector3(0, (acceleration + drag_factor) * mass, 0));

	// new algo: compute drag directly from linear momentum
	vector3 linear_momentum_xz = orientation.rotate(orientation.conj().rotate(linear_momentum).coeff_mul(vector3(1, 0, 1)));
	// maybe add some linear drag too, so low linear_momentum values give some noticeable
	// drag too (just squaring gives too low drag)
	// fixme: drag depends on cross section area, is lower in forward direction
	// than sideward direction!
	double lmfac = -linear_momentum_xz.square_length() / (mass * mass);
	F += linear_momentum_xz * lmfac;

	// Note! drag should be computed for all three dimensions, each with area,
	// to limit sideward/downward movement as well. We need to know the area
	// that is underwater then (air drag is negligible for ships). That
	// value can be computed when we know the volume below water and divide
	// this by length or width. Volume below water should be precomputed
	// like cross section, e.g. with is_inside test over voxels inside the ship.
	F.z += lift_force_sum; // buoyancy/gravity



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
	// linear velocity of a point is I^-1 * L cross r, where r is vector
	// from object center to the point, L is angular momentum.
	// When we compute it for r along y-axis (r = (0, ry, 0))
	// we thus have I^-1 * L = w, and velocity = w.z * ry
	// So velocity = z * tvr, where tvr is linear velocity of point that turns.
	// area per z is area/(2*L), this is needed for the integral.
	// torque is taken by multiplying D with distance from center (z).
	// and velocity(z) = z * tvr.
	// D_torque(z) = Dcoeff * densitiy * velocity(z)^2 * area / (2*L) * z / 2
	// Int z=-L...L   D_torque(z) dz
	// this would be 0, but drag for stern part of hull doesn't count negative, so:
	// = 2 * Int z=0...L  D_torque(z) dz
	// = 2 * Int z=0...L  Dcoeff * densitiy * z^3 * tvr^2 * area/(4*L) dz
	// = Dcoeff * density * tvr^2 * area / (2*L) * Int z=0...L  z^3 dz
	// = Dcoeff * density * tvr^2 * area / (2*L) * L^4 / 4
	// = Dcoeff * density * tvr^2 * area * L^2 / 8
	const double drag_coefficient = get_turn_drag_coeff();
	const double water_density = 1000.0;
	// compute turn velocities around the 3 axes (local)
	// w.xyz is turn velocity around xyz axis.
	vector3 w = inertia_tensor_inv * orientation.conj().rotate(angular_momentum);
	vector3 tvr(fabs(w.x), fabs(w.y), fabs(w.z));
	const vector3 v3one(1.0, 1.0, 1.0);
	const vector3 tvf = tvr.min(v3one);
	vector3 tvr2 = tvr.coeff_mul(tvr).coeff_mul(tvf) + tvr.coeff_mul(v3one-tvf);
	const vector3 L(size3d.y*0.5, size3d.x*0.5, size3d.y*0.5);
	//fixme: size3d.xyz is not always symmetric...
	const vector3 area(size3d.x*size3d.y, size3d.x*size3d.y*0.5*20, get_turn_drag_area());
	// local_torque is drag_torque plus rudder_torque
	//fixme without that 80 drag is too low, not only turn drag,
	//but also roll/yaw drag, ship capsizes without that!!
	vector3 local_torque = tvr2.coeff_mul(area).coeff_mul(L.coeff_mul(L))
		* (drag_coefficient * water_density * 0.125);
	if (w.x > 0.0) local_torque.x = -local_torque.x;
	if (w.y > 0.0) local_torque.y = -local_torque.y;
	if (w.z > 0.0) local_torque.z = -local_torque.z;

	// negate rudder_pos here, because turning is mathematical, so rudder left means
	// rudder_pos < 0 and this means ccw turning and this means turn velocity > 0!
	// fixme: store rudder pos in per-ship spec file.
	// fixme: rudder torque must have similar formula like above, depends on rudder area
	// factor here is 20000, that would be 20m^2 area (with 1000kg/m^3 density)
	// assume rudder area = 2% * turn drag area
	double rudder_torque = (size3d.y*0.5) * get_turn_drag_area()*0.02 * water_density * speed * sin(-rudder_pos * M_PI / 180.0);
	local_torque.z += rudder_torque;

	// positive torque turns counter clockwise! torque is in world space!
	T = orientation.rotate(local_torque) + dr_torque;

	//fixme: the AI uses turn radius to decide turning direction, that may give
	//wrong values with new physics!
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
										// fixme #2, we add z + 4m to avoid shell<->water surface collisions
										gm.spawn_gun_shell(new gun_shell(gm, get_pos() + vector3(0,0,4), direction, elevation, gun->initial_velocity, gun->shell_damage), 
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
	log_debug("man guns, is gun manned? " << has_guns() && is_gun_manned());
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
	log_debug("UNman guns, is gun manned? " << has_guns() && is_gun_manned());
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
