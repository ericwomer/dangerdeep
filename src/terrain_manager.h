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

// terrain manager - manages terrain rendering (geoclipmap)
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef TERRAIN_MANAGER_H
#define TERRAIN_MANAGER_H

#include "geoclipmap.h"
#include <memory>

class frustum;
class height_generator;

///\brief Manages terrain rendering using geoclipmap
class terrain_manager {
  public:
    /// Constructor
    /// @param nr_levels - number of clipmap levels
    /// @param resolution_n - resolution parameter (N in 2^N grid size)
    /// @param height_gen - height generator for terrain data
    terrain_manager(unsigned nr_levels, unsigned resolution_n, height_generator &height_gen);
    
    ~terrain_manager();

    /// Update viewer position for terrain LOD
    /// @param viewpos - current viewer position in world space
    void set_viewer_position(const vector3 &viewpos);

    /// Render terrain
    /// @param viewfrustum - viewing frustum for culling
    /// @param offset - offset for rendering
    /// @param mirrored - is view mirrored (for water reflection)
    /// @param above_water - is viewer above water
    void render(const frustum &viewfrustum, const vector3 &offset, bool mirrored, bool above_water);

    /// Get underlying geoclipmap (const access)
    const geoclipmap &get_geoclipmap() const { return *mygeoclipmap; }

    /// Toggle wireframe rendering mode
    void toggle_wireframe() { mygeoclipmap->wireframe = !mygeoclipmap->wireframe; }

  protected:
    std::unique_ptr<geoclipmap> mygeoclipmap;

  private:
    terrain_manager(const terrain_manager &) = delete;
    terrain_manager &operator=(const terrain_manager &) = delete;
};

#endif // TERRAIN_MANAGER_H
