// game
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "game.h"
#include <GL/gl.h>
#include <SDL/SDL.h>
#include "system.h"
#include <sstream>
#include "submarine_interface.h"
#include "ship_interface.h"

game::game(submarine* player_sub) : running(true), player(player_sub), time(0)
{
	submarines.push_back(player_sub);
	ui = new submarine_interface(player_sub);
	max_view_dist = 10000;	// fixme, introduce weather
}

game::game(ship* player_ship) : running(true), player(player_ship), time(0)
{
	ships.push_back(player_ship);
	ui = new ship_interface(player_ship);
	max_view_dist = 10000;	// fixme, introduce weather
}

game::~game()
{
	list<sea_object*> all = get_all_sea_objects();
	for (list<sea_object*>::iterator it = all.begin(); it != all.end(); ++it)
		delete (*it);
	delete ui;
}

void game::simulate(double delta_t)
{
	if (!running) return;

	if (!player->is_alive()) {
		running = false;
		return;
	}
	
	if (ships.size() == 0 && torpedoes.size() == 0 && depth_charges.size() == 0 &&
			airplanes.size() == 0 && gun_shells.size() == 0) {
		running = false;
		return;
	}

	for (list<ship*>::iterator it = ships.begin(); it != ships.end(); ) {
		list<ship*>::iterator it2 = it++;
		if (!(*it2)->is_defunct()) {
			(*it2)->simulate(*this, delta_t);
		} else {
			delete (*it2);
			ships.erase(it2);
		}
	}
	for (list<submarine*>::iterator it = submarines.begin(); it != submarines.end(); ) {
		list<submarine*>::iterator it2 = it++;
		if (!(*it2)->is_defunct()) {
			(*it2)->simulate(*this, delta_t);
		} else {
			delete (*it2);
			submarines.erase(it2);
		}
	}
	for (list<airplane*>::iterator it = airplanes.begin(); it != airplanes.end(); ) {
		list<airplane*>::iterator it2 = it++;
		if (!(*it2)->is_defunct()) {
			(*it2)->simulate(*this, delta_t);
		} else {
			delete (*it2);
			airplanes.erase(it2);
		}
	}
	for (list<torpedo*>::iterator it = torpedoes.begin(); it != torpedoes.end(); ) {
		list<torpedo*>::iterator it2 = it++;
		if (!(*it2)->is_defunct()) {
			(*it2)->simulate(*this, delta_t);
		} else {
			delete (*it2);
			torpedoes.erase(it2);
		}
	}
	for (list<depth_charge*>::iterator it = depth_charges.begin(); it != depth_charges.end(); ) {
		list<depth_charge*>::iterator it2 = it++;
		if (!(*it2)->is_defunct()) {
			(*it2)->simulate(*this, delta_t);
		} else {
			delete (*it2);
			depth_charges.erase(it2);
		}
	}
	for (list<gun_shell*>::iterator it = gun_shells.begin(); it != gun_shells.end(); ) {
		list<gun_shell*>::iterator it2 = it++;
		if (!(*it2)->is_defunct()) {
			(*it2)->simulate(*this, delta_t);
		} else {
			delete (*it2);
			gun_shells.erase(it2);
		}
	}
	time += delta_t;
	
	// remove old pings
	for (list<ping>::iterator it = pings.begin(); it != pings.end(); ) {
		list<ping>::iterator it2 = it++;
		if (time - it2->time > PINGREMAINTIME)
			pings.erase(it2);
	}
}

bool game::can_see(const sea_object* watcher, const submarine* sub) const
{
	vector3 pos = sub->get_pos();
	if (pos.z < -12) return false;	// fixme: per define
	if (pos.z < -8 && !sub->is_scope_up()) return false;	// fixme: per define
	
	// wether a sub is visible or not depends on the distance to the watcher
	// (and of the type of the watcher, e.g. the height of the masts of a watcher etc)
	// and if the sub's scope is up or the sub is surfaced, it's speed and course
	
	double sq = watcher->get_pos().xy().square_distance(sub->get_pos().xy());
	if (sq > 1e10/*SKYDOMERADIUS*SKYDOMERADIUS*/) return false; // fixme
	
	return true;
}

void game::dc_explosion(const vector3& pos)
{
}

void game::gs_impact(const vector3& pos)
{
}

void game::torp_explode(const vector3& pos)
{
}

list<vector3> game::ping_ASDIC(const vector2& pos, angle dir)
{
	// remember ping (for drawing)
	pings.push_back(ping(pos, dir, time));
	
	// calculate contacts
	list<vector3> contacts;
	
	// fixme: noise from ships can disturb ASDIC or may generate more contacs.
	// ocean floor echoes ASDIC etc...
	for (list<submarine*>::iterator it = submarines.begin(); it != submarines.end(); ++it) {
		vector3 subpos = (*it)->get_pos();
		if (subpos.z >= -2) continue;	// surfaced subs can't be detected
		double dist = subpos.distance(vector3(pos.x,pos.y,0));
		if (dist > ASDICRANGE) continue;
		angle contactangle(subpos.xy() - pos);
		angle diffang = dir - contactangle;
		double deltaang = diffang.value_pm180();
		if (deltaang < -PINGANGLE/2 || deltaang > PINGANGLE/2) continue;
		angle subtocontactangle = contactangle + angle(180) - (*it)->get_heading();
		// fixme: a rather crude approximation
		// fixme: correct values for angle dependency missing
		// depends on: sub distance, sub angle to ping, sub depth, sub type
		double probability = (0.3 + 0.7*fabs(subtocontactangle.sin()))
			* (1.0 - dist/ASDICRANGE)
			* (1.0-0.5*subpos.z/400.0);
		// fixme: this value may vary.
		// fixme: fuzzyness depends on distance
		if (probability > 0.3+(rnd(10))*0.02)
			contacts.push_back(subpos + vector3(rnd(40)-20,rnd(40)-20,rnd(40)-20));
	}
	
	return contacts;
}

list<sea_object*> game::get_all_sea_objects(void)
{
	list<sea_object*> result;
	for (list<ship*>::iterator it = ships.begin(); it != ships.end(); ++it)
		result.push_back(*it);
	for (list<submarine*>::iterator it = submarines.begin(); it != submarines.end(); ++it)
		result.push_back(*it);
	for (list<airplane*>::iterator it = airplanes.begin(); it != airplanes.end(); ++it)
		result.push_back(*it);
	for (list<torpedo*>::iterator it = torpedoes.begin(); it != torpedoes.end(); ++it)
		result.push_back(*it);
	for (list<depth_charge*>::iterator it = depth_charges.begin(); it != depth_charges.end(); ++it)
		result.push_back(*it);
	for (list<gun_shell*>::iterator it = gun_shells.begin(); it != gun_shells.end(); ++it)
		result.push_back(*it);
	return result;
}

ship* game::ship_in_direction_from_pos(const vector2& pos, angle direction)
{
	ship* result = 0;
	double angle_diff = 30;	// fixme: use range also, use ship width's etc.
	for (list<ship*>::iterator it = ships.begin(); it != ships.end(); ++it) {
		vector2 df = vector2((*it)->get_pos().x, (*it)->get_pos().y) - pos;
		double new_ang_diff = (angle(df)).diff(direction);
//		double range = diff.length();
		if (new_ang_diff < angle_diff) {
			angle_diff = new_ang_diff;
			result = *it;
		}
	}
	return result;
}

submarine* game::sub_in_direction_from_pos(const vector2& pos, angle direction)
{
	submarine* result = 0;
	double angle_diff = 30;	// fixme: use range also, use ship width's etc.
	for (list<submarine*>::iterator it = submarines.begin(); it != submarines.end(); ++it) {
		vector2 df = vector2((*it)->get_pos().x, (*it)->get_pos().y) - pos;
		double new_ang_diff = (angle(df)).diff(direction);
//		double range = diff.length();
		if (new_ang_diff < angle_diff) {
			angle_diff = new_ang_diff;
			result = *it;
		}
	}
	return result;
}

// main play loop
void game::main_playloop(class system& sys)
{
	unsigned frames = 1;
	unsigned lasttime = sys.millisec();
	double fpstime = 0;
	double totaltime = 0;
	
	// draw one initial frame
	ui->display(sys, *this);
	
	while (running && !ui->user_quits()) {
		sys.poll_event_queue();

		// this time_scaling is bad. hits may get computed wrong when time
		// scaling is too high. fixme
		unsigned thistime = sys.millisec();
		double delta_time = (thistime - lasttime)/1000.0 * ui->time_scaling();
		totaltime += (thistime - lasttime)/1000.0;
		lasttime = thistime;
		
		// next simulation step
		if (!ui->paused()) {
			simulate(delta_time);
			time += delta_time;
		}

		ui->display(sys, *this);
		++frames;

		// record fps
		if (totaltime - fpstime > 5) {
			fpstime = totaltime;
			ostringstream os;
			os << "$c0fffffps " << frames/totaltime;
			sys.add_console(os.str());
		}
		
		sys.swap_buffers();
	}
}
