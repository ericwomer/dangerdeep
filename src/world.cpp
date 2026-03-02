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
