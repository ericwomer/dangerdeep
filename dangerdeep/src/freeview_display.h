// user display: free 3d view
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef FREEVIEW_DISPLAY_H
#define FREEVIEW_DISPLAY_H

#include "user_display.h"

class freeview_display : public user_display
{

public:
	freeview_display ();
	virtual ~freeview_display ();

	virtual void display(class game& gm) const;
	virtual void process_input(class game& gm, const SDL_Event& event);
};

#endif
