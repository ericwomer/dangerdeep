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
#include "command.h"

#include "sub_gauges_display.h"
//#include "sub_periscope_display.h"
//#include "sub_uzo_display.h"
//#include "sub_bridge_display.h"
#include "map_display.h"
#include "sub_torpedo_display.h"
#include "sub_damage_display.h"
#include "logbook.h"
#include "ships_sunk_display.h"
//#include "freeview_display.h"

submarine_interface::submarine_interface(game& gm) : 
    	user_interface(gm)
{
	displays.resize(nr_of_displays);
	displays[display_mode_gauges] = new sub_gauges_display(*this);
	displays[display_mode_periscope] = new sub_gauges_display(*this);
	//displays[display_mode_periscope] = new sub_periscope_display(*this);
	displays[display_mode_uzo] = new sub_gauges_display(*this);
	//displays[display_mode_uzo] = new sub_uzo_display(*this);
	displays[display_mode_bridge] = new sub_gauges_display(*this);
	//displays[display_mode_bridge] = new sub_bridge_display(*this);
	displays[display_mode_map] = new map_display(*this);
	displays[display_mode_torpedoroom] = new sub_torpedo_display(*this);
	displays[display_mode_damagestatus] = new sub_damage_display(*this);
	displays[display_mode_logbook] = new captains_logbook_display(*this);
	displays[display_mode_successes] = new ships_sunk_display(*this);
	displays[display_mode_freeview] = new sub_gauges_display(*this);
	//displays[display_mode_freeview] = new freeview_display(*this);
}



submarine_interface::~submarine_interface()
{
}



void submarine_interface::process_input(game& gm, const SDL_Event& event)
{
	submarine* player = dynamic_cast<submarine*>(gm.get_player());
	// check for common keys
	if (event.type == SDL_KEYDOWN) {
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
					//add_captains_log_entry( gm, oss.str () );
					gm.send(new command_launch_torpedo(player, event.key.keysym.sym - SDLK_1, target));
					play_sound_effect(gm, se_submarine_torpedo_launch );
				}
				break;
			case SDLK_LEFT:
				gm.send(new command_rudder_hard_left(player));
				add_rudder_message(gm);
				break;
			case SDLK_RIGHT:
				gm.send(new command_rudder_hard_right(player));
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
			case SDLK_F2: current_display = display_mode_periscope; break;
			case SDLK_F3: current_display = display_mode_uzo; break;
			case SDLK_F4: current_display = display_mode_bridge; break;
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
				gm.send(new command_rudder_left(player));
				add_rudder_message(gm);
				break;
			case SDLK_RIGHT:
				gm.send(new command_rudder_right(player));
				add_rudder_message(gm);
				break;
			case SDLK_UP: gm.send(new command_planes_up(player, 1)); add_message(texts::get(37)); break;
			case SDLK_DOWN: gm.send(new command_planes_down(player, 1)); add_message(texts::get(38)); break;
			case SDLK_c:
				// fixme: we should introduce a new command here, because crash diving
				// is different from normal diving
				gm.send(new command_dive_to_depth(player, unsigned(player->get_alarm_depth())));
				add_message(texts::get(41));
				//add_captains_log_entry(gm, texts::get(41));
				break;
			case SDLK_d:
				if (player->has_snorkel () )
					{
						gm.send(new command_dive_to_depth(player, unsigned(player->get_snorkel_depth())));
						add_message(texts::get(12));
						//add_captains_log_entry ( gm, texts::get(97));
					}
				break;
			case SDLK_f:
				if ( player->has_snorkel () )
					{
						if ( player->is_snorkel_up () )
							{
								gm.send(new command_snorkel_down ( player ) );
								//fixme: was an if, why? say "snorkel down only when it was down"
								{
									add_message (texts::get(96));
									//add_captains_log_entry ( gm, texts::get(96));
								}
							}
						else
							{
								gm.send(new command_snorkel_up ( player ) );
								//fixme: was an if, why? say "snorkel up only when it was up"
								{
									add_message ( texts::get(95));
									//add_captains_log_entry ( gm, texts::get(95));
								}
							}
					}
				break;
			case SDLK_h:
				{
					angle new_course = player->get_heading () + bearing;
					bool turn_left = !player->get_heading().is_cw_nearer(new_course);
					gm.send(new command_head_to_ang (player, new_course, turn_left ));
				}
				break;
			case SDLK_p:
				gm.send(new command_dive_to_depth(player, unsigned(player->get_periscope_depth())));
				add_message(texts::get(40));
				//add_captains_log_entry ( gm, texts::get(40));
				break;	//fixme
			case SDLK_s:
				gm.send(new command_dive_to_depth(player, 0));
				add_message(texts::get(39));
				//add_captains_log_entry ( gm, texts::get(39));
				break;
			case SDLK_v:
				bearing = 0.0f;
				break;
			case SDLK_RETURN :
				gm.send(new command_rudder_midships(player));
				gm.send(new command_planes_middle(player));
				add_message(texts::get(42));
				break;
			case SDLK_1: gm.send(new command_set_throttle(player, ship::aheadslow)); add_message(texts::get(43)); break;
			case SDLK_2: gm.send(new command_set_throttle(player, ship::aheadhalf)); add_message(texts::get(44)); break;
			case SDLK_3: gm.send(new command_set_throttle(player, ship::aheadfull)); add_message(texts::get(45)); break;
			case SDLK_4: gm.send(new command_set_throttle(player, ship::aheadflank)); add_message(texts::get(46)); break;//flank/full change?
			case SDLK_5: gm.send(new command_set_throttle(player, ship::stop)); add_message(texts::get(47)); break;
			case SDLK_6: gm.send(new command_set_throttle(player, ship::reverse)); add_message(texts::get(48)); break;
			case SDLK_0: if (player->is_scope_up()) {
					gm.send(new command_scope_down(player)); add_message(texts::get(54)); } else {
					gm.send(new command_scope_up(player)); add_message(texts::get(55)); }
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
					//add_captains_log_entry( gm, oss.str () );
					gm.send(new command_launch_torpedo(player, -1, target));
					play_sound_effect (gm, se_submarine_torpedo_launch );
				}
				break;
			case SDLK_SPACE:
				target = gm.contact_in_direction(player, player->get_heading()+bearing);
				if (target)
					{
						add_message(texts::get(50));
						//add_captains_log_entry ( gm, texts::get(50));
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
						//add_captains_log_entry ( gm, oss.str () );
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
	user_interface::display(gm);

	submarine* player = dynamic_cast<submarine*>(gm.get_player());

	// panel is drawn in each display function, so the above code is all...

	// switch to map if sub is to deep. fixme collides with constness, make current_display mutable!?
	//or check it in input function.<-better?
/*
	double depth = player->get_depth();
	if ((depth > SUBMARINE_SUBMERGED_DEPTH &&
			(current_display == display_mode_uzo || current_display == display_mode_glasses ||
			 current_display == display_mode_bridge)) ||
		(depth > player->get_periscope_depth() &&
			(current_display == display_mode_periscope || current_display == display_mode_uzo ||
			 current_display == display_mode_glasses || current_display == display_mode_bridge)) ||
		(current_display == display_mode_periscope && !player->is_scope_up()))
			current_display = display_mode_map;
*/
}
