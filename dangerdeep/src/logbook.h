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

// handle log book entries
// subsim (C)+(W) Markus Petermann and Thorsten Jordan. SEE LICENSE

#ifndef LOGBOOK_H
#define LOGBOOK_H

#include <string>
#include <list>
using namespace std;

///\brief Simulates a log book.
class logbook
{
protected:
	list<string> entries;

public:
	logbook() {}
	virtual ~logbook () {}
	virtual void add_entry(const string& entry);
	virtual list<string>::const_iterator get_entry(unsigned i) const;
	virtual list<string>::const_iterator begin() const { return entries.begin(); }
	virtual list<string>::const_iterator end() const { return entries.end(); }
	virtual unsigned size() const { return entries.size(); }
};

#endif
