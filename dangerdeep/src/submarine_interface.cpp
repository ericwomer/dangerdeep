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
#include "sub_periscope_display.h"
#include "sub_uzo_display.h"
#include "sub_bridge_display.h"
#include "map_display.h"
#include "sub_torpedo_display.h"
#include "sub_damage_display.h"
#include "logbook.h"
#include "ships_sunk_record.h"
#include "freeview.h"

submarine_interface::submarine_interface(submarine* player_sub, game& gm) : 
    	user_interface( player_sub, gm )
{
	btn_menu = new widget_caller_button<game, void (game::*)(void)>(&gm, &game::stop, 1024-128-8, 128-40, 128, 32, texts::get(177));
	panel->add_child(btn_menu);

	displays.push_back(new sub_gauges_display(*this));
	displays.push_back(new sub_periscope_display(*this));
	displays.push_back(new sub_uzo_display(*this));
	displays.push_back(new sub_bridge_display(*this));
	displays.push_back(new map_display(*this));
	displays.push_back(new sub_torpedo_display(*this));
	displays.push_back(new sub_damage_display(*this));
	displays.push_back(new captains_logbook_display(*this));
	displays.push_back(new ships_sunk_display(*this));
	displays.push_back(new freeview_display(*this));
}

submarine_interface::~submarine_interface()
{
}

bool submarine_interface::keyboard_common(int keycode, class game& gm)
{
	submarine* player = dynamic_cast<submarine*> ( get_player() );

	// handle common keys (fixme: make configureable?)
	if (system::sys().key_shift()) {
		switch (keycode) {
			// torpedo launching
		case SDLK_1:
		case SDLK_2:
		case SDLK_3:
		case SDLK_4:
		case SDLK_5:
		case SDLK_6:
			if (player->can_torpedo_be_launched(gm, keycode - SDLK_1, target)) {
				add_message(texts::get(49));
				ostringstream oss;
				oss << texts::get(49);
				if (target)
					oss << " " << texts::get(6) << ": " << target->get_description ( 2 );
				add_captains_log_entry( gm, oss.str () );
				gm.send(new command_launch_torpedo(player, keycode - SDLK_1, target));
				play_sound_effect ( se_submarine_torpedo_launch );
			}
			break;
		case SDLK_LEFT:
			gm.send(new command_rudder_hard_left(player));
			add_rudder_message();
			break;
		case SDLK_RIGHT:
			gm.send(new command_rudder_hard_right(player));
			add_rudder_message();
			break;
			// view
		case SDLK_COMMA : bearing -= angle(10); break;
		case SDLK_PERIOD : bearing += angle(10); break;
		default: return false;
		}
	} else {	// no shift
		switch (keycode) {
			// viewmode switching
		case SDLK_F1: viewmode = display_mode_gauges; break;
		case SDLK_F2: viewmode = display_mode_periscope; break;
		case SDLK_F3: viewmode = display_mode_uzo; break;
		case SDLK_F4: viewmode = display_mode_bridge; break;
		case SDLK_F5: viewmode = display_mode_map; break;
		case SDLK_F6: viewmode = display_mode_torpedoroom; break;
		case SDLK_F7: viewmode = display_mode_damagestatus; break;
		case SDLK_F8: viewmode = display_mode_logbook; break;
		case SDLK_F9: viewmode = display_mode_successes; break;
		case SDLK_F10: viewmode = display_mode_freeview; freeviewpos = player_object->get_pos(); break;

			// time scaling fixme: too simple
		case SDLK_F11: if (time_scale_up()) { add_message(texts::get(31)); } break;
		case SDLK_F12: if (time_scale_down()) { add_message(texts::get(32)); } break;

			// control
		case SDLK_LEFT:
			gm.send(new command_rudder_left(player));
			add_rudder_message();
			break;
		case SDLK_RIGHT:
			gm.send(new command_rudder_right(player));
			add_rudder_message();
			break;
		case SDLK_UP: gm.send(new command_planes_up(player, 1)); add_message(texts::get(37)); break;
		case SDLK_DOWN: gm.send(new command_planes_down(player, 1)); add_message(texts::get(38)); break;
		case SDLK_c:
			// fixme: we should introduce a new command here, because crash diving
			// is different from normal diving
			gm.send(new command_dive_to_depth(player, unsigned(player->get_alarm_depth())));
			add_message(texts::get(41));
			add_captains_log_entry ( gm, texts::get(41));
			break;
		case SDLK_d:
			if ( player->has_snorkel () )
				{
					gm.send(new command_dive_to_depth(player, unsigned(player->get_snorkel_depth())));
					add_message ( texts::get(12));
					add_captains_log_entry ( gm, texts::get(97));
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
								add_captains_log_entry ( gm, texts::get(96));
							}
						}
					else
						{
							gm.send(new command_snorkel_up ( player ) );
							//fixme: was an if, why? say "snorkel up only when it was up"
							{
								add_message ( texts::get(95));
								add_captains_log_entry ( gm, texts::get(95));
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
			add_captains_log_entry ( gm, texts::get(40));
			break;	//fixme
		case SDLK_s:
			gm.send(new command_dive_to_depth(player, 0));
			add_message(texts::get(39));
			add_captains_log_entry ( gm, texts::get(39));
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
				add_captains_log_entry( gm, oss.str () );
				gm.send(new command_launch_torpedo(player, -1, target));
				play_sound_effect ( se_submarine_torpedo_launch );
			}
			break;
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
		case SDLK_i:
			// calculate distance to target for identification detail
			if (target)
				{
					ostringstream oss;
					oss << texts::get(79) << target->get_description(2); // fixme
					add_message( oss.str () );
					add_captains_log_entry ( gm, oss.str () );
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
		default: return false;		
		}
	}
	return true;
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
//	submarine* player = dynamic_cast<submarine*> ( get_player() );

	displays[viewmode]->display(gm);

	// panel is drawn in each display function, so the above code is all...

	// switch to map if sub is to deep. fixme collides with constness, make viewmode mutable!
	//or check it in input function.
	double depth = player->get_depth();
	if ((depth > SUBMARINE_SUBMERGED_DEPTH &&
			(viewmode == display_mode_uzo || viewmode == display_mode_glasses ||
			 viewmode == display_mode_bridge)) ||
		(depth > player->get_periscope_depth() &&
			(viewmode == display_mode_periscope || viewmode == display_mode_uzo ||
			 viewmode == display_mode_glasses || viewmode == display_mode_bridge)) ||
		(viewmode == display_mode_periscope && !player->is_scope_up()))
			viewmode = display_mode_map;

	switch (viewmode) {
		case display_mode_gauges:
			display_gauges(gm);
			break;
		case display_mode_periscope:
			display_periscope(gm);
			break;
		case display_mode_uzo:
			display_UZO(gm);
			break;
		case display_mode_glasses:
		case display_mode_bridge:
			if ( zoom_scope )
				display_glasses(gm);
			else
			display_bridge(gm);
			break;
		case display_mode_map:
			display_map(gm);
			break;
		case display_mode_torpedoroom:
			display_torpedoroom(gm);
			break;
		case display_mode_damagestatus:
			display_damagestatus(gm);
			break;
		case display_mode_logbook:
			display_logbook(gm);
			break;
		case display_mode_successes:
			display_successes(gm);
			break;
		case display_mode_freeview:
		default:
			display_freeview(gm);
			break;
	}
}

void submarine_interface::process_input(const list<SDL_Event>& events)
{
	//fixme
}


void submarine_interface::play_sound_effect_distance ( sound_effect se, double distance ) const
{
	sound* s = get_sound_effect ( se );

	if ( s )
	{
		submarine* sub = dynamic_cast<submarine*> ( get_player () );
		double h = 3000.0f;
		if ( sub && sub->is_submerged () )
			h = 10000.0f;

		s->play ( ( 1.0f - player_object->get_noise_factor () ) * exp ( - distance / h ) );
	}
}







//
//
//
///////////////////////////////////////// OLD
//
//


void submarine_interface::display_damagestatus(game& gm)
//fixme: divide display and key handling information!!!!!!!!!!!!!!
{
//	glClearColor(0.25, 0.25, 0.25, 0);	// isn't needed
//	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	system::sys().prepare_2d_drawing();
	sub_damage_disp->display(gm);
	draw_infopanel ( gm );

	// mouse processing;
	int mx;
	int my;
	int mb = system::sys().get_mouse_buttons();
	system::sys().get_mouse_position(mx, my);
	sub_damage_disp->check_mouse ( mx, my, mb );

	// note: mouse processing must be done first, to display pop-ups.
	system::sys().unprepare_2d_drawing();

	// keyboard processing, fixme: do we need extra keyboard input here?
	int key = system::sys().get_key().sym;
	while (key != 0) {
		if (!keyboard_common(key, gm)) {
			// specific keyboard processing
			sub_damage_disp->check_key ( key, gm );
		}
		key = system::sys().get_key().sym;
	}
}

