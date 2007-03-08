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

// text parser
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "parser.h"
#include "error.h"
#include <sstream>

using std::string;
using std::ifstream;
using std::istringstream;
using std::ostringstream;



parser::parser(const string& filename_, char separator_)
	: filename(filename_),
	  separator(separator_),
	  file(filename.c_str(), std::ios::in),
	  line(0),
	  currcol(string::npos)
{
	if (file.fail())
		throw file_read_error(filename);
	if (!next_line())
		throw ::error(string("Can't read in file ") + filename);
	if ((separator <= ' ' && separator != '\t') || separator == '"')
		throw std::invalid_argument("invalid separator!");
}



bool parser::next_line()
{
	while (!file.eof()) {
		getline(file, currline);
		++line;
		if (currline.empty())
			continue;
		currcol = 0;
		// terminate line if it ends with separator
		if (currline[currline.size()-1] == separator)
			currline += "\"\"";
		bool ok = next_column();
		if (ok)
			return true;
		// else: empty line, try next
	}
	return false;
}



bool parser::next_column()
{
	if (currcol >= currline.size())
		return false;
	// try to generate next cell
	cell.clear();
	bool in_string = false;
	int is_string = 0;
	for ( ; currcol < currline.size(); ++currcol) {
		char c = currline[currcol];
		char c2 = (currcol + 1 < currline.size()) ? currline[currcol+1] : 0;
		if (in_string) {
			// just append any character until end of string
			if (c == '"') {
				if (c2 == '"') {
					// double string, change to normal
					cell += '"';
					++currcol;
					continue;
				}
				in_string = false;
				continue;
			}
			if (c == '\\') {
				if (c2 == 'n') {
					cell += '\n';
					++currcol;
					continue;
				} else if (c2 == 't') {
					cell += '\t';
					++currcol;
					continue;
				}
			}
			cell += c;
		} else {
			// ignore whitespaces
			if (c == separator) {
				++currcol;
				break;
			}
			if (c == ' ' || c == '\t')
				continue;
			if (is_string > 0) {
				// encountered any non-separator or non-whitespace,
				// but we already had a string
				error("error in read, character between end of string and next separator");
			}
			if (c == '"') {
				if (is_string < 0)
					error("error in read, character before begin of string");
				in_string = true;
				is_string = 1;
				continue;
			}
			// else just add character
			is_string = -1;
			cell += c;
		}
	}
	if (in_string)
		error("unterminated string");
	return true;
}



bool parser::get_cell_number(unsigned& n) const
{
	std::istringstream iss(cell);
	iss >> n;
	return !iss.fail();
}



void parser::error(const std::string& text)
{
	ostringstream oss;
	oss << "Parse error in file \"" << filename << "\", line " << line << ", column " << unsigned(currcol) << ", error: " << text;
	throw ::error(oss.str());
}
