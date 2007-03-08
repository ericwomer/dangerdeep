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

#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <fstream>

/// generic parser for tabular text files
class parser
{
 public:
	/// open file with given separator
	parser(const std::string& filename_, char separator_ = ';');

	/// advance to next line
	///@returns true when next line could be read, false on end of file
	bool next_line();

	/// advance to next column of table
	///@returns true when next column could be read, false on end of line
	bool next_column();

	/// get text of current cell
	std::string get_cell() const { return cell; }

	/// get text of current cell as number, returns true if possible
	bool get_cell_number(unsigned& n) const;

	/// report error at current position (throws error)
	void error(const std::string& text);

 protected:
	std::string filename;
	char separator;
	std::ifstream file;
	std::string currline;
	unsigned line;
	std::string::size_type currcol;
	std::string cell;
};

#endif
