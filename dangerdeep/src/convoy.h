// convoys
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef CONVOY_H
#define CONVOY_H

#include "ship.h"
#include "global_data.h"
#include "ai.h"
#include "parser.h"

class convoy
{
protected:
	ai* myai;
	list<ship*> merchants, warships, escorts;

	convoy();
	convoy(const convoy& other);
	convoy& operator= (const convoy& other);

public:
	enum types { small, medium, large, battleship, supportgroup, carrier };
	virtual ~convoy() {}
	static convoy* create(types type_);
	static convoy* create(parser& p);
//	virtual void simulate(class game& gm, double delta_time);
//	virtual void display(void) const = 0;

	ai* get_ai(void) { return myai; }
};

#endif
