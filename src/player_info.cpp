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

// Player information data structure
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "player_info.h"
#include "global_data.h"
#include "xml.h"
#include <string>

using namespace std;

player_info::player_info()
    : name("Heinz Mustermann"), flotilla(1), submarineid("U 999"), photo("1") {
    // generate a random soldbuch_nr between 1 and 9999
    soldbuch_nr = str(rnd(9999) + 1);

    // generate a random bloodgroup
    const string bloodgroups[] = {"A", "B", "AB", "0"};
    bloodgroup = bloodgroups[rnd(4)];

    // there are just 3 sizes
    gasmask_size = str(rnd(3) + 1);

    /* first part of the marine roll nr is a character that specifies the naval command
     * for submarines that should be the naval command west --> W
     * the second part is a contnious number that is unique for every soldier in the roll (of his flotilla)
     * 20.000 as max value should be high enough
     * third part is unknown so just take the soldbuch nr */
    marine_roll = "W " + str(rnd(20000) + 1) + " / " + soldbuch_nr;
}

player_info::player_info(const xml_elem &parent) {
    name = parent.attr("name");
    photo = parent.attr("photo");
    if (photo.empty())
        photo = "1";
    flotilla = parent.attru("flotilla");
    submarineid = parent.attr("submarineid");
    soldbuch_nr = parent.attr("soldbuch_nr");
    gasmask_size = parent.attr("gasmask_size");
    bloodgroup = parent.attr("bloodgroup");
    marine_roll = parent.attr("marine_roll");
    marine_group = parent.attr("marine_group");
    for (xml_elem::iterator it = parent.iterate("promotions"); !it.end(); it.next()) {
        career.push_back(it.elem().attr("date"));
    }
}

void player_info::save(xml_elem &parent) const {
    parent.set_attr(name, "name");
    parent.set_attr(flotilla, "flottila");
    parent.set_attr(submarineid, "submarineid");
    parent.set_attr(soldbuch_nr, "soldbuch_nr");
    parent.set_attr(gasmask_size, "gasmask_size");
    parent.set_attr(bloodgroup, "bloodgroup");
    parent.set_attr(marine_roll, "marine_roll");
    parent.set_attr(marine_group, "marine_group");
    xml_elem xml_career = parent.add_child("promotions");
    for (list<string>::const_iterator it = career.begin(); it != career.end(); ++it) {
        xml_elem elem = xml_career.add_child("promotion");
        elem.set_attr(*it, "date");
    }
}
