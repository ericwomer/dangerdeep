// user display: general map view
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef MAP_DISPLAY_H
#define MAP_DISPLAY_H

#include "user_display.h"

class map_display : public user_display
{

public:
	map_display ();
	virtual ~map_display ();

	virtual void display(class game& gm) const;
	virtual void process_input(class game& gm, const SDL_Event& event);
};

#endif
