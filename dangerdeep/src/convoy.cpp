// convoys
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "convoy.h"
#include "game.h"
#include "tokencodes.h"

convoy::convoy(class game& gm, convoy::types type_) : sea_object()
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
	vis_cross_section_factor = CROSS_SECTION_VIS_CONVOY;
}

convoy::convoy(class game& gm, parser& p) : sea_object()
{
	remaining_time = rnd() * AI_THINK_CYCLE_TIME;
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
			position = vector3(x, y, 0);
		} else if (p.type() == TKN_SPEED) {
			p.consume();
			p.parse(TKN_ASSIGN);
			max_speed = kts2ms(p.parse_number());
			p.parse(TKN_SEMICOLON);
		} else if (p.type() == TKN_HEADING) {
			p.consume();
			p.parse(TKN_ASSIGN);
			heading = angle(p.parse_number());
			p.parse(TKN_SEMICOLON);
		} else if (p.type() == TKN_WAYPOINT) {
			p.consume();
			p.parse(TKN_ASSIGN);
			int x = p.parse_number();
			p.parse(TKN_COMMA);
			int y = p.parse_number();
			p.parse(TKN_SEMICOLON);
			waypoints.push_back(vector2(x, y));
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

	// set values
	acceleration = 0.1;
	turn_rate = 0.05;
	set_throttle(aheadflank);
	speed = get_throttle_speed();
	head_to = heading;

	// calculate positions and speeds of convoy's ships.
	// set their waypoints from convoy's waypoints.
	for (list<pair<ship*, vector2> >::iterator it = merchants.begin(); it != merchants.end(); ++it) {
		it->second = it->first->get_pos().xy();
		it->first->position += position;
		it->first->speed = speed;
		for (list<vector2>::iterator it2 = waypoints.begin(); it2 != waypoints.end(); ++it2) {
			it->first->get_ai()->add_waypoint(*it2 + it->second);
		}
	}
	for (list<pair<ship*, vector2> >::iterator it = warships.begin(); it != warships.end(); ++it) {
		it->second = it->first->get_pos().xy();
		it->first->position += position;
		it->first->speed = speed;
		for (list<vector2>::iterator it2 = waypoints.begin(); it2 != waypoints.end(); ++it2) {
			it->first->get_ai()->add_waypoint(*it2 + it->second);
		}
	}
	for (list<pair<ship*, vector2> >::iterator it = escorts.begin(); it != escorts.end(); ++it) {
		it->second = it->first->get_pos().xy();
		it->first->position += position;
		it->first->speed = speed;
		for (list<vector2>::iterator it2 = waypoints.begin(); it2 != waypoints.end(); ++it2) {
			it->first->get_ai()->add_waypoint(*it2 + it->second);
		}
	}
	
}

void convoy::simulate(game& gm, double delta_time)
{
	sea_object::simulate(gm, delta_time);

	// check for ships to be erased
	for (list<pair<ship*, vector2> >::iterator it = merchants.begin(); it != merchants.end(); ) {
		list<pair<ship*, vector2> >::iterator it2 = it; ++it;
		if (it2->first->is_defunct())
			merchants.erase(it2);
	}
	for (list<pair<ship*, vector2> >::iterator it = warships.begin(); it != warships.end(); ) {
		list<pair<ship*, vector2> >::iterator it2 = it; ++it;
		if (it2->first->is_defunct())
			warships.erase(it2);
	}
	for (list<pair<ship*, vector2> >::iterator it = escorts.begin(); it != escorts.end(); ) {
		list<pair<ship*, vector2> >::iterator it2 = it; ++it;
		if (it2->first->is_defunct())
			escorts.erase(it2);
	}

	// ai behaviour
	// ai action in time intervals
	remaining_time -= delta_time;
	if (remaining_time > 0) {
		return;
	} else {
		remaining_time = AI_THINK_CYCLE_TIME;
	}

	// follow waypoints
	if (waypoints.size() > 0) {
		set_course_to_pos(waypoints.front());
		if (get_pos().xy().distance(waypoints.front()) < WPEXACTNESS) {
/*
			if (cyclewaypoints)
				waypoints.push_back(waypoints.front());
*/				
			waypoints.erase(waypoints.begin());
		}
	}

	// set actions for convoy's ships.
	// civil ships continue their course with zigzags eventually
//fixme: the ships don't follow their waypoint exactly, they're zigzagging wild around it
//if i use set_course_to_pos direct, everything is fine. maybe the ai of each ship
//must "think" shortly after setting the waypoint
//fixme: don't set the immidiate next wp, just use the next convoy wp + rel. position as waypoint!
//or set all wps at the beginning. fixme is this really a good idea?
//this could be done in the constructor!
	for (list<pair<ship*, vector2> >::iterator it = merchants.begin(); it != merchants.end(); ++it) {
//		it->first->get_ai()->set_waypoint(position.xy() + it->second);
	}

	// war ships follow their course, with zigzags / evasive manouvers / increasing speed
	for (list<pair<ship*, vector2> >::iterator it = warships.begin(); it != warships.end(); ++it) {
//		it->first->get_ai()->set_waypoint(position.xy() + it->second);
	}
	
	// escorts follow their escort pattern or attack if alarmed
	for (list<pair<ship*, vector2> >::iterator it = escorts.begin(); it != escorts.end(); ++it) {
//		it->first->get_ai()->set_waypoint(position.xy() + it->second);
	}

	// convoy erased?
	if (merchants.size() + warships.size() + escorts.size() == 0)
		alive_stat = defunct;
}

void convoy::add_contact(const vector3& pos)	// fixme: simple, crude, ugly
{
	for (list<pair<ship*, vector2> >::iterator it = escorts.begin(); it != escorts.end(); ++it) {
		it->first->get_ai()->attack_contact(pos);
	}
}
