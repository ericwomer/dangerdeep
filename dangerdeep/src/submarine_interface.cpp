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

// user interface for controlling a submarine
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#ifdef USE_NATIVE_GL
#include <gl.h>
#else
#include "oglext/OglExt.h"
#endif
#include <glu.h>
#include <SDL.h>

#include <sstream>
#include <map>
#include <set>
#include <list>
using namespace std;
#include "date.h"
#include "submarine_interface.h"
#include "system.h"
#include "game.h"
#include "texts.h"
#include "sound.h"
#include "image.h"
#include "widget.h"
#include "keys.h"
#include "cfg.h"

#include "sub_gauges_display.h"
#include "sub_periscope_display.h"
#include "sub_uzo_display.h"
#include "sub_bridge_display.h"
#include "map_display.h"
#include "sub_torpedo_display.h"
#include "sub_damage_display.h"
#include "logbook_display.h"
#include "ships_sunk_display.h"
#include "freeview_display.h"
#include "sub_tdc_display.h"
#include "sub_torpsetup_display.h"

#include "sub_control_popup.h"
#include "sub_tdc_popup.h"
#include "sub_ecard_popup.h"

submarine_interface::submarine_interface(game& gm) : 
    	user_interface(gm), selected_tube(0)
{
	displays.resize(nr_of_displays);
	displays[display_mode_gauges] = new sub_gauges_display(*this);
	displays[display_mode_periscope] = new sub_periscope_display(*this);
	displays[display_mode_uzo] = new sub_uzo_display(*this);
	displays[display_mode_bridge] = new sub_bridge_display(*this);
	displays[display_mode_map] = new map_display(*this);
	displays[display_mode_torpedoroom] = new sub_torpedo_display(*this);
	displays[display_mode_damagestatus] = new sub_damage_display(*this);
	displays[display_mode_logbook] = new logbook_display(*this);
	displays[display_mode_successes] = new ships_sunk_display(*this);
	displays[display_mode_freeview] = new freeview_display(*this);
	displays[display_mode_tdc] = new sub_tdc_display(*this);
	displays[display_mode_torpsetup] = new sub_torpsetup_display(*this);

	popups.resize(nr_of_popups);
	popups[popup_mode_control] = new sub_control_popup(*this);
	popups[popup_mode_tdc] = new sub_tdc_popup(*this);
	popups[popup_mode_ecard] = new sub_ecard_popup(*this);
	
	submarine* player = dynamic_cast<submarine*>(gm.get_player());
	player->start_throttle_sound();

	add_loading_screen("submarine interface initialized");
}



submarine_interface::~submarine_interface()
{
}



void submarine_interface::fire_tube(submarine* player, int nr)
{
	if (NULL != target && target != player)	{
		submarine::stored_torpedo::st_status tube_status = submarine::stored_torpedo::st_empty;		
		
		// fixme: ask TDC!
		// depends on direction of target and wether we have bow/stern tube and wether the
		// tube is not empty!
		if (true /*player->can_torpedo_be_launched(nr, target, tube_status)*/) {
			add_message(texts::get(49));
			ostringstream oss;
			oss << texts::get(49);
			if (target)
				oss << " " << texts::get(6) << ": " << target->get_description(2);
			mygame->add_logbook_entry(oss.str());
			player->launch_torpedo(nr, target);
			play_sound_effect(se_submarine_torpedo_launch, player, player);
		} else {
			string failed_to_fire_msg;
			
			if (-1 != nr) {
				switch (tube_status)
				{
					case submarine::stored_torpedo::st_empty:
						failed_to_fire_msg = texts::get(762);  
						break;
					case submarine::stored_torpedo::st_reloading:
						failed_to_fire_msg = texts::get(763);
						break;
					case submarine::stored_torpedo::st_unloading:
						failed_to_fire_msg = texts::get(764);
						break;
					default:
						// could happen when tube is loaded
						// but gyro angle is invalid for
						// current target
						return;
				}
			} else {
				if (true == player->get_torpedoes().empty())
					failed_to_fire_msg = texts::get(765);
				else
					failed_to_fire_msg = texts::get(766);
			}
			
			add_message(failed_to_fire_msg);
		}
	} else
		add_message(texts::get(80));
}



void submarine_interface::process_input(const SDL_Event& event)
{
	submarine* player = dynamic_cast<submarine*>(mygame->get_player());

	/*
	// check output for key input
	if (event.type == SDL_KEYDOWN) {
		cout << "pressed key " << SDL_GetKeyName(event.key.keysym.sym) << " for keysym " << unsigned(event.key.keysym.sym)
		<< " mod " << int(event.key.keysym.mod) << " unicode " << event.key.keysym.unicode << " scancode " <<
		unsigned(event.key.keysym.scancode) << "\n";
	}
	*/

	// check for common keys
	if (event.type == SDL_KEYDOWN) {
		const cfg& mycfg = cfg::instance();

		if (mycfg.getkey(KEY_SHOW_GAUGES_SCREEN).equal(event.key.keysym)) {
			set_current_display(display_mode_gauges);
		} else if (mycfg.getkey(KEY_SHOW_PERISCOPE_SCREEN).equal(event.key.keysym)) {
			if (player->get_depth() > player->get_periscope_depth()) {
				add_message(texts::get(28));
				// will later be replaced when scope can be raised in smaller steps...
				// no. height of scope and en/disabling are not the same.
			} else {
				set_current_display(display_mode_periscope);
			}
		} else if (mycfg.getkey(KEY_SHOW_UZO_SCREEN).equal(event.key.keysym)) {
			if (player->is_submerged()) {
				add_message(texts::get(27));
			} else {
				set_current_display(display_mode_uzo);
			}
		} else if (mycfg.getkey(KEY_SHOW_BRIDGE_SCREEN).equal(event.key.keysym)) {
			if (player->is_submerged()) {
				add_message(texts::get(27));
			} else {
				set_current_display(display_mode_bridge);
			}
		} else if (mycfg.getkey(KEY_SHOW_MAP_SCREEN).equal(event.key.keysym)) {
			set_current_display(display_mode_map);
		} else if (mycfg.getkey(KEY_SHOW_TORPEDO_SCREEN).equal(event.key.keysym)) {
			set_current_display(display_mode_torpedoroom);
		} else if (mycfg.getkey(KEY_SHOW_DAMAGE_CONTROL_SCREEN).equal(event.key.keysym)) {
			set_current_display(display_mode_damagestatus);
		} else if (mycfg.getkey(KEY_SHOW_LOGBOOK_SCREEN).equal(event.key.keysym)) {
			set_current_display(display_mode_logbook);
		} else if (mycfg.getkey(KEY_SHOW_SUCCESS_RECORDS_SCREEN).equal(event.key.keysym)) {
			set_current_display(display_mode_successes);
		} else if (mycfg.getkey(KEY_SHOW_FREEVIEW_SCREEN).equal(event.key.keysym)) {
			set_current_display(display_mode_freeview);
		} else if (mycfg.getkey(KEY_RUDDER_LEFT).equal(event.key.keysym)) {
			player->rudder_left();
			add_rudder_message();
		} else if (mycfg.getkey(KEY_RUDDER_HARD_LEFT).equal(event.key.keysym)) {
			player->rudder_hard_left();
			add_rudder_message();
		} else if (mycfg.getkey(KEY_RUDDER_RIGHT).equal(event.key.keysym)) {
			player->rudder_right();
			add_rudder_message();
		} else if (mycfg.getkey(KEY_RUDDER_HARD_RIGHT).equal(event.key.keysym)) {
			player->rudder_hard_right();
			add_rudder_message();
		} else if (mycfg.getkey(KEY_RUDDER_UP).equal(event.key.keysym)) {
			player->planes_up(1);
			add_message(texts::get(37));
		} else if (mycfg.getkey(KEY_RUDDER_HARD_UP).equal(event.key.keysym)) {
			player->planes_up(2);
			add_message(texts::get(37));
		} else if (mycfg.getkey(KEY_RUDDER_DOWN).equal(event.key.keysym)) {			
			add_message(texts::get(38));
			player->planes_down(1);
		} else if (mycfg.getkey(KEY_RUDDER_HARD_DOWN).equal(event.key.keysym)) {			
			add_message(texts::get(38));
			player->planes_down(2);
		} else if (mycfg.getkey(KEY_CENTER_RUDDERS).equal(event.key.keysym)) {
			player->rudder_midships();
			player->planes_middle();
			add_message(texts::get(42));
		} else if (mycfg.getkey(KEY_THROTTLE_SLOW).equal(event.key.keysym)) {
			player->set_throttle(ship::aheadslow);
			add_message(texts::get(43));
		} else if (mycfg.getkey(KEY_THROTTLE_HALF).equal(event.key.keysym)) {
			player->set_throttle(ship::aheadhalf);
			add_message(texts::get(44));
		} else if (mycfg.getkey(KEY_THROTTLE_FULL).equal(event.key.keysym)) {
			player->set_throttle(ship::aheadfull);
			add_message(texts::get(45));
		} else if (mycfg.getkey(KEY_THROTTLE_FLANK).equal(event.key.keysym)) {
			player->set_throttle(ship::aheadflank);
			add_message(texts::get(46));
		} else if (mycfg.getkey(KEY_THROTTLE_STOP).equal(event.key.keysym)) {
			player->set_throttle(ship::stop);
			add_message(texts::get(47));
		} else if (mycfg.getkey(KEY_THROTTLE_REVERSE).equal(event.key.keysym)) {
			player->set_throttle(ship::reverse);
			add_message(texts::get(48));
		} else if (mycfg.getkey(KEY_FIRE_TUBE_1).equal(event.key.keysym)) {
			fire_tube(player, 0);
		} else if (mycfg.getkey(KEY_FIRE_TUBE_2).equal(event.key.keysym)) {
			fire_tube(player, 1);
		} else if (mycfg.getkey(KEY_FIRE_TUBE_3).equal(event.key.keysym)) {
			fire_tube(player, 2);
		} else if (mycfg.getkey(KEY_FIRE_TUBE_4).equal(event.key.keysym)) {
			fire_tube(player, 3);
		} else if (mycfg.getkey(KEY_FIRE_TUBE_5).equal(event.key.keysym)) {
			fire_tube(player, 4);
		} else if (mycfg.getkey(KEY_FIRE_TUBE_6).equal(event.key.keysym)) {
			fire_tube(player, 5);
		} else if (mycfg.getkey(KEY_SELECT_TARGET).equal(event.key.keysym)) {
			sea_object* tgt = mygame->contact_in_direction(player, get_absolute_bearing());
			// set initial tdc values, also do that when tube is switched
			player->set_target(tgt);

			target = mygame->contact_in_direction(player, get_absolute_bearing());
			if (target) {
				add_message(texts::get(50));
				mygame->add_logbook_entry(texts::get(50));
			} else {
				add_message(texts::get(51));
			}
		} else if (mycfg.getkey(KEY_SCOPE_UP_DOWN).equal(event.key.keysym)) {
			if (player->is_scope_up()) {
				player->scope_down();
				add_message(texts::get(54));
			} else {
				player->scope_up();
				add_message(texts::get(55));
			}
		} else if (mycfg.getkey(KEY_CRASH_DIVE).equal(event.key.keysym)) {
			// fixme: we should introduce a new command here, because crash diving
			// is different from normal diving
			add_message(texts::get(41));
			mygame->add_logbook_entry(texts::get(41));
			player->dive_to_depth(unsigned(player->get_alarm_depth()));
		} else if (mycfg.getkey(KEY_GO_TO_SNORKEL_DEPTH).equal(event.key.keysym)) {
			if (player->has_snorkel () ) {
				player->dive_to_depth(unsigned(player->get_snorkel_depth()));
				add_message(texts::get(12));
				mygame->add_logbook_entry(texts::get(97));
			}
		} else if (mycfg.getkey(KEY_TOGGLE_SNORKEL).equal(event.key.keysym)) {
			if ( player->has_snorkel () ) {
				if ( player->is_snorkel_up () ) {
					player->snorkel_down();
					//fixme: was an if, why? say "snorkel down only when it was down"
					add_message (texts::get(96));
					mygame->add_logbook_entry(texts::get(96));
				} else {
					player->snorkel_up();
					//fixme: was an if, why? say "snorkel up only when it was up"
					add_message ( texts::get(95));
					mygame->add_logbook_entry(texts::get(95));
				}
			}
		} else if (mycfg.getkey(KEY_SET_HEADING_TO_VIEW).equal(event.key.keysym)) {
			angle new_course = get_absolute_bearing();
			bool turn_left = !player->get_heading().is_cw_nearer(new_course);
			player->head_to_ang(new_course, turn_left);
		} else if (mycfg.getkey(KEY_IDENTIFY_TARGET).equal(event.key.keysym)) {
			// calculate distance to target for identification detail
			if (target) {
				ostringstream oss;
				oss << texts::get(79) << target->get_description(2); // fixme
				add_message( oss.str () );
				mygame->add_logbook_entry(oss.str());
			} else {
				add_message(texts::get(80));
			}
		} else if (mycfg.getkey(KEY_GO_TO_PERISCOPE_DEPTH).equal(event.key.keysym)) {
			add_message(texts::get(40));
			mygame->add_logbook_entry(texts::get(40));
			player->dive_to_depth(unsigned(player->get_periscope_depth()));
		} else if (mycfg.getkey(KEY_GO_TO_SURFACE).equal(event.key.keysym)) {
			player->dive_to_depth(0);
			add_message(texts::get(39));
			mygame->add_logbook_entry(texts::get(39));
		} else if (mycfg.getkey(KEY_FIRE_TORPEDO).equal(event.key.keysym)) {
			fire_tube(player, -1);
		} else if (mycfg.getkey(KEY_SET_VIEW_TO_HEADING).equal(event.key.keysym)) {
			bearing = (bearing_is_relative) ? 0.0 : player->get_heading();
		} else if (mycfg.getkey(KEY_TURN_VIEW_LEFT).equal(event.key.keysym)) {
			add_bearing(angle(-1));
		} else if (mycfg.getkey(KEY_TURN_VIEW_LEFT_FAST).equal(event.key.keysym)) {
			add_bearing(angle(-10));
		} else if (mycfg.getkey(KEY_TURN_VIEW_RIGHT).equal(event.key.keysym)) {
			add_bearing(angle(1));
		} else if (mycfg.getkey(KEY_TURN_VIEW_RIGHT_FAST).equal(event.key.keysym)) {
			add_bearing(angle(10));
		} else if (mycfg.getkey(KEY_TIME_SCALE_UP).equal(event.key.keysym)) {
			if (time_scale_up()) {
				add_message(texts::get(31));
			}
		} else if (mycfg.getkey(KEY_TIME_SCALE_DOWN).equal(event.key.keysym)) {
			if (time_scale_down()) {
				add_message(texts::get(32));
			}
		} else if (mycfg.getkey(KEY_FIRE_DECK_GUN).equal(event.key.keysym)) {
			if (true == player->has_deck_gun())
			{
				if (false == player->is_submerged())
				{
					if (NULL != target && player != target)
					{						
						int res = player->fire_shell_at(*target);
						
						if (TARGET_OUT_OF_RANGE == res)
							add_message(texts::get(218));
						else if (NO_AMMO_REMAINING == res)
							add_message(texts::get(219));
						else if (RELOADING == res)
							add_message(texts::get(758));
						else if (GUN_NOT_MANNED == res)
							add_message(texts::get(759));
						else if (GUN_TARGET_IN_BLINDSPOT == res)
							add_message(texts::get(760));					
					}
					else
						add_message(texts::get(80));
				}
				else
					add_message(texts::get(27));
			}
		} else if (mycfg.getkey(KEY_TOGGLE_MAN_DECK_GUN).equal(event.key.keysym)) {
			if (true == player->has_deck_gun())
			{
				if (false == player->is_submerged())
				{
					if (event.key.keysym.mod & (KMOD_LSHIFT | KMOD_RSHIFT))
					{
						if (true == player->toggle_gun_manning())
							add_message(texts::get(761));
						else
							add_message(texts::get(754));
					}
				}			
				else
					add_message(texts::get(27));
			}	
		} else if (mycfg.getkey(KEY_SHOW_TDC_SCREEN).equal(event.key.keysym)) {
			set_current_display(display_mode_tdc);
		} else if (mycfg.getkey(KEY_SHOW_TORPSETUP_SCREEN).equal(event.key.keysym)) {
			set_current_display(display_mode_torpsetup);
		} else {
			// rest of the keys per switch (not user defineable)
			// quit, screenshot, pause etc.
			switch (event.key.keysym.sym) {
			case SDLK_ESCAPE:
				mygame->stop();
				break;
			case SDLK_PRINT:
				sys().screenshot();
				sys().add_console("screenshot taken.");
				break;
			case SDLK_PAUSE:
				pause_game(!pause);
				break;
			default:
				// let display handle the key
				user_interface::process_input(event);
			}
		}
	} else {
		// fixme panel input. but panel visibility depens on each display... (yet)
		//a small(er) panel could be visible everywhere
		user_interface::process_input(event);
	}

}

/*
bool submarine_interface::object_visible(sea_object* so,
	const vector2& dl, const vector2& dr) const //fixme buggy
{
	vector2 p = so->get_pos().xy();
	double rad = so->get_length()/2, s, t;	// most objects are longer than wide...fixme
	s = p.x*dl.x + p.y*dl.y;
	t = p.y*dl.x - p.x*dl.y;
	if (s < -rad || t > rad) return false;
	s = p.x*dr.x + p.y*dr.y;
	t = p.y*dr.x - p.x*dr.y;
	if (s < -rad || t < -rad) return false;
	return true;
}
*/
	
void submarine_interface::display(void) const
{
	submarine* player = dynamic_cast<submarine*>(mygame->get_player());
	if ((current_display == display_mode_uzo || current_display == display_mode_bridge) &&
	    player->is_submerged()) {
		set_current_display(display_mode_periscope);
	}
	if (current_display == display_mode_periscope && player->get_depth() > player->get_periscope_depth()) {
		set_current_display(display_mode_map);
	}

	user_interface::display();

	// panel is drawn in each display function, so the above code is all...
}
