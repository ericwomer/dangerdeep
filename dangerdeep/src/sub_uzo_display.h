// user display: submarine's uzo (U-Boot-Zieloptik = uboat target optics)
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef SUB_UZO_DISPLAY_H
#define SUB_UZO_DISPLAY_H

#include "user_display.h"

class sub_uzo_display : public user_display
{

public:
	sub_uzo_display ();
	virtual ~sub_uzo_display ();

	virtual void display(class game& gm) const;
	virtual void process_input(class game& gm, const SDL_Event& event);
};

#endif
