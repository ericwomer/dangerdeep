// convoys
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "convoy.h"
#include "game.h"
#include "tokencodes.h"

convoy::convoy(class game& gm, convoy::types type_, convoy::esctypes esct_) : sea_object()
{
	vis_cross_section_factor = CROSS_SECTION_VIS_CONVOY;

	waypoints.push_back(vector2(0, 0));
	for (int wp = 0; wp < 4; ++wp)
		waypoints.push_back(vector2(rnd()*300000-150000,rnd()*300000-150000));
	heading = angle(*(++waypoints.begin()) - *(waypoints.begin()));
	vector2 coursevec = heading.direction();
	
	double intershipdist = 1000.0;	// distance between ships.
	double convoyescortdist = 3000.0; // distance of escorts to convoy
	double interescortdist = 1500.0;	// distance between escorts
	
	// merchant convoy?
	if (type_ == small || type_ == medium || type_ == large) {
		unsigned cvsize = unsigned(type_);
		
		// speed? could be a slow or fast convoy (~4 or ~8 kts).
		throttle = 4 + rnd(2)*4;
		max_speed = speed = kts2ms(throttle);
	
		// compute size and structure of convoy
		unsigned nrships = (2<<cvsize)*10+rnd(10)-5;
		unsigned sqrtnrships = unsigned(floor(sqrt(float(nrships))));
		unsigned shps = 0;
		for (unsigned j = 0; j <= sqrtnrships; ++j) {
			if (shps >= nrships) break;
			int dy = int(j)-sqrtnrships/2;
			for (unsigned i = 0; i <= sqrtnrships; ++i) {
				if (shps >= nrships) break;
				int dx = int(i)-sqrtnrships/2;
				float d = 4*float(dx*dx+dy*dy)/nrships;
				ship::types shiptype = ship::mediummerchant;
				if (d < 0.2) {
					shiptype = ship::mediumtroopship;
				} else {
					unsigned r = rnd(3);
					if (r == 0) shiptype = ship::mediummerchant;
					if (r == 1) shiptype = ship::largefreighter;
					if (r == 2) shiptype = ship::mediumfreighter;
				}
				ship* s = ship::create(shiptype);
				vector2 pos = vector2(
					dx*intershipdist + rnd()*60.0-30.0,
					dy*intershipdist + rnd()*60.0-30.0 );
				pos = pos.matrixmul(coursevec, coursevec.orthogonal());
				s->position.x = waypoints.begin()->x + pos.x;
				s->position.y = waypoints.begin()->y + pos.y;
				s->heading = s->head_to = heading;
				s->speed = speed;
				s->throttle = throttle;
				merchants.push_back(make_pair(s, pos));
				++shps;
			}
		}
		
		// compute nr of escorts
		unsigned nrescs = esct_ * 5;
		for (unsigned i = 0; i < nrescs; ++i) {
			//fixme: give commands/tasks to escorts: "patrol left side of convoy" etc.
			int side = i % 4;
			float dx = 0, dy = 0, nx = 0, ny = 0;
			switch (side) {
				case 0: dx = 0; dy = 1; break;
				case 1: dx = 1; dy = 0; break;
				case 2: dx = 0; dy = -1; break;
				case 3: dx = -1; dy = 0; break;
			}
			nx = -dy; ny = dx;
			dx *= sqrtnrships/2 * intershipdist + convoyescortdist;
			dy *= sqrtnrships/2 * intershipdist + convoyescortdist;
			nx *= (int(nrescs/4)-1)*interescortdist-int(i/4)*interescortdist;
			ny *= (int(nrescs/4)-1)*interescortdist-int(i/4)*interescortdist;
			unsigned esctp = rnd(2);
			ship *s = ship::create(esctp == 0 ? ship::destroyertribal : ship::corvette);
			vector2 pos = vector2(
				dx+nx + rnd()*100.0-50.0,
				dy+ny + rnd()*100.0-50.0 );
			pos = pos.matrixmul(coursevec, coursevec.orthogonal());
			s->position.x = waypoints.begin()->x + pos.x;
			s->position.y = waypoints.begin()->y + pos.y;
			s->heading = s->head_to = heading;
			s->speed = speed;
			s->throttle = throttle;
			escorts.push_back(make_pair(s, pos));
		}
			
		return;
	}
	
	// fixme
	switch (type_) {
		case small: break;
		case medium: break;
		case large: break;
		case battleship: break;
		case supportgroup: break;
		case carrier: break;
	}
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
