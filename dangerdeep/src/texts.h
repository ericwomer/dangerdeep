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

// texts
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef TEXTS_H
#define TEXTS_H

#include <vector>
#include <string>

///\brief Stores text objects for various languages. Used for internationalization.
class texts
{
 public:
	enum category { common, languages, nr_of_categories };
 private:
	std::vector<std::vector<std::string> > strings;
	void read_category(category ct);
	std::string language_code;

	static const texts& texts::obj();

	texts(const std::string& langcode = "en");
 public:
	static void set_language(const std::string& language_code);
	static std::string get_language_code();
	static std::string get(unsigned no, category ct = common);
	static std::string numeric_from_date(const class date& d);
};

#endif
