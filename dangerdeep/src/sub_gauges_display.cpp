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

// user display: submarine's gauges
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "system.h"
#include "image.h"
#include "texture.h"
#include "game.h"
#include "submarine.h"
#include "sub_gauges_display.h"
#include "user_interface.h"
#include "global_data.h"

sub_gauges_display::indicator::indicator(const sdl_image& s, unsigned x_, unsigned y_, unsigned w_, unsigned h_)
	: mytex(s, x_, y_, w_, h_, texture::LINEAR, texture::CLAMP),
	  x(x_), y(y_), w(w_), h(h_)
{
}



void sub_gauges_display::indicator::display(double angle) const
{
	mytex.draw_rot(x+w/2, y+h/2, angle, w/2, h/2);
}



bool sub_gauges_display::indicator::is_over(int mx, int my) const
{
	return (mx >= int(x) && my >= int(y) && mx < int(x+w) && my < int(y+h));
}



angle sub_gauges_display::indicator::get_angle(int mx, int my) const
{
	// need to negate y, because onscreen y is down
	return angle(vector2(mx - int(x + w/2), int(y + h/2) - my));
}



sub_gauges_display::sub_gauges_display(user_interface& ui_) : user_display(ui_), throttle_angle(0)
{
}



void sub_gauges_display::display(class game& gm) const
{
	submarine* player = dynamic_cast<submarine*> ( gm.get_player () );
	system::sys().prepare_2d_drawing();

	controlscreen->draw(0, 0);

	// the absolute numbers here depend on the graphics!
	indicator_compass->display(-player->get_heading().value());
//	indicator_battery->display(0);
//	indicator_compressor->display(0);
//	indicator_diesel->display(0);
	indicator_bow_depth_rudder->display(player->get_bow_rudder()*-2.0);
	indicator_stern_depth_rudder->display(-player->get_stern_rudder()*-2.0);
	indicator_depth->display(player->get_depth()*1.36-136.0);
	indicator_knots->display(fabs(player->get_speed())*22.33512-131);
	indicator_main_rudder->display(player->get_rudder_pos()*2.25);
	compute_throttle_angle(player->get_throttle());
	indicator_mt->display(throttle_angle);

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
	ui.draw_infopanel(true);

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
		if (indicator_compass->is_over(mx, my)) {
			sub->head_to_course(indicator_compass->get_angle(mx, my));
		} else if (indicator_depth->is_over(mx, my)) {
			angle mang = indicator_depth->get_angle(mx, my) - angle(225);
			// 135° are 100m
			if (mang.value()/1.35 < 270) {
				sub->dive_to_depth(unsigned(mang.value()/1.35));
			}
		} else if( indicator_bow_depth_rudder->is_over(mx,my)){
			angle mang(indicator_bow_depth_rudder->get_angle(mx,my)-angle(270));
			double pos = myclamp(mang.value_pm180() / 60.0, -1.0, 1.0);
			sub->set_bow_depth_rudder(-pos);
		} else if( indicator_stern_depth_rudder->is_over(mx,my)){
			angle mang(angle(90) - indicator_stern_depth_rudder->get_angle(mx,my));
			double pos = myclamp(mang.value_pm180() / 60.0, -1.0, 1.0);
			sub->set_stern_depth_rudder(-pos);
		} else if (indicator_mt->is_over(mx, my)) {
			// 270° in 15 steps, 45°-315°, so 18° per step.
			unsigned opt = unsigned( ((indicator_mt->get_angle(mx, my) - angle(45)).value()) / 18.0);
			if (opt >= 15) opt = 14;
			switch (opt) {
			case 0: sub->set_throttle(ship::reversefull); break;
			case 1: sub->set_throttle(ship::reversehalf); break;
			case 2: sub->set_throttle(ship::reverse); break;
			case 7: sub->set_throttle(ship::stop); break;
			case 10: sub->set_throttle(ship::aheadlisten); break;
			case 11: sub->set_throttle(ship::aheadslow); break;
			case 12: sub->set_throttle(ship::aheadhalf); break;
			case 13: sub->set_throttle(ship::aheadfull); break;
			case 14: sub->set_throttle(ship::aheadflank); break;
			case 3: // reverse small ?
			case 4: // loading (battery)
			case 5: // both machines 10 rpm less (?)
			case 6: // use electric engines
			case 8: // attention
			case 9: // diesel engines
				break;
			}
		}
		break;
	default:
		break;
	}
}



int sub_gauges_display::compute_throttle_angle(int throttle_pos) const
{
	int throttle_goal;

	switch(submarine::throttle_status(throttle_pos)){
	case submarine::reversefull: throttle_goal = -125; break;
	case submarine::reversehalf: throttle_goal = -107; break;                             
	case submarine::reverse: throttle_goal = -90; break;
	case submarine::aheadlisten: throttle_goal = 54; break;
	case submarine::aheadslow: throttle_goal = 72; break;
	case submarine::aheadhalf: throttle_goal = 90; break;
	case submarine::aheadfull: throttle_goal = 108; break;
	case submarine::aheadflank: throttle_goal = 126; break;
	case submarine::stop:
	default: throttle_goal = 0;
	}

	if( throttle_angle < throttle_goal )
		++throttle_angle;
	else if( throttle_angle > throttle_goal )
		--throttle_angle;

	return throttle_angle;
}



void sub_gauges_display::enter(bool is_day)
{
	if (is_day)
		controlscreen.reset(new image(get_image_dir() + "daylight_typevii_controlscreen_background.jpg"));
	else
		controlscreen.reset(new image(get_image_dir() + "redlight_typevii_controlscreen_background.jpg"));

	sdl_image compassi(get_image_dir() + "compass_outer_masked.png");
	sdl_image dials(get_image_dir() + (is_day ? "daylight_controlscreen_pointers.png" : "redlight_controlscreen_pointers.png"));
	//fusebox lights are missing here
	// old controlscreen layers, will now belong to TypeIIA/B uboat controlscreen
	// new ones are for TypeVII uboat controlscreen
	//	indicators.resize(nr_of_indicators);
	//	indicator_compass->set(compassi, (SDL_Surface*)0, 35, 451, 226, 226);
	//	indicator_battery->set(dialsday, dialsnight, 276, 358, 58, 58);
	//	indicator_compressor->set(dialsday, dialsnight, 353, 567, 76, 76);
	//	indicator_diesel->set(dialsday, dialsnight, 504, 567, 76, 76);
	//	indicator_bow_depth_rudder->set(dialsday, dialsnight, 693, 590, 88, 88);
	//	indicator_stern_depth_rudder->set(dialsday, dialsnight, 881, 590, 88, 88);
	//	indicator_depth->set(dialsday, dialsnight, 420, 295, 168, 168);
	//	indicator_knots->set(dialsday, dialsnight, 756, 44, 94, 94);
	//	indicator_main_rudder->set(dialsday, dialsnight, 788, 429, 96, 96);
	//	indicator_mt->set(dialsday, dialsnight, 426, 37, 158, 158);
	indicator_compass.reset(new indicator(compassi, 87, 378, 338, 338));
	indicator_bow_depth_rudder.reset(new indicator(dials, 563, 550, 148, 148));
	indicator_stern_depth_rudder.reset(new indicator(dials, 795, 550, 148, 148));
	indicator_depth.reset(new indicator(dials, 391, 50, 270, 270));
	indicator_knots.reset(new indicator(dials, 782, 95, 132, 132));
	indicator_main_rudder.reset(new indicator(dials, 681, 337, 152, 152));
	indicator_mt.reset(new indicator(dials, 88, 93, 173, 173));
}



void sub_gauges_display::leave()
{
	controlscreen.reset();
	indicator_compass.reset();
	indicator_bow_depth_rudder.reset();
	indicator_stern_depth_rudder.reset();
	indicator_depth.reset();
	indicator_knots.reset();
	indicator_main_rudder.reset();
	indicator_mt.reset();
}
