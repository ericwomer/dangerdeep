// user display: submarine's gauges
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "system.h"
#include "image.h"
#include "texture.h"
#include "game.h"
#include "command.h"
#include "sub_gauges_display.h"

sub_gauges_display::indicator::indicator() :
	mytex(0), x(0), y(0), w(0), h(0)
{
}

sub_gauges_display::indicator::~indicator()
{
	delete mytex;
}

void sub_gauges_display::indicator::display(const double& angle) const
{
	mytex->draw_rot(x+w/2, y+h/2, angle, w/2, h/2);
}

void sub_gauges_display::indicator::set(SDL_Surface* s, unsigned x_, unsigned y_, unsigned w_, unsigned h_)
{
	mytex = new texture(s, x_, y_, w_, h_, GL_LINEAR, GL_CLAMP_TO_EDGE);
	x = x_;
	y = y_;
	w = w_;
	h = h_;
}

bool sub_gauges_display::indicator::is_over(int mx, int my) const
{
	return (mx >= int(x) && my >= int(y) && mx < int(x+w) && my < int(y+h));
}

angle sub_gauges_display::indicator::get_angle(int mx, int my) const
{
	return angle(vector2(mx - int(x + w/2), my - int(y + h/2)));
}

sub_gauges_display::sub_gauges_display() :
	controlscreen_normallight(get_image_dir() + "ControlScreen_NormalLight.png"),
	controlscreen_nightlight(get_image_dir() + "ControlScreen_NightLight.png")
{
	image compassi(get_image_dir() + "ControlScreen_Compass.png");
	image dials(get_image_dir() + "ControlScreen_Dials.png");
	//fixme: at night we have different dials! store two textures per dial!
	indicators.resize(nr_of_indicators);
	indicators[compass].set(compassi.get_SDL_Surface(), 35, 451, 226, 226);
	indicators[battery].set(dials.get_SDL_Surface(), 276, 358, 58, 58);
	indicators[compressor].set(dials.get_SDL_Surface(), 353, 567, 76, 76);
	indicators[diesel].set(dials.get_SDL_Surface(), 504, 567, 76, 76);
	indicators[bow_depth_rudder].set(dials.get_SDL_Surface(), 693, 590, 88, 88);
	indicators[stern_depth_rudder].set(dials.get_SDL_Surface(), 881, 590, 88, 88);
	indicators[depth].set(dials.get_SDL_Surface(), 420, 295, 168, 168);
	indicators[knots].set(dials.get_SDL_Surface(), 756, 44, 94, 94);
	indicators[main_rudder].set(dials.get_SDL_Surface(), 788, 429, 96, 96);
	indicators[mt].set(dials.get_SDL_Surface(), 442, 52, 126, 126);
}

sub_gauges_display::~sub_gauges_display()
{
}

void sub_gauges_display::display(class game& gm) const
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

void sub_gauges_display::process_input(class game& gm, const SDL_Event& event)
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

void submarine_interface::display_UZO(game& gm)
{
	submarine* player = dynamic_cast<submarine*> ( get_player() );

	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	unsigned res_x = system::sys().get_res_x(), res_y = system::sys().get_res_y();

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	system::sys().gl_perspective_fovx (5.0, 2.0/1.0, 5.0, gm.get_max_view_distance());
	glViewport(0, res_y/3, res_x, res_x/2);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glRotatef(-90, 1,0,0);	// swap y and z coordinates (camera looks at +y axis)

	vector3 viewpos = player->get_pos() + vector3(0, 0, 6);
	// no torpedoes, no DCs, no player
	draw_view(gm, viewpos, 0, res_y/3, res_x, res_x/2, player->get_heading()+bearing, 0, true, false, false);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glViewport(0, 0, res_x, res_y);
	glMatrixMode(GL_MODELVIEW);
	
	system::sys().prepare_2d_drawing();
	uzo->draw(0, 0, 512, 512);
	uzo->draw_hm(512, 0, 512, 512);
	draw_infopanel(gm);
	system::sys().unprepare_2d_drawing();

	// keyboard processing
	int key = system::sys().get_key().sym;
	while (key != 0) {
		if (!keyboard_common(key,  gm)) {
			// specific keyboard processing
		}
		key = system::sys().get_key().sym;
	}
}
