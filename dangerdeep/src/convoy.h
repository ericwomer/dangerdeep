// convoys
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef CONVOY_H
#define CONVOY_H

#include "ship.h"
#include "global_data.h"
#include "parser.h"

class convoy : public sea_object
{
protected:
	friend class game; // for initialization	

	class ai* myai;

	list<pair<ship*, vector2> > merchants, warships, escorts;
	list<vector2> waypoints;

	convoy(const convoy& other);
	convoy& operator= (const convoy& other);
	
	double remaining_time;	// time to next thought/situation analysis, fixme move to ai!

public:
	enum types { small, medium, large, battleship, supportgroup, carrier };
	enum esctypes { etnone, etsmall, etmedium, etlarge };	// escort size

	convoy();
	virtual ~convoy();
	void load(istream& in, class game& g);
	void save(ostream& out, const class game& g) const;
	convoy(class game& gm, types type_, esctypes esct_);
	convoy(class game& gm, parser& p);
	virtual class ai* get_ai(void) { return myai; }
	virtual void simulate(class game& gm, double delta_time);
	virtual void display(void) const {}
	virtual void add_contact(const vector3& pos);
};

#endif
