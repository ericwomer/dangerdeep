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

// Game loader - centralizes game state deserialization from XML
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef GAME_LOADER_H
#define GAME_LOADER_H

#include <string>

class game;

/// Loads game state from XML (mission or savegame file)
/// Declared as friend of game to populate protected members during construction
class game_loader {
  public:
    /// Load game state from XML file into an initialized game object
    static void load(game &g, const std::string &filename);
};

#endif
