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
		if (p.type() == TKN_POSITION) {
			p.consume();
			p.parse(TKN_ASSIGN);
			int x = p.parse_number();
			p.parse(TKN_COMMA);
			int y = p.parse_number();
			p.parse(TKN_SEMICOLON);
			position = vector2(x, y);
		} else if (p.type() == TKN_SHIP) {
			ship* shp = ship::create(p);
			gm.spawn_ship(shp);
			pair<ship*, vector2> sp = make_pair(shp, vector2(0, 0));
			if (shp->is_merchant())		// one of these must be true
				merchants.push_back(sp);
			else if (shp->is_warship())
				warships.push_back(sp);
			else if (shp->is_escort())
				escorts.push_back(sp);
		} else {
			p.error("Expected definition");
		}
	}
	p.consume();
	myai = 0; // new ai(0, ai::convoy); // fixme

	// calculate position
	for (list<pair<ship*, vector2> >::iterator it = merchants.begin(); it != merchants.end(); ++it) {
		it->second = it->first->get_pos().xy();
		vector2 p = position + it->second;
		it->first->position = vector3(p.x, p.y, 0);
	}
	for (list<pair<ship*, vector2> >::iterator it = warships.begin(); it != warships.end(); ++it) {
		it->second = it->first->get_pos().xy();
		vector2 p = position + it->second;
		it->first->position = vector3(p.x, p.y, 0);
	}
	for (list<pair<ship*, vector2> >::iterator it = escorts.begin(); it != escorts.end(); ++it) {
		it->second = it->first->get_pos().xy();
		vector2 p = position + it->second;
		it->first->position = vector3(p.x, p.y, 0);
	}
}

void convoy::simulate(game& gm, double delta_time)
{
//	myai->act(gm, delta_time);
}
