// ships
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef SHIP_H
#define SHIP_H

#include "sea_object.h"
#include "global_data.h"
#include "ai.h"

class ship : public sea_object
{
protected:
	ai* myai;
	unsigned type;

	damage_status stern_damage, midship_damage, bow_damage;
	
	ship() : sea_object() {}
	ship(const ship& other);
	ship& operator= (const ship& other);
public:
	virtual ~ship() {}

	// types:
	// 0 medium merchant
	// 1 medium troopship
	// 2 tribal class destroyer
	// 3 malaya class battleship
	ship(unsigned type_, const vector3& pos, angle heading = 0);
	virtual void simulate(class game& gm, double delta_time);
//	virtual void simulate_escort(class game& gm, double delta_time);
	virtual void fire_shell_at(const vector2& pos);
	virtual void damage(const vector3& fromwhere, unsigned strength);
	virtual unsigned calc_damage(void) const;	// returns damage in percent (0 means dead)
	virtual void sink(void);
	virtual void kill(void);
	virtual void display(void) const;
	ai* get_ai(void) { return myai; }
};

#endif
