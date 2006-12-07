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

// user display: submarine's tdc
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "system.h"
#include "image.h"
#include "texture.h"
#include "game.h"
#include "sub_tdc_display.h"
#include "submarine_interface.h"
#include "submarine.h"
#include "keys.h"
#include "cfg.h"
#include "global_data.h"
#include <sstream>

using std::vector;
using std::list;
using std::string;
using std::ostringstream;

static const int tubelightcoordx[6] = {
	 81,
	203,
	324,
	446,
	569,
	693,
};



sub_tdc_display::scheme_screen1::scheme_screen1(bool day)
{
	const string x = day ? "TDCScreen1_Daylight" : "TDCScreen1_Redlight";
	background.reset(new image(get_image_dir() + x + "_Base_image.jpg|png"));
	aob_ptr.set(x + "_Lagenwinkel_pointer_rotating.png", 484, 426, 512, 551);
	aob_inner.set(x + "_AngleOnBow_innerdial_rotating.png", 417, 456, 512, 550);
	spread_ang_ptr.set(x + "_Facherwinkel_pointer_rotating.png", 799, 502, 846, 527);
	spread_ang_mkr.set(x + "_Facherwinkel_usermarker_rotating.png", 950, 512, 846, 527);
	firesolution.reset(new texture(get_image_dir() + x + "_firesolution_slidingscale_pointer.png"));
	parallax_ptr.set(x + "_parallaxwinkel_pointer_rotating.png", 820, 104, 846, 201);
	parallax_mkr.set(x + "_parallaxwinkel_usermarker_rotating.png", 952, 186, 846, 201);
	torptime_min.set(x + "_TorpedoLaufzeit_pointer_minutes_rotating.png", 175, 484, 195, 563);
	torptime_sec.set(x + "_TorpedoLaufzeit_pointer_seconds_rotating.png", 170, 465, 195, 563);
	torp_speed.set(x + "_TorpGeschwindigkeit_innerdial_rotating.png", 406, 83, 512, 187);
	target_pos.set(x + "_Zielposition_pointer_rotating.png", 158, 86, 183, 183);
	target_speed.set(x + "_ZielTorpGeschwindigkeit_pointer_rotating.png", 484, 62, 511, 187);
}



sub_tdc_display::scheme_screen2::scheme_screen2(bool day)
{
	const string x = day ? "TDCScreen2_Daylight" : "TDCScreen2_Redlight";
	background.reset(new image(get_image_dir() + x + "_base_image.jpg"));
	for (unsigned i = 0; i < 6; ++i) {
		tubelight[i].set(x + "_tube" + str(i+1) + "_on.png", tubelightcoordx[i], 605);
	}
	firebutton.set(x + "_FireButton_ON.png", 68, 92);
	automode[0].set(x + "_AutoManualKnob_automode.png", 900, 285);
	automode[1].set(x + "_AutoManualKnob_manualmode.png", 900, 285);
	gyro_360.set(x + "_HGyro_pointer_main_rotating.png", 383, 406, 431, 455);
	gyro_10.set(x + "_HGyro_pointer_refinement_rotating.png", 188, 378, 212, 455);
	brightness.set(x + "_Brightness_dial_pointer_rotating.png", 897, 478, 911, 526);
	target_course_360.set(x + "_VGyro_pointer_main_rotating.png", 695, 373, 721, 453);
	target_course_10.set(x + "_VGyro_pointer_refinement_rotating.png", 696, 152, 721, 233);
	target_range_ptr.set(x + "_Zielentfernung_pointer_rotating.png", 317, 98, 341, 194);
	target_range_mkr.set(x + "_Zielentfernung_user_marker_rotating.png", 325, 295, 341, 194);
}



sub_tdc_display::sub_tdc_display(user_interface& ui_)
	: user_display(ui_), show_screen1(true)
{
}



void sub_tdc_display::process_input(class game& gm, const SDL_Event& event)
{
	submarine* sub = dynamic_cast<submarine*>(gm.get_player());
	int mx, my;
	submarine_interface& si = dynamic_cast<submarine_interface&>(ui);
	tdc& TDC = sub->get_tdc();

	if (show_screen1) {
		if (!myscheme1.get()) throw error("sub_tdc_display::process_input without scheme!");
		const scheme_screen1& s = *myscheme1;

		switch (event.type) {
		case SDL_MOUSEBUTTONDOWN:
			{
				mx = event.button.x;
				my = event.button.y;
				// check if mouse is over parallax display
				int parasz = s.parallax_ptr.centery - s.parallax_ptr.top + 20;
				if (mx >= s.parallax_ptr.centerx - parasz
				    && mx <= s.parallax_ptr.centerx + parasz
				    && my >= s.parallax_ptr.centery - parasz
				    && my <= s.parallax_ptr.centery + parasz) {
					angle userang(vector2(mx - s.parallax_ptr.centerx, -my + s.parallax_ptr.centery));
					double usera = userang.value_pm180() / 6;
					if (usera < -25) usera = -25;
					if (usera > 25) usera = 25;
					TDC.set_additional_parallaxangle(usera);
				}

			}
			break;
		case SDL_MOUSEMOTION:
			if (event.motion.state & SDL_BUTTON_LMASK) {
				mx = event.motion.x;
				my = event.motion.y;
				// check if mouse is over parallax display, fixme: same code as above, group it!
				int parasz = s.parallax_ptr.centery - s.parallax_ptr.top + 20;
				if (mx >= s.parallax_ptr.centerx - parasz
				    && mx <= s.parallax_ptr.centerx + parasz
				    && my >= s.parallax_ptr.centery - parasz
				    && my <= s.parallax_ptr.centery + parasz) {
					angle userang(vector2(mx - s.parallax_ptr.centerx, -my + s.parallax_ptr.centery));
					double usera = userang.value_pm180() / 6;
					if (usera < -25) usera = -25;
					if (usera > 25) usera = 25;
					TDC.set_additional_parallaxangle(usera);
				}
			}
			break;
		default:
			break;
		}

	} else {
		if (!myscheme2.get()) throw error("sub_tdc_display::process_input without scheme!");
		const scheme_screen2& s = *myscheme2;

		switch (event.type) {
		case SDL_MOUSEBUTTONDOWN:
			{
				mx = event.button.x;
				my = event.button.y;
				// check if mouse is over tube indicators
				unsigned nrtubes = sub->get_nr_of_bow_tubes() + sub->get_nr_of_stern_tubes();
				for (unsigned i = 0; i < nrtubes; ++i) {
					if (s.tubelight[i].is_mouse_over(mx, my)) {
						si.select_tube(i);
					}
				}

				// fire button
				if (s.firebutton.is_mouse_over(mx, my)) {
					si.fire_tube(sub, si.get_selected_tube());
				}

				// auto mode
				else if (s.automode[0].is_mouse_over(mx, my)) {
					TDC.set_auto_mode(!TDC.auto_mode_enabled());
				}
			}
			break;
		default:
			break;
		}
	}

/*
	switch (event.type) {
	case SDL_KEYDOWN:
		//fixme
	default: break;
	}
*/

	if (event.type == SDL_KEYDOWN) {
		if (cfg::instance().getkey(KEY_TOGGLE_POPUP).equal(event.key.keysym)) {
			next_sub_screen(gm.is_day_mode());
		}
	}
}



void sub_tdc_display::display(class game& gm) const
{
	submarine* player = dynamic_cast<submarine*>(gm.get_player());

	sys().prepare_2d_drawing();
	glColor3f(1,1,1);

	const tdc& TDC = player->get_tdc();

	if (show_screen1) {
		if (!myscheme1.get()) throw error("sub_tdc_display::display without scheme!");
		const scheme_screen1& s = *myscheme1;

		// draw torpedo speed dial (15deg = 0, 5knots = 30deg)
		// torpedo speed (depends on selected tube!), but TDC is already set accordingly
		s.torp_speed.draw(sea_object::ms2kts(TDC.get_torpedo_speed()) * 330.0/55 + 15);

		// angle on the bow finer value, note use real fmod here...
		s.aob_inner.draw(fmod(TDC.get_angle_on_the_bow().value_pm180(), 10.0) * -36.0);

		// background
		s.background->draw(0, 0);

		// angle on the bow coarse value
		s.aob_ptr.draw(TDC.get_angle_on_the_bow().value_pm180());

		// spread angle, fixme: add. lead angle is not right...
		// this means angle of spread when firing multiple torpedoes... this has to be (re)defined
		// the captain could fake additional lead angle by manipulating bearing etc.
		// this should be done to compensate ship turning or zig-zagging
		s.spread_ang_ptr.draw(0.0 /*TDC.get_spread_angle().value()*/ /20 * 180.0 - 90);
		s.spread_ang_mkr.draw(15.0/*TDC.get_user_spread_angle().value()*/ /20 * 180.0 - 90);	//fixme

		// fire solution quality
		double quality = 0.333; // per cent, fixme, request from sub! depends on crew
		s.firesolution->draw(268 - int(187*quality + 0.5), 418);
	
		// parallax angle (fixme: why should the user set an angle? extra-correction here? is like
		// additional lead angle...)
		// 6 pointer degrees for 1 real degree, marker - 90
		double parang = TDC.get_parallax_angle().value_pm180();
		// clamp value (maybe tweak value so that pointer shakes when reaching the limit?)
		if (parang < -26) parang = -26;
		if (parang > 26) parang = 26;
		s.parallax_ptr.draw(parang * 6);
		s.parallax_mkr.draw(TDC.get_additional_parallaxangle().value_pm180() * 6 - 90);

		// torpedo run time
		double t = TDC.get_torpedo_runtime();
		s.torptime_sec.draw(myfmod(t, 60) * 6);
		s.torptime_min.draw(myfmod(t, 3600) * 0.1);

		// target bearing (influenced by quality!)
		s.target_pos.draw((TDC.get_bearing() - player->get_heading()).value());
		
		// target speed
		s.target_speed.draw(15 + sea_object::ms2kts(TDC.get_target_speed()) * 330.0/55);

	} else {
		if (!myscheme2.get()) throw error("sub_tdc_display::display without scheme!");
		const scheme_screen2& s = *myscheme2;

		// background
		s.background->draw(0, 0);

		// draw tubes if ready
		for (unsigned i = 0; i < 6; ++i) {
			if (player->is_tube_ready(i)) {
				s.tubelight[i].draw();
			}
		}

		// fire button
		unsigned selected_tube = dynamic_cast<const submarine_interface&>(ui).get_selected_tube();
		if (player->is_tube_ready(selected_tube) && TDC.solution_valid()) {
			s.firebutton.draw();
		}

		// automatic fire solution on / off switch
		s.automode[TDC.auto_mode_enabled() ? 0 : 1].draw();

		// draw gyro pointers
		angle leadangle = TDC.get_lead_angle();
		s.gyro_360.draw(leadangle.value());
		s.gyro_10.draw(myfmod(leadangle.value(), 10.0) * 36.0);

		// target values (influenced by quality!)
		double tgtcourse = TDC.get_target_course().value();
		s.target_course_360.draw(tgtcourse);
		s.target_course_10.draw(myfmod(tgtcourse, 10.0) * 36.0);

		// target range
		double tgtrange = TDC.get_target_distance();
		// clamp displayed value
		if (tgtrange < 300) tgtrange = 300;
		if (tgtrange > 11000) tgtrange = 11000;
		// compute non-linear dial value
		tgtrange = sqrt(12.61855670103 * tgtrange - 3685.567010309);
		s.target_range_ptr.draw(tgtrange);
		//fixme: get tgt range marker also... or store it in this screen class?
		//hmm no the TDC needs to now user input, so store it there...

		// fixme: show some sensible value
		s.brightness.draw(45);
	}

	ui.draw_infopanel(true);
	sys().unprepare_2d_drawing();
}



void sub_tdc_display::next_sub_screen(bool is_day)
{
	show_screen1 = !show_screen1;
	leave();
	enter(is_day);
}



void sub_tdc_display::enter(bool is_day)
{
	if (show_screen1)
		myscheme1.reset(new scheme_screen1(is_day));
	else
		myscheme2.reset(new scheme_screen2(is_day));
}



void sub_tdc_display::leave()
{
	myscheme1.reset();
	myscheme2.reset();
}



#if 0 //old

void submarine_interface::display_tdc(game& gm)
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
	tdc->draw(2*256, 0);
	addleadangle->draw(768, 512, 256, 256);

	// Draw lead angle value.
	double la = player->get_trp_setup().addleadangle.value();
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

			player->set_trp_addleadangle(lav);
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
