// Object to create and display logbook entries.
// subsim (C)+(W) Markus Petermann and Thorsten Jordan. SEE LICENSE

#ifndef LOGBOOK_H
#define LOGBOOK_H

#include <string>
using namespace std;
#include "user_display.h"
#include "date.h"

//
// Class logbook
//
class logbook
{
public:
	typedef map<unsigned, string> logbook_entry_map;
	typedef logbook_entry_map::iterator logbook_entry_map_iterator;
	typedef logbook_entry_map::const_iterator logbook_entry_map_const_iterator;

private:
	logbook_entry_map logbook_entries;

public:
	logbook();
	virtual ~logbook ();
	virtual void add_entry ( const string& entry )
	{ logbook_entries[logbook_entries.size ()] = entry; }
	friend ostream& operator << ( ostream& os, const logbook& lb );
	virtual string get_entry ( unsigned i ) const {
		logbook_entry_map_const_iterator it = logbook_entries.find(i);
		if (it == logbook_entries.end()) return "";
		else return it->second; }
	virtual unsigned size () const { return logbook_entries.size (); }
	virtual void remove_entry ( unsigned i );
};

ostream& operator << ( ostream& os, const logbook& lb );



//
// Class logbook_display
//
class logbook_display : public user_display
{
protected:
	logbook lb;
	unsigned actual_entry;

	virtual void print_buffer ( unsigned i, const string& t ) const;
	static void format_line ( list<string>& entries_list, const string& line );

public:
	virtual ~logbook_display() {}
	virtual void display ( class game& gm ) const = 0;
	virtual void process_input(class game& gm, const SDL_Event& event) = 0;
	virtual void add_entry ( const date& d, const string& entry );
	virtual void next_page ();
	virtual void previous_page ();
};



//
// Class captains_logbook_display
//
class captains_logbook_display : public logbook_display
{
	void init ();

public:
	captains_logbook_display();
	virtual ~captains_logbook_display () {};
	virtual void display(class game& gm) const;
	virtual void process_input(class game& gm, const SDL_Event& event);
};

#endif /* LOGBOOK_H_ */
