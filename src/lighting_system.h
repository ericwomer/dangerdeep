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

// Lighting system - manages astronomical calculations for sun/moon and lighting
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef LIGHTING_SYSTEM_H
#define LIGHTING_SYSTEM_H

#include "color.h"
#include "vector3.h"

/// Manages astronomical calculations and lighting computations
class lighting_system {
  private:
    double current_time; ///< Current game time (seconds since 1.1.1939)

  public:
    lighting_system();
    ~lighting_system() = default;

    // Non-copyable
    lighting_system(const lighting_system &) = delete;
    lighting_system &operator=(const lighting_system &) = delete;

    /// Update current time for calculations
    void set_time(double time) { current_time = time; }

    /// Get current time
    double get_time() const { return current_time; }

    /// Compute light brightness at a given position
    /// @param viewpos - position in world coordinates
    /// @param sundir - output: direction to sun (normalized)
    /// @return brightness value [0.2 - 1.0]
    double compute_light_brightness(const vector3 &viewpos, vector3 &sundir) const;

    /// Compute light color based on sun elevation (warm at dawn/dusk)
    /// @param viewpos - position in world coordinates
    /// @return color with elevation-based warm tint
    colorf compute_light_color(const vector3 &viewpos) const;

    /// Compute sun position relative to viewpos
    /// @param viewpos - position in world coordinates
    /// @return sun position vector
    vector3 compute_sun_pos(const vector3 &viewpos) const;

    /// Compute moon position relative to viewpos
    /// @param viewpos - position in world coordinates
    /// @return moon position vector
    vector3 compute_moon_pos(const vector3 &viewpos) const;

    /// Check if it's day mode (sun is above horizon enough)
    /// @param viewpos - position in world coordinates
    /// @return true if day mode, false if night
    bool is_day_mode(const vector3 &viewpos) const;
};

#endif
