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

// user display: submarine's periscope
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "system.h"
#include "image.h"
#include "texture.h"
#include "game.h"
#include "sub_periscope_display.h"
#include "submarine_interface.h"
#include "submarine.h"
#include "keys.h"
#include "cfg.h"
#include "global_data.h"
#include "primitives.h"
#include <iostream>
using std::cout;


void sub_periscope_display::pre_display(game& gm) const
{
	glClear(GL_DEPTH_BUFFER_BIT);
}



freeview_display::projection_data sub_periscope_display::get_projection_data(class game& gm) const
{
	projection_data pd;
	pd.x = 453*sys().get_res_x()/1024;
	pd.y = (768-424-193)*sys().get_res_x()/1024;
	pd.w = 424*sys().get_res_x()/1024;
	pd.h = 424*sys().get_res_x()/1024;
	// with normal fov of 70 degrees, this is 1.5 / 6.0 magnification
	pd.fov_x = zoomed ? 13.31 : 50.05;	//fixme: historic values?
	pd.near_z = 1.0;
	pd.far_z = gm.get_max_view_distance();
	pd.fullscreen = false;
	return pd;
}



vector3 sub_periscope_display::get_viewpos(class game& gm) const
{
	const submarine* sub = dynamic_cast<const submarine*>(gm.get_player());
	return sub->get_pos() + add_pos + vector3(0, 0, 6) * sub->get_scope_raise_level();
}



void sub_periscope_display::set_modelview_matrix(game& gm, const vector3& viewpos) const
{
	glLoadIdentity();

	// set up rotation (player's view direction) - we have no elevation for the periscope.
	// so set standard elevation of 90°
	glRotated(-90.0f,1,0,0);

	// if we're aboard the player's vessel move the world instead of the ship
	if (aboard) {
		// This should be a negative angle, but nautical view dir is clockwise,
		// OpenGL uses ccw values, so this is a double negation
		glRotated(ui.get_relative_bearing().value(),0,0,1);
		gm.get_player()->get_orientation().conj().rotmat4().multiply_gl();
	} else {
		// This should be a negative angle, but nautical view dir is clockwise,
		// OpenGL uses ccw values, so this is a double negation
		glRotated(ui.get_absolute_bearing().value(),0,0,1);
	}

	// set up modelview matrix as if player is at position (0, 0, 0), so do NOT set a translational part.
	// This is done to avoid rounding errors caused by large x/y values (modelview matrix seems to store floats,
	// but coordinates are in real meters, so float is not precise enough).
}



void sub_periscope_display::post_display(game& gm) const
{
	if (use_hqsfx) {
		// here we render scope view as blurred, watery image
		viewtex->set_gl_texture();
		projection_data pd = get_projection_data(gm);
		// copy visible part of viewport to texture
		// fixme: w/h must be powers of 2. here we have 424. could work for newer
		// cards though (non-power-2-tex)
		glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, pd.x, pd.y, pd.w, pd.h, 0);
		// now render texture as 2d image combined with blur texture.
		glPushMatrix();
		glLoadIdentity();
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		glActiveTexture(GL_TEXTURE0);
		viewtex->set_gl_texture();
		glActiveTexture(GL_TEXTURE1);
		blurtex->set_gl_texture();
		// bind shader...
		glsl_blurview->use();
		glsl_blurview->set_gl_texture(*viewtex, loc_tex_view, 0);
		glsl_blurview->set_gl_texture(*blurtex, loc_tex_blur, 1);
		double blur_y_off = myfrac(gm.get_time() / 10.0);
		glsl_blurview->set_uniform(loc_blur_texc_offset, vector3(blur_y_off, 0, 0));
		primitives::textured_quad(vector2f(-1,-1), vector2f(2,2)).render();
		// unbind shader
		glsl_blurview->use_fixed();
		glBindTexture(GL_TEXTURE_2D, 0);
		glActiveTexture(GL_TEXTURE0);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
	}

	if (gm.get_player()->get_target()) {
		projection_data pd = get_projection_data(gm);
		ui.show_target(pd.x, pd.y, pd.w, pd.h, get_viewpos(gm));
	}

	sys().prepare_2d_drawing();
	glColor4f(1,1,1,1);

	// draw compass bar. at most 230 pixel can be seen (of 1878 total width), center is at x=667 on screen
	// so 360*230/1878 = 44.1 degrees can be seen
	// visible area on screen is centerpos +- 115
	// as first translate bearing to pixel pos
	int w = int(compassbar_tex->get_width());
	unsigned h = compassbar_tex->get_height();
	int centerpixelpos = int((ui.get_relative_bearing().value() * w + 0.5) / 360);
	// now draw the bar once or twice. center at x=667. visible area is 667+-115
	if (centerpixelpos <= 115) {
		int xi = 667 - w - centerpixelpos;
		compassbar_tex->draw_subimage(xi, 586, w, h, 0, 0, w, h);
	}
	int xi = 667 - centerpixelpos;
	compassbar_tex->draw_subimage(xi, 586, w, h, 0, 0, w, h);
	if (centerpixelpos > w - 115) {
		int xi = 667 + w - centerpixelpos;
		compassbar_tex->draw_subimage(xi, 586, w, h, 0, 0, w, h);
	}

	// draw background
	background->draw(0, 0);

	// draw clock pointers
	double t = gm.get_time();
	double hourang = 360.0*myfrac(t / (86400/2));
	double minuteang = 360*myfrac(t / 3600);
	clock_hours_pointer->draw_rot(946, 294, hourang);
	clock_minutes_pointer->draw_rot(946, 294, minuteang);

	ui.draw_infopanel(true);
	sys().unprepare_2d_drawing();
}



sub_periscope_display::sub_periscope_display(user_interface& ui_)
	: freeview_display(ui_), zoomed(false), use_hqsfx(false)
{
	add_pos = vector3(0, 0, 8);//fixme, depends on sub
	aboard = true;
	withunderwaterweapons = false;//they can be seen when scope is partly below water surface, fixme
	drawbridge = false;

	use_hqsfx = cfg::instance().getb("use_hqsfx");
	viewtex.reset(new texture(512, 512, GL_RGB, texture::LINEAR, texture::CLAMP_TO_EDGE));
	glsl_blurview.reset(new glsl_shader_setup(get_shader_dir() + "blurview.vshader",
						  get_shader_dir() + "blurview.fshader"));
	glsl_blurview->use();
	loc_blur_texc_offset = glsl_blurview->get_uniform_location("blur_texc_offset");
	loc_tex_view = glsl_blurview->get_uniform_location("tex_view");
	loc_tex_blur = glsl_blurview->get_uniform_location("tex_blur");
	glsl_blurview->use_fixed();
	/* Note 2007/05/08:
	   we can have a better Blur texture if we generate it at runtime.
	   What makes up the blur texture?
	   Water running down over the lens of the periscope.
	   So use one texture, no UV scrolling, just (re)generate it every frame.
	   Make an empty (black) texture, draw a set of drops on it (GL_POINTS
	   of a certain size with a texture that is like a blurmap for a
	   water drop).
	   Simulate each drop (moving down by gravity, adding more drops when
	   periscope collides with water surface).
	   Render the drops to the texture (with textured GL_POINTS).
	   Use the texture to render the final effect.
	   This can be done quick & cheap, but the current scrolling texture
	   is also ok.
	*/
	blurtex.reset(new texture(get_texture_dir() + "blurtest.png", texture::LINEAR, texture::REPEAT));
}



sub_periscope_display::~sub_periscope_display()
{
}



void sub_periscope_display::process_input(class game& gm, const SDL_Event& event)
{
	switch (event.type) {
	case SDL_KEYDOWN:
		if (cfg::instance().getkey(KEY_TOGGLE_ZOOM_OF_VIEW).equal(event.key.keysym)) {
			zoomed = !zoomed;
		} 
                break;
        case SDL_MOUSEBUTTONDOWN:
                if (event.button.button == SDL_BUTTON_WHEELUP) {
                        zoomed = true;
                } else if (event.button.button == SDL_BUTTON_WHEELDOWN) {
                        zoomed = false;
                }
                break;
	case SDL_MOUSEMOTION:
		if (event.motion.state & SDL_BUTTON_LMASK) {
			if (event.motion.yrel != 0) {
				// remove y motion, replace by scope raise/lower code
				submarine* s = dynamic_cast<submarine*>(gm.get_player());
				s->scope_to_level(s->get_scope_raise_level() - event.motion.yrel / 100.0f);
				SDL_Event e = event;
				e.motion.yrel = 0;
				freeview_display::process_input(gm, e);
				return;
			}
		}
		break;
	default: break;
	}
	freeview_display::process_input(gm, event);
}



void sub_periscope_display::display(class game& gm) const
{
	// with new compassbar lower 32 pixel of 3d view are not visible... maybe shrink 3d view? fixme
	//fixme: add specials for underwater rendering here... or in freeview class!
	freeview_display::display(gm);
}



unsigned sub_periscope_display::get_popup_allow_mask() const
{
	return
		(1 << submarine_interface::popup_mode_control) |
		(1 << submarine_interface::popup_mode_tdc) |
		(1 << submarine_interface::popup_mode_ecard) |		
		(1 << submarine_interface::popup_mode_recogmanual);
}



void sub_periscope_display::enter(bool is_day)
{
	if (is_day)
		background.reset(new image(get_image_dir() + "periscope_daylight.jpg|png"));
	else
		background.reset(new image(get_image_dir() + "periscope_redlight.jpg|png"));

	clock_minutes_pointer = texture::ptr(new texture(get_image_dir() + "clock_minutes_pointer.png"));
	clock_hours_pointer = texture::ptr(new texture(get_image_dir() + "clock_hours_pointer.png"));

	compassbar_tex.reset(new texture(get_image_dir() + "periscope_compassbar.png", texture::NEAREST, texture::CLAMP_TO_EDGE));
}



void sub_periscope_display::leave()
{
	background.reset();
	clock_minutes_pointer.reset();
	clock_hours_pointer.reset();
	compassbar_tex.reset();
}
