// user interface for controlling a submarine
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

submarine_interface::submarine_interface(game& gm) : 
    	user_interface(gm)
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
	add_loading_screen("submarine interface initialized");
}



submarine_interface::~submarine_interface()
{
}



void submarine_interface::process_input(game& gm, const SDL_Event& event)
{
	submarine* player = dynamic_cast<submarine*>(gm.get_player());

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
		if (mycfg.getkey(KEY_FIRE_TUBE_1).equal(event.key.keysym)) {
			// test. fixme replace all key commands, here and in *display classes
			cout << "it works!\n";
		}

		if (event.key.keysym.mod & (KMOD_LSHIFT | KMOD_RSHIFT)) {
			switch (event.key.keysym.sym) {
				// torpedo launching
			case SDLK_1:
			case SDLK_2:
			case SDLK_3:
			case SDLK_4:
			case SDLK_5:
			case SDLK_6:
				if (player->can_torpedo_be_launched(gm, event.key.keysym.sym - SDLK_1, target)) {
					add_message(texts::get(49));
					ostringstream oss;
					oss << texts::get(49);
					if (target)
						oss << " " << texts::get(6) << ": " << target->get_description(2 );
					gm.add_logbook_entry(oss.str());
					player->launch_torpedo(gm, event.key.keysym.sym - SDLK_1, target);
					play_sound_effect(gm, se_submarine_torpedo_launch );
				}
				break;
			case SDLK_LEFT:
				player->rudder_hard_left();
				add_rudder_message(gm);
				break;
			case SDLK_RIGHT:
				player->rudder_hard_right();
				add_rudder_message(gm);
				break;
				// view
			//case SDLK_COMMA : bearing -= angle(10); break;//fixme: bearing to ui!
			//case SDLK_PERIOD : bearing += angle(10); break;
			default:
				user_interface::process_input(gm, event);
			}
		} else {	// no shift
			switch (event.key.keysym.sym) {
			// display switching
			case SDLK_F1: current_display = display_mode_gauges; break;
			case SDLK_F2:
				if (player->get_depth() > player->get_periscope_depth()) {
					add_message(texts::get(28));
					// will later be replaced when scope can be raised in smaller steps...
					// no. height of scope and en/disabling are not the same.
				} else {
					current_display = display_mode_periscope;
				}
				break;
			case SDLK_F3:
				if (player->is_submerged()) {
					add_message(texts::get(27));
				} else {
					current_display = display_mode_uzo;
				}
				break;
			case SDLK_F4:
				if (player->is_submerged()) {
					add_message(texts::get(27));
				} else {
					current_display = display_mode_bridge;
				}
				break;
			case SDLK_F5: current_display = display_mode_map; break;
			case SDLK_F6: current_display = display_mode_torpedoroom; break;
			case SDLK_F7: current_display = display_mode_damagestatus; break;
			case SDLK_F8: current_display = display_mode_logbook; break;
			case SDLK_F9: current_display = display_mode_successes; break;
			case SDLK_F10: current_display = display_mode_freeview; /*freeviewpos = player_object->get_pos();*/ break;

				// time scaling fixme: too simple
			case SDLK_F11: if (time_scale_up()) { add_message(texts::get(31)); } break;
			case SDLK_F12: if (time_scale_down()) { add_message(texts::get(32)); } break;

				// control
			case SDLK_LEFT:
				player->rudder_left();
				add_rudder_message(gm);
				break;
			case SDLK_RIGHT:
				player->rudder_right();
				add_rudder_message(gm);
				break;
			case SDLK_UP: player->planes_up(1); add_message(texts::get(37)); break;
			case SDLK_DOWN: player->planes_down(1); add_message(texts::get(38)); break;
			case SDLK_c:
				// fixme: we should introduce a new command here, because crash diving
				// is different from normal diving
				player->dive_to_depth(unsigned(player->get_alarm_depth()));
				add_message(texts::get(41));
				gm.add_logbook_entry(texts::get(41));
				break;
			case SDLK_d:
				if (player->has_snorkel () )
					{
						player->dive_to_depth(unsigned(player->get_snorkel_depth()));
						add_message(texts::get(12));
						gm.add_logbook_entry(texts::get(97));
					}
				break;
			case SDLK_f:
				if ( player->has_snorkel () )
					{
						if ( player->is_snorkel_up () )
							{
								player->snorkel_down();
								//fixme: was an if, why? say "snorkel down only when it was down"
								{
									add_message (texts::get(96));
									gm.add_logbook_entry(texts::get(96));
								}
							}
						else
							{
								player->snorkel_up();
								//fixme: was an if, why? say "snorkel up only when it was up"
								{
									add_message ( texts::get(95));
									gm.add_logbook_entry(texts::get(95));
								}
							}
					}
				break;
			case SDLK_h:
				{
					angle new_course = player->get_heading () + bearing;
					bool turn_left = !player->get_heading().is_cw_nearer(new_course);
					player->head_to_ang(new_course, turn_left);
				}
				break;
			case SDLK_p:
				player->dive_to_depth(unsigned(player->get_periscope_depth()));
				add_message(texts::get(40));
				gm.add_logbook_entry(texts::get(40));
				break;	//fixme
			case SDLK_s:
				player->dive_to_depth(0);
				add_message(texts::get(39));
				gm.add_logbook_entry(texts::get(39));
				break;
			case SDLK_v:
				bearing = 0.0f;
				break;
			case SDLK_RETURN :
				player->rudder_midships();
				player->planes_middle();
				add_message(texts::get(42));
				break;
			case SDLK_1: player->set_throttle(ship::aheadslow); add_message(texts::get(43)); break;
			case SDLK_2: player->set_throttle(ship::aheadhalf); add_message(texts::get(44)); break;
			case SDLK_3: player->set_throttle(ship::aheadfull); add_message(texts::get(45)); break;
			case SDLK_4: player->set_throttle(ship::aheadflank); add_message(texts::get(46)); break;//flank/full change?
			case SDLK_5: player->set_throttle(ship::stop); add_message(texts::get(47)); break;
			case SDLK_6: player->set_throttle(ship::reverse); add_message(texts::get(48)); break;
			case SDLK_0: if (player->is_scope_up()) {
					player->scope_down(); add_message(texts::get(54)); } else {
					player->scope_up(); add_message(texts::get(55)); }
				break;

				// view
			case SDLK_COMMA : bearing -= angle(1); break;
			case SDLK_PERIOD : bearing += angle(1); break;

				// weapons, fixme
			case SDLK_t:
				if (player->can_torpedo_be_launched(gm, -1, target)) {
					add_message(texts::get(49));
					ostringstream oss;
					oss << texts::get(49);
					if (target)
						oss << " " << texts::get(6) << ": " << target->get_description ( 2 );
					gm.add_logbook_entry(oss.str());
					player->launch_torpedo(gm, -1, target);
					play_sound_effect (gm, se_submarine_torpedo_launch );
				}
				break;
			case SDLK_SPACE:
				target = gm.contact_in_direction(player, player->get_heading()+bearing);
				if (target)
					{
						add_message(texts::get(50));
						gm.add_logbook_entry(texts::get(50));
					}
				else
					add_message(texts::get(51));
				break;
			case SDLK_i:
				// calculate distance to target for identification detail
				if (target)
					{
						ostringstream oss;
						oss << texts::get(79) << target->get_description(2); // fixme
						add_message( oss.str () );
						gm.add_logbook_entry(oss.str());
					}
				else
					{
						add_message(texts::get(80));
					}
				break;

				// quit, screenshot, pause etc.
			case SDLK_ESCAPE:
				gm.stop();
				break;
			case SDLK_PRINT: system::sys().screenshot(); system::sys().add_console("screenshot taken."); break;
			case SDLK_PAUSE: pause = !pause;
				if (pause) add_message(texts::get(52));
				else add_message(texts::get(53));
				break;
			default:
				// let display handle the key
				user_interface::process_input(gm, event);
			}

		}
	} else {
		// fixme panel input. but panel visibility depens on each display... (yet)
		//a small(er) panel could be visible everywhere
		user_interface::process_input(gm, event);
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
	
void submarine_interface::display(game& gm) const
{
	submarine* player = dynamic_cast<submarine*>(gm.get_player());
	if ((current_display == display_mode_uzo || current_display == display_mode_bridge) &&
	    player->is_submerged()) {
		current_display = display_mode_periscope;
	}
	if (current_display == display_mode_periscope && player->get_depth() > player->get_periscope_depth()) {
		current_display = display_mode_map;
	}

	user_interface::display(gm);

	// panel is drawn in each display function, so the above code is all...
}
