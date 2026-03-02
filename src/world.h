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

// world - game world container with all entities
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef WORLD_H
#define WORLD_H

#include <list>
#include <memory>
#include <vector>

// Forward declarations
class ship;
class submarine;
class airplane;
class torpedo;
class depth_charge;
class gun_shell;
class water_splash;
class particle;
class convoy;
class sea_object;

///\brief Container for all game world entities and their interactions
class world {
  public:
    world();
    ~world();

    // Entity containers
    const std::vector<std::unique_ptr<ship>>& get_ships() const { return ships; }
    const std::vector<std::unique_ptr<submarine>>& get_submarines() const { return submarines; }
    const std::vector<std::unique_ptr<airplane>>& get_airplanes() const { return airplanes; }
    const std::vector<std::unique_ptr<torpedo>>& get_torpedoes() const { return torpedoes; }
    const std::vector<std::unique_ptr<depth_charge>>& get_depth_charges() const { return depth_charges; }
    const std::vector<std::unique_ptr<gun_shell>>& get_gun_shells() const { return gun_shells; }
    const std::vector<std::unique_ptr<water_splash>>& get_water_splashes() const { return water_splashes; }
    const std::vector<std::unique_ptr<convoy>>& get_convoys() const { return convoys; }
    const std::vector<std::unique_ptr<particle>>& get_particles() const { return particles; }

    // Mutable access (for game logic)
    std::vector<std::unique_ptr<ship>>& get_ships_mut() { return ships; }
    std::vector<std::unique_ptr<submarine>>& get_submarines_mut() { return submarines; }
    std::vector<std::unique_ptr<airplane>>& get_airplanes_mut() { return airplanes; }
    std::vector<std::unique_ptr<torpedo>>& get_torpedoes_mut() { return torpedoes; }
    std::vector<std::unique_ptr<depth_charge>>& get_depth_charges_mut() { return depth_charges; }
    std::vector<std::unique_ptr<gun_shell>>& get_gun_shells_mut() { return gun_shells; }
    std::vector<std::unique_ptr<water_splash>>& get_water_splashes_mut() { return water_splashes; }
    std::vector<std::unique_ptr<convoy>>& get_convoys_mut() { return convoys; }
    std::vector<std::unique_ptr<particle>>& get_particles_mut() { return particles; }

    // Spawn entities
    void spawn_ship(std::unique_ptr<ship> s);
    void spawn_submarine(std::unique_ptr<submarine> u);
    void spawn_airplane(std::unique_ptr<airplane> a);
    void spawn_torpedo(std::unique_ptr<torpedo> t);
    void spawn_gun_shell(std::unique_ptr<gun_shell> s);
    void spawn_depth_charge(std::unique_ptr<depth_charge> dc);
    void spawn_water_splash(std::unique_ptr<water_splash> ws);
    void spawn_convoy(std::unique_ptr<convoy> cv);
    void spawn_particle(std::unique_ptr<particle> pt);

    // Cleanup defunct entities
    void cleanup_defunct_entities();

    // Get count of entities
    size_t get_ship_count() const { return ships.size(); }
    size_t get_submarine_count() const { return submarines.size(); }
    size_t get_airplane_count() const { return airplanes.size(); }
    size_t get_torpedo_count() const { return torpedoes.size(); }

  protected:
    // Entity containers
    std::vector<std::unique_ptr<ship>> ships;
    std::vector<std::unique_ptr<submarine>> submarines;
    std::vector<std::unique_ptr<airplane>> airplanes;
    std::vector<std::unique_ptr<torpedo>> torpedoes;
    std::vector<std::unique_ptr<depth_charge>> depth_charges;
    std::vector<std::unique_ptr<gun_shell>> gun_shells;
    std::vector<std::unique_ptr<water_splash>> water_splashes;
    std::vector<std::unique_ptr<convoy>> convoys;
    std::vector<std::unique_ptr<particle>> particles;

  private:
    world(const world&) = delete;
    world& operator=(const world&) = delete;
};

#endif
