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

// Physics system - manages collision detection and response
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef PHYSICS_SYSTEM_H
#define PHYSICS_SYSTEM_H

#include "vector3.h"
#include <vector>

class sea_object;
class ship;

/// Manages physics simulation: collision detection and response
class physics_system {
  public:
    physics_system() = default;
    ~physics_system() = default;

    // Non-copyable
    physics_system(const physics_system &) = delete;
    physics_system &operator=(const physics_system &) = delete;

    /// Check for collisions between all ships and apply collision response
    /// @param allships - vector of all ships in the game (including submarines)
    void check_collisions(const std::vector<ship *> &allships);

    /// Compute and apply collision response impulse between two objects
    /// @param a - first colliding object
    /// @param b - second colliding object
    /// @param collision_pos - position where collision occurred
    void collision_response(sea_object &a, sea_object &b, const vector3 &collision_pos);
};

#endif
