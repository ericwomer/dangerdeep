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

// user display: submarine's UZO
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "system.h"
#include "image.h"
#include "texture.h"
#include "game.h"
#include "torpedo.h"
#include "torpedo_camera_display.h"
#include "submarine_interface.h"



void torpedo_camera_display::pre_display(game& gm) const
{
	if (!trackobj)
		return;
	if (!trackobj->is_reference_ok()) {
		trackobj = 0;
		return;
	}
	glClear(GL_DEPTH_BUFFER_BIT);
}



freeview_display::projection_data torpedo_camera_display::get_projection_data(class game& gm) const
{
	projection_data pd;
	pd.x = sys().get_res_x() * 3 / 4;
       	pd.y = 0;
	pd.w = sys().get_res_x() / 4;
	pd.h = sys().get_res_y() / 4;
	pd.fov_x = 70.0;
	pd.near_z = 1.0;
	pd.far_z = gm.get_max_view_distance();
	pd.fullscreen = false;
	return pd;
}



void torpedo_camera_display::post_display(game& gm) const
{
	if (!trackobj)
		return;
	// nothing to do
}



vector3 torpedo_camera_display::get_viewpos(class game& gm) const
{
	if (trackobj)
		return trackobj->get_pos() + add_pos;
	return vector3();
}



torpedo_camera_display::torpedo_camera_display(user_interface& ui_) : freeview_display(ui_), trackobj(0)
{
	add_pos = vector3(0, 0, 0.5); // on the back of the torpedo like riding a whale...
	aboard = true; // ?
	withunderwaterweapons = true;
	drawbridge = false;
}



unsigned torpedo_camera_display::get_popup_allow_mask() const
{
	return 0;
}



void torpedo_camera_display::enter(bool is_day)
{
}



void torpedo_camera_display::leave()
{
}
