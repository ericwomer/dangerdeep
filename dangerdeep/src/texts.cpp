// texts
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

//#include "texts.h"



unsigned language = 0;

#define DEFTYPE
#define content(a, b) ={a, b}

#define TEXTS_H_USECPP
#include "texts.h"



// new code

#include "parser.h"
#include "global_data.h"

#define TEXTS_DIR "texts/"

static char* textfilenames[] = {
	"english",
	"german"
};

vector<string> texts::txts;

void texts::set_language(languages l)
{
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

