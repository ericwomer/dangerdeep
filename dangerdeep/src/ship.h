// ships
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef SHIP_H
#define SHIP_H

#include "sea_object.h"
#include "global_data.h"
#include "ai.h"
#include "parser.h"

class ship : public sea_object
{
	friend class convoy;
	
protected:
	ai* myai;
	unsigned tonnage;	// in BRT

	damage_status stern_damage, midship_damage, bow_damage;
	
	ship();
	ship(const ship& other);
	ship& operator= (const ship& other);

	bool parse_attribute(parser& p);	// returns false if invalid token found
	
public:
	// fixme: add more types
	enum types { mediummerchant, mediumtroopship, destroyertribal, battleshipmalaya };
	virtual ~ship() { delete myai; }
	static ship* create(types type_);
	static ship* create(parser& p);
	virtual void simulate(class game& gm, double delta_time);
	virtual void display(void) const = 0;

	virtual void fire_shell_at(const vector2& pos);	// to subclass?

	virtual void damage(const vector3& fromwhere, unsigned strength);
	virtual unsigned calc_damage(void) const;	// returns damage in percent (0 means dead)
	virtual void sink(void);
	virtual void kill(void);
	ai* get_ai(void) { return myai; }
	unsigned get_tonnage(void) const { return tonnage; }
	
	virtual bool is_merchant(void) const { return false; }
	virtual bool is_escort(void) const { return false; }
	virtual bool is_warship(void) const { return false; }
};

#endif
