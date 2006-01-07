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

class parser;

#ifndef PARSER_H
#define PARSER_H

//#define USETHISOTHER	// define for replacement of this/other with another id

#include <string>
using namespace std;

#include "tokenizer.h"

class parser
{
	tokenizer* tkn;
	string filename;
#ifdef USETHISOTHER	
	string thisid, otherid;
#endif	

	public:	
	void error(const string& s);
	void parse(int type);
	string parse_string(void);
	int parse_number(void);
	string parse_id(void);
	bool parse_bool(void);
#ifdef USETHISOTHER
	void register_this(const string& s) { thisid = s; };
	void register_other(const string& s) { otherid = s; };
	string get_this(void) const { return thisid; }
	string get_other(void) const { return otherid; }
#endif	
	int type(void) const { return tkn->get_current().type; };
	string text(void) const { return tkn->get_current().text; };	
	bool is_empty(void) const { return tkn->is_empty(); };
	void consume(void);
		
	parser(const string& filename_);
	~parser();
};

#endif
