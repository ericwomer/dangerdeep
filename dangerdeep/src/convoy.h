// convoys
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef CONVOY_H
#define CONVOY_H

#include "ship.h"
#include "global_data.h"

class convoy : public sea_object
{
protected:
	friend class game; // for initialization	

	class ai* myai;

	list<pair<ship*, vector2> > merchants, warships, escorts;//fixme: make ref's here
	list<vector2> waypoints;

	convoy();
	convoy(const convoy& other);
	convoy& operator= (const convoy& other);
	
	double remaining_time;	// time to next thought/situation analysis, fixme move to ai!

public:
	enum types { small, medium, large, battleship, supportgroup, carrier };
	enum esctypes { etnone, etsmall, etmedium, etlarge };	// escort size

	convoy(class game& gm, types type_, esctypes esct_);	// create custom convoy
	convoy(class game& gm, class TiXmlElement* parent);	// create convoy from mission xml file
	virtual ~convoy();
	void load(istream& in, class game& g);
	void save(ostream& out, const class game& g) const;
	
	unsigned get_nr_of_ships(void) const;
	virtual bool is_defunct(void) const;

	virtual class ai* get_ai(void) { return myai; }
	virtual void simulate(class game& gm, double delta_time);
	virtual void display(void) const {}
	virtual void add_contact(const vector3& pos);
};

#endif
