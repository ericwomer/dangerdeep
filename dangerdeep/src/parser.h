// text parser
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

class parser;

#ifndef PARSER_H
#define PARSER_H

using namespace std;

#include <string>
#include "tokenizer.h"

class parser
{
	tokenizer* tkn;
	string filename, thisid, otherid;

	public:	
	void error(const string& s);
	void parse(int type);
	string parse_string(void);
	unsigned parse_number(void);
	string parse_id(void);
	bool parse_bool(void);
	void register_thisid(const string& s) { thisid = s; };
	void register_otherid(const string& s) { otherid = s; };
	int type(void) const { return tkn->get_current().type; };
	string text(void) const { return tkn->get_current().text; };	
	bool is_empty(void) const { return tkn->is_empty(); };
	void consume(void);
		
	parser(const string& filename_);
	~parser();
};

#endif
