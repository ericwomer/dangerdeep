// Submarine control popup.
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef SUB_CONTROL_POPUP_H
#define SUB_CONTROL_POPUP_H

#include "user_popup.h"
#include "image.h"

class sub_control_popup : public user_popup
{
protected:
	image::ptr background_daylight;
	image::ptr background_nightlight;

public:
	sub_control_popup(class user_interface& ui_);
	virtual ~sub_control_popup();

	virtual void display(class game& gm) const;

	virtual bool process_input(class game& gm, const SDL_Event& event);
};

#endif /* SUB_CONTROL_POPUP_H */
