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

	ship() {};
	ship(const ship& other);
	ship& operator= (const ship& other);
public:
	virtual ~ship() {}

	enum damagetypes { nodamage, flooding, wrecked };
	damagetypes bow, midships, stern;
	
	// types:
	// 0 medium merchant
	// 1 medium troopship
	// 2 tribal class destroyer
	// 3 malaya class battleship
	ship(unsigned type_, const vector3& pos, angle heading = 0);
	virtual void simulate(class game& gm, double delta_time);
	void simulate_escort(class game& gm, double delta_time);
	void fire_shell_at(const vector2& pos);
	virtual void display(void) const;
	ai* get_ai(void) { return myai; }
};

#endif
