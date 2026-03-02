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

// coast renderer implementation
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "coast_renderer.h"

coast_renderer::coast_renderer(const std::string &map_filename)
    : mycoastmap(map_filename) {
}

coast_renderer::~coast_renderer() {
}

void coast_renderer::finish_construction() {
    mycoastmap.finish_construction();
}

void coast_renderer::render(const vector2 &viewpos, double max_view_dist, bool mirrored) {
    mycoastmap.render(viewpos, max_view_dist, mirrored);
}
