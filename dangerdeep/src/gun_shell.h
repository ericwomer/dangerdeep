// gun shells
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef GUN_SHELL_H
#define GUN_SHELL_H

#include "sea_object.h"

#define GUN_SHELL_HITPOINTS 0.2	// fixme
#define AIR_RESISTANCE 0.05	// factor of velocity that gets subtracted
				// from it to slow the shell down
#define GUN_SHELL_INITIAL_VELOCITY	200.0f	// m/sec, low while testing

class gun_shell : public sea_object
{
protected:
	vector3 velocity;	// current velocity
	vector3 oldpos;		// position at last iteration (for collision detection)

	gun_shell& operator=(const gun_shell& other);
	gun_shell(const gun_shell& other);
public:
	gun_shell() {};
	virtual ~gun_shell() {};
	void load(istream& in, class game& g);
	void save(ostream& out, const class game& g) const;
	gun_shell(const sea_object& parent, angle direction, angle elevation,
		double initial_velocity = GUN_SHELL_INITIAL_VELOCITY);
	virtual void simulate(class game& gm, double delta_time);
	virtual void display(void) const;
};

#endif
