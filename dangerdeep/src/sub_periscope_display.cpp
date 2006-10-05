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
	return pd;
}



void sub_periscope_display::post_display(game& gm) const
{
	if (ui.get_target()) {
		projection_data pd = get_projection_data(gm);
		ui.show_target(pd.x, pd.y, pd.w, pd.h);
	}

	sys().prepare_2d_drawing();
	glColor4f(1,1,1,1);

	// draw compass bar. at most 230 pixel can be seen (of 1878 total width), center is at x=667 on screen
	// so 360*230/1878 = 44.1 degrees can be seen
	// 230 is less than the size of one part of the bar (width 235, last 233), so draw at most two
	// visible area on screen is centerpos +- 115
	// as first translate bearing to pixel pos
	int centerpixelpos = int((ui.get_relative_bearing().value() * 1878 + 0.5) / 360);
	// now compute which parts can be seen
	// draw part i at x_i = 667+widths[0...i-1]-centerpixelpos
	// if x_i >= 667-115 and x_i < 667+115 then draw it
	unsigned ws = compassbar_width.size();
	for (int i = 0, cw = 0; i < int(ws); ++i) {
		int xi = 667 + cw - centerpixelpos;
		int w = int(compassbar_width[i]);
		// check 360->0 wrap
		if (xi + w < 667-115)
			xi += 1878;
		// check 360->0 wrap other side
		if (xi >= 667+115)
			xi -= 1878;
		// check if part can be seen
		if (667-115 < xi + w && xi < 667+115) {
			unsigned h = compassbar_tex[i]->get_height();
			compassbar_tex[i]->draw_subimage(xi, 586, w, h, 0, 0, w, h);
		}
		cw += w;
	}

	// draw background
	bool is_day = gm.is_day_mode();
	if (is_day) {
		background_normallight->draw(0, 0);
	} else {
		background_nightlight->draw(0, 0);
	}

	// draw clock pointers
	double t = gm.get_time();
	double hourang = 360.0*myfrac(t / (86400/2));
	double minuteang = 360*myfrac(t / 3600);
	clock_hours_pointer->draw_rot(946, 294, hourang);
	clock_minutes_pointer->draw_rot(946, 294, minuteang);

	ui.draw_infopanel(true);
	sys().unprepare_2d_drawing();
}



sub_periscope_display::sub_periscope_display(user_interface& ui_) : freeview_display(ui_), zoomed(false)
{
	background_normallight.reset(new image(get_image_dir() + "periscope_daylight.jpg|png"));
	background_nightlight.reset(new image(get_image_dir() + "periscope_redlight.jpg|png"));

	clock_minutes_pointer = texture::ptr(new texture(get_image_dir() + "clock_minutes_pointer.png"));
	clock_hours_pointer = texture::ptr(new texture(get_image_dir() + "clock_hours_pointer.png"));

	// read compass bar image and cut to eight separate textures
	image compassbari(get_image_dir() + "periscope_compassbar.png");
	const unsigned nrparts = 8;	// 1878/8 ~ 234 so it fits in a 256 texture
	unsigned tw = compassbari.get_width();
	unsigned pw = (tw + (nrparts/2)) / nrparts;
	compassbar_width.resize(nrparts);
	compassbar_tex.resize(nrparts);
	for (unsigned i = 0; i < nrparts; ++i) {
		compassbar_width[i] = (i + 1 < nrparts) ? pw : tw - (nrparts-1)*pw;
		compassbar_tex[i] = new texture(compassbari.get_SDL_Surface(), i*pw, 0,
						compassbar_width[i], compassbari.get_height(),
						texture::NEAREST, texture::CLAMP_TO_EDGE);
	}

	pos = vector3(0, 0, 12);//fixme, depends on sub
	aboard = true;
	withunderwaterweapons = false;//they can be seen when scope is partly below water surface, fixme
	drawbridge = false;
}



sub_periscope_display::~sub_periscope_display()
{
	for (unsigned i = 0; i < compassbar_tex.size(); ++i)
		delete compassbar_tex[i];
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
	default: break;
	}
	freeview_display::process_input(gm, event);
}



void sub_periscope_display::display(class game& gm) const
{
	submarine* player = dynamic_cast<submarine*> ( gm.get_player() );

	// with new compassbar lower 32 pixel of 3d view are not visible... maybe shrink 3d view? fixme
	
	// if the periscope is down draw nothing (all black)
	if (true == player->is_scope_up())
		freeview_display::display(gm);
	else
	{
		glClearColor(0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT);
		post_display(gm);
	}
}



unsigned sub_periscope_display::get_popup_allow_mask() const
{
	return
		(1 << submarine_interface::popup_mode_control) |
		(1 << submarine_interface::popup_mode_tdc) |
		(1 << submarine_interface::popup_mode_ecard);
}



#if 0 //old

void submarine_interface::display_periscope(game& gm)
{
	submarine* player = dynamic_cast<submarine*> ( get_player() );

	glClear(GL_DEPTH_BUFFER_BIT);

	unsigned res_x = sys().get_res_x(), res_y = sys().get_res_y();

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	// scope zoom modes are x1,5 and x6,0. This is meant as width of image.
	// E.g. normal width of view is tan(fov/2)*distance.
	// with arbitrary zoom z we have (fov = normal fov angle, newfov = zoom fov angle)
	// z*tan(newfov/2)*distance = tan(fov/2)*distance =>
	// newfov = 2*atan(tan(fov/2)/z).
	// Our values: fov = 90deg, z = 1.5;6.0, newfov = 67.380;18.925
	// or fov = 70deg, newfov = 50.05;13.31
	double fov = 67.380f;

	if ( zoom_scope )
		fov = 18.925f;
	
	sys().gl_perspective_fovx (fov, 1.0/1.0, 5.0, gm.get_max_view_distance());
	glViewport(res_x/2, res_y/3, res_x/2, res_x/2);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glRotatef(-90, 1,0,0);	// swap y and z coordinates (camera looks at +y axis)

	vector3 viewpos = player->get_pos() + vector3(0, 0, 12+14);//fixme: +14 to be above waves ?!
	// no torpedoes, no DCs, no player
	draw_view(gm, viewpos, res_x/2, res_y/3, res_x/2, res_x/2, ui.get_absolute_bearing(), 0, true, false, false);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glViewport(0, 0, res_x, res_y);
	glMatrixMode(GL_MODELVIEW);
	
	sys().prepare_2d_drawing();
	set_display_color ( gm );
	for (int x = 0; x < 3; ++x)
		psbackgr->draw(x*256, 512, 256, 256);
	periscope->draw(2*256, 0);
	addleadangle->draw(768, 512, 256, 256);

	// Draw lead angle value.
	//interface changed, code is obsolete anyway
	double la = player->get_trp_addleadangle().value();

	if ( la > 180.0f )
		la -= 360.0f;

	int lax = 896 + int ( 10.8f * la );

	glBindTexture(GL_TEXTURE_2D, 0);
	glBegin ( GL_TRIANGLE_STRIP );
	glColor3f ( 1.0f, 0.0f, 0.0f );
	glVertex2i ( lax-1, 522 );
	glVertex2i ( lax-1, 550 );
	glVertex2i ( lax+1, 522 );
	glVertex2i ( lax+1, 550 );
	glEnd ();

	angle targetbearing;
	angle targetaob;
	angle targetrange;
	angle targetspeed;
	angle targetheading;
	if (target) {
		pair<angle, double> br = player->bearing_and_range_to(target);
		targetbearing = br.first;
		targetaob = player->estimate_angle_on_the_bow(br.first, target->get_heading());
		unsigned r = unsigned(round(br.second));
		if (r > 9999) r = 9999;
		targetrange = r*360.0/9000.0;
		targetspeed = target->get_speed()*360.0/sea_object::kts2ms(36);
		targetheading = target->get_heading();
	}
	draw_gauge(gm, 1, 0, 0, 256, targetbearing, texts::get(12));
	draw_gauge(gm, 3, 256, 0, 256, targetrange, texts::get(13));
	draw_gauge(gm, 2, 0, 256, 256, targetspeed, texts::get(14));
	draw_gauge(gm, 1, 256, 256, 256, targetheading, texts::get(15));
	const vector<submarine::stored_torpedo>& torpedoes = player->get_torpedoes();
	pair<unsigned, unsigned> bow_tube_indices = player->get_bow_tube_indices();
	pair<unsigned, unsigned> stern_tube_indices = player->get_stern_tube_indices();
	for (unsigned i = bow_tube_indices.first; i < bow_tube_indices.second; ++i) {
		int j = i-bow_tube_indices.first;
		draw_torpedo(gm, true, (j/4)*128, 512+(j%4)*16, torpedoes[i]);
	}
	for (unsigned i = stern_tube_indices.first; i < stern_tube_indices.second; ++i) {
		draw_torpedo(gm, false, 256, 512+(i-stern_tube_indices.first)*16, torpedoes[i]);
	}
	glColor3f(1,1,1);
	draw_infopanel(gm);
	sys().unprepare_2d_drawing();

	// mouse handling
	int mx;
	int my;
	int mb = sys().get_mouse_buttons();
	sys().get_mouse_position(mx, my);

	if (mb & sys().left_button) {
		// Evaluate lead angle box.
		if ( mx >= 776 && mx <= 1016 && my >= 520 && my <= 552 )
		{
			double lav = double ( mx - 896 ) / 10.8f;
			if ( lav < - 10.0f )
				lav = -10.0f;
			else if ( lav > 10.0f )
				lav = 10.0f;

			player->trp_setup(dynamic_cast<submarine_interface*>(ui)->get_selected_tube()).addleadangle = lav;
		}
	}

	// keyboard processing
	int key = sys().get_key().sym;
	while (key != 0) {
		if (!keyboard_common(key, gm)) {
			// specific keyboard processing
			switch ( key ) {
				case SDLK_y:
					if ( zoom_scope )
						zoom_scope = false;
					else
						zoom_scope = true;
				break;				
			}
		}
		key = sys().get_key().sym;
	}
}

#endif
