// game
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "game.h"
#include <GL/gl.h>
#include <SDL/SDL.h>
#include "system.h"
#include <sstream>
#include "submarine_interface.h"

game::game(submarine* player_sub) : running(true), player(player_sub), time(0)
{
	submarines.push_back(player);
	ui = new submarine_interface(player);
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
	time += delta_t;
	
	// remove old pings
	for (list<ping>::iterator it = pings.begin(); it != pings.end(); ) {
		list<ping>::iterator it2 = it++;
		if (time - it2->time > PINGREMAINTIME)
			pings.erase(it2);
	}
}

void game::dc_explosion(const depth_charge& dc)
{
	// are subs affected?
	// fixme: ships cna be damaged by DCs also...
	for (list<submarine*>::iterator it = submarines.begin(); it != submarines.end(); ++it) {
		double deadly_radius = DEADLY_DC_RADIUS_SURFACE +
			(*it)->get_pos().z * DEADLY_DC_RADIUS_200M / 200;
		vector3 special_dist = (*it)->get_pos() - dc.get_pos();
		special_dist.z *= 2;	// depth differences change destructive power
		if (special_dist.length() <= deadly_radius) {
			system::sys()->add_console("depth charge hit!");
			(*it)->kill();	// sub is killed.
		}
		// fixme handle damages!
	}
}

list<sea_object*> game::ping_ASDIC(const vector2& pos, angle dir)
{
	// remember ping (for drawing)
	pings.push_back(ping(pos, dir, time));
	
	// calculate contacts
	list<sea_object*> contacts;
	
	// fixme: noise from ships can disturb ASDIC or may generate more contacs.
	// ocean floor echoes ASDIC etc...
	for (list<submarine*>::iterator it = submarines.begin(); it != submarines.end(); ++it) {
		vector3 subpos = (*it)->get_pos();
		double dist = subpos.square_distance(vector3(pos.x,pos.y,0));
		if (dist > ASDICRANGE*ASDICRANGE) continue;
		angle diffang = dir - angle(subpos.xy() - pos);
		double deltaang = diffang.value_pm180();
		if (deltaang < -PINGANGLE/2 || deltaang > PINGANGLE/2) continue;
		contacts.push_back(*it);
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
