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
#include "convoy.h"

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
	list<convoy*> convoys;
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

	void compute_max_view_dist(void);
	void simulate(double delta_t);

	double get_time(void) const { return time; };
	double get_max_view_distance(void) const { return max_view_dist; }

	// compute visibility data
	list<ship*> visible_ships(const vector3& pos);
	list<submarine*> visible_submarines(const vector3& pos);
	list<airplane*> visible_airplanes(const vector3& pos);
	list<torpedo*> visible_torpedoes(const vector3& pos);
	list<depth_charge*> visible_depth_charges(const vector3& pos);
	list<gun_shell*> visible_gun_shells(const vector3& pos);
	
	list<ship*> hearable_ships(const vector3& pos);
	list<submarine*> hearable_submarines(const vector3& pos);
	
//	bool can_see(const sea_object* watcher, const submarine* sub) const;

	// create new objects
	void spawn_ship(ship* s) { ships.push_back(s); };
	void spawn_submarine(submarine* u) { submarines.push_back(u); };
	void spawn_airplane(airplane* a) { airplanes.push_back(a); };
	void spawn_torpedo(torpedo* t) { torpedoes.push_back(t); };
	void spawn_gun_shell(gun_shell* s) { gun_shells.push_back(s); };
	void spawn_depth_charge(depth_charge* dc) { depth_charges.push_back(dc); };
	void spawn_convoy(convoy* cv) { convoys.push_back(cv); }

	// simulation events
//fixme: send messages about them to ui (remove sys-console printing in torpedo.cpp etc)
	void dc_explosion(const vector3& pos);	// depth charge exploding
	void gs_impact(const vector3& pos);	// gun shell impact
	void torp_explode(const vector3& pos);	// torpedo explosion/impact
	void ship_sunk(unsigned tonnage);	// a ship sinks

	// simulation actions
	list<vector3> ping_ASDIC(const vector2& pos, angle dir);	// returns contacts

	// various functions (fixme: sort, cleanup)
	const list<ping>& get_pings(void) const { return pings; };

	bool check_torpedo_hit(torpedo* t, bool runlengthfailure, bool failure);
	ship* ship_in_direction_from_pos(const vector2& pos, angle direction);
	submarine* sub_in_direction_from_pos(const vector2& pos, angle direction);

	// fixme: remove sea_object::is_collision , replace with some function(s) of this class

	double water_depth(const vector2& pos) const;

/*
	// compute visibility information for any object
	// fixme separate for airplanes, ships, subs, torpedos, dcs?
	list<sea_object*> objects_seen(const vector3& pos, angle dir);
	list<sea_object*> objects_heard(const vector3& pos, angle dir);
	list<sea_object*> objects_radardetected(const vector3& pos, angle dir);
*/

	// fixme: superseded by game(parser& p)	
	game(const char* filename) { read(filename); };
	void read(const char* filename);
	void write(const char* filename) const;
	
	void main_playloop(class system& sys);
};

#endif
