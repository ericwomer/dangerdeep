// Object to create and display logbook entries.
// subsim (C)+(W) Markus Petermann and Thorsten Jordan. SEE LICENSE

#ifndef LOGBOOK_DISPLAY_H
#define LOGBOOK_DISPLAY_H

#include "user_display.h"

class logbook_display : public user_display
{
protected:
	unsigned actual_entry;

	void print_buffer(unsigned i, const string& t) const;
	virtual void next_page(unsigned nrentries);
	virtual void previous_page(unsigned nrentries);

public:
	logbook_display(class user_interface& ui_) : user_display(ui_), actual_entry(0) {}
	virtual ~logbook_display() {}
	virtual void display(class game& gm) const;
	virtual void process_input(class game& gm, const SDL_Event& event);
};

#endif
