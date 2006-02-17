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

#include "texts.h"

#include "parser.h"
#include "global_data.h"
#include "error.h"
#include "tokencodes.h"
#include "date.h"
#include "datadirs.h"
#include <sstream>
#include <memory>
using namespace std;

#define TEXTS_DIR "texts/"

static char* categoryfiles[texts::nr_of_categories] = {
	"common",
	"languages",
};

auto_ptr<texts> texts_singleton_handler;

const texts& texts::obj()
{
	if (!texts_singleton_handler.get())
		texts_singleton_handler.reset(new texts());
	return *texts_singleton_handler.get();
}

texts::texts(const string& langcode) : language_code(langcode)
{
	strings.resize(nr_of_categories);
	for (unsigned i = 0; i < nr_of_categories; ++i)
		read_category(category(i));
}

void texts::read_category(category ct)
{
	string catfilename = categoryfiles[ct];
	parser p(get_data_dir() + TEXTS_DIR + catfilename + ".csv");
	// as first read language codes / count number of languages
	if (p.is_empty()) throw error("empty texts file: " + catfilename);
	string s = p.parse_string();
	if (s != "CODE") throw error("no CODE keyword in texts file: " + catfilename);
	p.parse(TKN_SEMICOLON);
	unsigned lcn = 0;
	vector<string> language_codes;
	while (true) {
		string lc = p.parse_string();
		if (lc == language_code) lcn = language_codes.size();
		language_codes.push_back(lc);
		if (p.type() != TKN_SEMICOLON) break;
		p.consume();
	}
/*
  if (ct == 0) {
  // read date format string
}
*/

	// now read strings
	vector<string>& txt = strings[ct];
	while (p.type() == TKN_NUMBER) {
		int n = p.parse_number();
		if (n < 0) throw error("negative text number in file: " + catfilename);
		if (unsigned(n) >= txt.size())
			txt.resize(n + 1);
		for (unsigned i = 0; i < language_codes.size(); ++i) {
			p.parse(TKN_SEMICOLON);
			string s;
			if (p.type() == TKN_NUMBER) {
				ostringstream oss;
				oss << p.parse_number();
				s = oss.str();
			} else {
				s = p.parse_string();
			}
			if (i == lcn)
				txt[n] = s;
		}
	}
}

void texts::set_language(const string& langcode)
{
	texts_singleton_handler.reset(new texts(langcode));
}

string texts::get_language_code()
{
	return obj().language_code;
}

string texts::get(unsigned no, category ct)
{
	const texts& t = obj();
	if (ct >= nr_of_categories)
		throw error("invalid category for texts::get()");
	const vector<string>& tx = t.strings[ct];
	if (no >= tx.size())
		throw error("invalid text nummer for texts::get()");
	return tx[no];
}

string texts::numeric_from_date(const date& d)
{
	ostringstream oss;
	string lc = obj().language_code;
	if (lc == "en") {
		// format mm/dd/yyyy
		oss << d.get_value(date::month) << "/" << d.get_value(date::day) << "/"
		    << d.get_value(date::year);
	} else if (lc == "de") {// format dd.mm.yyyy
		oss << d.get_value(date::day) << "." << d.get_value(date::month) << "."
		    << d.get_value(date::year);
	} else if (lc == "it") {
		// format dd.mm.yyyy, fixme Giuseppe, how is the format?
		oss << d.get_value(date::day) << "." << d.get_value(date::month) << "."
		    << d.get_value(date::year);
	}
	return oss.str();
}
