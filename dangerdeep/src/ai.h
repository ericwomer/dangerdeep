// AI for various objects
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef AI_H
#define AI_H

#include "vector3.h"
#include "sea_object.h"
#include "global_data.h"

#define AI_AWARENESS_DISTANCE 20000	// 20km should be enough
#define WPEXACTNESS 100			// how exact a waypoint has to be hit in meters
#define AI_THINK_CYCLE_TIME 10		// sec

class ai
{
public:
	enum types { dumb, escort };
	enum states { retreat, followpath, followtarget,
		searchenemy, attackenemy };

protected:
	types type;
	states state;
	unsigned zigzagstate;
	sea_object* parent;
	sea_object* target;
	double last_thought_time;
	bool cyclewaypoints;
	
	list<vector2> waypoints;
	
	ai();
	ai(const ai& other);
	ai& operator= (const ai& other);

public:

	ai(sea_object* parent_, types type_) : type(type_), state(followpath),
		zigzagstate(0),
		parent(parent_), target(0), last_thought_time(-AI_THINK_CYCLE_TIME),
		cyclewaypoints(false) {};
	virtual ~ai() {};
	
	void clear_waypoints(void) { waypoints.clear(); };
	void add_waypoint(const vector2& wp) { waypoints.push_back(wp); };

	// set and attack target or be alerted (t == 0), maximum alert
	virtual void search_enemy(void);	// has no effect when ai is already attacking
	virtual void attack(sea_object* t = 0);
	virtual void follow(sea_object* t);
	void cycle_waypoints(bool cycle = true) { cyclewaypoints = cycle; };
	virtual void act(class game& gm, double delta_time);

	// various ai's and helper functions
	virtual void set_zigzag(bool stat = true);
	virtual void act_escort(class game& g, double delta_time);
	virtual void act_dumb(class game& g, double delta_time);
	virtual void set_course_to_target(void);
	virtual void set_course_to_pos(const vector2& pos);
};

#endif
