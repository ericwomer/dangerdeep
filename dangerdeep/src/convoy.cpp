// convoys
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "convoy.h"
#include "game.h"
#include "tokencodes.h"

convoy::convoy(class game& gm, convoy::types type_)
{
	// fixme
	switch (type_) {
		case small: break;
		case medium: break;
		case large: break;
		case battleship: break;
		case supportgroup: break;
		case carrier: break;
	}
	myai = 0;
}

convoy::convoy(class game& gm, parser& p)
{
	p.parse(TKN_CONVOY);
	p.parse(TKN_SLPARAN);
	while (p.type() != TKN_SRPARAN) {
		if (p.type() != TKN_SHIP)
			p.error("Expected ship definition");	// fixme airplanes!
		ship* shp = ship::create(p);
		gm.spawn_ship(shp);
		if (shp->is_merchant())		// one of these must be true
			merchants.push_back(shp);
		else if (shp->is_warship())
			warships.push_back(shp);
		else if (shp->is_escort())
			escorts.push_back(shp);
	}
	p.consume();
	myai = 0; // new ai(0, ai::convoy); // fixme

	// calculate position
	position = vector2(0, 0);
	unsigned nr_ships = 0;
	for (list<ship*>::iterator it = merchants.begin(); it != merchants.end(); ++it) {
		position += (*it)->get_pos().xy();
		++nr_ships;
	}
	for (list<ship*>::iterator it = warships.begin(); it != warships.end(); ++it) {
		position += (*it)->get_pos().xy();
		++nr_ships;
	}
	for (list<ship*>::iterator it = escorts.begin(); it != escorts.end(); ++it) {
		position += (*it)->get_pos().xy();
		++nr_ships;
	}
	position = position * (1.0/nr_ships);
}

void convoy::simulate(game& gm, double delta_time)
{
//	myai->act(gm, delta_time);
}
