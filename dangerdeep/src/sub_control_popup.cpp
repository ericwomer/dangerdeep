// Submarine control popup.
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "sub_control_popup.h"
#include "system.h"
#include "global_data.h"
#include "game.h"
#include "keys.h"
#include "cfg.h"



sub_control_popup::sub_control_popup(user_interface& ui_) : user_popup(ui_)
{
	x = 8;
	y = 134;
	background_daylight = image::ptr(new image(get_image_dir() + "popup_TDC_daylight.png"));
	background_nightlight = image::ptr(new image(get_image_dir() + "popup_TDC_redlight.png"));
}



sub_control_popup::~sub_control_popup()
{
}



bool sub_control_popup::process_input(class game& gm, const SDL_Event& event)
{
	switch (event.type) {
	default: return false;
	}
	return false;
}



void sub_control_popup::display(class game& gm) const
{
	sys().prepare_2d_drawing();
	glColor3f(1,1,1);

	bool is_day = gm.is_day_mode();
	if (is_day)
		background_daylight->draw(x, y);
	else
		background_nightlight->draw(x, y);
	sys().unprepare_2d_drawing();
}
