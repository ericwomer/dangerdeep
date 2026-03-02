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

// coast renderer - manages coastline rendering
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef COAST_RENDERER_H
#define COAST_RENDERER_H

#include "coastmap.h"
#include "vector2.h"

///\brief Manages coastline rendering
class coast_renderer {
  public:
    /// Constructor
    /// @param map_filename - path to coastmap XML file
    explicit coast_renderer(const std::string &map_filename);
    
    ~coast_renderer();

    /// Complete initialization (deferred loading)
    void finish_construction();

    /// Render coastline
    /// @param viewpos - viewer position (2D, xy plane)
    /// @param max_view_dist - maximum viewing distance
    /// @param mirrored - is view mirrored (for water reflection)
    void render(const vector2 &viewpos, double max_view_dist, bool mirrored);

    /// Get underlying coastmap (const access)
    const coastmap &get_coastmap() const { return mycoastmap; }

  protected:
    coastmap mycoastmap;

  private:
    coast_renderer(const coast_renderer &) = delete;
    coast_renderer &operator=(const coast_renderer &) = delete;
};

#endif // COAST_RENDERER_H
