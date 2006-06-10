/*
Danger from the Deep - Open source submarine simulation
Copyright (C) 2003-2006  Thorsten Jordan, Luis Barrancos and others.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

// convoys
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "convoy.h"
#include "game.h"
#include "tokencodes.h"
#include "ai.h"
#include "system.h"
#include "ship.h"
#include "datadirs.h"
#include "global_data.h"


// fixme: the whole file is covered with outcommented lines that have to be
// implemented in a sane way with the new class model (2004/07/15)


convoy::convoy(game& gm_)
	: gm(gm_), remaining_time(0)
{
}



convoy::convoy(game& gm_, convoy::types type_, convoy::esctypes esct_)
	: gm(gm_), remaining_time(0)
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
		name = "Custom"; // fixme...
		unsigned cvsize = unsigned(type_);
		
		// speed? could be a slow or fast convoy (~4 or ~8 kts).
		int throttle = 4 + rnd(2)*4;
		velocity = sea_object::kts2ms(throttle);
	
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
				//each ship in data/ships stores its type.
				//but probability can not be stored...
				string shiptype = "merchant_medium";
				if (d < 0.2) {
					unsigned r = rnd(4);
					if (r == 0) shiptype = "troopship_medium";
					if (r == 1) shiptype = "tanker_small";
					if (r >= 2 && r <= 3) shiptype = "tanker_medium";
				} else {
					unsigned r = rnd(11);
					if (r == 0) shiptype = "merchant_large";
					if (r == 1) shiptype = "merchant_medium";
					if (r == 2) shiptype = "merchant_small";
					if (r == 3) shiptype = "freighter_large";
					if (r == 4) shiptype = "freighter_medium";
                    if (r == 5) shiptype = "libertyship_1941_data";
                    if (r == 6) shiptype = "libertyship_1942_data";
                    if (r == 7) shiptype = "libertyship_1943_data";
                    if (r == 8) shiptype = "fortship_can1941_data";
                    if (r == 9) shiptype = "fortship_can1943_data";
					if (r == 10) shiptype = "fortship_nolisciv_data";
				}
				xml_doc doc(get_ship_dir() + shiptype + ".xml");
				doc.load();
				ship* s = new ship(gm, doc.first_child());
				vector2 pos = vector2(
					dx*intershipdist + rnd()*60.0-30.0,
					dy*intershipdist + rnd()*60.0-30.0 );
				pos = pos.matrixmul(coursevec, coursevec.orthogonal());
				s->manipulate_position((waypoints.front() + pos).xy0());
				s->manipulate_heading(heading);
				s->manipulate_speed(velocity);
				s->set_throttle(throttle);
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
			xml_doc doc(get_ship_dir() + shiptype + ".xml");
			doc.load();
			ship* s = new ship(gm, doc.first_child());
			vector2 pos = vector2(
				dx+nx + rnd()*100.0-50.0,
				dy+ny + rnd()*100.0-50.0 );
			pos = pos.matrixmul(coursevec, coursevec.orthogonal());
			s->manipulate_position((waypoints.front() + pos).xy0());
			s->manipulate_heading(heading);
			s->manipulate_speed(velocity);
			s->set_throttle(throttle);
			escorts.push_back(make_pair(s, pos));
		}
			
		// spawn the objects in class game after creating them
		for (list<pair<ship*, vector2> >::iterator it = merchants.begin(); it != merchants.end(); ++it) {
			gm.spawn_ship(it->first);
		}
		for (list<pair<ship*, vector2> >::iterator it = warships.begin(); it != warships.end(); ++it) {
			gm.spawn_ship(it->first);
		}
		for (list<pair<ship*, vector2> >::iterator it = escorts.begin(); it != escorts.end(); ++it) {
			gm.spawn_ship(it->first);
		}

		return;
	}

	// fixme
	name = "SC-122";
	
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



convoy::convoy(class game& gm_, const vector2& pos, const std::string& name_)
	: gm(gm_), position(pos), name(name_)
{
}



bool convoy::add_ship(ship* shp)
{
	vector2 spos = shp->get_pos().xy() - position;
	switch (shp->get_class()) {
	case WARSHIP:
		warships.push_back(make_pair(shp, spos));
		return true;
	case ESCORT:
		escorts.push_back(make_pair(shp, spos));
		return true;
	case MERCHANT:
		merchants.push_back(make_pair(shp, spos));
		return true;
	default:
		// can't add this to convoy
		return false;
	}
}



void convoy::load(const xml_elem& parent)
{
	name = parent.attr("name");
	position = parent.child("position").attrv2();
	velocity = parent.child("velocity").attrf();
	xml_elem mc = parent.child("merchants");
	merchants.clear();
	//merchants.reserve(mc.attru("nr"));
	for (xml_elem::iterator it = mc.iterate("merchant"); !it.end(); it.next()) {
		merchants.push_back(make_pair(gm.load_ship_ptr(it.elem().attru("ref")), it.elem().attrv2()));
	}
	xml_elem ws = parent.child("warships");
	warships.clear();
	//warships.reserve(ws.attru("nr"));
	for (xml_elem::iterator it = ws.iterate("warship"); !it.end(); it.next()) {
		warships.push_back(make_pair(gm.load_ship_ptr(it.elem().attru("ref")), it.elem().attrv2()));
	}
	xml_elem es = parent.child("escorts");
	escorts.clear();
	//escorts.reserve(es.attru("nr"));
	for (xml_elem::iterator it = es.iterate("escort"); !it.end(); it.next()) {
		escorts.push_back(make_pair(gm.load_ship_ptr(it.elem().attru("ref")), it.elem().attrv2()));
	}
	xml_elem wp = parent.child("waypoints");
	waypoints.clear();
	//waypoints.reserve(wp.attru("nr"));
	for (xml_elem::iterator it = wp.iterate("waypoint"); !it.end(); it.next()) {
		waypoints.push_back(it.elem().attrv2());
	}
}



void convoy::save(xml_elem& parent) const
{
	parent.set_attr(name, "name");
	parent.add_child("position").set_attr(position);
	parent.add_child("velocity").set_attr(velocity);
	xml_elem mc = parent.add_child("merchants");
	mc.set_attr(unsigned(merchants.size()), "nr");
	for (list<pair<ship*, vector2> >::const_iterator it = merchants.begin(); it != merchants.end(); ++it) {
		xml_elem mc2 = mc.add_child("merchant");
		mc2.set_attr(gm.save_ptr(it->first), "ref");
		mc2.set_attr(it->second);
	}
	xml_elem ws = parent.add_child("warships");
	ws.set_attr(unsigned(warships.size()), "nr");
	for (list<pair<ship*, vector2> >::const_iterator it = warships.begin(); it != warships.end(); ++it) {
		xml_elem ws2 = ws.add_child("warship");
		ws2.set_attr(gm.save_ptr(it->first), "ref");
		ws2.set_attr(it->second);
	}
	xml_elem es = parent.add_child("escorts");
	es.set_attr(unsigned(escorts.size()), "nr");
	for (list<pair<ship*, vector2> >::const_iterator it = escorts.begin(); it != escorts.end(); ++it) {
		xml_elem es2 = es.add_child("escort");
		es2.set_attr(gm.save_ptr(it->first), "ref");
		es2.set_attr(it->second);
	}
	xml_elem wp = parent.add_child("waypoints");
	wp.set_attr(unsigned(waypoints.size()), "nr");
	for (list<vector2>::const_iterator it = waypoints.begin(); it != waypoints.end(); ++it) {
		wp.add_child("pos").set_attr(*it);
	}
}



unsigned convoy::get_nr_of_ships(void) const
{
	return merchants.size() + warships.size() + escorts.size();
}



void convoy::simulate(double delta_time)
{
	//if (is_defunct()) return;

	//fixme: only control target every n seconds or so.
	// compute global velocity. as first direction to destination.
	// aim to next waypoint if that isn't already reached.
	vector2 dir;
	while (!waypoints.empty()) {
		if (position.square_distance(waypoints.front()) < 10.0) {	// 3.3m are exact enough
			waypoints.pop_front();
		} else {
			dir = (waypoints.front() - position).normal();
			break;
		}
	}
	// note that convoy stops after last waypoints was reached because dir is zero in that case
	vector2 global_velocity = dir * velocity;

	// add better moving simulation? but it should be exact enough for convoys.
	position += global_velocity * delta_time;

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

	//fixme: set target course for ships here, but only every n seconds.
	//note that this must not override the ship's current steering, because it could
	//do an evasive manouver etc.
	//An better alternative would be if ships could request their target position
	//at the convoy and do that every n seconds. The AI of a ship could do so.

//	if ( myai )//fixme: my is ai sometimes != 0 although it is disabled?!
//		myai->act(gm, delta_time);

	// convoy erased?
	if (merchants.size() + warships.size() + escorts.size() == 0) {
		//fixme
		//destroy();
	}
}



void convoy::add_contact(const vector3& pos)	// fixme: simple, crude, ugly
{
	for (list<pair<ship*, vector2> >::iterator it = escorts.begin(); it != escorts.end(); ++it) {
		it->first->get_ai()->attack_contact(pos);
	}
}
