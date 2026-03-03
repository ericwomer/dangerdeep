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

// Ping manager - manages active sonar pings
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "ping_manager.h"
#include "xml.h"

ping::ping(const xml_elem &parent) {
    pos.x = parent.attrf("posx");
    pos.y = parent.attrf("posy");
    dir = angle(parent.attrf("dir"));
    time = parent.attrf("time");
    range = parent.attrf("range");
    ping_angle = angle(parent.attrf("ping_angle"));
}

void ping::save(xml_elem &parent) const {
    parent.set_attr(pos.x, "posx");
    parent.set_attr(pos.y, "posy");
    parent.set_attr(dir.value(), "dir");
    parent.set_attr(time, "time");
    parent.set_attr(range, "range");
    parent.set_attr(ping_angle.value(), "ping_angle");
}

ping_manager::ping_manager() : ping_remain_time(1.0) {
}

void ping_manager::add_ping(const vector2 &pos, angle dir, double time, double range, const angle &ping_angle) {
    pings.push_back(ping(pos, dir, time, range, ping_angle));
}

void ping_manager::update(double current_time) {
    for (std::list<ping>::iterator it = pings.begin(); it != pings.end();) {
        std::list<ping>::iterator it2 = it++;
        if (current_time - it2->time > ping_remain_time) {
            pings.erase(it2);
        }
    }
}

void ping_manager::load(const xml_elem &parent) {
    pings.clear();
    xml_elem pngs = parent.child("pings");
    for (xml_elem::iterator it = pngs.iterate("ping"); !it.end(); it.next()) {
        pings.push_back(ping(it.elem()));
    }
}

void ping_manager::save(xml_elem &parent) const {
    xml_elem pngs = parent.add_child("pings");
    pngs.set_attr(unsigned(pings.size()), "nr");
    for (std::list<ping>::const_iterator it = pings.begin(); it != pings.end(); ++it) {
        xml_elem png = pngs.add_child("ping");
        it->save(png);
    }
}
