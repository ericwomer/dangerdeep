// ai
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "date.h"
#include "ai.h"
#include "game.h"
#include "convoy.h"
#include "sea_object.h"
#include "gun_shell.h"
#include "depth_charge.h"
#include "command.h"

//#define WPEXACTNESS 100			// how exact a waypoint has to be hit in meters
//#define AI_THINK_CYCLE_TIME 10		// sec
#define DC_ATTACK_RADIUS 100	// distance to target before DC launching starts
#define DC_ATTACK_RUN_RADIUS 600	// distance to contact until escort switches to
					// maximum speed

// fixme:
// should ai know class game?
// instead of calling game::spawn_dc call parent->fire_dc etc.
// (make use of command encapsulation for network play!)

// fixme: zigzag is possible, but never used.
// zig zag for:
// convoys do a large scale zig zag to make it difficult for submarines to keep contact
// merchants should zigzig when convoy is attacked (or they are) to avoid torpedo hits
// escorts should zigzag when patrolling (or maybe their speed makes zigzagging unneccessary?)
// escorts should zigzag when following/hunting a submarine that is in torpedo range, but not
// to close (250-400m minimum, 3000m? maximum) to avoid headon torpedo hits

map<double, double> ai::dist_angle_relation;
#define MAX_ANGLE 45.0
#define ANGLE_GAP 0.1

void ai::fill_dist_angle_relation_map(void)
{
	if (dist_angle_relation.size() > 0) return;
	for (double a = 0; a < MAX_ANGLE+ANGLE_GAP; a += ANGLE_GAP) {
		angle elevation(a);
		double z = 4;	// meters, initial height above water
		double vz = GUN_SHELL_INITIAL_VELOCITY * elevation.sin();
		double dist = 0;
		double vdist = GUN_SHELL_INITIAL_VELOCITY * elevation.cos();
		
		for (double dt = 0; dt < 120.0; dt += 0.01) {
			dist += vdist * dt;
			z += vz * dt;
			vz += -GRAVITY * dt;
			if (z <= 0) break;
		}
		
		dist_angle_relation[dist] = a;
	}
}

// ai computation between is randomly interleaved between frames to avoid
// time consumption peeks every AI_THINK_CYCLE_TIME seconds
ai::ai(sea_object* parent_, types type_) : type(type_), state(followpath),
	zigzagstate(0), attackrun(false), evasive_manouver(false),
	rem_manouver_time(0), parent(parent_), followme(0),
	myconvoy(0), has_contact(false),
	remaining_time(rnd() * AI_THINK_CYCLE_TIME),
	cyclewaypoints(false)
{
	fill_dist_angle_relation_map();
}

ai::ai(istream& in, class game& g)
{
	type = types(read_u8(in));
	state = states(read_u8(in));
	zigzagstate = read_u32(in);
	attackrun = read_bool(in);
	evasive_manouver = read_bool(in);
	rem_manouver_time = read_double(in);
	parent = g.read_sea_object(in);
	followme = g.read_sea_object(in);
	myconvoy = g.read_convoy(in);
	has_contact = read_bool(in);
	contact.x = read_double(in);
	contact.y = read_double(in);
	contact.z = read_double(in);
	remaining_time = read_double(in);
}

void ai::save(ostream& out, const class game& g) const
{
	write_u8(out, Uint8(type));
	write_u8(out, Uint8(state));
	write_u32(out, zigzagstate);
	write_bool(out, attackrun);
	write_bool(out, evasive_manouver);
	write_double(out, rem_manouver_time);
	g.write(out, parent);
	g.write(out, followme);
	g.write(out, myconvoy);
	write_bool(out, has_contact);
	write_double(out, contact.x);
	write_double(out, contact.y);
	write_double(out, contact.z);
	write_double(out, remaining_time);
}


void ai::relax(game& gm)
{
	has_contact = false;
	state = (followme != 0) ? followobject : followpath;
	gm.send(new command_set_throttle(parent, sea_object::aheadsonar));
	attackrun = false;
}

void ai::attack_contact(const vector3& c)
{
	has_contact = true;
	contact = c;
	state = attackcontact;
}

void ai::follow(sea_object* t)
{
	followme = t;
	state = (followme != 0) ? followobject : followpath;
}

void ai::act(class game& gm, double delta_time)
{
	remaining_time -= delta_time;
	if (remaining_time > 0) {
		return;
	} else {
		remaining_time = AI_THINK_CYCLE_TIME * ( 0.75f + 0.25f * rnd ( 1 ) );
	}

	switch (type) {
		case escort: act_escort(gm, delta_time); break;
		default: act_dumb(gm, delta_time); break;
	}
	
	if (zigzagstate > 0) {
		if (zigzagstate == 5)
			gm.send(new command_rudder_left(parent));
		else if (zigzagstate == 15)
			gm.send(new command_rudder_right(parent));
		++zigzagstate;
		if (zigzagstate > 20)
			zigzagstate = 1;
	}
}

void ai::set_zigzag(bool stat)
{
	if (stat)
		zigzagstate = 1;
	else
		zigzagstate = 0;
}

void ai::act_escort(game& gm, double delta_time)
{
	// always watch out/listen/ping for the enemy
	// watch around

	// fixme: a list of submarine* is bad (more information given than
	// what is really visible, not compatible to network play!
	// but how else should the ai ask for course and speed?
	// a contact should be of the form: position, course, type.
	// contact's speed should be determined by the ai itself.
	// but how can the ai identify contacts? by the objects adress i.e. pointer?
	// this would be nearly the same as returning a list of pointers (see above).

	// A dead, crippled or sinking unit reacts like a dump unit. It does
	// not fire shells, depth charges etc. on enemy units and does not try
	// to locate them anymore.
	if ( !parent->is_alive () )
	{
		act_dumb(gm, delta_time);
		return;
	}

	double dist = 1e12;
	submarine* nearest_contact = 0;
	list<submarine*> subs;
	gm.visible_submarines(subs, parent);
	for (list<submarine*>::iterator it = subs.begin(); it != subs.end(); ++it) {
		double d = (*it)->get_pos().xy().square_distance(parent->get_pos().xy());
		if (d < dist) {
			dist = d;
			nearest_contact = *it;
		}
	}
	if (nearest_contact) {	// is there a contact?
		fire_shell_at(gm, *nearest_contact);
		attack_contact(nearest_contact->get_pos());
		if (myconvoy) myconvoy->add_contact(nearest_contact->get_pos());
		gm.send(new command_set_throttle(parent, sea_object::aheadflank));
		attackrun = true;
	}

	if (!attackrun) {	// nothing found? try a ping or listen
		// high speeds do not allow for listening or sonar.
	
		// listen for subs
		list<submarine*> hearable_subs;
		gm.sonar_submarines(hearable_subs, parent);
		if (hearable_subs.size() > 0) {
			attack_contact(hearable_subs.front()->get_pos());
		} else {
			// ping around to find something
			list<vector3> contacts;
			gm.ping_ASDIC(contacts, parent, true);//fixme add command!!!!
			if (contacts.size() > 0) {
				// fixme: choose best contact!
				if (myconvoy) myconvoy->add_contact(contacts.front());
				attack_contact(contacts.front());
			}
		}
	}

	if (state == followpath || state == followobject) {
		act_dumb(gm, delta_time);
	} else if (state == attackcontact) {	// attack sonar/visible contact

		if (!(evasive_manouver && rem_manouver_time > 0)) {
			evasive_manouver = ! parent->set_course_to_pos(contact.xy());//fixme move function to ai!!!
			if (evasive_manouver) {
				// wait for half circle to complete
				double waittime = 180.0 / (parent->get_turn_rate().value() * parent->get_speed());
				rem_manouver_time = ceil(waittime/AI_THINK_CYCLE_TIME) * AI_THINK_CYCLE_TIME;
			}
		}

		vector2 delta = contact.xy() - parent->get_pos().xy();
		double cd = delta.length();
		if (cd > DC_ATTACK_RUN_RADIUS && !attackrun) {
			list<vector3> contacts;
			gm.ping_ASDIC(contacts, parent, false, angle(delta));//fixme add command
			if (contacts.size() > 0) {	// update contact
				// fixme: choose best contact!
				if (myconvoy) myconvoy->add_contact(contacts.front());
				attack_contact(contacts.front());
			}
			//set_zigzag((cd > 500 && cd < 2500));//fixme test hack, doesn't work
		} else {
			gm.send(new command_set_throttle(parent, sea_object::aheadflank));
			attackrun = true;
			//set_zigzag(false);//fixme test hack, doesn't work
		}

		if (cd < DC_ATTACK_RADIUS) {
			gm.spawn_depth_charge(new depth_charge(*parent, -contact.z));//fixme add command
			// the escort must run with maximum speed until the depth charges
			// have exploded to avoid suicide. fixme
			// fixme: just ai hacking/testing.
			// after spawning a DC start pinging again.
			relax(gm);
		}
	}
	
	if (rem_manouver_time > 0) rem_manouver_time -= AI_THINK_CYCLE_TIME;
}

void ai::act_dumb(game& gm, double delta_time)
{
	if (state == followobject && followme != 0) {
		parent->set_course_to_pos(followme->get_pos().xy());
		//gm.send(new command_set_course_to_pos(parent, followme->get_pos().xy()));//argh fixme
	} else if (state == followpath) {
		if (waypoints.size() > 0) {
			parent->set_course_to_pos(waypoints.front());//argh fixme
			//gm.send(new command_set_course_to_pos(parent, waypoints.front()));
			if (parent->get_pos().xy().distance(waypoints.front()) < WPEXACTNESS) {
				if (cyclewaypoints)
					waypoints.push_back(waypoints.front());
				waypoints.erase(waypoints.begin());
			}
		}
	}
}

void ai::fire_shell_at(game& gm, const sea_object& s)
{
	// maybe we should not use current position but rather
	// estimated position at impact!
	vector2 deltapos = s.get_pos().xy() - parent->get_pos().xy();
	double distance = deltapos.length();
	angle direction(deltapos);

	double max_shooting_distance = (dist_angle_relation.rbegin())->first;
	if (distance > max_shooting_distance) return;	// can't do anything
	
	// initial angle: estimate distance and fire, remember angle
	// next shots: adjust angle after distance fault:
	//	estimate new distance from old and fault
	//	select new angle, fire.
	//	use an extra bit of correction for wind etc.
	//	to do that, we need to know where the last shot impacted!

	map<double, double>::iterator it = dist_angle_relation.lower_bound(distance);
	if (it == dist_angle_relation.end()) return;	// unsuccesful angle, fixme
	angle elevation = angle(it->second);

	// fixme: for a smart ai: try to avoid firing at friendly ships that are in line
	// of fire.
	
	// fixme: snap angle values to simulate real cannon accuracy.

	// fixme: adapt direction & elevation to course and speed of target!
	gm.spawn_gun_shell(new gun_shell(*parent, direction, elevation));//fixme add command

	last_elevation = elevation;	
	last_azimuth = direction;
}






#if 0	// gunnery code
#include <cstdlib>
#include <iostream>
#include <cmath>
#include <list>

const float PI = 3.1415962;
const float EPS = 0.1;

using namespace std;

float quantify(float a, float b)
{
	return round(a/b)*b;
}

float random_part(float min, float max)
{
	float d = max - min;
	float r = float(rand() % 1001) / 1000.0;
	return min + r * d;
}

float deg_to_rad(float deg)
{
	return deg * PI / 180.0;
}

float rad_to_deg(float rad)
{
	return rad * 180.0 / PI;
}

float estimate_distance(float d)
{
	if (fabs(d) > 10000.0)
		return round(d/2000.0)*2000.0;
	if (fabs(d) > 5000.0)
		return round(d/1000.0)*1000.0;
	if (fabs(d) > 2000.0)
		return round(d/500.0)*500.0;
	if (fabs(d) > 200.0)
		return round(d/50.0)*50.0;
	return round(d/25.0)*25.0;
}

float shot_speed(float a, float v0, float t)
{
	return v0*exp(-a*t/v0);
}

float shot_distance(float alpha, float a, float v0, float t)
{
	return cos(alpha)*t*shot_speed(a, v0, t);
}

float shot_height(float alpha, float a, float v0, float g, float t)
{
	return sin(alpha)*t*shot_speed(a, v0, t) + g*t*t/2;
}

float impact_distance(float alpha, float a, float v0, float g)
{
	float t = 120.0, delta_t = t/2;
	while(1) {
		float h = shot_height(alpha, a, v0, g, t);
		if (h > EPS)
			t += delta_t;
		else if (h < -EPS)
			t -= delta_t;
		else
			break;
		delta_t /= 2;
	}
	return shot_distance(alpha, a, v0, t);
}

float height(float alpha, float a, float v0, float g, float dist)
{
	float t = 120.0, delta_t = t/2;
	while(1) {
		float d = dist - shot_distance(alpha, a, v0, t);
		if (d > EPS)
			t += delta_t;
		else if (d < -EPS)
			t -= delta_t;
		else
			break;
		delta_t /= 2;
	}
	return shot_height(alpha, a, v0, g, t);
}

// shot angle estimation
std::list<std::pair<double, double> > estimations;
const int STEPS = 45;

float gauss(float x)
{
	return exp(-x*x/2);
}

float initialize_angle_estimation(float a, float v0, float g, float min_elev, float max_elev, float min_angle_diff)
{
	for (int i = 0; i <= STEPS; ++i) {
		float ang = min_elev + i * (max_elev - min_elev) / STEPS;
		ang = quantify(ang, min_angle_diff);
		float dist = impact_distance(deg_to_rad(ang), a, v0, g);
		dist = quantify(dist, 50.0);
		estimations.push_back(std::make_pair(ang, dist));
	}
}

float estimate_angle(float a, float v0, float g, float dist)
{
	std::list<std::pair<double, double> >::iterator it = estimations.begin();

//	std::list<std::pair<double, double> >::iterator it3 = estimations.begin();
//	for ( ; it3 != estimations.end(); ++it3)
//		cout << "learned: angle " << it3->first << ", " << it3->second << "\n";

	while (it != estimations.end() && it->second < dist)
		++it;
	if (it == estimations.end())
		return -1;
	if (it == estimations.begin())
		return 0;
	std::list<std::pair<double, double> >::iterator it2 = it;
	--it;
	float diff = (dist - it->second) / (it2->second - it->second);
	float ang = it->first + diff * (it2->first - it->first);
	return deg_to_rad(ang);
}

void learn_angle_dist_relation(float ang, float dist)
{
	std::list<std::pair<double, double> >::iterator it = estimations.begin();
	while (it != estimations.end() && it->first < ang)
		++it;
	if (it == estimations.end())
		return;
	if (it == estimations.begin())
		return;
	if (fabs(it->first - ang) < EPS)
		it->second = dist;
	else
		estimations.insert(it, std::make_pair(ang, dist));

	// sort distance part
	it = estimations.begin();
	while (it != estimations.end()) {
		std::list<std::pair<double, double> >::iterator it2 = it;
		++it2;
		while (it2 != estimations.end()) {
			if (it->second > it2->second) {
				float tmp = it2->second;
				it2->second = it->second;
				it->second = tmp;
			}
			++it2;
		}
		++it;
	}
}

float hit(float h1, float h2, float eh)
{
	return (h2 <= eh && h1 >= -10.0);
}

int main(int argc, char** argv)
{
	float enemy_distance = 22000;
	float enemy_width = 30;
	float min_elev = 0;
	float max_elev = 45;
	float g = -9.8062;
	float v0 = 600;
	float a = 2.0;	// air resistance
	float min_angle_diff = 0.1;	// very small. but must be that size!?
	float enemy_height = 30.0;

	if (argc > 1)
		enemy_distance = atof(argv[1]);
	
	srand(time(0));
	
	initialize_angle_estimation(a, v0, g, min_elev, max_elev, min_angle_diff);

	// see enemy and estimate range
	cout << "Gunnery test.\nEnemy range " << enemy_distance << "\n";
	float estimated_distance = estimate_distance(enemy_distance * random_part(0.95, 1.05));
	cout << "Estimated range " << estimated_distance << "\n";

	float wind_resist = random_part(-0.5, 0.5);
	a += wind_resist;

	float alpha = estimate_angle(a, v0, g, estimated_distance);
	alpha = quantify(alpha, deg_to_rad(8*min_angle_diff));
	float alpha2 = rad_to_deg(alpha);
	if (alpha < 0) {
		cout << "Target out of range!\n";
		return 0;
	}
	cout << "Inital elevation " << alpha2 << " degrees.\n";

	// now fire, check for hit and retry
	int hits = 0;
	while (hits < 4) {
		// calculate height of projectile over target
		float h1 = height(alpha, a, v0, g, enemy_distance - enemy_width/2);
		float h2 = height(alpha, a, v0, g, enemy_distance + enemy_width/2);
		float impact_angle = rad_to_deg(atan((h1 - h2)/enemy_width));
		if (hit(h1, h2, enemy_height)) {
			cout << "Hit!!! Angle " << impact_angle << "\n";
			++hits;
		}
		cout << "Heights over target " << h1 << ", " << h2 << "\n";
		float id = impact_distance(alpha, a, v0, g);
		cout << "Impact at " << id << "\n";
		float estimated_fault = estimate_distance(id - enemy_distance + random_part(-10, 10));
		cout << "Estimated fault " << estimated_fault << "\n";

		estimated_distance -= estimated_fault;

		alpha = estimate_angle(a, v0, g, estimated_distance);
		alpha = quantify(alpha, deg_to_rad(min_angle_diff));
		alpha2 = rad_to_deg(alpha);
		if (alpha < 0) {
			cout << "Target out of range!\n";
			return 0;
		}
		cout << "New elevation " << alpha2 << " degrees.\n";
	}
}
#endif
