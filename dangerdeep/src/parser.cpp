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
#include <vector>
#include <sstream>
#include <algorithm>
#include <iostream>

using std::string;
using std::ostringstream;
using std::stringstream;
using std::vector;

#include "tokencodes.h"
token tokens[] = {
	token(TKN_TRUE, "true"),
	token(TKN_FALSE, "false"),
	token(TKN_MINUS, "-"),
	token(TKN_SEMICOLON, ";"),

	token()
};

void parser::error(const string& s)
{
	ostringstream oss;
	oss << s << "\nFile: " << filename
		<< ", Line " << tkn->get_line()
		<< ", Col " << tkn->get_column()
		<< "\nParsed token text '" << tkn->get_current().text << "'\n";
	throw ::error(oss.str());
}

parser::parser(const string& filename_)
{
	filename = filename_;
	int count = 0;
	vector<token> tokenlist;
	while (!tokens[count].is_empty())
		count++;
	tokenlist.reserve(count);
	for (int i = 0; i < count; i++)
		tokenlist.push_back(tokens[i]);
	tkn = new tokenizer(filename, tokenlist, TKN_NONE, TKN_STRING, TKN_NUMBER, TKN_ID);
}

parser::~parser()
{
	delete tkn;
}

void parser::parse(int type)
{
	if (tkn->get_current().type != type) {
		int i = 0;
		for ( ; !tokens[i].is_empty(); ++i)
			if (tokens[i].type == type)
				break;
		error(string("expected token \"")+string(tokens[i].text)
			+string("\" but a different token was found"));
	}
	tkn->read_next();
}

string parser::parse_string()
{
	if (tkn->get_current().type != TKN_STRING)
		error("expected string");
	tkn->read_next();
	string tmp = tkn->get_previous().text;

	// remove " signs
	tmp = tmp.substr(1, tmp.length() - 2);

	// translate returns and tabs
	for (string::size_type st = tmp.find("\\n"); st != string::npos; st = tmp.find("\\n"))
		tmp.replace(st, 2, "\n");
	for (string::size_type st = tmp.find("\\t"); st != string::npos; st = tmp.find("\\t"))
		tmp.replace(st, 2, "\t");
		
	return tmp;
}

int parser::parse_number()
{
	bool negative = false;
	if (tkn->get_current().type == TKN_MINUS) {
		negative = true;
		tkn->read_next();
	}
	if (tkn->get_current().type != TKN_NUMBER)
		error("expected number");
	tkn->read_next();
	stringstream s(tkn->get_previous().text);
	int n = 0;
	s >> n;
	return (negative) ? -n : n;
}

string parser::parse_id()
{
	if (tkn->get_current().type != TKN_ID) {
		error("expected identifier");
	}
	tkn->read_next();
#ifdef USETHISOTHER	
	// filter and replace this/other
	if (tkn->get_previous().text == "this") {
		if (thisid == "")
			error("parsed \"this\", but can't resolve it");
		return thisid;
	}
	if (tkn->get_previous().text == "other") {
		if (otherid == "")
			error("parsed \"other\", but can't resolve it");
		return otherid;
	}
#endif	
	return tkn->get_previous().text;
}

bool parser::parse_bool()
{
	bool v = false;
	switch(tkn->get_current().type) {
		case TKN_TRUE:
			v = true;
			break;
		case TKN_FALSE:
			v = false;
			break;
		default:
			error("expected true/false");
	}
	tkn->read_next();
	return v;
}			

void parser::consume()
{
	if (tkn->is_empty())
		error("consume() called, but no more tokens left");
	tkn->read_next();
}
