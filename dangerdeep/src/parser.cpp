// text parser
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "parser.h"
#include "system.h"
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
	token(TKN_MINUS, "-"),
	token(TKN_PLAYER, "player"),
	token(TKN_SUBMARINE, "submarine"),
	token(TKN_SHIP, "ship"),
	token(TKN_TYPEVIIC, "typeVIIc"),
	token(TKN_TYPEIXC40, "typeIXc40"),
	token(TKN_TYPEXXI, "typeXXI"),
	token(TKN_POSITION, "position"),
	token(TKN_ASSIGN, "="),
	token(TKN_COMMA, ","),
	token(TKN_SEMICOLON, ";"),
	token(TKN_HEADING, "heading"),
	token(TKN_SCOPEUP, "scopeup"),
	token(TKN_MAXDEPTH, "maxdepth"),
	token(TKN_THROTTLE, "throttle"),
	token(TKN_STOP, "stop"),
	token(TKN_REVERSE, "reverse"),
	token(TKN_AHEADLISTEN, "listen"),
	token(TKN_AHEADSONAR, "sonar"),
	token(TKN_AHEADSLOW, "slow"),
	token(TKN_AHEADHALF, "half"),
	token(TKN_AHEADFULL, "full"),
	token(TKN_AHEADFLANK, "flank"),
	token(TKN_TORPEDOES, "torpedoes"),
	token(TKN_TXTNONE, "none"),
	token(TKN_T1, "T1"),
	token(TKN_T2, "T2"),
	token(TKN_T3, "T3"),
	token(TKN_T3A, "T3a"),
	token(TKN_T4, "T4"),
	token(TKN_T5, "T5"),
	token(TKN_T11, "T11"),
	token(TKN_T1FAT, "T1FAT"),
	token(TKN_T3FAT, "T3FAT"),
	token(TKN_T6LUT, "T6LUT"),
	token(TKN_LARGEMERCHANT, "largemerchant"),
	token(TKN_MEDIUMMERCHANT, "mediummerchant"),
	token(TKN_SMALLMERCHANT, "smallmerchant"),
	token(TKN_MEDIUMTROOPSHIP, "mediumtroopship"),
	token(TKN_DESTROYERTRIBAL, "destroyertribal"),
	token(TKN_BATTLESHIPMALAYA, "battleshipmalaya"),
	token(TKN_CARRIERBOGUE, "carrierbogue"),
	token(TKN_CORVETTE, "corvette"),
	token(TKN_LARGEFREIGHTER, "largefreighter"),
	token(TKN_MEDIUMFREIGHTER, "mediumfreighter"),
	token(TKN_SMALLMERCHANT, "smalltanker"),
	token(TKN_CONVOY, "convoy"),
	token(TKN_WAYPOINT, "waypoint"),
	token(TKN_SPEED, "speed"),
	token(TKN_TIME, "time"),
	token(TKN_SNORKEL, "snorkel"),
	token(TKN_FUEL, "fuel"),
	token(TKN_BATTERY, "battery"),

	token()
};

void parser::error(const string& s)
{
	ostringstream oss;
	oss << s << "\nFile: " << filename
		<< ", Line " << tkn->get_line()
		<< ", Col " << tkn->get_column()
		<< "\nParsed token text '" << tkn->get_current().text << "'\n";
	system::sys()->myassert(false, oss.str());
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

	// remove " signs
	tmp = tmp.substr(1, tmp.length() - 2);

	// translate returns and tabs
	for (string::size_type st = tmp.find("\\n"); st != string::npos; st = tmp.find("\\n"))
		tmp.replace(st, 2, "\n");
	for (string::size_type st = tmp.find("\\t"); st != string::npos; st = tmp.find("\\t"))
		tmp.replace(st, 2, "\t");
		
	return tmp;
}

int parser::parse_number(void)
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

string parser::parse_id(void)
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
