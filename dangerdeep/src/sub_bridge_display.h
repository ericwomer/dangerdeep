// user display: submarine's bridge
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef SUB_BRIDGE_DISPLAY_H
#define SUB_BRIDGE_DISPLAY_H

#include "user_display.h"

class sub_bridge_display : public user_display
{

public:
	sub_bridge_display ();
	virtual ~sub_bridge_display ();

	virtual void display(class game& gm) const;
	virtual void process_input(class game& gm, const SDL_Event& event);
};

#endif
