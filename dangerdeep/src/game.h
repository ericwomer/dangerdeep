// game
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef SIMULATION_H
#define SIMULATION_H

#define PINGREMAINTIME 1.0	// seconds
#define PINGANGLE 15		// angle
#define PINGLENGTH 1000		// meters. for drawing
#define ASDICRANGE 1500.0	// meters fixme: historic values?

#include <list>
#include <vector>
using namespace std;
#include "ship.h"
#include "submarine.h"
#include "airplane.h"
#include "torpedo.h"
#include "depth_charge.h"
#include "gun_shell.h"
#include "vector2.h"
#include "model.h"
#include "global_data.h"
#include "user_interface.h"
#include "parser.h"

class game	// our "world" with physics etc.
{
public:
	struct ping {
		vector2 pos;
		angle dir;
		double time;
		ping(const vector2& p, angle d, double t) : pos(p), dir(d), time(t) {}
	};

protected:
	list<ship*> ships;
	list<submarine*> submarines;
	list<airplane*> airplanes;
	list<torpedo*> torpedoes;
	list<depth_charge*> depth_charges;
	list<gun_shell*> gun_shells;
	bool running;	// if this is false, the player was killed
	
	// the player and matching ui (note that playing is not limited to submarines!)
	sea_object* player;
	user_interface* ui;
	
	double time;	// global time (in seconds since 1.1.1939, 0:0 hrs)
	
	enum weathers { sunny, clouded, raining, storm };
	double max_view_dist;	// maximum visibility according to weather conditions
	
	list<ping> pings;
	
	game();
	game& operator= (const game& other);
	game(const game& other);

public:
	game(parser& p);
	~game();

	void simulate(double delta_t);

	double get_time(void) const { return time; };
//	sea_object* get_player(void) { return player; };
	double get_max_view_distance(void) const { return max_view_dist; }
	
	// get view info
	bool can_see(const sea_object* watcher, const submarine* sub) const;

	void spawn_ship(ship* s) { ships.push_back(s); };
	void spawn_submarine(submarine* u) { submarines.push_back(u); };
	void spawn_torpedo(torpedo* t) { torpedoes.push_back(t); };
	void spawn_gun_shell(gun_shell* s) { gun_shells.push_back(s); };
	void spawn_depth_charge(depth_charge* dc) { depth_charges.push_back(dc); };

	// actions important to simulation (for drawing/sound simulation!)
	void dc_explosion(const vector3& pos);	// depth charge exploding
	void gs_impact(const vector3& pos);	// gun shell impact
	void torp_explode(const vector3& pos);	// torpedo explosion/impact

	list<vector3> ping_ASDIC(const vector2& pos, angle dir);	// returns contacts
	const list<ping>& get_pings(void) const { return pings; };
	
	ship* ship_in_direction_from_pos(const vector2& pos, angle direction);
	submarine* sub_in_direction_from_pos(const vector2& pos, angle direction);
	list<ship*>& get_ships(void) { return ships; };
	list<submarine*>& get_submarines(void) { return submarines; };
	list<airplane*>& get_airplanes(void) { return airplanes; };
	list<torpedo*>& get_torpedoes(void) { return torpedoes; };
	list<depth_charge*>& get_depth_charges(void) { return depth_charges; };
	list<gun_shell*>& get_gun_shells(void) { return gun_shells; };
	// note: no reference, a copy is needed
	list<sea_object*> get_all_sea_objects(void);

	double water_depth(const vector2& pos) const;

	// compute visibility information for any object
	// fixme separate for airplanes, ships, subs, torpedos, dcs?
	list<sea_object*> objects_seen(const vector3& pos, angle dir);
	list<sea_object*> objects_heard(const vector3& pos, angle dir);
	list<sea_object*> objects_radardetected(const vector3& pos, angle dir);
	
	game(const char* filename) { read(filename); };
	void read(const char* filename);
	void write(const char* filename) const;
	
	void main_playloop(class system& sys);
};

#endif
