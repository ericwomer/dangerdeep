// user display: submarine's torpedo room
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef SUB_TORPEDO_DISPLAY_H
#define SUB_TORPEDO_DISPLAY_H

#include "user_display.h"

class sub_torpedo_display : public user_display
{

public:
	sub_torpedo_display ();
	virtual ~sub_torpedo_display ();

	virtual void display(class game& gm) const;
	virtual void process_input(class game& gm, const SDL_Event& event);
};

#endif
