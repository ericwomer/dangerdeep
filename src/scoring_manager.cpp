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

// Scoring manager - manages sunken ships records
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "scoring_manager.h"
#include "xml.h"

sink_record::sink_record(const xml_elem &parent) {
    dat.load(parent);
    descr = parent.attr("descr");
    mdlname = parent.attr("mdlname");
    tons = parent.attru("tons");
    specfilename = parent.attr("specfilename");
    layoutname = parent.attr("layoutname");
}

void sink_record::save(xml_elem &parent) const {
    dat.save(parent);
    parent.set_attr(descr, "descr");
    parent.set_attr(mdlname, "mdlname");
    parent.set_attr(tons, "tons");
    parent.set_attr(specfilename, "specfilename");
    parent.set_attr(layoutname, "layoutname");
}

void scoring_manager::record_sunk_ship(date d, const std::string &descr, const std::string &mdlname,
                                       const std::string &specfilename, const std::string &layoutname, unsigned tons) {
    sunken_ships.push_back(sink_record(d, descr, mdlname, specfilename, layoutname, tons));
}

unsigned scoring_manager::total_tonnage() const {
    unsigned total = 0;
    for (const auto &record : sunken_ships) {
        total += record.tons;
    }
    return total;
}

void scoring_manager::load(const xml_elem &parent) {
    sunken_ships.clear();
    xml_elem sks = parent.child("sunken_ships");
    for (xml_elem::iterator it = sks.iterate("sink_record"); !it.end(); it.next()) {
        sunken_ships.push_back(sink_record(it.elem()));
    }
}

void scoring_manager::save(xml_elem &parent) const {
    xml_elem sks = parent.add_child("sunken_ships");
    sks.set_attr(unsigned(sunken_ships.size()), "nr");
    for (std::list<sink_record>::const_iterator it = sunken_ships.begin(); it != sunken_ships.end(); ++it) {
        xml_elem sr = sks.add_child("sink_record");
        it->save(sr);
    }
}
