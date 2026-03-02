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

// weather renderer - manages and renders weather effects (rain, snow)
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef WEATHER_RENDERER_H
#define WEATHER_RENDERER_H

#include "ptrvector.h"
#include <memory>

class texture;

///\brief Renders weather effects like rain and snow
class weather_renderer {
  public:
    weather_renderer();
    ~weather_renderer();

    /// Draw weather effects for current time
    /// @param current_time - current game time for animation
    void draw(double current_time) const;

    /// Check if weather effects are enabled
    bool is_enabled() const;

  protected:
    /// Rain animation textures (if RAIN is defined)
    ptrvector<texture> raintex;

    /// Snow animation textures (if SNOW is defined)
    ptrvector<texture> snowtex;

    /// Initialize rain textures
    void init_rain();

    /// Initialize snow textures
    void init_snow();

  private:
    weather_renderer(const weather_renderer &) = delete;
    weather_renderer &operator=(const weather_renderer &) = delete;
};

#endif // WEATHER_RENDERER_H
