// game
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef SIMULATION_H
#define SIMULATION_H

#include <list>
#include <vector>
using namespace std;
#include "ship.h"
#include "submarine.h"
#include "airplane.h"
#include "torpedo.h"
#include "depth_charge.h"
#include "vector2.h"
#include "model.h"
#include "global_data.h"
#include "user_interface.h"

class game	// our "world" with physics etc.
{
protected:
	list<ship*> ships;
	list<submarine*> submarines;
	list<airplane*> airplanes;
	list<torpedo*> torpedoes;
	list<depth_charge*> depth_charges;
	bool running;	// if this is false, the player was killed
	
	// fixme later we may control a destroyer?
	submarine* player; // sea_object* player;
	user_interface* ui; //submarine_interface* ui;
	
	double time;	// global time (in seconds since 1.1.1939, 0:0 hrs)
	
	enum weathers { sunny, clouded, raining, storm };
	double max_view_dist;	// maximum visibility according to weather conditions
	
	game();
	game& operator= (const game& other);
	game(const game& other);

public:
	game(submarine* player_sub);
	~game();

	void simulate(double delta_t);

	double get_time(void) const { return time; };
	submarine* get_player(void) { return player; };
	double get_max_view_distance(void) const { return max_view_dist; }

	void spawn_ship(ship* s) { ships.push_back(s); };
	void spawn_submarine(submarine* u) { submarines.push_back(u); };
	void spawn_torpedo(torpedo* t) { torpedoes.push_back(t); };
//	void spawn_shell(shell* s) { shells.push_back(s); };
	void spawn_depth_charge(depth_charge* dc) { depth_charges.push_back(dc); };


	// actions important to simulation
	void dc_explosion(const depth_charge& dc);
	void ping_ASDIC(const vector2& pos, angle dir) {};//fixme: put to another place
	
	ship* ship_in_direction_from_pos(const vector2& pos, angle direction);
	list<ship*> get_ships(void) { return ships; };
	list<submarine*> get_submarines(void) { return submarines; };
	list<airplane*> get_airplanes(void) { return airplanes; };
	list<torpedo*> get_torpedoes(void) { return torpedoes; };
	list<depth_charge*> get_depth_charges(void) { return depth_charges; };
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
