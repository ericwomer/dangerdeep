// user display: submarine's bridge
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef SUB_BRIDGE_DISPLAY_H
#define SUB_BRIDGE_DISPLAY_H

#include "user_display.h"

class sub_bridge_display : public freeview_display // user_display
{
	void prepare_display(void) const;
	void set_projection_matrix_and_viewport(void) const;
	void set_modelview_matrix(void) const;

public:
	sub_bridge_display ();
	virtual ~sub_bridge_display ();

	virtual void display(class game& gm) const;
	virtual void process_input(class game& gm, const SDL_Event& event);
};

#endif
