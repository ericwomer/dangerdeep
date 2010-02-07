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

// ai
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "date.h"
#include "ai.h"
#include "game.h"
#include "convoy.h"
#include "sea_object.h"
#include "depth_charge.h"
#include "global_data.h"
#include "submarine.h"
using std::vector;
using std::list;
using std::string;

// fixme: we have bspline code ready. convoys should follow their routes along a bspline
// curve for realistic results.
#define WPEXACTNESS 100			// how exact a waypoint has to be hit in meters
#define AI_THINK_CYCLE_TIME 10		// sec
#define DC_ATTACK_RADIUS 100		// distance to target before DC launching starts
#define DC_ATTACK_RUN_RADIUS 600	// distance to contact until escort switches to
					// maximum speed

// fixme:
// should ai know class game?
// instead of calling game::spawn_dc call parent->fire_dc etc.

// fixme: zigzag is possible, but never used.
// zig zag for:
// convoys do a large scale zig zag to make it difficult for submarines to keep contact
// merchants should zigzig when convoy is attacked (or they are) to avoid torpedo hits
// escorts should zigzag when patrolling (or maybe their speed makes zigzagging unneccessary?)
// escorts should zigzag when following/hunting a submarine that is in torpedo range, but not
// to close (250-400m minimum, 3000m? maximum) to avoid headon torpedo hits

// ai computation between is randomly interleaved between frames to avoid
// time consumption peeks every AI_THINK_CYCLE_TIME seconds
ai::ai(ship* parent_, types type_) : type(type_), state(followpath),
	zigzagstate(1/*0 fixme*/), attackrun(false), evasive_manouver(false),
	rem_manouver_time(0), parent(parent_), followme(0),
	myconvoy(0), has_contact(false),
	remaining_time(rnd() * AI_THINK_CYCLE_TIME),
	cyclewaypoints(false)
{	
}



ai::~ai()
{
}



void ai::load(game& gm, const xml_elem& parentnode)
{
	type = types(parentnode.attru("type"));
	state = states(parentnode.attru("state"));
	zigzagstate = parentnode.attru("zigzagstate");
	attackrun = parentnode.attrb("attackrun");
	evasive_manouver = parentnode.attrb("evasive_manouver");
	rem_manouver_time = parentnode.attrf("rem_manouver_time");
	parent = gm.load_ship_ptr(parentnode.attru("parent"));
	followme = gm.load_ptr(parentnode.attru("followme"));
	myconvoy = gm.load_convoy_ptr(parentnode.attru("myconvoy"));
	if (parentnode.has_child("contact")) {
		contact = parentnode.child("contact").attrv3();
	}
	remaining_time = parentnode.attrf("remaining_time");
	main_course = parentnode.child("main_course").attra();
	xml_elem wp = parentnode.child("waypoints");
	waypoints.clear();
	for (xml_elem::iterator it = wp.iterate("waypoint"); !it.end(); it.next()) {
		waypoints.push_back(it.elem().attrv2());
	}
	cyclewaypoints = wp.attrb("cyclewaypoints");
}



void ai::save(game& gm, xml_elem& parentnode) const
{
	parentnode.set_attr(unsigned(type), "type");
	parentnode.set_attr(unsigned(state), "state");
	parentnode.set_attr(zigzagstate, "zigzagstate");
	parentnode.set_attr(attackrun, "attackrun");
	parentnode.set_attr(evasive_manouver, "evasive_manouver");
	parentnode.set_attr(rem_manouver_time, "rem_manouver_time");
	parentnode.set_attr(gm.save_ptr(parent), "parent");
	parentnode.set_attr(gm.save_ptr(followme), "followme");
	parentnode.set_attr(gm.save_ptr(myconvoy), "myconvoy");
	if (has_contact) {
		parentnode.add_child("contact").set_attr(contact);
	}
	parentnode.set_attr(remaining_time, "remaining_time");
	parentnode.add_child("main_course").set_attr(main_course);
	xml_elem wp = parentnode.add_child("waypoints");
	for (list<vector2>::const_iterator it = waypoints.begin(); it != waypoints.end(); ++it) {
		wp.add_child("waypoint").set_attr(*it);
	}
	wp.set_attr(cyclewaypoints, "cyclewaypoints");
}


void ai::relax(game& gm)
{
	has_contact = false;
	state = (followme) ? followobject : followpath;
	parent->set_throttle(ship::aheadsonar);
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
	state = (followme) ? followobject : followpath;
}

void ai::act(class game& gm, double delta_time)
{
	remaining_time -= delta_time;
	if (remaining_time > 0) {
		return;
	} else {
		remaining_time = AI_THINK_CYCLE_TIME * ( 0.75f + 0.25f * rnd ( 1 ) );
	}
	
	if (!parent) return;

	switch (type) {
		case escort: act_escort(gm, delta_time); break;
		case convoy: act_convoy(gm, delta_time); break;
		default: act_dumb(gm, delta_time); break;
	}
	
	if (zigzagstate > 0) {	// this depends on ai type, convoys zigzag different! fixme
		// course diff up to 45 deg for dd's, depends on ship type
		if (zigzagstate == 7)
			parent->head_to_course(main_course - angle(45), -1);
		else if (zigzagstate == 13)
			parent->head_to_course(main_course + angle(45), 1);
		++zigzagstate;
		if (zigzagstate > 18)
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
	// any subs in visual range to attack?
	vector<submarine*> subs = gm.visible_submarines(parent);
	for (vector<submarine*>::iterator it = subs.begin(); it != subs.end(); ++it) {
		double d = (*it)->get_pos().xy().distance(parent->get_pos().xy());
		if (d < dist) {
			dist = d;
			nearest_contact = *it;
		}
	}
	
	if (!nearest_contact) {		
		// any subs in radar range to attack?
		subs = gm.radar_submarines(parent);
		for (vector<submarine*>::iterator it = subs.begin(); it != subs.end(); ++it) {
			double d = (*it)->get_pos().xy().distance(parent->get_pos().xy());
			if (d < dist) {
				dist = d;
				nearest_contact = *it;
			}
		}
	}
		
	if (nearest_contact) {	// is there a contact?
		if (dist <= parent->max_gun_range())
		{
			if (ship::GUN_NOT_MANNED == parent->fire_shell_at(*nearest_contact))
				parent->man_guns();
		}
		attack_contact(nearest_contact->get_pos());
		if (myconvoy) myconvoy->add_contact(nearest_contact->get_pos());
		parent->set_throttle(ship::aheadflank);
		attackrun = true;
	}

	if (!attackrun) {	// nothing found? try a ping or listen
		// high speeds do not allow for listening or sonar.
	
		// listen for subs
		vector<sonar_contact> hearable_subs = gm.sonar_submarines(parent);
		if (hearable_subs.size() > 0) {
			attack_contact(hearable_subs.front().pos.xy0());
		} else {
			// ping around to find something
			list<vector3> contacts;
			gm.ping_ASDIC(contacts, parent, true);
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
			evasive_manouver = ! set_course_to_pos(gm, contact.xy());//fixme move function to ai!!!
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
			gm.ping_ASDIC(contacts, parent, false, angle(delta));
			if (contacts.size() > 0) {	// update contact
				// fixme: choose best contact!
				if (myconvoy) myconvoy->add_contact(contacts.front());
				attack_contact(contacts.front());
			}
			set_zigzag((cd > 500 && cd < 2500) ? 1 : 0); // zig-zag near the sub - fixme test hack, doesn't work
		} else {
			parent->set_throttle(ship::aheadflank);
			attackrun = true;
			set_zigzag(0); // run straight - fixme test hack, doesn't work
		}

		if (cd < DC_ATTACK_RADIUS) {
			// fixme: get real pos for dc throwing...
			gm.spawn_depth_charge(new depth_charge(gm, -contact.z, parent->get_pos()));
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
	if (state == followobject && followme) {
		set_course_to_pos(gm, followme->get_pos().xy());
	} else if (state == followpath) {
		if (waypoints.size() > 0) {
			set_course_to_pos(gm, waypoints.front());
			if (parent->get_pos().xy().distance(waypoints.front()) < WPEXACTNESS) {
				if (cyclewaypoints)
					waypoints.push_back(waypoints.front());
				waypoints.erase(waypoints.begin());
			}
		}
	}
}

void ai::act_convoy(game& gm, double delta_time)
{
	// follow waypoints
	if (waypoints.size() > 0) {
		set_course_to_pos(gm, waypoints.front());
		if (parent->get_pos().xy().distance(waypoints.front()) < WPEXACTNESS) {
/*
			if (cyclewaypoints)
				waypoints.push_back(waypoints.front());
*/				
			waypoints.erase(waypoints.begin());
		}
	}

	// set actions for convoy's ships.
	// civil ships continue their course with zigzags eventually
//fixme: the ships don't follow their waypoint exactly, they're zigzagging wild around it
//if i use set_course_to_pos direct, everything is fine. maybe the ai of each ship
//must "think" shortly after setting the waypoint
//fixme: don't set the immidiate next wp, just use the next convoy wp + rel. position as waypoint!
//or set all wps at the beginning. fixme is this really a good idea?
//this could be done in the constructor!
//	for (list<pair<ship*, vector2> >::iterator it = merchants.begin(); it != merchants.end(); ++it) {
//		it->first->get_ai()->set_waypoint(position.xy() + it->second);
//	}

	// war ships follow their course, with zigzags / evasive manouvers / increasing speed
//	for (list<pair<ship*, vector2> >::iterator it = warships.begin(); it != warships.end(); ++it) {
//		it->first->get_ai()->set_waypoint(position.xy() + it->second);
//	}
	
	// escorts follow their escort pattern or attack if alarmed
//	for (list<pair<ship*, vector2> >::iterator it = escorts.begin(); it != escorts.end(); ++it) {
//		it->first->get_ai()->set_waypoint(position.xy() + it->second);
//	}
}

bool ai::set_course_to_pos(game& gm, const vector2& pos)
{
	vector2 d = pos - parent->get_pos().xy();
	vector2 hd = parent->get_heading().direction();
	double a = d.x*hd.x + d.y*hd.y;
	double b = d.x*hd.y - d.y*hd.x;
	// if a is < 0 then target lies behind our pos.
	// if b is < 0 then target is left, else right of our pos.
	double r1 = (b == 0) ? 1e10 : (a*a + b*b)/fabs(2*b);
	// fixme: a*a + b*b = |d|^2 * |hd|^2 = 1 * 1 = 1 !
	double r2 = 1.0/parent->get_turn_rate().rad();
	if (a <= 0) {	// target is behind us
		if (b < 0) {	// target is left
			main_course = parent->get_heading() - angle(180);
			parent->head_to_course(main_course, -1);
		} else {
			main_course = parent->get_heading() + angle(180);
			parent->head_to_course(main_course, 1);
		}
		return false;
	} else if (r2 > r1) {	// target can not be reached with smallest curve possible
		if (b < 0) {	// target is left
			main_course = parent->get_heading() + angle(180);
			parent->head_to_course(main_course, 1);
		} else {
			main_course = parent->get_heading() - angle(180);
			parent->head_to_course(main_course, -1);
		}
		return false;
	} else {	// target can be reached, steer curve
		parent->head_to_course(angle::from_math(atan2(d.y, d.x)), (b < 0) ? -1 : 1);
//	this code computes the curve that hits the target
//	but it is much better to turn fast and then steam straight ahead.
//	however, the straight path does not hit the target exactly, since the ship moves
//	while turning. In reality the ship would turn until it is facing the target
//	directly. Here the ai recomputes the path every 10seconds, so this doesn't matter.
//	2004/02/24. an even better course would be: turn to a course so that when it is reached
//	the target position lies exactly in that direction, e.g. target bearing is 30 degrees,
//	if we turn to 30, we're already a bit off the target (bearing then is not 0, maybe 5)
//	so turn to ~35 degrees and then run straight. Such exact path computation is not
//	realistic though, humans are no computers...
/*
		double needed_turn_rate = (r1 == 0) ? 0 : 1.0/r1; //speed/r1;
		double fac = ((180.0*needed_turn_rate)/PI)/fabs(turn_rate.value_pm180());
		head_chg = (b < 0) ? -fac : fac;
*/		
		return true;
	}
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
