// user display: submarine's bridge
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef SUB_BRIDGE_DISPLAY_H
#define SUB_BRIDGE_DISPLAY_H

#include "freeview_display.h"

class sub_bridge_display : public freeview_display
{
	void pre_display(class game& gm) const;
	projection_data get_projection_data(class game& gm) const;
	void post_display(class game& gm) const;

	class texture* glasses_tex;

	bool glasses_in_use;

public:
	sub_bridge_display(class user_interface& ui_);
	virtual ~sub_bridge_display();

	//overload for glasses key handling ('y')
	virtual void process_input(class game& gm, const SDL_Event& event);
};

#endif
