// game
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef SIMULATION_H
#define SIMULATION_H

#define PINGREMAINTIME 1.0	// seconds
#define PINGANGLE 15		// angle
#define PINGLENGTH 1000		// meters. for drawing
#define ASDICRANGE 1500.0	// meters fixme: historic values?
#define MAX_ACUSTIC_CONTACTS 5	// max. nr of simultaneous trackable acustic contacts

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
#include "water_splash.h"

class game	// our "world" with physics etc.
{
public:
	struct ping {
		vector2 pos;
		angle dir;
		double time;
		angle pingAngle;
		double range;
		ping(const vector2& p, angle d, double t, const double& range,
			const angle& pingAngle ) :
			pos(p), dir(d), time(t), range ( range ), pingAngle ( pingAngle )
			{}
	};

protected:
	list<ship*> ships;
	list<submarine*> submarines;
	list<airplane*> airplanes;
	list<torpedo*> torpedoes;
	list<depth_charge*> depth_charges;
	list<gun_shell*> gun_shells;
	list<convoy*> convoys;
	list<water_splash*> water_splashs;
	bool running;	// if this is false, the player was killed
	
	// the player and matching ui (note that playing is not limited to submarines!)
	sea_object* player;
	user_interface* ui;
	
	double time;	// global time (in seconds since 1.1.1939, 0:0 hrs)
	double last_trail_time;	// for position trail recording
	
	enum weathers { sunny, clouded, raining, storm };//fixme
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
	/**
		This method is needed to verify for day and night mode for the
		display methods within the user interfaces.
		@return true when day mode, false when night mode
	*/
	bool is_day_mode () const;
	/**
		This method calculates a depth depending factor. A deep diving
		submarine is harder to detect with ASDIC than a submarine at
		periscope depth.
		@param sub location vector of submarine
		@return depth factor
	*/
	virtual double get_depth_factor ( const vector3& sub ) const;

	// compute visibility data
	virtual void visible_ships(list<ship*>& result, const sea_object* o);
	virtual void visible_submarines(list<submarine*>& result, const sea_object* o);
	virtual void visible_airplanes(list<airplane*>& result, const sea_object* o);
	virtual void visible_torpedoes(list<torpedo*>& result, const sea_object* o);
	virtual void visible_depth_charges(list<depth_charge*>& result, const sea_object* o);
	virtual void visible_gun_shells(list<gun_shell*>& result, const sea_object* o);
	virtual void visible_water_splashes ( list<water_splash*>& result, const sea_object* o );

	virtual void sonar_ships ( list<ship*>& result, const sea_object* o );
	virtual void sonar_submarines ( list<submarine*>& result, const sea_object* o );
	virtual ship* sonar_acoustical_torpedo_target ( const torpedo* o );
	
	// list<*> radardetected_ships(...);	// later!

	void convoy_positions(list<vector2>& result) const;	// fixme
	
//	bool can_see(const sea_object* watcher, const submarine* sub) const;

	// create new objects
	void spawn_ship(ship* s) { ships.push_back(s); };
	void spawn_submarine(submarine* u) { submarines.push_back(u); };
	void spawn_airplane(airplane* a) { airplanes.push_back(a); };
	void spawn_torpedo(torpedo* t) { torpedoes.push_back(t); };
	void spawn_gun_shell(gun_shell* s) { gun_shells.push_back(s); };
	void spawn_depth_charge(depth_charge* dc) { depth_charges.push_back(dc); };
	void spawn_convoy(convoy* cv) { convoys.push_back(cv); }
	void spawn_water_splash ( water_splash* ws ) { water_splashs.push_back ( ws ); }

	// simulation events
//fixme: send messages about them to ui (remove sys-console printing in torpedo.cpp etc)
	void dc_explosion(const vector3& pos);	// depth charge exploding
	bool gs_impact(const vector3& pos);	// gun shell impact
	void torp_explode(const vector3& pos);	// torpedo explosion/impact
	void ship_sunk(unsigned tonnage);	// a ship sinks

	// simulation actions
	virtual void ping_ASDIC(list<vector3>& contacts, sea_object* d,
		const bool& move_sensor, const angle& dir = angle ( 0.0f ) );

	// various functions (fixme: sort, cleanup)
	const list<ping>& get_pings(void) const { return pings; };

	bool check_torpedo_hit(torpedo* t, bool runlengthfailure, bool failure);

	sea_object* contact_in_direction(const sea_object* o, const angle& direction);
	ship* ship_in_direction_from_pos(const sea_object* o, const angle& direction);
	submarine* sub_in_direction_from_pos(const sea_object* o, const angle& direction);

	bool is_collision(const sea_object* s1, const sea_object* s2) const;
	bool is_collision(const sea_object* s, const vector2& pos) const;

	double water_depth(const vector2& pos) const;

	// fixme: superseded by game(parser& p)	
	game(const char* filename) { read(filename); };
	void read(const char* filename);
	void write(const char* filename) const;
	
	void main_playloop(class system& sys);
};

#endif
