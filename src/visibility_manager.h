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

// Visibility manager - computes maximum viewing distance based on lighting
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef VISIBILITY_MANAGER_H
#define VISIBILITY_MANAGER_H

/// Manages visibility range calculations
class visibility_manager {
  private:
    double max_view_dist;

  public:
    visibility_manager() : max_view_dist(0.0) {}
    ~visibility_manager() = default;

    // Non-copyable
    visibility_manager(const visibility_manager &) = delete;
    visibility_manager &operator=(const visibility_manager &) = delete;

    /// Compute maximum viewing distance based on light brightness
    void compute(double light_brightness);

    /// Get current maximum viewing distance
    double get_max_distance() const { return max_view_dist; }

    /// Set maximum viewing distance (for loading from save)
    void set_max_distance(double dist) { max_view_dist = dist; }
};

#endif
