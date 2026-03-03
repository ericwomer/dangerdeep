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

#include "visibility_manager.h"

void visibility_manager::compute(double light_brightness) {
    // Base visibility is 5km, plus up to 25km based on light brightness
    // This must depend also on weather, fog, rain etc. in the future
    max_view_dist = 5000.0 + light_brightness * 25000.0;
}
