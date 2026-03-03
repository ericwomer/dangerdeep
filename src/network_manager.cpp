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

// Network manager - manages multiplayer networking (currently unused/legacy)
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "network_manager.h"

network_manager::network_manager()
    : type(NetworkType::SinglePlayer), server_connection(nullptr) {
}

network_manager::~network_manager() {
    // Note: network_connection cleanup would go here if multiplayer were active
    // Currently this is legacy/unused code
}
