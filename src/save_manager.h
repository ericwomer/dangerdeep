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

// Save manager - centralizes savegame serialization to XML
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef SAVE_MANAGER_H
#define SAVE_MANAGER_H

#include <string>

class game;

/// Manages serialization of game state to XML save files
class save_manager {
  public:
    save_manager() = default;

    /// Serialize game state to XML file
    void save(const game &g, const std::string &savefilename, const std::string &description) const;

    /// Read description string from save file (for load menu)
    static std::string read_description_of_savegame(const std::string &filename);
};

#endif
