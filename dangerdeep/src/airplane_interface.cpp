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

// interface for controlling an airplane
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL.h>

#include <sstream>
#include <map>
#include <set>
#include <list>
using std::map;
using std::list;
using std::set;
#include "user_display.h"
#include "airplane_interface.h"
#include "system.h"
#include "game.h"
#include "texts.h"
#include "image.h"
#include "command.h"
//#include "widget.h"

airplane_interface::airplane_interface(airplane* player_plane, game& gm) : 
    	user_interface( player_plane, gm )
{
//	btn_menu = new widget_caller_button<game, void (game::*)()>(&gm, &game::stop, 1024-128-8, 128-40, 128, 32, texts::get(177));
//	panel->add_child(btn_menu);
}

airplane_interface::~airplane_interface()
{
}

bool airplane_interface::keyboard_common(int keycode, class game& gm)
{
	airplane* player = dynamic_cast<airplane*> ( get_player() );

	// handle common keys (fixme: make configureable?)
	if (sys().key_shift()) {
		switch (keycode) {
			default: return false;
		}
	} else {	// no shift
		switch (keycode) {
			// viewmode switching
//			case SDLK_F1: viewmode = display_mode_gauges; break;

			// time scaling fixme: too simple
			case SDLK_F11: if (time_scale_up()) { add_message(texts::get(31)); } break;
			case SDLK_F12: if (time_scale_down()) { add_message(texts::get(32)); } break;

			// control
/*
			case SDLK_LEFT:
				player->roll_left();
				break;
			case SDLK_RIGHT:
				player->roll_right();
				break;
			case SDLK_UP: player->pitch_down(); break;
			case SDLK_DOWN: player->pitch_up(); break;
*/			

			// weapons, fixme
/*
			case SDLK_SPACE:
				target = gm.contact_in_direction(player, player->get_heading()+bearing);
				if (target)
				{
					add_message(texts::get(50));
					add_captains_log_entry ( gm, texts::get(50));
				}
				else
					add_message(texts::get(51));
				break;
*/				

			// quit, screenshot, pause etc.
			case SDLK_ESCAPE:
				gm.stop();
				break;
			case SDLK_PRINT: sys().screenshot(); sys().add_console("screenshot taken."); break;
			case SDLK_PAUSE: pause = !pause;
				if (pause) add_message(texts::get(52));
				else add_message(texts::get(53));
				break;
			default: return false;		
		}
	}
	return true;
}

void airplane_interface::display(game& gm)
{
	airplane* player = dynamic_cast<airplane*> ( get_player() );

	display_cockpit(gm);
/*
	switch (viewmode) {
		case display_mode_gauges:
			display_gauges(gm);
			break;
		case display_mode_periscope:
			display_periscope(gm);
			break;
		case display_mode_successes:
			display_successes(gm);
			break;
		case display_mode_freeview:
		default:
			display_freeview(gm);
			break;
	}
*/	
}

void airplane_interface::display_cockpit(game& gm)
{
	airplane* player = dynamic_cast<airplane*> ( get_player() );

	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	unsigned res_x = sys().get_res_x(), res_y = sys().get_res_y();


	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	vector3 viewpos = player->get_pos() + vector3(0, 0, 2);

	glRotatef(-90,1,0,0);
	double angle;
	vector3 axis;
	player->get_orientation().angleaxis(angle, axis);
	glRotated(-angle, axis.x, axis.y, axis.z);
	glRotatef(90,1,0,0);	// compensate for later rotation
	draw_view(gm, viewpos, 0,0,res_x,res_y, 0, 0, false, false, false);
	glLoadIdentity();
	glTranslatef(0,-40,-100);
	glRotatef(-90, 1, 0, 0);
	player->display();
	
	sys().prepare_2d_drawing();
	glBindTexture(GL_TEXTURE_2D, 0);
	glColor4f(1,1,1,1);
	glBegin(GL_LINES);
	glVertex2f(res_x/2, res_y/2-res_y/20);
	glVertex2f(res_x/2, res_y/2+res_y/20);
	glVertex2f(res_x/2-res_y/20, res_y/2);
	glVertex2f(res_x/2+res_y/20, res_y/2);
	glEnd();
	ostringstream oss;
	oss << "height " << player->get_pos().z << "\nspeed " << player->get_speed() * 3.6 << "km/h\norientation (quat): "
	<< player->get_orientation();
//	\nheading " << player->get_heading().value()
//	<< "\npitch " << player->get_pitch().value() << "\nroll " << player->get_roll().value();
	font_arial->print(0, 0, oss.str());
	sys().unprepare_2d_drawing();

	// mouse handling
	int mx;
	int my;
	int mb = sys().get_mouse_buttons();
	sys().get_mouse_position(mx, my);

	// keyboard processing
	int key = sys().get_key().sym;

	if (sys().is_key_down(SDLK_LEFT))
		gm.send(new command_roll_left(player));
	else if (sys().is_key_down(SDLK_RIGHT))
		gm.send(new command_roll_right(player));
	else
		gm.send(new command_roll_zero(player));
	if (sys().is_key_down(SDLK_UP))
		gm.send(new command_pitch_down(player));
	else if (sys().is_key_down(SDLK_DOWN))
		gm.send(new command_pitch_up(player));
	else
		gm.send(new command_pitch_zero(player));

	while (key != 0) {
		if (!keyboard_common(key, gm)) {
			// specific keyboard processing
/*			
			switch ( key ) {
				case SDLK_y:
					if ( zoom_scope )
						zoom_scope = false;
					else
						zoom_scope = true;
					break;
			}
*/			
		}
		key = sys().get_key().sym;
	}
}
