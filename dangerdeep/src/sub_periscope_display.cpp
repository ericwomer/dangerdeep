// user display: submarine's gauges
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "system.h"
#include "image.h"
#include "texture.h"
#include "game.h"
#include "sub_periscope_display.h"
#include "user_interface.h"



void sub_periscope_display::pre_display(game& gm) const
{
	glClear(GL_DEPTH_BUFFER_BIT);
}



freeview_display::projection_data sub_periscope_display::get_projection_data(class game& gm) const
{
	projection_data pd;
	pd.x = 453*system::sys().get_res_x()/1024;
	pd.y = (768-424-193)*system::sys().get_res_x()/1024;
	pd.w = 424*system::sys().get_res_x()/1024;
	pd.h = 424*system::sys().get_res_x()/1024;
	// with normal fov of 70 degrees, this is 1.5 / 6.0 magnification
	pd.fov_x = zoomed ? 13.31 : 50.05;	//fixme: historic values?
	pd.near_z = 1.0;
	pd.far_z = gm.get_max_view_distance();
	return pd;
}



void sub_periscope_display::post_display(game& gm) const
{
	system::sys().prepare_2d_drawing();
	glColor3f(1,1,1);
	periscopeimg->draw(0, 0);
//	ui.draw_infopanel(gm);
	system::sys().unprepare_2d_drawing();
}



sub_periscope_display::sub_periscope_display(user_interface& ui_) : freeview_display(ui_), zoomed(false)
{
	periscopeimg = new image(get_image_dir() + "periscope_wip.png", true);

	pos = vector3(0, 0, 12);//fixme, depends on sub
	aboard = true;
	withunderwaterweapons = false;//they can be seen when scope is partly below water surface, fixme
	drawbridge = false;
}



sub_periscope_display::~sub_periscope_display()
{
	delete periscopeimg;
}






#if 0 //old

void submarine_interface::display_periscope(game& gm)
{
	submarine* player = dynamic_cast<submarine*> ( get_player() );

	glClear(GL_DEPTH_BUFFER_BIT);

	unsigned res_x = system::sys().get_res_x(), res_y = system::sys().get_res_y();

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
	
	system::sys().gl_perspective_fovx (fov, 1.0/1.0, 5.0, gm.get_max_view_distance());
	glViewport(res_x/2, res_y/3, res_x/2, res_x/2);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glRotatef(-90, 1,0,0);	// swap y and z coordinates (camera looks at +y axis)

	vector3 viewpos = player->get_pos() + vector3(0, 0, 12+14);//fixme: +14 to be above waves ?!
	// no torpedoes, no DCs, no player
	draw_view(gm, viewpos, res_x/2, res_y/3, res_x/2, res_x/2, player->get_heading()+bearing, 0, true, false, false);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glViewport(0, 0, res_x, res_y);
	glMatrixMode(GL_MODELVIEW);
	
	system::sys().prepare_2d_drawing();
	set_display_color ( gm );
	for (int x = 0; x < 3; ++x)
		psbackgr->draw(x*256, 512, 256, 256);
	periscope->draw(2*256, 0);
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
	system::sys().unprepare_2d_drawing();

	// mouse handling
	int mx;
	int my;
	int mb = system::sys().get_mouse_buttons();
	system::sys().get_mouse_position(mx, my);

	if (mb & system::sys().left_button) {
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
	int key = system::sys().get_key().sym;
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
		key = system::sys().get_key().sym;
	}
}

#endif
