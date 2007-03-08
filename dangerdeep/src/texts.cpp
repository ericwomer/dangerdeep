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
#include "date.h"
#include "datadirs.h"
#include <sstream>
#include <memory>
using namespace std;

#define TEXTS_DIR "texts/"

static char* categoryfiles[texts::nr_of_categories] = {
	"common",
	"languages",
	"formats"
};

auto_ptr<texts> texts_singleton_handler;

vector<string> texts::available_language_codes;



const texts& texts::obj()
{
	if (!texts_singleton_handler.get())
		texts_singleton_handler.reset(new texts());
	return *texts_singleton_handler.get();
}



texts::texts(const string& langcode) : language_code(langcode)
{
	if (available_language_codes.empty())
		read_available_language_codes();

	bool ok = false;
	for (vector<string>::const_iterator it = available_language_codes.begin();
	     it != available_language_codes.end(); ++it) {
		if (*it == language_code) {
			ok = true;
			break;
		}
	}

	if (!ok) {
		throw error(string("invalid language code: ") + language_code);
	}

	strings.resize(nr_of_categories);
	for (unsigned i = 0; i < nr_of_categories; ++i)
		read_category(category(i));
}



void texts::read_category(category ct)
{
	string catfilename = categoryfiles[ct];
	parser p(get_data_dir() + TEXTS_DIR + catfilename + ".csv");
	// as first read language codes / count number of languages
	if (p.get_cell() != "CODE") p.error("no CODE keyword in texts file");
	unsigned lcn = 0;
	for (unsigned i = 0; i < available_language_codes.size(); ++i) {
		if (!p.next_column()) p.error("no columns left in texts file");
		string lc = p.get_cell();
		if (lc != available_language_codes[i])
			p.error(string("invalid language code marker found, expected \"")
				+ available_language_codes[i] + "\", got \"" + lc + "\"!");
		if (lc == language_code) lcn = i;
	}

	// now read strings
	vector<string>& txt = strings[ct];
	while (p.next_line()) {
		unsigned n = 0;
		bool ok = p.get_cell_number(n);
		if (!ok) p.error("invalid line");
		if (n >= txt.size())
			txt.resize(n + 1);
		for (unsigned i = 0; i < available_language_codes.size(); ++i) {
			if (!p.next_column()) p.error("no columns left in texts file");
			string s = p.get_cell();
			if (i == lcn)
				txt[n] = s;
		}
	}
}



void texts::set_language(const string& langcode)
{
	texts_singleton_handler.reset(new texts(langcode));
}



void texts::set_language(unsigned nr)
{
	if (available_language_codes.empty())
		read_available_language_codes();
	if (nr >= available_language_codes.size())
		throw error(string("trying to set illegal language nr, valid 0...")
			    + str(available_language_codes.size()) + ", requested " + str(nr));
	texts_singleton_handler.reset(new texts(available_language_codes[nr]));
}



string texts::get_language_code()
{
	return obj().language_code;
}



unsigned texts::get_current_language_nr()
{
	string lg = get_language_code();
	for (unsigned i = 0; i < available_language_codes.size(); ++i) {
		if (available_language_codes[i] == lg)
			return i;
	}
	return 0; // should never reach this
}



string texts::get(unsigned no, category ct)
{
	const texts& t = obj();
	if (ct >= nr_of_categories)
		throw error("invalid category for texts::get()");
	const vector<string>& tx = t.strings[ct];
	if (no >= tx.size())
		throw error(string("invalid text nummer for texts::get() ") + str(no)
			    + string(", valid 0...") + str(tx.size())
			    + string(" category=") + str(int(ct)));
	return tx[no];
}



string texts::numeric_from_date(const date& d)
{
	const string& fmt = get(0, formats);
	string res;
	for (string::size_type p = 0; p < fmt.length(); ) {
		if (fmt[p] == 'd') {
			// day
			res += str(d.get_value(date::day));
			++p;
			while (p < fmt.length() && fmt[p] == 'd')
				++p;
		} else if (fmt[p] == 'm') {
			// month
			res += str(d.get_value(date::month));
			++p;
			while (p < fmt.length() && fmt[p] == 'm')
				++p;
		} else if (fmt[p] == 'y') {
			// year
			res += str(d.get_value(date::year));
			++p;
			while (p < fmt.length() && fmt[p] == 'y')
				++p;
		} else {
			res += fmt[p];
			++p;
		}
	}
	return res;
}



string texts::numeric_from_daytime(const date& d)
{
	const string& fmt = get(1, formats);
	string res;
	for (string::size_type p = 0; p < fmt.length(); ) {
		if (fmt[p] == 'h') {
			// hour
			res += str(d.get_value(date::hour));
			++p;
			while (p < fmt.length() && fmt[p] == 'h')
				++p;
		} else if (fmt[p] == 'm') {
			// minute
			res += str_wf(d.get_value(date::minute), 2);
			++p;
			while (p < fmt.length() && fmt[p] == 'm')
				++p;
		} else if (fmt[p] == 's') {
			// second
			res += str_wf(d.get_value(date::second), 2);
			++p;
			while (p < fmt.length() && fmt[p] == 's')
				++p;
		} else {
			res += fmt[p];
			++p;
		}
	}
	return res;
}



void texts::read_available_language_codes()
{
	available_language_codes.clear();
	parser p(get_data_dir() + TEXTS_DIR + "languages.csv");
	if (p.get_cell() != "CODE") p.error("no CODE keyword in texts file");
	while (p.next_column()) {
		string lc = p.get_cell();
		available_language_codes.push_back(lc);
	}
}



unsigned texts::get_nr_of_available_languages()
{
	if (available_language_codes.empty())
		read_available_language_codes();
	return available_language_codes.size();
}
