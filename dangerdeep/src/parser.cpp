// text parser
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "parser.h"
#include <vector>
#include <sstream>
#include <algorithm>
#include <iostream>

#include "tokencodes.h"
token tokens[] = {
	token(TKN_SLPARAN, "{"),
	token(TKN_SRPARAN, "}"),
	token(TKN_TRUE, "true"),
	token(TKN_FALSE, "false"),
/*
	token(TKN_DOT, "."),
	token(TKN_SEMICOLON, ";"),
	token(TKN_ASSIGN, "="),
	token(TKN_COMMA, ","),
	token(TKN_LPARAN, "("),
	token(TKN_RPARAN, ")"),
	
	token(TKN_LESS, "<"),
	token(TKN_LESSEQUAL, "<="),
	token(TKN_GREATER, ">"),
	token(TKN_GREATEREQUAL, ">="),
	token(TKN_EQUAL, "=="),
	token(TKN_NOTEQUAL, "!="),
	token(TKN_PLUSPLUS, "++"),
	token(TKN_MINUSMINUS, "--"),

	token(TKN_PLAYER, "player"),
	token(TKN_THING, "thing"),
	token(TKN_PERSON, "person"),
	token(TKN_ROOM, "room"),
	token(TKN_NAME, "name"),
	token(TKN_CLICK, "click"),
	token(TKN_LOOK, "look"),
	token(TKN_USE, "use"),
	token(TKN_SIZE, "size"),
	token(TKN_WEIGHT, "weight"),
	token(TKN_FLAGS, "flags"),
	token(TKN_ON, "on"),
	token(TKN_MAXSIZE, "maxsize"),
	token(TKN_MAXWEIGHT, "maxweight"),
	token(TKN_THINGS, "things"),
	token(TKN_PERSONS, "persons"),
	token(TKN_ENTER, "enter"),
	token(TKN_LEAVE, "leave"),

	token(TKN_SAY, "say"),
	token(TKN_TELEPORT, "teleport"),
	token(TKN_DEFAULT, "default"),
	token(TKN_TO, "to"),
	token(TKN_IF, "if"),
	token(TKN_THEN, "then"),
	token(TKN_ELSE, "else"),
	token(TKN_ELSEIF, "elseif"),
	token(TKN_END, "end"),
	token(TKN_AND, "and"),
	token(TKN_OR, "or"),
	token(TKN_NOT, "not"),
//	token(TKN_THIS, "this"),
//	token(TKN_OTHER, "other"),
//	token(TKN_ACTOR, "actor"),
	token(TKN_SWITCH, "switch"),
	token(TKN_TAKE, "take"),
	token(TKN_CONTAINS, "contains"),
	token(TKN_IS, "is"),
	token(TKN_INVERT, "invert"),
	token(TKN_MOVE, "move"),
	token(TKN_PLAY, "play"),
	token(TKN_SET, "set"),
	token(TKN_RESIZE, "resize"),
	token(TKN_SELECT, "select"),
	token(TKN_WALK, "walk"),
	token()
*/	
};

void parser::error(const string& s)
{
	ostringstream oss;
	oss << s << "\nFile: " << filename
		<< ", Line " << tkn->get_line()
		<< ", Col " << tkn->get_column()
		<< "\nParsed token text '" << tkn->get_current().text << "'\n";
//	::error(oss.str());
	cerr << oss.str();//fixme
	assert(false);//fixme
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

string parser::parse_string(void)
{
	if (tkn->get_current().type != TKN_STRING)
		error("expected string");
	tkn->read_next();
	string tmp = tkn->get_previous().text;
	return tmp.substr(1, tmp.length() - 2);
}

unsigned parser::parse_number(void)
{
	if (tkn->get_current().type != TKN_NUMBER)
		error("expected number");
	tkn->read_next();
	stringstream s(tkn->get_previous().text);
	unsigned n = 0;
	s >> n;
	return n;
}

string parser::parse_id(void)
{
	if (tkn->get_current().type != TKN_ID) {
		error("expected identifier");
	}
	tkn->read_next();
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
	return tkn->get_previous().text;
}

bool parser::parse_bool(void)
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

void parser::consume(void)
{
	if (tkn->is_empty())
		error("consume() called, but no more tokens left");
	tkn->read_next();
}
