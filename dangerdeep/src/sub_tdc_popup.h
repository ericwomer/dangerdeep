// Submarine tdc popup.
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef SUB_TDC_POPUP_H
#define SUB_TDC_POPUP_H

#include "user_popup.h"
#include "image.h"

class sub_tdc_popup : public user_popup
{
protected:
	image::ptr background_daylight;
	image::ptr background_nightlight;

public:
	sub_tdc_popup(class user_interface& ui_);
	virtual ~sub_tdc_popup();

	virtual void display(class game& gm) const;

	virtual bool process_input(class game& gm, const SDL_Event& event);
};

#endif /* SUB_TDC_POPUP_H */
