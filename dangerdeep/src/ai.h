// AI for various objects
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef AI_H
#define AI_H

#include "vector3.h"
#include "sea_object.h"
#include "global_data.h"

#define WPEXACTNESS 100			// how exact a waypoint has to be hit in meters
#define AI_THINK_CYCLE_TIME 10		// sec
#define DC_ATTACK_RADIUS 100		// distance to target before DC launching starts
#define DC_ATTACK_RUN_RADIUS 200	// distance to contact until escort switches to
					// maximum speed

class ai
{
public:
	enum types { dumb, escort };
	enum states { retreat, followpath, followobject,
		attackcontact };

protected:
	types type;
	states state;
	unsigned zigzagstate;
	bool attackrun;		// true when running full speed shortly before the attack
	sea_object* parent;
	sea_object* followme;
	bool has_contact;
	vector3 contact;	// position of target to attack
	double remaining_time;	// time to next thought/situation analysis
	bool cyclewaypoints;
	
	list<vector2> waypoints;
	
	ai();
	ai(const ai& other);
	ai& operator= (const ai& other);

public:

	// ai computation between is rndly interleaved between frames to avoid
	// time consumption peeks every AI_THINK_CYCLE_TIME seconds
	ai(sea_object* parent_, types type_) : type(type_), state(followpath),
		zigzagstate(0), parent(parent_), followme(0),
		has_contact(false),
		remaining_time(rnd() * AI_THINK_CYCLE_TIME),
		cyclewaypoints(false) {};
	virtual ~ai() {};
	
	void clear_waypoints(void) { waypoints.clear(); };
	void add_waypoint(const vector2& wp) { waypoints.push_back(wp); };

	virtual void relax(void);	// follow path/object, remove contact info
	virtual void attack_contact(const vector3& c);
	virtual void follow(sea_object* t = 0);	// follows path if t is 0
	void cycle_waypoints(bool cycle = true) { cyclewaypoints = cycle; };
	virtual void act(class game& gm, double delta_time);

	// various ai's and helper functions, fixme replace with subclasses
	virtual void set_zigzag(bool stat = true);
	virtual void act_escort(class game& g, double delta_time);
	virtual void act_dumb(class game& g, double delta_time);
	virtual void set_course_to_pos(const vector2& pos);
	virtual void fire_shell_at(class game& gm, const sea_object& s);
};

#endif
