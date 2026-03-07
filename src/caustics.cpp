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

// underwater caustic simulation
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include <float.h>
#include <iomanip>
#include <sstream>

using namespace std;

#include "caustics.h"
#include "datadirs.h"

static constexpr unsigned FRAME_TEXTURE_COUNT = 32;

caustics::caustics()
    : mytime(-DBL_MAX), current_texture(0) {
    //	load caustic maps "caustic??.png"
    for (unsigned int i = 0; i < FRAME_TEXTURE_COUNT; i++) {
        stringstream filename;
        filename << "caustic" << setfill('0') << setw(2) << i << ".png";

        texture_pointers.push_back(std::make_unique<texture>(get_texture_dir() + filename.str(), texture::LINEAR));
    }
}

void caustics::set_time(double tm) {
    const double frame_duration = 1.0 / 25.0; // 25 fps animation
    while ((tm - mytime) >= frame_duration) {
        mytime += frame_duration;
        current_texture = (current_texture + 1) % FRAME_TEXTURE_COUNT;
    }
    if (tm > mytime + frame_duration * FRAME_TEXTURE_COUNT)
        mytime = tm; // sync when far behind (e.g. after load)
}

texture *caustics::get_map() const {
    return texture_pointers[current_texture].get();
}
