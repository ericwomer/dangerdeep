// user display: submarine's tdc
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "system.h"
#include "image.h"
#include "texture.h"
#include "game.h"
#include "sub_tdc_display.h"
#include "user_interface.h"
#include "submarine.h"
#include "keys.h"
#include "cfg.h"
#include <sstream>
using namespace std;



sub_tdc_display::sub_tdc_display(user_interface& ui_) : user_display(ui_)
{
	background_normallight = new image(get_image_dir() + "TDC_daylight_base.png", true);
//	background_nightlight = new image(get_image_dir() + "TDC_redlight_base.png", true);

	tube_textures_daylight.resize(6);
//	tube_textures_nightlight.resize(6);
	for (unsigned i = 0; i < 6; ++i) {
		ostringstream osn;
		osn << (i+1);
		tube_textures_daylight[i] = new texture(get_image_dir() + "TDC_daylight_tube" + osn.str() + ".png");
//		tube_textures_nightlight[i] = new texture(get_image_dir() + "TDC_redlight_tube" + osn.str() + ".png");
	}

	clock_big_pointer_daylight = new texture(get_image_dir() + "TDC_daylight_clockbigptr.png");
//	clock_big_pointer_nightlight = new texture(get_image_dir() + "TDC_redlight_clockbigptr.png");
	clock_small_pointer_daylight = new texture(get_image_dir() + "TDC_daylight_clocksmlptr.png");
//	clock_small_pointer_nightlight = new texture(get_image_dir() + "TDC_redlight_clocksmlptr.png");
	targetpos_ptr_daylight = new texture(get_image_dir() + "TDC_daylight_targetposition.png");
//	targetpos_ptr_nightlight = new texture(get_image_dir() + "TDC_redlight_targetposition.png");
	targetcourse_ptr_daylight = new texture(get_image_dir() + "TDC_daylight_targetcourse.png");
//	targetcourse_ptr_nightlight = new texture(get_image_dir() + "TDC_redlight_targetcourse.png");
}



sub_tdc_display::~sub_tdc_display()
{
	delete background_normallight;
//	delete background_nightlight;

	for (unsigned i = 0; i < 6; ++i) {
		delete tube_textures_daylight[i];
//		delete tube_textures_redlight[i];
	}

	delete clock_big_pointer_daylight;
//	delete clock_big_pointer_nightlight;
	delete clock_small_pointer_daylight;
//	delete clock_small_pointer_nightlight;
	delete targetpos_ptr_daylight;
//	delete targetpos_ptr_nightlight;
	delete targetcourse_ptr_daylight;
//	delete targetcourse_ptr_nightlight;
}



void sub_tdc_display::process_input(class game& gm, const SDL_Event& event)
{
	switch (event.type) {
	case SDL_KEYDOWN:
		//fixme
	default: break;
	}
}



void sub_tdc_display::display(class game& gm) const
{
	submarine* player = dynamic_cast<submarine*>(gm.get_player());

	// test: clear background, later this is not necessary, fixme
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	sys().prepare_2d_drawing();
	glColor3f(1,1,1);

	// draw background
	bool is_day = true;//gm.is_day_mode();
	if (is_day) {
		background_normallight->draw(0, 0);
	} else {
		background_nightlight->draw(0, 0);
	}

	// draw clock pointers
/*
	double t = gm.get_time();
	double hourang = 360.0*myfrac(t / (86400/2));
	double minuteang = 360*myfrac(t / 3600);
	clock_hours_pointer->draw_rot(946, 294, hourang);
	clock_minutes_pointer->draw_rot(946, 294, minuteang);
*/

	// get pointer to target and values
	sea_object* target = ui.get_target();
	double tgtcourse = 0, tgtpos = 0;
	if (target) {
		tgtcourse = target->get_heading().value();
		tgtpos = (angle(target->get_pos().xy() - player->get_pos().xy())
			  - player->get_heading()).value();
	}

	// draw target position
	if (is_day) {
		targetpos_ptr_daylight->draw_rot(126, 188, tgtpos);
	} else {
		targetpos_ptr_nightlight->draw_rot(126, 188, tgtpos);
	}

	// draw target course
	if (is_day) {
		targetcourse_ptr_daylight->draw_rot(586, 453, tgtcourse);
	} else {
		targetcourse_ptr_nightlight->draw_rot(586, 453, tgtcourse);
	}

	// draw tubes if ready
	unsigned tubex[6] = { 33,153,274,395,521,647};
	unsigned tubey[6] = {618,618,618,618,618,618};
	for (unsigned i = 0; i < 6; ++i) {
		if (player->is_tube_ready(i)) {
			if (is_day) {
				tube_textures_daylight[i]->draw(tubex[i], tubey[i]);
			} else {
				tube_textures_nightlight[i]->draw(tubex[i], tubey[i]);
			}
		}
	}

	sys().unprepare_2d_drawing();
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
