// game
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "game.h"
#include <GL/gl.h>
#include <SDL/SDL.h>
#include "system.h"
#include <sstream>
#include "submarine_interface.h"
#include "ship_interface.h"
#include "tokencodes.h"
#include "texts.h"

game::game(parser& p) : running(true), time(0)
{
	player = 0;
	ui = 0;
	compute_max_view_dist();
	while (!p.is_empty()) {
		bool nextisplayer = false;
		if (p.type() == TKN_PLAYER) {
			if (player != 0) {
				p.error("Player defined twice!");
			}
			p.consume();
			nextisplayer = true;
		}
		switch (p.type()) {
			case TKN_SUBMARINE: {
				submarine* sub = submarine::create(p);
				spawn_submarine(sub);
				if (nextisplayer) {
					player = sub;
					ui = new submarine_interface(sub);
				}
				break; }
			case TKN_SHIP: {
				ship* shp = ship::create(p);
				spawn_ship(shp);
				if (nextisplayer) {
					player = shp;
					ui = new ship_interface(shp);
				}
				break; }
			case TKN_CONVOY: {
				convoy* cv = new convoy(*this, p);
				spawn_convoy(cv);
				break; }
/*			
			case TKN_DESCRIPTION:
			case TKN_WEATHER:
			case TKN_TIME:	// and date
*/
			default: p.error("Expected definition");
		}
	}
	if (player == 0) p.error("No player defined!");
}

game::~game()
{
	for (list<ship*>::iterator it = ships.begin(); it != ships.end(); ++it)
		delete (*it);
	for (list<submarine*>::iterator it = submarines.begin(); it != submarines.end(); ++it)
		delete (*it);
	for (list<airplane*>::iterator it = airplanes.begin(); it != airplanes.end(); ++it)
		delete (*it);
	for (list<torpedo*>::iterator it = torpedoes.begin(); it != torpedoes.end(); ++it)
		delete (*it);
	for (list<depth_charge*>::iterator it = depth_charges.begin(); it != depth_charges.end(); ++it)
		delete (*it);
	for (list<gun_shell*>::iterator it = gun_shells.begin(); it != gun_shells.end(); ++it)
		delete (*it);
	for (list<convoy*>::iterator it = convoys.begin(); it != convoys.end(); ++it)
		delete (*it);
	delete ui;
}

void game::compute_max_view_dist(void)
{
	double dt = get_day_time(get_time());
	if (dt < 1) { max_view_dist = 5000; return; }
	if (dt < 2) { max_view_dist = 5000 + 25000*fmod(dt,1); return; }
	if (dt < 3) { max_view_dist = 30000; return; }
	max_view_dist = 30000 - 25000*fmod(dt,1);
}

void game::simulate(double delta_t)
{
	if (!running) return;

	if (!player->is_alive()) {
		running = false;
		return;
	}
	
	if (/* submarines.size() == 0 && */ ships.size() == 0 && torpedoes.size() == 0 && depth_charges.size() == 0 &&
			airplanes.size() == 0 && gun_shells.size() == 0) {
		running = false;
		return;
	}

	compute_max_view_dist();

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
	for (list<convoy*>::iterator it = convoys.begin(); it != convoys.end(); ) {
		list<convoy*>::iterator it2 = it++;
		if (!(*it2)->is_defunct()) {
			(*it2)->simulate(*this, delta_t);
		} else {
			delete (*it2);
			convoys.erase(it2);
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

list<ship*> game::visible_ships(const vector3& pos)
{
	list<ship*> result;
	double d = get_max_view_distance();
	d = d*d;
	for (list<ship*>::iterator it = ships.begin(); it != ships.end(); ++it) {
		if ((*it)->get_pos().xy().square_distance(pos.xy()) < d) {
			result.push_back(*it);
		}
	}
	return result;
//	return ships;	// test hack fixme
}

list<submarine*> game::visible_submarines(const vector3& pos)
{
	list<submarine*> result;
	double d = get_max_view_distance();
	d = d*d;
	for (list<submarine*>::iterator it = submarines.begin(); it != submarines.end(); ++it) {
		if ((*it)->get_pos().xy().square_distance(pos.xy()) < d) {
			// the probabilty of visibility depends on indivial values
			// relative course, distance to and type of watcher.
			// (height of masts, experience etc.), weather fixme
			float prob = (*it)->surface_visibility(pos.xy());
			if (prob < 0.25) continue;	// fixme: add some randomization!
			result.push_back(*it);
		}
	}
	return result;
//	return list<submarine*>();	// test hack fixme
}

list<airplane*> game::visible_airplanes(const vector3& pos)
{
	list<airplane*> result;
	double d = get_max_view_distance();
	d = d*d;
	for (list<airplane*>::iterator it = airplanes.begin(); it != airplanes.end(); ++it) {
		if ((*it)->get_pos().xy().square_distance(pos.xy()) < d) {
			result.push_back(*it);
		}
	}
	return result;
}

list<torpedo*> game::visible_torpedoes(const vector3& pos)
{
	list<torpedo*> result;
	double d = get_max_view_distance();
	d = d*d;
	for (list<torpedo*>::iterator it = torpedoes.begin(); it != torpedoes.end(); ++it) {
		if ((*it)->get_pos().xy().square_distance(pos.xy()) < d) {
			result.push_back(*it);
		}
	}
	return result;
}

list<depth_charge*> game::visible_depth_charges(const vector3& pos)
{
	list<depth_charge*> result;
	double d = get_max_view_distance();
	d = d*d;
	for (list<depth_charge*>::iterator it = depth_charges.begin(); it != depth_charges.end(); ++it) {
		if ((*it)->get_pos().xy().square_distance(pos.xy()) < d) {
			result.push_back(*it);
		}
	}
	return result;
}

list<gun_shell*> game::visible_gun_shells(const vector3& pos)
{
	list<gun_shell*> result;
	double d = get_max_view_distance();
	d = d*d;
	for (list<gun_shell*>::iterator it = gun_shells.begin(); it != gun_shells.end(); ++it) {
		if ((*it)->get_pos().xy().square_distance(pos.xy()) < d) {
			result.push_back(*it);
		}
	}
	return result;
}

// fixme: noises are disturbing each other!
list<ship*> game::hearable_ships(const vector3& pos)
{
	list<ship*> result;
	for (list<ship*>::iterator it = ships.begin(); it != ships.end(); ++it) {
		double d = (*it)->get_pos().xy().distance(pos.xy());
		double distfac = d/30000;
		double noisefac = (*it)->get_throttle_speed()/(*it)->get_max_speed();
//printf("df nf %f %f\n",distfac,noisefac);		
		if (noisefac - distfac > 0.1) {
			result.push_back(*it);
		}
	}
	return result;
}

list<submarine*> game::hearable_submarines(const vector3& pos)
{
	// fixme: course changes acoustic profile, and also depth, subtype etc.
	list<submarine*> result;
	for (list<submarine*>::iterator it = submarines.begin(); it != submarines.end(); ++it) {
		double d = (*it)->get_pos().xy().distance(pos.xy());
		double distfac = d/5000;
		double noisefac = (*it)->get_throttle_speed()/(*it)->get_max_speed();
//printf("df nf %f %f\n",distfac,noisefac);		
		if (noisefac - distfac > 0.1) {
			result.push_back(*it);
		}
	}
	return result;
}

list<vector2> game::convoy_positions(void) const
{
	list<vector2> result;
	for (list<convoy*>::const_iterator it = convoys.begin(); it != convoys.end(); ++it) {
		result.push_back((*it)->get_pos().xy());
	}
	return result;
}

void game::dc_explosion(const vector3& pos)
{
	// are subs affected?
	// fixme: ships can be damaged by DCs also...
	for (list<submarine*>::iterator it = submarines.begin(); it != submarines.end(); ++it) {
		double deadly_radius = DEADLY_DC_RADIUS_SURFACE +
			(*it)->get_pos().z * DEADLY_DC_RADIUS_200M / 200;
		vector3 sdist = (*it)->get_pos() - pos;
		sdist.z *= 2;	// depth differences change destructive power
		/*
		if (sdist.length() <= damage_radius) {
			(*it)->dc_damage(pos);
		}
		*/
		if (sdist.length() <= deadly_radius) {
			// ui->add_message(TXT_Depthchargehit[language]);
			system::sys()->add_console("depth charge hit!");
			(*it)->kill();	// sub is killed. //  fixme handle damages!
		}
	}
}

void game::gs_impact(const vector3& pos)	// fixme: vector2 would be enough
{
	return;//fixme testing
	for (list<ship*>::iterator it = ships.begin(); it != ships.end(); ++it) {
		// fixme: we need a special collision detection, because
		// the shell is so fast that it can be collisionfree with *it
		// delta_time ago and now, but hit *it in between
		if ((*it)->is_collision(pos.xy())) {
			(*it)->damage((*it)->get_pos() /*fixme*/,GUN_SHELL_HITPOINTS);
			return;	// only one hit possible
		}
	}
	for (list<submarine*>::iterator it = submarines.begin(); it != submarines.end(); ++it) {
		if ((*it)->is_collision(pos.xy())) {
			(*it)->damage((*it)->get_pos() /*fixme*/,GUN_SHELL_HITPOINTS);
			return; // only one hit possible
		}
	}
}

void game::torp_explode(const vector3& pos)
{
}

void game::ship_sunk(unsigned tons)
{
	ui->record_ship_tonnage(tons);
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

bool game::check_torpedo_hit(torpedo* t, bool runlengthfailure, bool failure)
{
	for (list<ship*>::iterator it = ships.begin(); it != ships.end(); ++it) {
		if (t->is_collision(*it)) {
			if (runlengthfailure) {
				ui->add_message(TXT_Torpedodudrangetooshort[language]);
				return true;
			}
			if (failure) {
				ui->add_message(TXT_Torpedodud[language]);
				return true;
			}
			(*it)->damage((*it)->get_pos(), G7A_HITPOINTS);//fixme
			torp_explode(t->get_pos());
			return true;	// only one hit possible
		}
	}
	for (list<submarine*>::iterator it = submarines.begin(); it != submarines.end(); ++it) {
//	fixme remove this and compute correct initial torpedo position	
//		if (*it == parent && run_length <= parent->get_length()) continue;
		if (t->is_collision(*it)) {
			if (runlengthfailure) {
				ui->add_message(TXT_Torpedodudrangetooshort[language]);
				return true;
			}
			if (failure) {
				ui->add_message(TXT_Torpedodud[language]);
				return true;
			}
			(*it)->damage((*it)->get_pos(), G7A_HITPOINTS);//fixme
			torp_explode(t->get_pos());
			return true; // only one hit possible
		}
	}
	return false;
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
