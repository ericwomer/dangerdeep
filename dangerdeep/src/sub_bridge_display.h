// user display: submarine's bridge
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef SUB_BRIDGE_DISPLAY_H
#define SUB_BRIDGE_DISPLAY_H

#include "user_display.h"

class sub_bridge_display : public freeview_display // user_display
{
	void pre_display(void) const;
	void get_viewport(unsigned& x, unsigned& y, unsigned& w, unsigned& h) const;
	void set_projection_matrix(void) const;
	void set_modelview_matrix(void) const;
	void post_display(void) const;

public:
	sub_bridge_display ();
	virtual ~sub_bridge_display ();

	virtual void display(class game& gm) const;
	virtual void process_input(class game& gm, const SDL_Event& event);
};

#endif
