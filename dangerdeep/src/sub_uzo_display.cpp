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
#include "submarine.h"
#include "sub_uzo_display.h"
#include "submarine_interface.h"
#include "keys.h"
#include "cfg.h"



void sub_uzo_display::pre_display(game& gm) const
{
	glClear(GL_DEPTH_BUFFER_BIT);
}



freeview_display::projection_data sub_uzo_display::get_projection_data(class game& gm) const
{
	projection_data pd;
	pd.x = 0;
       	pd.y = 0;
	pd.w = sys().get_res_x();
	pd.h = sys().get_res_y();
	// with normal fov of 70 degrees, this is 1.5 / 6.0 magnification
	pd.fov_x = zoomed ? 13.31 : 50.05;	//fixme: historic values?
	pd.near_z = 1.0;
	pd.far_z = gm.get_max_view_distance();
	pd.fullscreen = true;
	return pd;
}



void sub_uzo_display::set_modelview_matrix(game& gm, const vector3& viewpos) const
{
	glLoadIdentity();

	// set up rotation (player's view direction)
	// limit elevation to -20...20 degrees.
	float elev = -ui.get_elevation().value();
	const float LIMIT = 20.0f;
	if (elev < -LIMIT - 90.0f) elev = -LIMIT - 90.0f;
	if (elev > +LIMIT - 90.0f) elev = +LIMIT - 90.0f;
	glRotated(elev,1,0,0);

	// This should be a negative angle, but nautical view dir is clockwise,
	// OpenGL uses ccw values, so this is a double negation
	glRotated(ui.get_absolute_bearing().value(),0,0,1);

	// if we're aboard the player's vessel move the world instead of the ship
	if (aboard) {
		const sea_object* pl = gm.get_player();
		double rollfac = (dynamic_cast<const ship*>(pl))->get_roll_factor();
		ui.rotate_by_pos_and_wave(pl->get_pos(), pl->get_heading(),
					  pl->get_length(), pl->get_width(), rollfac, true);
	}

	// set up modelview matrix as if player is at position (0, 0, 0), so do NOT set a translational part.
	// This is done to avoid rounding errors caused by large x/y values (modelview matrix seems to store floats,
	// but coordinates are in real meters, so float is not precise enough).
}



void sub_uzo_display::post_display(game& gm) const
{
	if (gm.get_player()->get_target()) {
		projection_data pd = get_projection_data(gm);
		ui.show_target(pd.x, pd.y, pd.w, pd.h, get_viewpos(gm));
	}

	sys().prepare_2d_drawing();
	
	int tex_w = compass->get_width();
	int tex_h = compass->get_height();

	int bearing = int(tex_w*ui.get_relative_bearing().value()/360);

	if( bearing>dx && bearing<tex_w-dx){
		compass->draw_subimage(xi, yi, comp_size, tex_h, bearing-dx, 0, comp_size, tex_h);
	} else {
		int dx1=0,dx2=0;
		if( bearing<dx ){ dx1=dx-bearing; dx2=dx+bearing; } else if( bearing>tex_w-dx ){ dx1=dx+(tex_w-bearing); dx2=comp_size-dx; }
		compass->draw_subimage(xi, yi, dx1, tex_h, tex_w-(dx1), 0, dx1, tex_h);
		compass->draw_subimage(xi+dx1, yi, dx2, tex_h, 0, 0, dx2, tex_h );
	}
	
	uzotex->draw(0, 0, 1024, 768);
	ui.draw_infopanel(true);

	sys().unprepare_2d_drawing();
}



sub_uzo_display::sub_uzo_display(user_interface& ui_) : freeview_display(ui_), zoomed(false)
{
	add_pos = vector3(0, 0, 6);//fixme, depends on sub
	aboard = true;
	withunderwaterweapons = false;
	drawbridge = false;
}



void sub_uzo_display::process_input(class game& gm, const SDL_Event& event)
{
	submarine* sub = dynamic_cast<submarine*>(gm.get_player());
	if (sub->is_submerged()) {
	}
	switch (event.type) {
	case SDL_KEYDOWN:
		if (cfg::instance().getkey(KEY_TOGGLE_ZOOM_OF_VIEW).equal(event.key.keysym)) {
			zoomed = !zoomed;
		} else {
			switch(event.key.keysym.sym) {
				// filter away keys NP_1...NP_9 to avoid moving viewer like in freeview mode
			case SDLK_KP8: return;
			case SDLK_KP2: return;
			case SDLK_KP4: return;
			case SDLK_KP6: return;
			case SDLK_KP1: return;
			case SDLK_KP3: return;
			default: break;
			}
		}
                break;
        case SDL_MOUSEBUTTONDOWN:
                if (event.button.button == SDL_BUTTON_WHEELUP) {
                        zoomed = true;
                } else if (event.button.button == SDL_BUTTON_WHEELDOWN) {
                        zoomed = false;
                }
                break;
	default: break;
	}
	freeview_display::process_input(gm, event);
}



unsigned sub_uzo_display::get_popup_allow_mask() const
{
	return
		(1 << submarine_interface::popup_mode_ecard);
}



void sub_uzo_display::enter(bool is_day)
{
	uzotex.reset(new texture(get_texture_dir() + "uzo.png"));
	if (is_day)
		compass.reset(new texture(get_texture_dir() + "uzo_compass_daylight.png"));
	else
		compass.reset(new texture(get_texture_dir() + "uzo_compass_redlight.png"));
	
	comp_size = int(compass->get_width()/3.6);
	dx = int(comp_size*0.5);
	xi = 512-dx;
	yi=705;
}



void sub_uzo_display::leave()
{
	uzotex.reset();
	compass.reset();
}
