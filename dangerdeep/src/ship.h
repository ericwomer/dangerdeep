// ships
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef SHIP_H
#define SHIP_H

#include "sea_object.h"
#include "global_data.h"
#include "parser.h"

class ship : public sea_object
{
	friend class convoy;
	friend class game;	// for initialization

protected:
	class ai* myai;
	unsigned tonnage;	// in BRT

	// fixme: replace by finer model: -> damage editor!
	damage_status stern_damage, midship_damage, bow_damage;

	// Fuel percentage: 0 = empty, 1 = full.
	double fuel_level;
	double fuel_value_a;
	double fuel_value_t;

	unsigned shipclass;

	ship();
	ship(const ship& other);
	ship& operator= (const ship& other);

	bool parse_attribute(parser& p);	// returns false if invalid token found
	/**
		This method calculates the hourly fuel consumption. An
		exponential is used as a model basing on some fuel consumption values.
		@return double hourly percentage fuel consumption value
	*/
	virtual double get_fuel_consumption_rate () const
	{ return fuel_value_a * ( exp ( get_throttle_speed () / fuel_value_t ) - 1.0f ); }

	/**
		This method is called by the simulate method to recalculate the
		actual fuel status.
		@param delta_time time advance since last call
	*/
	virtual void calculate_fuel_factor ( double delta_time );
	
	class smoke_stream* mysmoke;
	
public:
	enum shipclasses {
		WARSHIP,
		ESCORT,
		MERCHANT,
		SUBMARINE
	};
	
	// create empty object from specification xml file
	ship(const string& specfilename_);
	
	// create ship from parser input (mission file), will read type from parser input
	ship(parser& p);

	virtual ~ship();

	virtual void load(istream& in, class game& g);
	virtual void save(ostream& out, const class game& g) const;

	virtual unsigned get_class(void) const { return shipclass; }

	virtual void simulate(class game& gm, double delta_time);

	virtual void sink(void);

	// command interface
	virtual void fire_shell_at(const vector2& pos);	// to subclass?

	virtual bool has_smoke(void) const { return mysmoke != 0; }
	virtual void smoke_display(double degr) const;

	virtual bool damage(const vector3& fromwhere, unsigned strength);
	virtual unsigned calc_damage(void) const;	// returns damage in percent (100 means dead)
	virtual class ai* get_ai(void) { return myai; }
	// this depends on ship's tonnage, type, draught and depth (subs/sinking ships)
	virtual double get_roll_factor(void) const;
	virtual unsigned get_tonnage(void) const { return tonnage; }
	virtual double get_fuel_level () const { return fuel_level; }
};

#endif
