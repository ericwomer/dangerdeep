// convoys
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef CONVOY_H
#define CONVOY_H

#include "ship.h"
#include "global_data.h"
#include "ai.h"
#include "parser.h"

class convoy	// fixme: derive from sea_object
{
protected:
	ai* myai;
	list<pair<ship*, vector2> > merchants, warships, escorts;
	vector2 position;

	convoy();
	convoy(const convoy& other);
	convoy& operator= (const convoy& other);

public:
	enum types { small, medium, large, battleship, supportgroup, carrier };
	virtual ~convoy() {}
	convoy(class game& gm, types type_);	// fixme implement
	convoy(class game& gm, parser& p);
	virtual void simulate(class game& gm, double delta_time);
	virtual bool is_defunct(void) const { return merchants.size() + warships.size() + escorts.size() == 0; }

	vector2 get_pos(void) const { return position; }
	ai* get_ai(void) { return myai; }
};

#endif
