// convoys
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "convoy.h"
#include "game.h"
#include "tokencodes.h"
#include "binstream.h"
#include "ai.h"
#include "system.h"


// fixme: the whole file is covered with outcommented lines that have to be
// implemented in a sane way with the new class model (2004/07/15)


convoy::convoy(game& gm_)
	: sea_object(gm_, ""), myai(0)
{
}



convoy::~convoy()
{
}



convoy::convoy(class game& gm_, convoy::types type_, convoy::esctypes esct_) : sea_object(gm_), myai(0)
{
	//myai = new ai(this, ai::convoy);

	waypoints.push_back(vector2(0, 0));
	for (int wp = 0; wp < 4; ++wp)
		waypoints.push_back(vector2(rnd()*300000-150000,rnd()*300000-150000));
	angle heading = angle(*(++waypoints.begin()) - *(waypoints.begin()));
	vector2 coursevec = heading.direction();
	//position = vector2(0, 0);

	double intershipdist = 1000.0;	// distance between ships. it is 1000m sidewards and 600m forward... fixme
	double convoyescortdist = 3000.0; // distance of escorts to convoy
	double interescortdist = 1500.0;	// distance between escorts, seems a bit low...
	
	// merchant convoy?
	if (type_ == small || type_ == medium || type_ == large) {
		unsigned cvsize = unsigned(type_);
		
		// speed? could be a slow or fast convoy (~4 or ~8 kts).
		int throttle = 4 + rnd(2)*4;
		velocity = heading.direction() * kts2ms(throttle);
	
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
				ship* s = new ship(gm, &doc);
				vector2 pos = vector2(
					dx*intershipdist + rnd()*60.0-30.0,
					dy*intershipdist + rnd()*60.0-30.0 );
				pos = pos.matrixmul(coursevec, coursevec.orthogonal());
				s->position.x = waypoints.begin()->x + pos.x;
				s->position.y = waypoints.begin()->y + pos.y;
				s->heading = s->head_to = heading;
				s->velocity = velocity;
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
			ship* s = new ship(gm, &doc);
			vector2 pos = vector2(
				dx+nx + rnd()*100.0-50.0,
				dy+ny + rnd()*100.0-50.0 );
			pos = pos.matrixmul(coursevec, coursevec.orthogonal());
			s->position.x = waypoints.begin()->x + pos.x;
			s->position.y = waypoints.begin()->y + pos.y;
			s->heading = s->head_to = heading;
			s->velocity = velocity;
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



convoy::convoy(class game& gm_, const xml_elem& parent)
	: sea_object(gm_, "")
convoy::convoy(class game& gm_, TiXmlElement* parent) : sea_object(gm_), myai(0)
{
	sea_object::parse_attributes(parent);

	// set values
//	myai = new ai(this, ai::convoy);
//	acceleration = 0.1;	// do we need accel and turn_Rate now?
//	turn_rate = 0.05;
//	if (speed < 0) speed = (throttle >= 0) ? kts2ms(throttle) : 0;
//	max_speed = speed;
//	head_to = heading;

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
		ship* shp = new ship(gm, &doc);
		gm.spawn_ship(shp);
		shp->parse_attributes(eship);
		vector2 relpos = shp->position.xy();
		shp->position += position;
		shp->heading = shp->head_to = heading;
		shp->velocity = velocity;
		shp->throttle = get_speed();//throttle;//fixme - a convoy has no throttle...
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



void convoy::load(const xml_elem& parent)
{
	sea_object::load(parent);
	xml_elem mc = parent.child("merchants");
	merchants.clear();
	merchants.reserve(mc.attru("nr"));
	for (xml_elem::iterator it = mc.iterate("merchant"); !it.end(); it.next()) {
		merchants.push_back(make_pair(gm.load_ship_ptr(it.elem().attru("ref")),
					      vector2(it.elem().attrf("posx"),
						      it.elem().attrf("posy"))));
	}
	xml_elem ws = parent.child("warships");
	warships.clear();
	warships.reserve(ws.attru("nr"));
	for (xml_elem::iterator it = ws.iterate("warship"); !it.end(); it.next()) {
		warships.push_back(make_pair(gm.load_ship_ptr(it.elem().attru("ref")),
					      vector2(it.elem().attrf("posx"),
						      it.elem().attrf("posy"))));
	}
	xml_elem es = parent.child("escorts");
	escorts.clear();
	escorts.reserve(es.attru("nr"));
	for (xml_elem::iterator it = es.iterate("escort"); !it.end(); it.next()) {
		escorts.push_back(make_pair(gm.load_ship_ptr(it.elem().attru("ref")),
					      vector2(it.elem().attrf("posx"),
						      it.elem().attrf("posy"))));
	}
	xml_elem wp = parent.child("waypoints");
	waypoints.clear();
	waypoints.reserve(wp.attru("nr"));
	for (xml_elem::iterator it = wp.iterate("waypoint"); !it.end(); it.next()) {
		waypoints.push_back(vector2(it.elem().attrf("x"),
					    it.elem().attrf("y")));
	}
}



void convoy::save(xml_elem& parent) const
{
	sea_object::save(parent);
	xml_elem mc = parent.add_child("merchants");
	mc.set_attr(merchants.size(), "nr");
	for (list<pair<ship*, vector2> >::const_iterator it = merchants.begin(); it != merchants.end(); ++it) {
		xml_elem mc2 = mc.add_child("merchant");
		mc2.set_attr(gm.save_ptr(it->first), "ref");
		mc2.set_attr(it->second.x, "posx");
		mc2.set_attr(it->second.x, "posy");
	}
	xml_elem ws = parent.add_child("warships");
	ws.set_attr(warships.size(), "nr");
	for (list<pair<ship*, vector2> >::const_iterator it = warships.begin(); it != warships.end(); ++it) {
		xml_elem ws2 = mc.add_child("warship");
		ws2.set_attr(gm.save_ptr(it->first), "ref");
		ws2.set_attr(it->second.x, "x");
		ws2.set_attr(it->second.y, "y");
	}
	xml_elem es = parent.add_child("escorts");
	es.set_attr(escorts.size(), "nr");
	for (list<pair<ship*, vector2> >::const_iterator it = escorts.begin(); it != escorts.end(); ++it) {
		xml_elem es2 = mc.add_child("escort");
		es2.set_attr(gm.save_ptr(it->first), "ref");
		es2.set_attr(it->second.x, "x");
		es2.set_attr(it->second.y, "y");
	}
	xml_elem wp = parent.add_child("waypoints");
	wp.set_attr(waypoints.size(), "nr");
	for (list<vector2>::const_iterator it = waypoints.begin(); it != waypoints.end(); ++it) {
		xml_elem wp2 = mc.add_child("pos");
		wp2.set_attr(it->x, "x");
		wp2.set_attr(it->y, "y");
	}
}



unsigned convoy::get_nr_of_ships(void) const
{
	return merchants.size() + warships.size() + escorts.size();
}



void convoy::simulate(double delta_time)
{
	sea_object::simulate(delta_time);//remove, replace by position update in ai-class
	if (is_defunct()) return;

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

//	if ( myai )//fixme: my is ai sometimes != 0 although it is disabled?!
//		myai->act(gm, delta_time);

	// convoy erased?
	if (merchants.size() + warships.size() + escorts.size() == 0)
		destroy();
}



void convoy::add_contact(const vector3& pos)	// fixme: simple, crude, ugly
{
	for (list<pair<ship*, vector2> >::iterator it = escorts.begin(); it != escorts.end(); ++it) {
		it->first->get_ai()->attack_contact(pos);
	}
}
