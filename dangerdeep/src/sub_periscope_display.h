// user display: submarine's periscope
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef SUB_PERISCOPE_DISPLAY_H
#define SUB_PERISCOPE_DISPLAY_H

#include "user_display.h"

class sub_periscope_display : public user_display
{

public:
	sub_periscope_display ();
	virtual ~sub_periscope_display ();

	virtual void display(class game& gm) const;
	virtual void process_input(class game& gm, const SDL_Event& event);
};

#endif
