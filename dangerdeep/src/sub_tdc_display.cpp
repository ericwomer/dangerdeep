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


sub_tdc_display::rotat_tex::rotat_tex() : tex(0), left(0), top(0), centerx(0), centery(0)
{
}

sub_tdc_display::rotat_tex::~rotat_tex()
{
	delete tex;
}

void sub_tdc_display::rotat_tex::set(const char* filename, int left_, int top_, int centerx_, int centery_)
{
	delete tex;	// paranoia
	tex = new texture(get_image_dir() + filename);
	left = left_;
	top = top_;
	centerx = centerx_;
	centery = centery_;
}

void sub_tdc_display::rotat_tex::draw(double angle) const
{
	// fixme: maybe rotate around pixel center (x/y + 0.5)
	tex->draw_rot(centerx, centery, angle, centerx - left, centery - top);
}


sub_tdc_display::scheme::scheme()
{
	memset(this, 0, sizeof(*this));
}



sub_tdc_display::scheme::~scheme()
{
	delete background;
	for (unsigned i = 0; i < 6; ++i) {
		delete tubelight[i];
		delete tubeswitch[i];
	}
	delete firebutton;
	delete automode[0];
	delete automode[1];
	delete firesolutionquality;
}



sub_tdc_display::sub_tdc_display(user_interface& ui_) : user_display(ui_)
{
	selected_tube = 0;
	selected_mode = 0;

	normallight.background = new image(get_image_dir() + "TDC_daylight_base.png", true);
//	nightlight.background = new image(get_image_dir() + "TDC_redlight_base.png", true);

	normallight.clockbig.set("TDC_daylight_clockbigptr.png", 921, 124, 930, 134);
//	nightlight.clockbig.set("TDC_redlight_clockbigptr.png", 921, 124, 930, 134);
	normallight.clocksml.set("TDC_daylight_clocksmlptr.png", 926, 76, 929, 99);
//	nightlight.clocksml.set("TDC_redlight_clocksmlptr.png", 926, 76, 929, 99);
	normallight.targetcourse.set("TDC_daylight_targetcourse.png", 567, 365, 585, 451);
//	nightlight.targetcourse.set("TDC_redlight_targetcourse.png", 567, 365, 585, 451);
	normallight.targetrange.set("TDC_daylight_targetrange.png", 755, 231, 774, 317);
//	nightlight.targetrange.set("TDC_redlight_targetrange.png", 755, 231, 774, 317);
	normallight.targetspeed.set("TDC_daylight_targetspeed.png", 568, 101, 586, 187);
//	nightlight.targetspeed.set("TDC_redlight_targetspeed.png", 568, 101, 586, 187);
	normallight.spreadangle.set("TDC_daylight_spreadangle.png", 339, 102, 358, 188);
//	nightlight.spreadangle.set("TDC_redlight_spreadangle.png", 339, 102, 358, 188);
	normallight.targetpos.set("TDC_daylight_targetposition.png", 102, 109, 128, 188);
//	nightlight.targetpos.set("TDC_redlight_targetposition.png", 102, 109, 128, 188);
	normallight.gyro_a.set("TDC_daylight_gyroa.png", 66, 389, 127, 450);
//	nightlight.gyro_a.set("TDC_redlight_gyroa.png", 66, 389, 127, 450);
	normallight.gyro_v.set("TDC_daylight_gyrov.png", 282, 389, 345, 450);
//	nightlight.gyro_v.set("TDC_redlight_gyrov.png", 282, 389, 345, 450);
	normallight.torpspeed.set("TDC_daylight_torpspeed.png", 512, 116, 585, 188);
//	nightlight.torpspeed.set("TDC_redlight_torpspeed.png", 512, 116, 585, 188);

	for (unsigned i = 0; i < 6; ++i) {
		ostringstream osn;
		osn << (i+1);
		normallight.tubelight[i] = new texture(get_image_dir() + "TDC_daylight_tube" + osn.str() + ".png");
//		nightlight.tubelight[i] = new texture(get_image_dir() + "TDC_redlight_tube" + osn.str() + ".png");
		normallight.tubeswitch[i] = new texture(get_image_dir() + "TDC_daylight_switchtube" + osn.str() + ".png");
//		nightlight.tubeswitch[i] = new texture(get_image_dir() + "TDC_redlight_switchtube" + osn.str() + ".png");
	}

	normallight.firebutton = new texture(get_image_dir() + "TDC_daylight_firebutton.png");
//	nightlight.firebutton = new texture(get_image_dir() + "TDC_redlight_firebutton.png");
	normallight.automode[0] = new texture(get_image_dir() + "TDC_daylight_autoswitchon.png");
//	nightlight.automode[0] = new texture(get_image_dir() + "TDC_redlight_autoswitchon.png");
	normallight.automode[1] = new texture(get_image_dir() + "TDC_daylight_autoswitchoff.png");
//	nightlight.automode[1] = new texture(get_image_dir() + "TDC_redlight_autoswitchoff.png");
	normallight.firesolutionquality = new texture(get_image_dir() + "TDC_daylight_firesolutionquality.png");
//	nightlight.firesolutionquality = new texture(get_image_dir() + "TDC_redlight_firesolutionquality.png");
}



sub_tdc_display::~sub_tdc_display()
{
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

	// determine time of day
	bool is_day = true;//gm.is_day_mode();
	const scheme& s = (is_day) ? normallight : nightlight;

	// draw background pointers/dials: gyros, firesolutionquality
	s.gyro_a.draw(33/*fixme*/);
	s.gyro_v.draw(33/*fixme*/);
	float quality = 0.333f; // per cent, fixme
	s.firesolutionquality->draw(926, 50+int(288*quality+0.5f));

	// draw background
	s.background->draw(0, 0);

	// clock (torpedo run time), fixme: use real time as test
	double t = gm.get_time();
	double hourang = 360.0*myfrac(t / (86400/2));
	double minuteang = 360*myfrac(t / 3600);
	s.clocksml.draw(hourang);
	s.clockbig.draw(minuteang);

	// torpedo speed (depends on selected tube!), fixme
	// 512,116, 585,188

	// target values (influenced by quality!)
	// get pointer to target and values
	sea_object* target = ui.get_target();
	double tgtcourse = 0, tgtpos = 0, tgtrange = 0, tgtspeed = 0;
	if (target) {
		vector2 tgtxy = target->get_pos().xy();
		vector2 plyxy = player->get_pos().xy();
		vector2 delta = tgtxy - plyxy;
		tgtcourse = target->get_heading().value();
		tgtpos = (angle(delta) - player->get_heading()).value();
		tgtrange = delta.length();
		tgtspeed = sea_object::ms2kts(target->get_speed());
	}
	s.targetcourse.draw(tgtcourse);
	s.targetpos.draw(tgtpos);
	if (tgtrange > 11000) tgtrange = 11000; // clamp display
	s.targetrange.draw(tgtrange * 360 / 12000 + 15);
	s.targetspeed.draw(15+tgtspeed*30/5);

	// spread angle
	s.spreadangle.draw(77/*fixme*/);

	// draw tubes if ready
	unsigned tubex[6] = { 31,150,271,392,518,641};
	unsigned tubey[6] = {617,610,612,612,617,611};
	for (unsigned i = 0; i < 6; ++i) {
		if (player->is_tube_ready(i)) {
			s.tubelight[i]->draw(tubex[i], tubey[i]);
		}
	}

	// tube turn switch
	s.tubeswitch[selected_tube]->draw(760, 492);
	if (player->is_tube_ready(selected_tube)) {
		s.firebutton->draw(885, 354);
	}

	// automatic fire solution on / off switch
	s.automode[selected_mode]->draw(713, 93);

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
