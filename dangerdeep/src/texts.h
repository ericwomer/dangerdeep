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
#include <sstream>

///\brief Stores text objects for various languages. Used for internationalization.
class texts
{
 public:
	enum category {
		common,
		languages,
		formats,
		nr_of_categories };
 private:
	std::vector<std::vector<std::string> > strings;
	void read_category(category ct);
	std::string language_code;

	static const texts& obj();

	texts(const std::string& langcode = "en");

	static std::vector<std::string> available_language_codes;
 public:
	static void set_language(const std::string& language_code);
	static void set_language(unsigned nr);
	static std::string get_language_code();
	static unsigned get_current_language_nr();
	static std::string get(unsigned no, category ct = common);
	template<typename T> static std::string get_replace(unsigned no, const T& parameter, category ct = common)
	{
		std::ostringstream oss;
		oss << parameter;
		std::string tmp = get(no, ct);
		std::size_t st = tmp.find("{}");
		if (st != std::string::npos) {
			tmp.replace(st, 2, oss.str());
		}
		return tmp;
	}
	static std::string numeric_from_date(const class date& d);
	static std::string numeric_from_daytime(const class date& d); // hours:minutes
	static void read_available_language_codes();
	static unsigned get_nr_of_available_languages();
};

#endif
