// logbook
// subsim (C)+(W) Markus Petermann and Thorsten Jordan. SEE LICENSE

#include <map>
#include <iostream>
#include <sstream>
using namespace std;
#include "date.h"
#include "global_data.h"
#include "logbook.h"



void logbook::add_entry(const string& entry)
{
	entries.push_back(entry);
}



list<string>::const_iterator logbook::get_entry(unsigned i) const
{
	list<string>::const_iterator it = entries.begin();
	while (i > 0 && it != entries.end()) {
		--i;
		++it;
	}
	return it;
}
