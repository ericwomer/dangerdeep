// convoys
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "convoy.h"
#include "game.h"
#include "tokencodes.h"
#include "binstream.h"
#include "ai.h"
#include "system.h"
#include "tinyxml/tinyxml.h"



// empty c'tor needed for loading games
convoy::convoy() : myai(0)
{
}



convoy::~convoy()
{
	delete myai;
}



convoy::convoy(class game& gm, convoy::types type_, convoy::esctypes esct_) : sea_object(), myai(0)
{
	myai = new ai(this, ai::convoy);

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
				//fixme!!! replace by parsing ship dir for available types or hardcode them!!!
				string shiptype = "merchant_medium";
				if (d < 0.2) {
					unsigned r = rnd(2);
					if (r == 0) shiptype = "troopship_medium";
					if (r == 1) shiptype = "tanker_small";
				} else {
					unsigned r = rnd(5);
					if (r == 0) shiptype = "merchant_large";
					if (r == 1) shiptype = "merchant_medium";
					if (r == 2) shiptype = "merchant_small";
					if (r == 3) shiptype = "freighter_large";
					if (r == 4) shiptype = "freighter_medium";
				}
				TiXmlDocument doc(get_ship_dir() + shiptype + ".xml");
				doc.LoadFile();
				ship* s = new ship(&doc);
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
			string shiptype = (esctp == 0 ? "destroyer_tribal" : "corvette");
			TiXmlDocument doc(get_ship_dir() + shiptype + ".xml");
			doc.LoadFile();
			ship* s = new ship(&doc);
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



convoy::convoy(class game& gm, TiXmlElement* parent) : sea_object()
{
	sea_object::parse_attributes(parent);

	// set values
	myai = new ai(this, ai::convoy);
	acceleration = 0.1;	// do we need accel and turn_Rate now?
	turn_rate = 0.05;
	if (speed < 0) speed = (throttle >= 0) ? kts2ms(throttle) : 0;
	max_speed = speed;
	head_to = heading;

	TiXmlElement* epath = parent->FirstChildElement("path");
	if (epath) {
		TiXmlElement* ewaypoint = epath->FirstChildElement("waypoint");
		for ( ; ewaypoint != 0; ewaypoint = ewaypoint->NextSiblingElement("waypoint")) {
			vector2 wp;
			ewaypoint->Attribute("x", &wp.x);
			ewaypoint->Attribute("y", &wp.x);
			waypoints.push_back(wp);
		}
	}
	
	TiXmlElement* eship = parent->FirstChildElement("ship");
	for ( ; eship != 0; eship = eship->NextSiblingElement("ship")) {
		string shiptype = XmlAttrib(eship, "type");
		TiXmlDocument doc(get_ship_dir() + shiptype + ".xml");
		doc.LoadFile();
		ship* shp = new ship(&doc);
		gm.spawn_ship(shp);
		shp->parse_attributes(eship);
		vector2 relpos = shp->position.xy();
		shp->position += position;
		shp->heading = shp->head_to = heading;
		shp->speed = speed;
		shp->throttle = throttle;
		for (list<vector2>::iterator it = waypoints.begin(); it != waypoints.end(); ++it)
			shp->get_ai()->add_waypoint(*it + relpos);

		pair<ship*, vector2> sp = make_pair(shp, relpos);
		if (shp->get_class() == ship::MERCHANT)	// one of these must be true
			merchants.push_back(sp);
		else if (shp->get_class() == ship::WARSHIP)
			warships.push_back(sp);
		else if (shp->get_class() == ship::ESCORT)
			escorts.push_back(sp);
	}
}



void convoy::load(istream& in, class game& g)
{
	sea_object::load(in, g);

	if (read_bool(in))
		myai = new ai(in, g);	

	merchants.clear();
	for (unsigned s = read_u8(in); s > 0; --s) {
		ship* sh = g.read_ship(in);
		double x = read_double(in);
		double y = read_double(in);
		merchants.push_back(make_pair(sh, vector2(x, y)));
	}
	warships.clear();
	for (unsigned s = read_u8(in); s > 0; --s) {
		ship* sh = g.read_ship(in);
		double x = read_double(in);
		double y = read_double(in);
		warships.push_back(make_pair(sh, vector2(x, y)));
	}
	escorts.clear();
	for (unsigned s = read_u8(in); s > 0; --s) {
		ship* sh = g.read_ship(in);
		double x = read_double(in);
		double y = read_double(in);
		escorts.push_back(make_pair(sh, vector2(x, y)));
	}
	waypoints.clear();
	for (unsigned s = read_u16(in); s > 0; --s) {
		double x = read_double(in);
		double y = read_double(in);
		waypoints.push_back(vector2(x, y));
	}
}



void convoy::save(ostream& out, const class game& g) const
{
	sea_object::save(out, g);

	write_bool(out, (myai != 0));
	if (myai)
		myai->save(out, g);

	write_u8(out, merchants.size());
	for (list<pair<ship*, vector2> >::const_iterator it = merchants.begin(); it != merchants.end(); ++it) {
		g.write(out, it->first);
		write_double(out, it->second.x);
		write_double(out, it->second.y);
	}
	write_u8(out, warships.size());
	for (list<pair<ship*, vector2> >::const_iterator it = warships.begin(); it != warships.end(); ++it) {
		g.write(out, it->first);
		write_double(out, it->second.x);
		write_double(out, it->second.y);
	}
	write_u8(out, escorts.size());
	for (list<pair<ship*, vector2> >::const_iterator it = escorts.begin(); it != escorts.end(); ++it) {
		g.write(out, it->first);
		write_double(out, it->second.x);
		write_double(out, it->second.y);
	}
	write_u16(out, waypoints.size());
	for (list<vector2>::const_iterator it = waypoints.begin(); it != waypoints.end(); ++it) {
		write_double(out, it->x);
		write_double(out, it->y);
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

	if ( myai )
		myai->act(gm, delta_time);

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
