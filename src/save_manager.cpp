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

// Save manager - centralizes savegame serialization to XML
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "save_manager.h"
#include "airplane.h"
#include "date.h"
#include "convoy.h"
#include "depth_charge.h"
#include "error.h"
#include "game.h"
#include "gun_shell.h"
#include "sea_object.h"
#include "ship.h"
#include "submarine.h"
#include "torpedo.h"
#include "xml.h"
#include <string>

using std::string;

namespace {
const unsigned SAVEVERSION = 1;
const unsigned GAMETYPE = 0; // fixme, 0-mission, 1-patrol etc.
}

void save_manager::save(const game &g, const std::string &savefilename, const std::string &description) const {
    xml_doc doc(savefilename);
    xml_elem sg = doc.add_child("dftd-savegame");
    sg.set_attr(description.empty() ? "(Sin descripción)" : description, "description");
    sg.set_attr(SAVEVERSION, "version");
    sg.set_attr(GAMETYPE, "type");

    const world &w = g.get_world();

    xml_elem sh = sg.add_child("ships");
    sh.set_attr(unsigned(w.get_ships().size()), "nr");
    for (unsigned k = 0; k < w.get_ships().size(); ++k) {
        xml_elem e = sh.add_child("ship");
        e.set_attr(w.get_ships()[k]->get_specfilename(), "type");
        w.get_ships()[k]->save(e);
    }

    xml_elem su = sg.add_child("submarines");
    su.set_attr(unsigned(w.get_submarines().size()), "nr");
    for (unsigned k = 0; k < w.get_submarines().size(); ++k) {
        xml_elem e = su.add_child("submarine");
        e.set_attr(w.get_submarines()[k]->get_specfilename(), "type");
        w.get_submarines()[k]->save(e);
    }

    xml_elem ap = sg.add_child("airplanes");
    ap.set_attr(unsigned(w.get_airplanes().size()), "nr");
    for (unsigned k = 0; k < w.get_airplanes().size(); ++k) {
        xml_elem e = ap.add_child("airplane");
        e.set_attr(w.get_airplanes()[k]->get_specfilename(), "type");
        w.get_airplanes()[k]->save(e);
    }

    xml_elem tp = sg.add_child("torpedoes");
    tp.set_attr(unsigned(w.get_torpedoes().size()), "nr");
    for (unsigned k = 0; k < w.get_torpedoes().size(); ++k) {
        xml_elem e = tp.add_child("torpedo");
        e.set_attr(w.get_torpedoes()[k]->get_specfilename(), "type");
        w.get_torpedoes()[k]->save(e);
    }

    xml_elem dc = sg.add_child("depth_charges");
    dc.set_attr(unsigned(w.get_depth_charges().size()), "nr");
    for (unsigned k = 0; k < w.get_depth_charges().size(); ++k) {
        xml_elem e = dc.add_child("depth_charge");
        w.get_depth_charges()[k]->save(e);
    }

    xml_elem gs = sg.add_child("gun_shells");
    gs.set_attr(unsigned(w.get_gun_shells().size()), "nr");
    for (unsigned k = 0; k < w.get_gun_shells().size(); ++k) {
        xml_elem e = gs.add_child("gun_shell");
        w.get_gun_shells()[k]->save(e);
    }

    xml_elem cv = sg.add_child("convoys");
    cv.set_attr(unsigned(w.get_convoys().size()), "nr");
    for (unsigned k = 0; k < w.get_convoys().size(); ++k) {
        xml_elem e = cv.add_child("convoy");
        w.get_convoys()[k]->save(e);
    }

    // save player
    sea_object *player = g.get_player();
    submarine *tmpsub = dynamic_cast<submarine *>(player);
    string pltype;
    if (tmpsub) {
        pltype = "submarine";
    } else {
        ship *tmpship = dynamic_cast<ship *>(player);
        if (tmpship) {
            pltype = "ship";
        } else {
            airplane *tmpairpl = dynamic_cast<airplane *>(player);
            if (tmpairpl) {
                pltype = "airplane";
            } else {
                throw error("internal error: player is no sub, ship or airplane");
            }
        }
    }
    xml_elem pl = sg.add_child("player");
    pl.set_attr(g.save_ptr(player), "ref");
    pl.set_attr(pltype, "type");

    g.get_scoring_manager().save(sg);

    xml_elem gst = sg.add_child("state");
    gst.set_attr(g.get_time(), "time");
    date(unsigned(g.get_time())).save(gst);
    gst.set_attr(g.get_trail_manager().get_last_trail_time(), "last_trail_time");
    xml_elem equ = gst.add_child("equipment_date");
    g.get_equipment_date().save(equ);
    gst.set_attr(g.get_visibility_manager().get_max_distance(), "max_view_dist");

    g.get_ping_manager().save(sg);

    xml_elem pi = sg.add_child("player_info");
    g.get_player_info().save(pi);

    doc.save();
}

std::string save_manager::read_description_of_savegame(const std::string &filename) {
    xml_doc doc(filename);
    doc.load();
    xml_elem sg = doc.child("dftd-savegame");
    unsigned v = sg.attru("version");
    if (v != SAVEVERSION)
        return "<ERROR> Invalid version";
    string d = sg.attr("description");
    if (d.length() == 0)
        return "(Sin descripción)";
    return d;
}
