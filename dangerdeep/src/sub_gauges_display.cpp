// user display: submarine's gauges
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "system.h"
#include "image.h"
#include "texture.h"
#include "game.h"
#include "submarine.h"
#include "sub_gauges_display.h"
#include "user_interface.h"

sub_gauges_display::indicator::indicator() :
	mytexday(0), mytexnight(0), x(0), y(0), w(0), h(0)
{
}

sub_gauges_display::indicator::~indicator()
{
	delete mytexday;
	delete mytexnight;
}

void sub_gauges_display::indicator::display(bool is_day_mode, double angle) const
{
	const texture* tex = mytexday;
	if (!is_day_mode && mytexnight != 0)
		tex = mytexnight;
	tex->draw_rot(x+w/2, y+h/2, angle, w/2, h/2);
}

void sub_gauges_display::indicator::set(SDL_Surface* sday, SDL_Surface* snight, unsigned x_, unsigned y_, unsigned w_, unsigned h_)
{
	mytexday = new texture(sday, x_, y_, w_, h_, texture::LINEAR, texture::CLAMP_TO_EDGE);
	if (snight)
		mytexnight = new texture(snight, x_, y_, w_, h_, texture::LINEAR, texture::CLAMP_TO_EDGE);
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

sub_gauges_display::sub_gauges_display(user_interface& ui_) : user_display(ui_)
{
	controlscreen_normallight = image::ptr(new image(get_image_dir() + "controlscreen_daylight.png"));
	controlscreen_nightlight = image::ptr(new image(get_image_dir() + "red_controlscreen_rev.0.99_b21_base.png"));
	image compassi(get_image_dir() + "final_sw_compass.png");
	image dialsday(get_image_dir() + "dials_indicators.png");
	image dialsnight(get_image_dir() + "red_controlscreen_rev.0.99_b21_indicatorsMasked.png");
	//fusebox lights are missing here
	indicators.resize(nr_of_indicators);
	indicators[compass].set(compassi.get_SDL_Surface(), (SDL_Surface*)0, 35, 451, 226, 226);
	indicators[battery].set(dialsday.get_SDL_Surface(), dialsnight.get_SDL_Surface(), 276, 358, 58, 58);
	indicators[compressor].set(dialsday.get_SDL_Surface(), dialsnight.get_SDL_Surface(), 353, 567, 76, 76);
	indicators[diesel].set(dialsday.get_SDL_Surface(), dialsnight.get_SDL_Surface(), 504, 567, 76, 76);
	indicators[bow_depth_rudder].set(dialsday.get_SDL_Surface(), dialsnight.get_SDL_Surface(), 693, 590, 88, 88);
	indicators[stern_depth_rudder].set(dialsday.get_SDL_Surface(), dialsnight.get_SDL_Surface(), 881, 590, 88, 88);
	indicators[depth].set(dialsday.get_SDL_Surface(), dialsnight.get_SDL_Surface(), 420, 295, 168, 168);
	indicators[knots].set(dialsday.get_SDL_Surface(), dialsnight.get_SDL_Surface(), 756, 44, 94, 94);
	indicators[main_rudder].set(dialsday.get_SDL_Surface(), dialsnight.get_SDL_Surface(), 788, 429, 96, 96);
	indicators[mt].set(dialsday.get_SDL_Surface(), dialsnight.get_SDL_Surface(), 426, 37, 158, 158);
}

sub_gauges_display::~sub_gauges_display()
{
}

void sub_gauges_display::display(class game& gm) const
{
	submarine* player = dynamic_cast<submarine*> ( gm.get_player () );
	system::sys().prepare_2d_drawing();

	glColor3f(1,1,1);

	bool is_day = gm.is_day_mode();
	if (is_day) {
		controlscreen_normallight->draw(0, 0);
	} else {
		controlscreen_nightlight->draw(0, 0);
	}

	// the absolute numbers here depend on the graphics!
	indicators[compass].display(is_day, -player->get_heading().value());
	indicators[battery].display(is_day, 0);
	indicators[compressor].display(is_day, 0);
	indicators[diesel].display(is_day, 0);
	indicators[bow_depth_rudder].display(is_day, 0);
	indicators[stern_depth_rudder].display(is_day, 0);
	indicators[depth].display(is_day, player->get_depth()*1.0-51.0);
	indicators[knots].display(is_day, fabs(player->get_speed())*22.33512-133.6);
	indicators[main_rudder].display(is_day, player->get_rudder_pos()*3.5125);
	indicators[mt].display(is_day, 0);

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
	// fixme: actions are executed, but no messages are sent...
	submarine* sub = dynamic_cast<submarine*>(gm.get_player());
	int mx, my;
	switch (event.type) {
	case SDL_MOUSEBUTTONDOWN:
		mx = event.button.x;
		my = event.button.y;
		//if mouse is over control c, compute angle a, set matching command, fixme
		if (indicators[compass].is_over(mx, my)) {
			angle mang = angle(180)-indicators[compass].get_angle(mx, my);
			sub->head_to_ang(mang, mang.is_cw_nearer(sub->get_heading()));
		} else if (indicators[depth].is_over(mx, my)) {
			angle mang = angle(-39/*51-90*/) - indicators[depth].get_angle(mx, my);
			if (mang.value() < 270) {
				sub->dive_to_depth(unsigned(mang.value()));
			}
		} else if (indicators[mt].is_over(mx, my)) {
			unsigned opt = unsigned((indicators[mt].get_angle(mx, my) - angle(210)).value() / 20);
			if (opt >= 15) opt = 14;
			switch (opt) {
			case 0: sub->set_throttle(ship::aheadflank); break;
			case 1: sub->set_throttle(ship::aheadfull); break;
			case 2: sub->set_throttle(ship::aheadhalf); break;
			case 3: sub->set_throttle(ship::aheadslow); break;
			case 4: sub->set_throttle(ship::aheadlisten); break;
			case 7: sub->set_throttle(ship::stop); break;
			case 11: sub->set_throttle(ship::reverse); break;//fixme: various reverse speeds!
			case 12: sub->set_throttle(ship::reverse); break;
			case 13: sub->set_throttle(ship::reverse); break;
			case 14: sub->set_throttle(ship::reverse); break;
			case 5: // diesel engines
			case 6: // attention
			case 8: // electric engines
			case 9: // surface
			case 10:// dive
				break;
			}
		}
		break;
	default:
		break;
	}
}
