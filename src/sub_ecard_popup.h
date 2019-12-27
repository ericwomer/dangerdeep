/*
Danger from the Deep - Open source submarine simulation
Copyright (C) 2003-2006  Thorsten Jordan, Luis Barrancos and others.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

// Submarine recognition card (Erkennungskarte or e-card) popup.
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef SUB_ECARD_POPUP_H
#define SUB_ECARD_POPUP_H

#include <memory>
#include "user_popup.h"
#include "image.h"

class sub_ecard_popup : public user_popup
{
protected:
	std::auto_ptr<image> background_daylight;
	std::auto_ptr<image> background_nightlight;

public:
	sub_ecard_popup(class user_interface& ui_);
	virtual ~sub_ecard_popup();

	virtual void display(class game& gm) const;

	virtual bool process_input(class game& gm, const SDL_Event& event);
};

#endif /* SUB_ECARD_POPUP_H */
