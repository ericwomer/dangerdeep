// gun shells
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef GUN_SHELL_H
#define GUN_SHELL_H

#include "sea_object.h"

#define AIR_RESISTANCE 0.05	// factor of velocity that gets subtracted
				// from it to slow the shell down

class gun_shell : public sea_object
{
 private:
	gun_shell();
	gun_shell& operator=(const gun_shell& other);
	gun_shell(const gun_shell& other);
 protected:
	vector3 oldpos;		// position at last iteration (for collision detection)
	double damage_amount;

 public:
	gun_shell(game& gm_);	// for loading
	gun_shell(game& gm_, const vector3& pos, angle direction, angle elevation,
		double initial_velocity, double damage);	// for creation

	virtual void load(const xml_elem& parent);
	virtual void save(xml_elem& parent) const;

	virtual void simulate(double delta_time);
	virtual void display(void) const;
	virtual float surface_visibility(const vector2& watcher) const;
	// acceleration is only gravity and already handled by sea_object
	virtual double damage() const { return damage_amount; }
};

#endif
