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
#include "date.h"
#include "submarine_interface.h"
#include "system.h"
#include "game.h"
#include "texts.h"
#include "image.h"
#include "widget.h"
#include "keys.h"
#include "cfg.h"
#include "global_data.h"
#include "music.h"
#include "particle.h" // for hack, fixme
#include "log.h"

#include "sub_gauges_display.h"
#include "sub_periscope_display.h"
#include "sub_uzo_display.h"
#include "sub_bridge_display.h"
#include "map_display.h"
#include "sub_torpedo_display.h"
#include "sub_damage_display.h"
#include "logbook_display.h"
#include "freeview_display.h"
#include "sub_tdc_display.h"
#include "sub_torpsetup_display.h"
#include "sub_kdb_display.h"
#include "sub_ghg_display.h"
#include "sub_bg_display.h"
#include "sub_captainscabin_display.h"
#include "sub_soldbuch_display.h"
#include "sub_recogmanual_display.h"

#include "torpedo_camera_display.h"

#include "sub_control_popup.h"
#include "sub_tdc_popup.h"
#include "sub_ecard_popup.h"
#include "sub_recogmanual_popup.h"

using namespace std;

submarine_interface::submarine_interface(game& gm) : 
    	user_interface(gm), selected_tube(0), torpedo_cam_track_nr(0)
{
	submarine* player = dynamic_cast<submarine*>(gm.get_player());

	displays.resize(nr_of_displays);
	displays.reset(display_mode_gauges, new sub_gauges_display(*this));
	displays.reset(display_mode_periscope, new sub_periscope_display(*this));
	displays.reset(display_mode_uzo, new sub_uzo_display(*this));
	displays.reset(display_mode_bridge, new sub_bridge_display(*this));
	displays.reset(display_mode_map, new map_display(*this));
	displays.reset(display_mode_torpedoroom, new sub_torpedo_display(*this));
	displays.reset(display_mode_damagestatus, new sub_damage_display(*this));
	displays.reset(display_mode_logbook, new logbook_display(*this));
	displays.reset(display_mode_captainscabin, new sub_captainscabin_display(*this));
	displays.reset(display_mode_successes, new sub_soldbuch_display(*this));
	displays.reset(display_mode_recogmanual, new sub_recogmanual_display(*this));
	switch (player->get_hearing_device_type()) {
	case submarine::hearing_device_KDB:
		displays.reset(display_mode_sonar, new sub_kdb_display(*this));
		break;
	default:
	case submarine::hearing_device_GHG:
		displays.reset(display_mode_sonar, new sub_ghg_display(*this));
		break;
	case submarine::hearing_device_BG:
		displays.reset(display_mode_sonar, new sub_bg_display(*this));
		break;
	}
	displays.reset(display_mode_freeview, new freeview_display(*this));
	displays.reset(display_mode_tdc, new sub_tdc_display(*this));
	displays.reset(display_mode_torpsetup, new sub_torpsetup_display(*this));

	// fixme: use texture caches here too...
	popups.resize(nr_of_popups);
	popups.reset(popup_mode_control, new sub_control_popup(*this));
	popups.reset(popup_mode_tdc, new sub_tdc_popup(*this));
	popups.reset(popup_mode_ecard, new sub_ecard_popup(*this));
	popups.reset(popup_mode_recogmanual, new sub_recogmanual_popup(*this));

	// torpedo cam
	torpedo_cam_view.reset(new torpedo_camera_display(*this));

	// note: we could change the width of the menu dynamically, according to longest text of the
	// buttons.
// 	int maxs = 0;
// 	for (unsigned i = 247; i <= 260; ++i)
// 		maxs = std::max(widget::get_theme()->myfont->get_size(texts::get(i)).x, maxs);
	widget_menu* screen_selector_menu = new widget_menu(0, 0, /*maxs + 16*/ 256, 32, texts::get(247));
	screen_selector->add_child_near_last_child(screen_selector_menu);
	screen_selector_menu->set_entry_spacing(0);
	typedef widget_caller_button<submarine_interface, void (submarine_interface::*)()> wcbsubi;
	screen_selector_menu->add_entry(texts::get(248), new wcbsubi(this, &submarine_interface::goto_gauges));
	screen_selector_menu->add_entry(texts::get(249), new wcbsubi(this, &submarine_interface::goto_periscope));
	screen_selector_menu->add_entry(texts::get(250), new wcbsubi(this, &submarine_interface::goto_UZO));
	screen_selector_menu->add_entry(texts::get(251), new wcbsubi(this, &submarine_interface::goto_bridge));
	screen_selector_menu->add_entry(texts::get(252), new wcbsubi(this, &submarine_interface::goto_map));
	screen_selector_menu->add_entry(texts::get(253), new wcbsubi(this, &submarine_interface::goto_torpedomanagement));
	screen_selector_menu->add_entry(texts::get(254), new wcbsubi(this, &submarine_interface::goto_damagecontrol));
	screen_selector_menu->add_entry(texts::get(271), new wcbsubi(this, &submarine_interface::goto_captainscabin));
	screen_selector_menu->add_entry(texts::get(255), new wcbsubi(this, &submarine_interface::goto_logbook));
	screen_selector_menu->add_entry(texts::get(272), new wcbsubi(this, &submarine_interface::goto_successes));
	screen_selector_menu->add_entry(texts::get(256), new wcbsubi(this, &submarine_interface::goto_sonar));
	screen_selector_menu->add_entry(texts::get(257), new wcbsubi(this, &submarine_interface::goto_freeview));
	screen_selector_menu->add_entry(texts::get(258), new wcbsubi(this, &submarine_interface::goto_TDC));
	screen_selector_menu->add_entry(texts::get(259), new wcbsubi(this, &submarine_interface::goto_torpedosettings));
	screen_selector_menu->add_entry(texts::get(260), new widget_set_button<bool>(screen_selector_visible, false));
	screen_selector->clip_to_children_area();
	screen_selector->set_pos(vector2i(0, 0));

	// note! we could add a second menu with the most common actions here...

	// fixme: later set "current_display" to other default value, like captain's cabin.

	// Important! call enter() here for current display.
	displays[current_display]->enter(gm.is_day_mode());

	add_loading_screen("submarine interface initialized");
}



submarine_interface::~submarine_interface()
{
}



void submarine_interface::fire_tube(submarine* player, int nr)
{
	if (player->get_target() && player->get_target() != player) {
		bool ok = player->launch_torpedo(nr, player->get_target());
		if (ok) {
			add_message(texts::get(49));
			ostringstream oss;
			oss << texts::get(49);
			if (player->get_target())
				oss << " " << texts::get(6) << ": " << player->get_target()->get_description(2);
			mygame->add_logbook_entry(oss.str());
			play_sound_effect(SFX_TUBE_LAUNCH, player->get_pos());
		} else {
			add_message(texts::get(138));
		}

#if 0 // old code
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
			play_sound_effect(se_submarine_torpedo_launch, player->get_pos());
		} else {
			string failed_to_fire_msg;
			
			if (-1 != nr) {
				switch (tube_status)
				{
					case submarine::stored_torpedo::st_empty:
						failed_to_fire_msg = texts::get(134);  
						break;
					case submarine::stored_torpedo::st_reloading:
						failed_to_fire_msg = texts::get(135);
						break;
					case submarine::stored_torpedo::st_unloading:
						failed_to_fire_msg = texts::get(136);
						break;
					default:
						// could happen when tube is loaded
						// but gyro angle is invalid for
						// current target
						return;
				}
			} else {
				if (true == player->get_torpedoes().empty())
					failed_to_fire_msg = texts::get(137);
				else
					failed_to_fire_msg = texts::get(138);
			}
			
			add_message(failed_to_fire_msg);
		}
#endif
	} else {
		add_message(texts::get(80));
	}
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

	// fixme: if editor needs key input (CV name or mission description etc.)
	// we need to fetch the event to some widgets, and not use it here!

	// switch screen selector on if it is not visible
	if (event.type == SDL_MOUSEBUTTONDOWN) {
		if (event.button.button == SDL_BUTTON_RIGHT) {
			if (!main_menu_visible) {
				main_menu_visible = true;
				return;
			}
		}
	}
	// Note!!! do NOT unite this if with the if/else block below!
	// we need to check the event mentioned above extra!

	// check for common keys
	if (event.type == SDL_KEYDOWN) {
		const cfg& mycfg = cfg::instance();

		// SCREENS
		if (mycfg.getkey(KEY_SHOW_GAUGES_SCREEN).equal(event.key.keysym)) {
			goto_gauges();
		} else if (mycfg.getkey(KEY_SHOW_PERISCOPE_SCREEN).equal(event.key.keysym)) {
			goto_periscope();
		} else if (mycfg.getkey(KEY_SHOW_UZO_SCREEN).equal(event.key.keysym)) {
			goto_UZO();
		} else if (mycfg.getkey(KEY_SHOW_BRIDGE_SCREEN).equal(event.key.keysym)) {
			goto_bridge();
		} else if (mycfg.getkey(KEY_SHOW_MAP_SCREEN).equal(event.key.keysym)) {
			goto_map();
		} else if (mycfg.getkey(KEY_SHOW_TORPEDO_SCREEN).equal(event.key.keysym)) {
			goto_torpedomanagement();
		} else if (mycfg.getkey(KEY_SHOW_DAMAGE_CONTROL_SCREEN).equal(event.key.keysym)) {
			goto_damagecontrol();
		} else if (mycfg.getkey(KEY_SHOW_LOGBOOK_SCREEN).equal(event.key.keysym)) {
			goto_captainscabin();
		} else if (mycfg.getkey(KEY_SHOW_SUCCESS_RECORDS_SCREEN).equal(event.key.keysym)) {
			goto_sonar();
		} else if (mycfg.getkey(KEY_SHOW_FREEVIEW_SCREEN).equal(event.key.keysym)) {
			goto_freeview();
		} else if (mycfg.getkey(KEY_SHOW_TDC_SCREEN).equal(event.key.keysym)) {
			goto_TDC();
		} else if (mycfg.getkey(KEY_SHOW_TORPSETUP_SCREEN).equal(event.key.keysym)) {
			goto_torpedosettings();
		} else if (mycfg.getkey(KEY_SHOW_TORPEDO_CAMERA).equal(event.key.keysym)) {
			// show next torpedo in torpedo camera view
			torpedo_cam_track_nr++;
			
		// MOVEMENT
		} else if (mycfg.getkey(KEY_RUDDER_LEFT).equal(event.key.keysym)) {
			player->set_rudder(ship::rudderleft);
			add_message(texts::get(33));
		} else if (mycfg.getkey(KEY_RUDDER_HARD_LEFT).equal(event.key.keysym)) {
			player->set_rudder(ship::rudderfullleft);
			add_message(texts::get(35));
		} else if (mycfg.getkey(KEY_RUDDER_RIGHT).equal(event.key.keysym)) {
			player->set_rudder(ship::rudderright);
			add_message(texts::get(34));
		} else if (mycfg.getkey(KEY_RUDDER_HARD_RIGHT).equal(event.key.keysym)) {
			player->set_rudder(ship::rudderfullright);
			add_message(texts::get(36));
		} else if (mycfg.getkey(KEY_RUDDER_UP).equal(event.key.keysym)) {
			player->set_planes_to(-0.5);
			add_message(texts::get(37));
		} else if (mycfg.getkey(KEY_RUDDER_HARD_UP).equal(event.key.keysym)) {
			player->set_planes_to(-1.0);
			add_message(texts::get(37));
		} else if (mycfg.getkey(KEY_RUDDER_DOWN).equal(event.key.keysym)) {			
			add_message(texts::get(38));
			player->set_planes_to(0.5);
		} else if (mycfg.getkey(KEY_RUDDER_HARD_DOWN).equal(event.key.keysym)) {			
			add_message(texts::get(38));
			player->set_planes_to(1.0);
		} else if (mycfg.getkey(KEY_CENTER_RUDDERS).equal(event.key.keysym)) {
			player->set_rudder(ship::ruddermidships);
			player->set_planes_to(0);
			add_message(texts::get(42));
			
		// THROTTLE
		} else if (mycfg.getkey(KEY_THROTTLE_LISTEN).equal(event.key.keysym)) {
			player->set_throttle(ship::aheadlisten);
			add_message(texts::get(139));
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
		} else if (mycfg.getkey(KEY_THROTTLE_REVERSEHALF).equal(event.key.keysym)) {
			player->set_throttle(ship::reversehalf);
			add_message(texts::get(140));
		} else if (mycfg.getkey(KEY_THROTTLE_REVERSEFULL).equal(event.key.keysym)) {
			player->set_throttle(ship::reversefull);
			add_message(texts::get(141));
			
		// TORPEDOES
		} else if (mycfg.getkey(KEY_FIRE_TORPEDO).equal(event.key.keysym)) {
			fire_tube(player, -1);
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
			if (tgt) {
				add_message(texts::get(50));
				mygame->add_logbook_entry(texts::get(50));
			} else {
				add_message(texts::get(51));
			}
			
		// DEPTH, SNORKEL, SCOPE
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
			if (player->get_target()) {
				ostringstream oss;
				oss << texts::get(79) << player->get_target()->get_description(2); // fixme
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
			
		// VIEWS
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
			
		// TIME SCALE
		} else if (mycfg.getkey(KEY_TIME_SCALE_UP).equal(event.key.keysym)) {
			if (time_scale_up()) {
				add_message(texts::get(31));
			}
		} else if (mycfg.getkey(KEY_TIME_SCALE_DOWN).equal(event.key.keysym)) {
			if (time_scale_down()) {
				add_message(texts::get(32));
			}
			
		// GUNS
		} else if (mycfg.getkey(KEY_FIRE_DECK_GUN).equal(event.key.keysym)) {
			if (player->has_deck_gun()) {
				if (!player->is_submerged()) {
					if (player->get_target() && player->get_target() != player) {
						int res = player->fire_shell_at(*(player->get_target()));
						if (ship::TARGET_OUT_OF_RANGE == res)
							add_message(texts::get(218));
						else if (ship::NO_AMMO_REMAINING == res)
							add_message(texts::get(219));
						else if (ship::RELOADING == res)
							add_message(texts::get(130));
						else if (ship::GUN_NOT_MANNED == res)
							add_message(texts::get(131));
						else if (ship::GUN_TARGET_IN_BLINDSPOT == res)
							add_message(texts::get(132));
						else if (ship::GUN_FIRED == res)
							add_message(texts::get(270));
					} else {
						add_message(texts::get(80));
					}
				} else {
					add_message(texts::get(27));
				}
			}
		} else if (mycfg.getkey(KEY_TOGGLE_MAN_DECK_GUN).equal(event.key.keysym)) {
			if (player->has_deck_gun()) {
				if (!player->is_submerged()) {
					if (event.key.keysym.mod & (KMOD_LSHIFT | KMOD_RSHIFT)) {
						if (player->is_gun_manned()) {
							if (player->unman_guns())
								add_message(texts::get(126));
						} else {
							if (player->man_guns())
								add_message(texts::get(133));
						}
					}
				} else {
					add_message(texts::get(27));
				}
			} else {
				add_message(texts::get(269));
			}
		
		// DEFAULT
		} else {
			// rest of the keys per switch (not user defineable)
			// quit, screenshot, pause etc.
			switch (event.key.keysym.sym) {
			case SDLK_ESCAPE:
				request_abort();
				break;
			case SDLK_PRINT:
				sys().screenshot();
				log_info("screenshot taken.");
				break;
			case SDLK_PAUSE:
				toggle_pause();
				break;
#if 1 // fixme test hack
			case SDLK_r:
				mygame->spawn_particle(new fireworks_particle(mygame->get_player()->get_pos() + vector3(0, 0, 5)));
				break;
#endif
			default:
				// let display handle the key
				user_interface::process_input(event);
			}
		}
	
	// Try next level
	} else {
		// fixme panel input. but panel visibility depens on each display... (yet)
		//a small(er) panel could be visible everywhere
		user_interface::process_input(event);
	}

}



void submarine_interface::goto_gauges()
{
	set_current_display(display_mode_gauges);
}



void submarine_interface::goto_periscope()
{
	submarine* player = dynamic_cast<submarine*>(mygame->get_player());
	if (player->get_depth() > player->get_periscope_depth()) {
		add_message(texts::get(28));
		// will later be replaced when scope can be raised in smaller steps...
		// no. height of scope and en/disabling are not the same.
	} else {
		set_current_display(display_mode_periscope);
	}
}



void submarine_interface::goto_UZO()
{
	submarine* player = dynamic_cast<submarine*>(mygame->get_player());
	if (player->is_submerged()) {
		add_message(texts::get(27));
	} else {
		set_current_display(display_mode_uzo);
	}
}



void submarine_interface::goto_bridge()
{
	submarine* player = dynamic_cast<submarine*>(mygame->get_player());
	if (player->is_submerged()) {
		add_message(texts::get(27));
	} else {
		set_current_display(display_mode_bridge);
	}
}



void submarine_interface::goto_map()
{
	set_current_display(display_mode_map);
}



void submarine_interface::goto_torpedomanagement()
{
	set_current_display(display_mode_torpedoroom);
}



void submarine_interface::goto_damagecontrol()
{
	set_current_display(display_mode_damagestatus);
}



void submarine_interface::goto_captainscabin()
{
	set_current_display(display_mode_captainscabin);
}



void submarine_interface::goto_logbook()
{
	set_current_display(display_mode_logbook);
}



void submarine_interface::goto_successes()
{
	set_current_display(display_mode_successes);
}



void submarine_interface::goto_sonar()
{
	set_current_display(display_mode_sonar);
}



void submarine_interface::goto_freeview()
{
	set_current_display(display_mode_freeview);
}



void submarine_interface::goto_TDC()
{
	set_current_display(display_mode_tdc);
}



void submarine_interface::goto_torpedosettings()
{
	set_current_display(display_mode_torpsetup);
}


void submarine_interface::goto_recogmanual()
{
	set_current_display(display_mode_recogmanual);
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
	
void submarine_interface::toggle_popup()
{
	if (current_display == display_mode_tdc) {
		static_cast<sub_tdc_display*>(displays[current_display])->next_sub_screen(daymode);
	} else {
		user_interface::toggle_popup();
	}
}

void submarine_interface::display() const
{
	submarine* player = dynamic_cast<submarine*>(mygame->get_player());
	if ((current_display == display_mode_uzo || current_display == display_mode_bridge) &&
	    player->is_submerged()) {
		set_current_display(display_mode_periscope);
	}
	if (current_display == display_mode_periscope && player->get_depth() > player->get_periscope_depth()) {
		set_current_display(display_mode_map);
	}

	// machine sound
	unsigned thr = 0;
	switch (player->get_throttle()) {
	case ship::reversefull: thr = 100; break;
	case ship::reversehalf: thr =  66; break;
	case ship::reverse:     thr =  33; break;
	case ship::aheadlisten: thr =  20; break;
	case ship::aheadsonar:  thr =  20; break;
	case ship::aheadslow:   thr =  40; break;
	case ship::aheadhalf:   thr =  60; break;
	case ship::aheadfull:   thr =  80; break;
	case ship::aheadflank:  thr = 100; break;
	case ship::stop:        thr =   0; break;
	default:          thr =   0; break; // throttled speeds not supported here...
	}
	music::inst().play_sfx_machine(SFX_MACHINE_SUB_DIESEL, thr);

	user_interface::display();

	// panel is drawn in each display function, so the above code is all...

	if (torpedo_cam_track_nr > 0) {
		const torpedo* tt = mygame->get_torpedo_for_camera_track(torpedo_cam_track_nr-1);
		torpedo_cam_view->set_tracker(tt);
		if (tt) {
			/*
			glMatrixMode(GL_PROJECTION);
			glPushMatrix();
			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			*/
			torpedo_cam_view->display(*mygame);
			/*
			glMatrixMode(GL_PROJECTION);
			glPopMatrix();
			glMatrixMode(GL_MODELVIEW);
			glPopMatrix();
			*/
		} else {
			torpedo_cam_track_nr = 0;
		}
	} else {
		torpedo_cam_view->set_tracker(0);
	}
}
