// Submarine tdc popup.
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "sub_tdc_popup.h"
#include "system.h"
#include "global_data.h"
#include "game.h"
#include "keys.h"
#include "cfg.h"



sub_tdc_popup::sub_tdc_popup(user_interface& ui_) : user_popup(ui_)
{
	x = 9;
	y = 151;
	background_daylight.reset(new image(get_image_dir() + "popup_control_daylight.jpg|png"));
	background_nightlight.reset(new image(get_image_dir() + "popup_control_redlight.jpg|png"));
}



sub_tdc_popup::~sub_tdc_popup()
{
}



bool sub_tdc_popup::process_input(class game& gm, const SDL_Event& event)
{
	switch (event.type) {
	default: return false;
	}
	return false;
}



void sub_tdc_popup::display(class game& gm) const
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
