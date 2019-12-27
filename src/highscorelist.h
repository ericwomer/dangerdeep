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

// a highscore list
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef HIGHSCORELIST_H
#define HIGHSCORELIST_H

#include <iostream>
#include <vector>
#include <string>

///\brief Handles a list of high scores (hall of fame).
class highscorelist
{
	friend void check_for_highscore(const class game* );

public:
	struct entry {
		unsigned points;
		std::string name;
		// missing: maybe start & end date, realism factor, rank/merits, submarine number
		entry() : points(0), name("--------") {}
		entry(unsigned p, const std::string& n) : points(p), name(n) {}
		entry(std::istream& in);
		~entry() {}
		entry(const entry& e) : points(e.points), name(e.name) {}
		entry& operator= (const entry& e) { points = e.points; name = e.name; return *this; }
		void save(std::ostream& out) const;
		bool is_worse_than(unsigned pts) const;	// is entry worse than given value?
	};

protected:
	std::vector<entry> entries;
	
public:
	highscorelist(unsigned maxentries = 10);
	~highscorelist() {}
	highscorelist(const std::string& filename);	// read from file
	void save(const std::string& filename) const;
	unsigned get_listpos_for(unsigned points) const; // returns place in list or entries.size() if not in list
	bool is_good_enough(unsigned points) const; // check if score is good enough for an entry
	void record(unsigned points, const std::string& name); // record entry if it is good enough
	void show(class widget* parent) const;
};

#endif
