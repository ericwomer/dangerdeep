// texts
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef TEXTS_H
#define TEXTS_H

#include <vector>
#include <string>
using namespace std;

class texts {
	static vector<string> txts;

public:
	enum languages { english, german, italian, nr_of_languages };
	static languages language;

	static void set_language(languages l = english);
	static string get_language_code(void);
	static string get(unsigned no);
	static string numeric_from_date(const class date& d);
};

#endif
