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

sub_gauges_display::sub_gauges_display(user_interface& ui_) : user_display(ui_), throttle_angle(0)
{
	controlscreen_normallight.reset(new image(get_image_dir() + "daylight_typevii_controlscreen_background.jpg"));
	controlscreen_nightlight.reset(new image(get_image_dir() + "redlight_typevii_controlscreen_background.jpg"));
	image compassi(get_image_dir() + "compass_outer_masked.png");
	image dialsday(get_image_dir() + "daylight_controlscreen_pointers.png");
	image dialsnight(get_image_dir() + "redlight_controlscreen_pointers.png");
	//fusebox lights are missing here
	// old controlscreen layers, will now belong to TypeIIA/B uboat controlscreen
	// new ones are for TypeVII uboat controlscreen
	//	indicators.resize(nr_of_indicators);
	//	indicators[compass].set(compassi.get_SDL_Surface(), (SDL_Surface*)0, 35, 451, 226, 226);
	//	indicators[battery].set(dialsday.get_SDL_Surface(), dialsnight.get_SDL_Surface(), 276, 358, 58, 58);
	//	indicators[compressor].set(dialsday.get_SDL_Surface(), dialsnight.get_SDL_Surface(), 353, 567, 76, 76);
	//	indicators[diesel].set(dialsday.get_SDL_Surface(), dialsnight.get_SDL_Surface(), 504, 567, 76, 76);
	//	indicators[bow_depth_rudder].set(dialsday.get_SDL_Surface(), dialsnight.get_SDL_Surface(), 693, 590, 88, 88);
	//	indicators[stern_depth_rudder].set(dialsday.get_SDL_Surface(), dialsnight.get_SDL_Surface(), 881, 590, 88, 88);
	//	indicators[depth].set(dialsday.get_SDL_Surface(), dialsnight.get_SDL_Surface(), 420, 295, 168, 168);
	//	indicators[knots].set(dialsday.get_SDL_Surface(), dialsnight.get_SDL_Surface(), 756, 44, 94, 94);
	//	indicators[main_rudder].set(dialsday.get_SDL_Surface(), dialsnight.get_SDL_Surface(), 788, 429, 96, 96);
	//	indicators[mt].set(dialsday.get_SDL_Surface(), dialsnight.get_SDL_Surface(), 426, 37, 158, 158);
	indicators.resize(nr_of_indicators);
	indicators[compass].set(compassi.get_SDL_Surface(), (SDL_Surface*)0, 87, 378, 338, 338);
	indicators[bow_depth_rudder].set(dialsday.get_SDL_Surface(), dialsnight.get_SDL_Surface(), 563, 550, 148, 148);
	indicators[stern_depth_rudder].set(dialsday.get_SDL_Surface(), dialsnight.get_SDL_Surface(), 795, 550, 148, 148);
	indicators[depth].set(dialsday.get_SDL_Surface(), dialsnight.get_SDL_Surface(), 391, 50, 270, 270);
	indicators[knots].set(dialsday.get_SDL_Surface(), dialsnight.get_SDL_Surface(), 782, 95, 132, 132);
	indicators[main_rudder].set(dialsday.get_SDL_Surface(), dialsnight.get_SDL_Surface(), 681, 337, 152, 152);
	indicators[mt].set(dialsday.get_SDL_Surface(), dialsnight.get_SDL_Surface(), 88, 93, 173, 173);
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
//	indicators[battery].display(is_day, 0);
//	indicators[compressor].display(is_day, 0);
//	indicators[diesel].display(is_day, 0);
	indicators[bow_depth_rudder].display(is_day, player->get_bow_rudder()*.666);
	indicators[stern_depth_rudder].display(is_day, player->get_stern_rudder()*-.666);
	indicators[depth].display(is_day, player->get_depth()*1.36-136.0);
	indicators[knots].display(is_day, fabs(player->get_speed())*22.33512-131);
	indicators[main_rudder].display(is_day, player->get_rudder_pos()*2.25);
	compute_throttle_angle(player->get_throttle());
	indicators[mt].display(is_day, throttle_angle);

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
            angle mang = angle(-136) - indicators[depth].get_angle(mx, my);
			if (mang.value() < 270) {
				sub->dive_to_depth(unsigned(mang.value()));
			}
		} else if( indicators[bow_depth_rudder].is_over(mx,my)){
			angle mang( indicators[bow_depth_rudder].get_angle(mx,my)-angle(90) );
			double click_pos = 180 - mang.value();
			int status = int(round(click_pos*0.033333));
			if( status>=submarine::rudder_down_30 && status<=submarine::rudder_up_30)
				sub->bow_pos(status);
		} else if( indicators[stern_depth_rudder].is_over(mx,my)){
			angle mang( indicators[stern_depth_rudder].get_angle(mx,my)-angle(90) );
			double click_pos = mang.value();
			if( click_pos> 105 ) click_pos = -(360-click_pos);
			int status = int(round(click_pos*0.033333));
			if( status>=submarine::rudder_down_30 && status<=submarine::rudder_up_30 )
				sub->bow_pos(-status);
		} else if (indicators[mt].is_over(mx, my)) {
			unsigned opt = unsigned( ((indicators[mt].get_angle(mx, my) - angle(210)).value()) * 0.05 );
			if (opt >= 15) opt = 14;
			switch (opt) {
			case 0: sub->set_throttle(ship::aheadflank); break;
			case 1: sub->set_throttle(ship::aheadfull); break;
			case 2: sub->set_throttle(ship::aheadhalf); break;
			case 3: sub->set_throttle(ship::aheadslow); break;
			case 4: sub->set_throttle(ship::aheadlisten); break;
			case 7: sub->set_throttle(ship::stop); break;
			case 11: sub->set_throttle(ship::reversefull); break;//fixme: various reverse speeds!
			case 12: sub->set_throttle(ship::reversehalf); break;
			case 13: sub->set_throttle(ship::reverse); break;
			case 14: // reverse slow ?
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
