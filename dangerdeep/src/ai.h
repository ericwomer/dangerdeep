// AI for various objects
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef AI_H
#define AI_H

#include "vector3.h"
#include "global_data.h"
#include "angle.h"
#include "sea_object.h"
#include <list>
#include <map>
using namespace std;

class ai
{
public:
	enum types { dumb, escort, convoy };	// fixme: make heir classes for this
	enum states { retreat, followpath, followobject,
		attackcontact };

protected:
	types type;
	states state;
	unsigned zigzagstate;
	bool attackrun;		// true when running full speed shortly before the attack
	bool evasive_manouver;	// true when set_course tries an alternative route
	double rem_manouver_time; // remaining time that ai should wait for during an evasive manouver
	sea_object::ref parent;
	sea_object::ref followme;
	class convoy* myconvoy;	// convoy to which parent belongs (if any)
	bool has_contact;
	vector3 contact;	// position of target to attack
	double remaining_time;	// time to next thought/situation analysis
	
	bool cyclewaypoints;
	list<vector2> waypoints;
	
	angle last_elevation, last_azimuth;	// remeber last values.
	
	// some experience values of the crews to fire a grenade with right angle at any
	// target. This depends on canon type (shot speed, min/max angles etc.) so we need
	// several ai classes later.
	static map<double, double> dist_angle_relation;
	static void fill_dist_angle_relation_map(void);

	ai();
	ai(const ai& other);
	ai& operator= (const ai& other);

public:
	ai::ai(sea_object* parent_, types type_);
	virtual ~ai() {};

	ai(istream& in, class game& g);	// attention: all sea_objects must exist BEFORE this is called!
	void save(ostream& out, const class game& g) const;

	void clear_waypoints(void) { waypoints.clear(); };
	void add_waypoint(const vector2& wp) { waypoints.push_back(wp); };
	void set_convoy(class convoy* cv) { myconvoy = cv; }

	virtual void relax(class game& gm);	// follow path/object, remove contact info
	virtual void attack_contact(const vector3& c);
	virtual void follow(sea_object* t = 0);	// follows path if t is 0
	void cycle_waypoints(bool cycle = true) { cyclewaypoints = cycle; };
	virtual void act(class game& gm, double delta_time);

	// various ai's and helper functions, fixme replace with subclasses
	virtual void set_zigzag(bool stat = true);
	virtual void act_escort(class game& g, double delta_time);
	virtual void act_dumb(class game& g, double delta_time);
	virtual void act_convoy(class game& g, double delta_time);
	virtual void fire_shell_at(class game& gm, const sea_object& s);
	virtual bool set_course_to_pos(class game& gm, const vector2& pos);	// steer parent to pos, returns true if direct turn is possible
};

#endif
