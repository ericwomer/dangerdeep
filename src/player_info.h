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

// Player information data structure
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef PLAYER_INFO_H
#define PLAYER_INFO_H

#include <list>
#include <string>

class xml_elem;

/// Player information (career, documents, etc.)
struct player_info {
    std::string name;
    unsigned flotilla;
    std::string submarineid;
    std::string photo;

    std::string soldbuch_nr;
    std::string gasmask_size;
    std::string bloodgroup;
    std::string marine_roll;
    std::string marine_group;
    /* 'cause the career list is linear we do not need to store
     * ranks or paygroups. a list of the dates should be enough
     */
    std::list<std::string> career;

    player_info();
    player_info(const xml_elem &parent);
    void save(xml_elem &parent) const;
};

#endif
