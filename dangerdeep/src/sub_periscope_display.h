// user display: submarine's periscope
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef SUB_PERISCOPE_DISPLAY_H
#define SUB_PERISCOPE_DISPLAY_H

#include "freeview_display.h"
#include "image.h"
#include <vector>

class sub_periscope_display : public freeview_display
{
	void pre_display(class game& gm) const;
	projection_data get_projection_data(class game& gm) const;
	void post_display(class game& gm) const;

	image::ptr background_normallight;
	image::ptr background_nightlight;

	vector<class texture*> compassbar_tex;
	vector<unsigned> compassbar_width;

	texture::ptr clock_hours_pointer;
	texture::ptr clock_minutes_pointer;
	
	bool zoomed;	// use 1,5x (false) or 6x zoom (true)

public:
	sub_periscope_display(class user_interface& ui_);
	virtual ~sub_periscope_display();

	//overload for zoom key handling ('y') and TDC input
	virtual void process_input(class game& gm, const SDL_Event& event);
	virtual void display(class game& gm) const;

	virtual unsigned get_popup_allow_mask(void) const;
};

#endif
