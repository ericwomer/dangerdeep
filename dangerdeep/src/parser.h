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
