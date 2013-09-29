/*
Danger from the Deep - Open source submarine simulation
Copyright (C) 2003-2006  Thorsten Jordan, Luis Barrancos and others.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

// AI for various objects
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef AI_H
#define AI_H

/*
How AI should work.
We have convoys or single ships.
zig-zag large scale means changing course around main course
every few sea miles (to make it more difficult for subs to
keep contact)
zig-zag small scale means changing course periodically every
few seconds or minutes to avoid torpedo hits and make
course prediction more difficult for the sub.

Single ships, escort:
Normally zig-zag (maybe large scale + small scale),
Evade subs with high speed or counter attack

Single ships, merchant:
Zig-zag large scale, small scale when attacked,
try to avoid torpedoes when turning, or running away
when capabable of more than 15 knots.

Single ships, warships:
Fire at the sub, if detected. Run away and zig-zag.

Convoy:
central control!
Whole convoy zig-zags large scale around main course.
Merchants run straight, destroyers are distributed around
the convoy and given some duty (ping with sonar, watch, where
to look, secure front or side, help out if attacked, kill subs
when detected etc.)
Escorts run up and down the convoy at the side, zig-zagging and/or
turning, using ASDIC to ping.
Front escorts sweep the front of the whole convoy with ASDIC.
Backside escorts sweep also.
When the convoy is attacked, the convoy control determines which
escorts should engage the submarine. The nearest escort
should do so, the others should regroup to protect the convoy.
Do not let all escorts head to one side of a convoy (it could
be a trap of a wolfpack...)
Especially backside escorts could come for support. Do not send more
than 3-4 escorts to a sub (enough for hunting).
When detected multiple subs, distribute escorts to them, 1-2 for each
submarine.
NOTE: implement historical convoy tactics here! see those BBC website etc.
Leave an escort screen around the convoy if possible. It depends on
the time of war, if subs are rather attacked or the convoy is rather
protected. Escorts that watch and ping around the convoy can't really
protect it, if they do not attack detected submarines...
So rather attack detected submarines, than keep up the screen.
What is with warships? rather run away...

Programming model:

So we have single ship AI, sub types escort/merchant/warship.
And convoy ai... a general/sergeant model.
Each AI has states, what it does at the moment, with flags (should
zig-zag large scale, small scale etc.), switching between the states.
Convoy AI sends commands to the ship's AIs (-> changing states).
Ship AI has some higher priority, i.e. when self defending or attacking
a submarine (zig-zagging to evade torpedoes), the convoy AI should
not be able to change the state, at least until the imminent danger
is over... to avoid that the convoy sends a "run to position xyz" command,
so that the ship stops zig-zagging and a torpedo hits it meanwhile...

State of realization:

Only partly, only older, simpler model.


*/


#include "xml.h"
#include <list>

class ship;
class sea_object;
class convoy;
class game;

///\brief This class implements artificial intelligence for various objects.
///\todo split this class in heir classes for specific objects or AI types.
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
	ship* parent;		// fixme: should be sea_object and redefined by heirs!
	sea_object* followme;		// could be a sea_object instead of ship?
	class convoy* myconvoy;	// convoy to which parent belongs (if any)
	bool has_contact;
	vector3 contact;	// position of target to attack
	double remaining_time;	// time to next thought/situation analysis
	angle main_course;	// which angle to steer, ship zig-zags around it.
	
	bool cyclewaypoints;
	std::list<vector2> waypoints;	
	
	ai();
	ai(const ai& other);
	ai& operator= (const ai& other);

public:
	ai(ship* parent_, types type_);
	virtual ~ai();

	// attention: all sea_objects must exist BEFORE this is called!
	void load(game& gm_, const xml_elem& parent);
	void save(game& gm, xml_elem& parent) const;

private:
	void clear_waypoints() { waypoints.clear(); };
	void add_waypoint(const vector2& wp) { waypoints.push_back(wp); };
	void set_convoy(class convoy* cv) { myconvoy = cv; }

public:
	virtual void attack_contact(const vector3& c);
	virtual void act(class game& gm, double delta_time);

private:
	// various ai's and helper functions, fixme replace with subclasses
	virtual void set_zigzag(bool stat = true);
	virtual void act_escort(class game& g, double delta_time);
	virtual void act_dumb(class game& g, double delta_time);
	virtual void act_convoy(class game& g, double delta_time);
	virtual bool set_course_to_pos(class game& gm, const vector2& pos);	// steer parent to pos, returns true if direct turn is possible
	virtual void relax(class game& gm);	// follow path/object, remove contact info
	virtual void follow(sea_object* t = 0);	// follows path if t is 0
	void cycle_waypoints(bool cycle = true) { cyclewaypoints = cycle; };
};

#endif
