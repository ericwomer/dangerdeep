// texts
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "texts.h"

#include "parser.h"
#include "global_data.h"
#include "date.h"

#define TEXTS_DIR "texts/"

texts::languages texts::language = texts::english;

static char* textfilenames[] = {
	"english",
	"german"
};

vector<string> texts::txts;

void texts::set_language(languages l)
{
	language = l;
	parser p(get_data_dir() + TEXTS_DIR + textfilenames[l] + ".text");
	txts.clear();
	int count = 0;
	while (!p.is_empty()) {
		int n = p.parse_number();
		string s = p.parse_string();
		if (n >= count) {
			txts.resize(n+1);
			count = n+1;
		}
		txts[n] = s;
	}
}

string texts::get(unsigned no)
{
	if (no < txts.size())
		return txts[no];
	return "!INVALID TEXT NUMBER!";
}

string texts::numeric_from_date(const date& d)
{
	ostringstream oss;
	switch (language) {
		case english :	// format mm/dd/yyyy
			oss << d.get_value(date::month) << "/" << d.get_value(date::day) << "/"
				<< d.get_value(date::year);
			break;
		case german:	// format dd.mm.yyyy
			oss << d.get_value(date::day) << "." << d.get_value(date::month) << "."
				<< d.get_value(date::year);
			break;
	}
	return oss.str();
}
