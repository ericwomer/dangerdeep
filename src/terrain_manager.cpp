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

// terrain manager implementation
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "terrain_manager.h"
#include "frustum.h"
#include "vector3.h"

terrain_manager::terrain_manager(unsigned nr_levels, unsigned resolution_n, height_generator &height_gen)
    : mygeoclipmap(std::make_unique<geoclipmap>(nr_levels, resolution_n, height_gen)) {
}

terrain_manager::~terrain_manager() {
}

void terrain_manager::set_viewer_position(const vector3 &viewpos) {
    mygeoclipmap->set_viewerpos(viewpos);
}

void terrain_manager::render(const frustum &viewfrustum, const vector3 &offset, bool mirrored, bool above_water) {
    mygeoclipmap->display(viewfrustum, offset, mirrored, above_water);
}
