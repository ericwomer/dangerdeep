// Object to create and display the number and tonnage of sunk ships.
// subsim (C)+(W) Markus Petermann and Thorsten Jordan. SEE LICENSE

#ifndef SHIPS_SUNK_DISPLAY_H
#define SHIPS_SUNK_DISPLAY_H

#include "user_display.h"

class ships_sunk_display : public user_display
{
protected:
	unsigned first_displayed_object;
	virtual void next_page(unsigned nrships);
	virtual void previous_page(unsigned nrships);
public:
	ships_sunk_display(class user_interface& ui_);
	virtual ~ships_sunk_display ();

	virtual void display(class game& gm) const;
	virtual void process_input(class game& gm, const SDL_Event& event);
};

#endif
