// user display: submarine's bridge
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "system.h"
#include "image.h"
#include "texture.h"
#include "game.h"
#include "command.h"
#include "sub_bridge_display.h"

void sub_bridge_display::pre_display(void) const
{
	glClear(GL_DEPTH_BUFFER_BIT);
}



void sub_bridge_display::get_viewport(unsigned& x, unsigned& y, unsigned& w, unsigned& h) const
{
}



void sub_bridge_display::set_projection_matrix(void) const
{
}



void sub_bridge_display::set_modelview_matrix(void) const
{
}



void sub_bridge_display::post_display(void) const
{
}



sub_bridge_display::sub_bridge_display() :
{
}



sub_bridge_display::~sub_bridge_display()
{
}



void sub_bridge_display::display(class game& gm) const
{
	submarine* player = dynamic_cast<submarine*> ( gm.get_player () );
	system::sys().prepare_2d_drawing();

	if (true /*gm.is_day() fixme*/) {
		controlscreen_normallight.draw(0, 0);
	} else {
		controlscreen_nightlight.draw(0, 0);
	}

	// the absolute numbers here depend on the graphics!
	indicators[compass].display(-player->get_heading().value());
	indicators[battery].display(0);
	indicators[compressor].display(0);
	indicators[diesel].display(0);
	indicators[bow_depth_rudder].display(0);
	indicators[stern_depth_rudder].display(0);
	indicators[depth].display(player->get_depth()*1.0-51.0);
	indicators[knots].display(fabs(player->get_speed())*22.33512-133.6);
	indicators[main_rudder].display(player->get_rudder_pos()*3.5125);
	indicators[mt].display(0);

/*	// kept as text reference for tooltips/popups.
	angle player_speed = player->get_speed()*360.0/sea_object::kts2ms(36);
	angle player_depth = -player->get_pos().z;
	draw_gauge(gm, 1, 0, 0, 256, player->get_heading(), texts::get(1),
		player->get_head_to());
	draw_gauge(gm, 2, 256, 0, 256, player_speed, texts::get(4));
	draw_gauge(gm, 4, 2*256, 0, 256, player_depth, texts::get(5));
	draw_clock(gm, 3*256, 0, 256, gm.get_time(), texts::get(61));
	draw_manometer_gauge ( gm, 1, 0, 256, 256, player->get_fuel_level (),
		texts::get(101));
	draw_manometer_gauge ( gm, 1, 256, 256, 256, player->get_battery_level (),
		texts::get(102));
*/
//	draw_infopanel(gm);

	system::sys().unprepare_2d_drawing();
}

void sub_bridge_display::process_input(class game& gm, const SDL_Event& event)
{
	submarine* sub = dynamic_cast<submarine*>(gm.get_player());
	int mx, my;
	switch (event.type) {
	case SDL_MOUSEBUTTONDOWN:
		mx = event.button.x;
		my = event.button.y;
		//if mouse is over control c, compute angle a, set matching command, fixme
		if (indicators[compass].is_over(mx, my)) {
			angle mang = -indicators[compass].get_angle(mx, my);
			gm.send(new command_head_to_ang(sub, mang, mang.is_cw_nearer(sub->get_heading())));
		}
		break;
	default:
		break;
	}
}

void user_interface::display_bridge(game& gm)
{
	class system& sys = system::sys();
	unsigned res_x = system::sys().get_res_x();
	unsigned res_y = system::sys().get_res_y();
	sea_object* player = get_player();
    
	glClear(GL_DEPTH_BUFFER_BIT /* | GL_COLOR_BUFFER_BIT */);	// fixme remove color buffer bit for speedup
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glRotatef(-90, 1,0,0);	// swap y and z coordinates (camera looks at +y axis)

	vector2 phd = player->get_heading().direction();
	vector3 viewpos = player->get_pos() + vector3(0, 0, 6) + phd.xy0();
	// no torpedoes, no DCs, with player
	draw_view(gm, viewpos, 0,0,res_x,res_y, player->get_heading()+bearing, elevation, true, true, false);

	sys.prepare_2d_drawing();
	draw_infopanel(gm);
	sys.unprepare_2d_drawing();

	int mmx, mmy;
	sys.get_mouse_motion(mmx, mmy);	
	if (sys.get_mouse_buttons() & system::right_button) {
		SDL_ShowCursor(SDL_DISABLE);
		bearing += angle(float(mmx)/4);
		float e = elevation.value_pm180() - float(mmy)/4; // make this - to a + to invert mouse look
		if (e < 0) e = 0;
		if (e > 90) e = 90;
		elevation = angle(e);
	} else {
		SDL_ShowCursor(SDL_ENABLE);
	}

	// keyboard processing
	int key = sys.get_key().sym;
	while (key != 0) {
		if (!keyboard_common(key, gm)) {
			// specific keyboard processing
			switch ( key )
			{
				// Zoom view
				case SDLK_y:
					zoom_scope = true;
					break;
			}
		}
		key = sys.get_key().sym;
	}
}

void user_interface::display_glasses(class game& gm)
{
	class system& sys = system::sys();
	sea_object* player = get_player();

	unsigned res_x = system::sys().get_res_x();
	unsigned res_y = system::sys().get_res_y();

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	sys.gl_perspective_fovx (10.0, 2.0/1.0, 2.0, gm.get_max_view_distance());
	glViewport(0, res_y-res_x/2, res_x, res_x/2);
	glClear(GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glRotatef(-90, 1,0,0);	// swap y and z coordinates (camera looks at +y axis)

	vector3 viewpos = player->get_pos() + vector3(0, 0, 6);
	// no torpedoes, no DCs, no player
	draw_view(gm, viewpos, 0,res_y-res_x/2,res_x,res_x/2, player->get_heading()+bearing, 0, false/*true*/, false, false);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glViewport(0, 0, res_x, res_y);
	glMatrixMode(GL_MODELVIEW);

	sys.prepare_2d_drawing();
	glasses->draw(0, 0, 512, 512);
	glasses->draw_hm(512, 0, 512, 512);
	glBindTexture(GL_TEXTURE_2D, 0);
	color::black().set_gl_color();
	sys.draw_rectangle(0, 512, 1024, 256);
	draw_infopanel(gm);
	sys.unprepare_2d_drawing();

	// keyboard processing
	int key = sys.get_key().sym;
	while (key != 0) {
		if (!keyboard_common(key, gm)) {
			// specific keyboard processing
			switch ( key )
			{
				case SDLK_y:
					zoom_scope = false;
					break;
			}
		}
		key = sys.get_key().sym;
	}
}

