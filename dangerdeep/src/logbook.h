// handle log book entries
// subsim (C)+(W) Markus Petermann and Thorsten Jordan. SEE LICENSE

#ifndef LOGBOOK_H
#define LOGBOOK_H

#include <string>
#include <list>
using namespace std;

class logbook
{
protected:
	list<string> entries;

public:
	logbook() {}
	virtual ~logbook () {}
	virtual void add_entry(const string& entry);
	virtual list<string>::const_iterator get_entry(unsigned i) const;
	virtual list<string>::const_iterator begin(void) const { return entries.begin(); }
	virtual list<string>::const_iterator end(void) const { return entries.end(); }
	virtual unsigned size(void) const { return entries.size(); }
};

#endif
