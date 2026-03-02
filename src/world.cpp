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

#include "world.h"
#include "ship.h"
#include "submarine.h"
#include "airplane.h"
#include "torpedo.h"
#include "depth_charge.h"
#include "gun_shell.h"
#include "water_splash.h"
#include "particle.h"
#include "convoy.h"
#include "sensors.h"
#include "sonar.h"
#include "game.h"
#include <algorithm>

world::world() {
}

world::~world() {
}

void world::spawn_ship(std::unique_ptr<ship> s) {
    ships.push_back(std::move(s));
}

void world::spawn_submarine(std::unique_ptr<submarine> u) {
    submarines.push_back(std::move(u));
}

void world::spawn_airplane(std::unique_ptr<airplane> a) {
    airplanes.push_back(std::move(a));
}

void world::spawn_torpedo(std::unique_ptr<torpedo> t) {
    torpedoes.push_back(std::move(t));
}

void world::spawn_gun_shell(std::unique_ptr<gun_shell> s) {
    gun_shells.push_back(std::move(s));
}

void world::spawn_depth_charge(std::unique_ptr<depth_charge> dc) {
    depth_charges.push_back(std::move(dc));
}

void world::spawn_water_splash(std::unique_ptr<water_splash> ws) {
    water_splashes.push_back(std::move(ws));
}

void world::spawn_convoy(std::unique_ptr<convoy> cv) {
    convoys.push_back(std::move(cv));
}

void world::spawn_particle(std::unique_ptr<particle> pt) {
    particles.push_back(std::move(pt));
}

template <class T>
static void cleanup_container(std::vector<std::unique_ptr<T>>& container) {
    container.erase(
        std::remove_if(container.begin(), container.end(),
                      [](const std::unique_ptr<T>& p) {
                          return p && p->is_defunct();
                      }),
        container.end());
}

void world::cleanup_defunct_entities() {
    cleanup_container(ships);
    cleanup_container(submarines);
    cleanup_container(airplanes);
    cleanup_container(torpedoes);
    cleanup_container(depth_charges);
    cleanup_container(gun_shells);
    cleanup_container(water_splashes);
    cleanup_container(particles);
}

// Helper template for visibility detection
template <class T>
static std::vector<T*> visible_obj(const game* gm, const std::vector<std::unique_ptr<T>>& v, const sea_object* o) {
    std::vector<T*> result;
    const sensor* s = o->get_sensor(o->lookout_system);
    if (!s)
        return result;
    const lookout_sensor* ls = dynamic_cast<const lookout_sensor*>(s);
    if (!ls)
        return result;
    result.reserve(v.size());
    for (unsigned i = 0; i < v.size(); ++i) {
        if (v[i] && v[i]->is_reference_ok()) {
            if (ls->is_detected(gm, o, v[i].get()))
                result.push_back(v[i].get());
        }
    }
    return result;
}

std::vector<ship*> world::visible_ships(const game* gm, const sea_object* o) const {
    return visible_obj<ship>(gm, ships, o);
}

std::vector<submarine*> world::visible_submarines(const game* gm, const sea_object* o) const {
    return visible_obj<submarine>(gm, submarines, o);
}

std::vector<airplane*> world::visible_airplanes(const game* gm, const sea_object* o) const {
    return visible_obj<airplane>(gm, airplanes, o);
}

std::vector<torpedo*> world::visible_torpedoes(const game* gm, const sea_object* o) const {
    std::vector<torpedo*> result;
    for (unsigned k = 0; k < torpedoes.size(); ++k) {
        if (torpedoes[k] && torpedoes[k]->is_reference_ok()) {
            result.push_back(torpedoes[k].get());
        }
    }
    return result;
}

std::vector<depth_charge*> world::visible_depth_charges(const game* gm, const sea_object* o) const {
    return visible_obj<depth_charge>(gm, depth_charges, o);
}

std::vector<gun_shell*> world::visible_gun_shells(const game* gm, const sea_object* o) const {
    return visible_obj<gun_shell>(gm, gun_shells, o);
}

std::vector<water_splash*> world::visible_water_splashes(const game* gm, const sea_object* o) const {
    std::vector<water_splash*> result(water_splashes.size());
    for (unsigned k = 0; k < water_splashes.size(); ++k)
        result[k] = water_splashes[k].get();
    return result;
}

std::vector<particle*> world::visible_particles(const game* gm, const sea_object* o) const {
    std::vector<particle*> result;
    const sensor* s = o->get_sensor(o->lookout_system);
    if (!s)
        return result;
    const lookout_sensor* ls = dynamic_cast<const lookout_sensor*>(s);
    if (!ls)
        return result;
    result.reserve(particles.size());
    for (unsigned i = 0; i < particles.size(); ++i) {
        if (particles[i] == 0)
            throw error("particles[i] is 0!");
        if (ls->is_detected(gm, o, particles[i].get()))
            result.push_back(particles[i].get());
    }
    return result;
}

std::vector<sea_object*> world::visible_surface_objects(const game* gm, const sea_object* o) const {
    std::vector<sea_object*> result;
    auto shps = visible_ships(gm, o);
    for (unsigned i = 0; i < shps.size(); ++i)
        result.push_back(shps[i]);
    auto subs = visible_submarines(gm, o);
    for (unsigned i = 0; i < subs.size(); ++i)
        result.push_back(subs[i]);
    auto airs = visible_airplanes(gm, o);
    for (unsigned i = 0; i < airs.size(); ++i)
        result.push_back(airs[i]);
    return result;
}

std::vector<sea_object*> world::visible_sea_objects(const game* gm, const sea_object* o) const {
    std::vector<sea_object*> result;
    auto shps = visible_ships(gm, o);
    for (unsigned i = 0; i < shps.size(); ++i)
        result.push_back(shps[i]);
    auto subs = visible_submarines(gm, o);
    for (unsigned i = 0; i < subs.size(); ++i)
        result.push_back(subs[i]);
    auto airs = visible_airplanes(gm, o);
    for (unsigned i = 0; i < airs.size(); ++i)
        result.push_back(airs[i]);
    auto torps = visible_torpedoes(gm, o);
    for (unsigned i = 0; i < torps.size(); ++i)
        result.push_back(torps[i]);
    return result;
}

std::vector<sonar_contact> world::sonar_ships(const game* gm, const sea_object* o) const {
    std::vector<sonar_contact> result;
    const sensor* s = o->get_sensor(o->passive_sonar_system);
    if (!s)
        return result;
    const passive_sonar_sensor* pss = dynamic_cast<const passive_sonar_sensor*>(s);
    if (!pss)
        return result;

    result.reserve(ships.size());
    for (unsigned k = 0; k < ships.size(); ++k) {
        if (!ships[k]->is_reference_ok())
            continue;
        if (o == ships[k].get())
            continue;
        if (pss->is_detected(gm, o, ships[k].get()))
            result.push_back(sonar_contact(ships[k]->get_pos().xy(), ships[k]->get_class()));
    }
    return result;
}

std::vector<sonar_contact> world::sonar_submarines(const game* gm, const sea_object* o) const {
    std::vector<sonar_contact> result;
    const sensor* s = o->get_sensor(o->passive_sonar_system);
    if (!s)
        return result;
    const passive_sonar_sensor* pss = dynamic_cast<const passive_sonar_sensor*>(s);
    if (!pss)
        return result;

    for (unsigned k = 0; k < submarines.size(); ++k) {
        if (!submarines[k]->is_reference_ok())
            continue;
        if (o == submarines[k].get())
            continue;
        if (pss->is_detected(gm, o, submarines[k].get()))
            result.push_back(sonar_contact(submarines[k]->get_pos().xy(), submarines[k]->get_class()));
    }
    return result;
}

std::vector<sonar_contact> world::sonar_sea_objects(const game* gm, const sea_object* o) const {
    std::vector<sonar_contact> result;
    auto ships_contacts = sonar_ships(gm, o);
    auto subs_contacts = sonar_submarines(gm, o);
    result.reserve(ships_contacts.size() + subs_contacts.size());
    result.insert(result.end(), ships_contacts.begin(), ships_contacts.end());
    result.insert(result.end(), subs_contacts.begin(), subs_contacts.end());
    return result;
}

ship* world::sonar_acoustical_torpedo_target(const game* gm, const torpedo* o) const {
    ship* loudest_object = 0;
    double loudest_object_sf = 0;
    const sensor* s = o->get_sensor(o->passive_sonar_system);
    const passive_sonar_sensor* pss = 0;
    if (s)
        pss = dynamic_cast<const passive_sonar_sensor*>(s);

    if (pss) {
        for (unsigned k = 0; k < ships.size(); ++k) {
            double sf = 0.0f;
            if (pss->is_detected(sf, gm, o, ships[k].get())) {
                if (sf > loudest_object_sf) {
                    loudest_object_sf = sf;
                    loudest_object = ships[k].get();
                }
            }
        }

        for (unsigned k = 0; k < submarines.size(); ++k) {
            double sf = 0.0f;
            if (pss->is_detected(sf, gm, o, submarines[k].get())) {
                if (sf > loudest_object_sf) {
                    loudest_object_sf = sf;
                    loudest_object = submarines[k].get();
                }
            }
        }
    }
    return loudest_object;
}

std::vector<submarine*> world::radar_submarines(const game* gm, const sea_object* o) const {
    std::vector<submarine*> result;
    const sensor* s = o->get_sensor(o->radar_system);
    if (!s)
        return result;
    const lookout_sensor* ls = dynamic_cast<const lookout_sensor*>(s);
    if (!ls)
        return result;
    result.reserve(submarines.size());
    for (unsigned k = 0; k < submarines.size(); ++k) {
        if (ls->is_detected(gm, o, submarines[k].get()))
            result.push_back(submarines[k].get());
    }
    return result;
}

std::vector<ship*> world::radar_ships(const game* gm, const sea_object* o) const {
    std::vector<ship*> result;
    const sensor* s = o->get_sensor(o->radar_system);
    if (!s)
        return result;
    const lookout_sensor* ls = dynamic_cast<const lookout_sensor*>(s);
    if (!ls)
        return result;
    result.reserve(ships.size());
    for (unsigned k = 0; k < ships.size(); ++k) {
        if (ls->is_detected(gm, o, ships[k].get()))
            result.push_back(ships[k].get());
    }
    return result;
}

std::vector<sea_object*> world::radar_sea_objects(const game* gm, const sea_object* o) const {
    std::vector<sea_object*> result;
    auto shps = radar_ships(gm, o);
    for (unsigned i = 0; i < shps.size(); ++i)
        result.push_back(shps[i]);
    auto subs = radar_submarines(gm, o);
    for (unsigned i = 0; i < subs.size(); ++i)
        result.push_back(subs[i]);
    return result;
}
