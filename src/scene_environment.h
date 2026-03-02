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

// scene environment - manages sky and caustics for the scene
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef SCENE_ENVIRONMENT_H
#define SCENE_ENVIRONMENT_H

#include "caustics.h"
#include "sky.h"
#include <memory>

///\brief Manages environmental scene elements (sky, caustics)
///
/// This subsystem consolidates sky and caustics management, previously
/// scattered in user_interface. It provides unified time synchronization
/// for atmospheric effects like sky rendering and underwater caustics.
///
/// Responsibilities:
/// - Owns and manages sky (stars, sun, moon, atmospheric scattering)
/// - Owns and manages caustics (underwater light patterns)
/// - Synchronizes time updates to both components
class scene_environment {
  public:
    scene_environment();
    ~scene_environment();

    /// Update environment with current time
    /// @param time - current scene time for animation
    void set_time(double time);

    /// Get sky (const access)
    const sky &get_sky() const { return *mysky; }

    /// Get caustics (const access)
    const caustics &get_caustics() const { return mycaustics; }

  protected:
    std::unique_ptr<sky> mysky;
    caustics mycaustics;

  private:
    scene_environment(const scene_environment &) = delete;
    scene_environment &operator=(const scene_environment &) = delete;
};

#endif // SCENE_ENVIRONMENT_H
