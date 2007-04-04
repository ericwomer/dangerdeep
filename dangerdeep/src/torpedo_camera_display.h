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

// user display: torpedo tracking camera
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef TORPEDO_CAMERA_DISPLAY_H
#define TORPEDO_CAMERA_DISPLAY_H

#include "freeview_display.h"

class torpedo_camera_display : public freeview_display
{
	void pre_display(class game& gm) const;
	projection_data get_projection_data(class game& gm) const;
	void post_display(class game& gm) const;
	vector3 get_viewpos(class game& gm) const;

	mutable const class torpedo* trackobj;

public:
	torpedo_camera_display(class user_interface& ui_);

	virtual unsigned get_popup_allow_mask() const;

	void enter(bool is_day);
	void leave();

	void set_tracker(const class torpedo* t) { trackobj = t; }
};

#endif
