// texts
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef TEXTS_H
#define TEXTS_H

#include <vector>
using namespace std;

class texts {
	static vector<string> txts;

public:
	enum languages { english, german };
	static languages language;

	static void set_language(languages l = english);
	static string get(unsigned no);
};

#endif
