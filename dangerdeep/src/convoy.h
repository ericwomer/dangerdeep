// convoys
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef CONVOY_H
#define CONVOY_H

#include "ship.h"
#include "global_data.h"
#include "ai.h"
#include "parser.h"

class convoy : public sea_object
{
protected:
	list<pair<ship*, vector2> > merchants, warships, escorts;
	list<vector2> waypoints;

	convoy();
	convoy(const convoy& other);
	convoy& operator= (const convoy& other);
	
	double remaining_time;	// time to next thought/situation analysis

public:
	enum types { small, medium, large, battleship, supportgroup, carrier };
	virtual ~convoy() {}
	convoy(class game& gm, types type_);	// fixme implement
	convoy(class game& gm, parser& p);
	virtual void simulate(class game& gm, double delta_time);
	virtual void display(void) const {}
	virtual void add_contact(const vector3& pos);
    virtual convoy* get_convoi_ptr () { return this; }
};

#endif
