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

// logbook
// subsim (C)+(W) Markus Petermann and Thorsten Jordan. SEE LICENSE

#include <map>
#include <iostream>
#include <sstream>
using namespace std;
#include "date.h"
#include "global_data.h"
#include "logbook.h"



void logbook::add_entry(const string& entry)
{
	entries.push_back(entry);
}



list<string>::const_iterator logbook::get_entry(unsigned i) const
{
	list<string>::const_iterator it = entries.begin();
	while (i > 0 && it != entries.end()) {
		--i;
		++it;
	}
	return it;
}
