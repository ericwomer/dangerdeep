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
#include "user_display.h"
#include "logbook.h"
#include "submarine_interface.h"
#include "sub_damage_display.h"
#include "system.h"
#include "game.h"
#include "texts.h"
#include "sound.h"
#include "image.h"
#include "menu.h"	//fixme get rid of this
#include "widget.h"
#include "command.h"
extern void menu_notimplemented(void);	// fixme remove later.

submarine_interface::submarine_interface(submarine* player_sub, game& gm) : 
    	user_interface( player_sub, gm ), sub_damage_disp(new sub_damage_display(player_sub)),
    	torptranssrc(0xffff)
{
	btn_menu = new widget_caller_button<game, void (game::*)(void)>(&gm, &game::stop, 1024-128-8, 128-40, 128, 32, texts::get(177));
	panel->add_child(btn_menu);
}

submarine_interface::~submarine_interface()
{
	delete sub_damage_disp;
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
					if ( target )
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
				gm.send(new command_dive_to_depth(player, unsigned(player->get_max_depth())));
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
			case SDLK_1: gm.send(new command_set_throttle(player, sea_object::aheadslow)); add_message(texts::get(43)); break;
			case SDLK_2: gm.send(new command_set_throttle(player, sea_object::aheadhalf)); add_message(texts::get(44)); break;
			case SDLK_3: gm.send(new command_set_throttle(player, sea_object::aheadfull)); add_message(texts::get(45)); break;
			case SDLK_4: gm.send(new command_set_throttle(player, sea_object::aheadflank)); add_message(texts::get(46)); break;//flank/full change?
			case SDLK_5: gm.send(new command_set_throttle(player, sea_object::stop)); add_message(texts::get(47)); break;
			case SDLK_6: gm.send(new command_set_throttle(player, sea_object::reverse)); add_message(texts::get(48)); break;
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
					if ( target )
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
	
void submarine_interface::display(game& gm)
{
	submarine* player = dynamic_cast<submarine*> ( get_player() );

	// A dead or sinking target can be removed as selected target.
	if (target != 0 && (target->is_dead () || target->is_sinking ()))
		target = 0;

	// switch to map if sub is to deep.
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

void submarine_interface::display_periscope(game& gm)
{
	submarine* player = dynamic_cast<submarine*> ( get_player() );

	glClear(GL_DEPTH_BUFFER_BIT);

	unsigned res_x = system::sys().get_res_x(), res_y = system::sys().get_res_y();

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	// scope zoom modes are x1,5 and x6,0. This is meant as width of image.
	// E.g. normal width of view is tan(fov/2)*distance.
	// with arbitrary zoom z we have (fov = normal fov angle, newfov = zoom fov angle)
	// z*tan(newfov/2)*distance = tan(fov/2)*distance =>
	// newfov = 2*atan(tan(fov/2)/z).
	// Our values: fov = 90deg, z = 1.5;6.0, newfov = 67.380;18.925
	double fov = 67.380f;

	if ( zoom_scope )
		fov = 18.925f;
	
	system::sys().gl_perspective_fovx (fov, 1.0/1.0, 2.0, gm.get_max_view_distance());
	glViewport(res_x/2, res_y/3, res_x/2, res_x/2);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glRotatef(-90, 1,0,0);	// swap y and z coordinates (camera looks at +y axis)

	vector3 viewpos = player->get_pos() + vector3(0, 0, 12+14);//fixme: +14 to be above waves ?!
	// no torpedoes, no DCs, no player
	draw_view(gm, viewpos, player->get_heading()+bearing, 0, true, false, false);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glViewport(0, 0, res_x, res_y);
	glMatrixMode(GL_MODELVIEW);
	
	system::sys().prepare_2d_drawing();
	set_display_color ( gm );
	for (int x = 0; x < 3; ++x)
		psbackgr->draw(x*256, 512, 256, 256);
	periscope->draw(2*256, 0);
	addleadangle->draw(768, 512, 256, 256);

	// Draw lead angle value.
	double la = player->get_trp_addleadangle().value();

	if ( la > 180.0f )
		la -= 360.0f;

	int lax = 896 + int ( 10.8f * la );

	glBindTexture(GL_TEXTURE_2D, 0);
	glBegin ( GL_TRIANGLE_STRIP );
	glColor3f ( 1.0f, 0.0f, 0.0f );
	glVertex2i ( lax-1, 522 );
	glVertex2i ( lax-1, 550 );
	glVertex2i ( lax+1, 522 );
	glVertex2i ( lax+1, 550 );
	glEnd ();

	angle targetbearing;
	angle targetaob;
	angle targetrange;
	angle targetspeed;
	angle targetheading;
	if (target) {
		pair<angle, double> br = player->bearing_and_range_to(target);
		targetbearing = br.first;
		targetaob = player->estimate_angle_on_the_bow(br.first, target->get_heading());
		unsigned r = unsigned(round(br.second));
		if (r > 9999) r = 9999;
		targetrange = r*360.0/9000.0;
		targetspeed = target->get_speed()*360.0/sea_object::kts2ms(36);
		targetheading = target->get_heading();
	}
	draw_gauge(gm, 1, 0, 0, 256, targetbearing, texts::get(12));
	draw_gauge(gm, 3, 256, 0, 256, targetrange, texts::get(13));
	draw_gauge(gm, 2, 0, 256, 256, targetspeed, texts::get(14));
	draw_gauge(gm, 1, 256, 256, 256, targetheading, texts::get(15));
	const vector<submarine::stored_torpedo>& torpedoes = player->get_torpedoes();
	pair<unsigned, unsigned> bow_tube_indices = player->get_bow_tube_indices();
	pair<unsigned, unsigned> stern_tube_indices = player->get_stern_tube_indices();
	for (unsigned i = bow_tube_indices.first; i < bow_tube_indices.second; ++i) {
		int j = i-bow_tube_indices.first;
		draw_torpedo(gm, true, (j/4)*128, 512+(j%4)*16, torpedoes[i]);
	}
	for (unsigned i = stern_tube_indices.first; i < stern_tube_indices.second; ++i) {
		draw_torpedo(gm, false, 256, 512+(i-stern_tube_indices.first)*16, torpedoes[i]);
	}
	glColor3f(1,1,1);
	draw_infopanel(gm);
	system::sys().unprepare_2d_drawing();

	// mouse handling
	int mx;
	int my;
	int mb = system::sys().get_mouse_buttons();
	system::sys().get_mouse_position(mx, my);

	if (mb & system::sys().left_button) {
		// Evaluate lead angle box.
		if ( mx >= 776 && mx <= 1016 && my >= 520 && my <= 552 )
		{
			double lav = double ( mx - 896 ) / 10.8f;
			if ( lav < - 10.0f )
				lav = -10.0f;
			else if ( lav > 10.0f )
				lav = 10.0f;

			gm.send(new command_set_trp_addleadangle(player, lav));
		}
	}

	// keyboard processing
	int key = system::sys().get_key().sym;
	while (key != 0) {
		if (!keyboard_common(key, gm)) {
			// specific keyboard processing
			switch ( key ) {
				case SDLK_y:
					if ( zoom_scope )
						zoom_scope = false;
					else
						zoom_scope = true;
					break;
			}
		}
		key = system::sys().get_key().sym;
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
	system::sys().gl_perspective_fovx (5.0, 2.0/1.0, 2.0, gm.get_max_view_distance());
	glViewport(0, res_y/3, res_x, res_x/2);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glRotatef(-90, 1,0,0);	// swap y and z coordinates (camera looks at +y axis)

	vector3 viewpos = player->get_pos() + vector3(0, 0, 6);
	// no torpedoes, no DCs, no player
	draw_view(gm, viewpos, player->get_heading()+bearing, 0, true, false, false);

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

void submarine_interface::display_torpedoroom(game& gm)
{
	submarine* player = dynamic_cast<submarine*> ( get_player () );

	// draw display without display color.
	glColor4f(1,1,1,1);
	// draw background
	system::sys().prepare_2d_drawing();
	background->draw_tiles(0, 0, 1024, 768, 8, 6);
	glClear(GL_DEPTH_BUFFER_BIT);

	// draw sub model	
	glPushMatrix();
	glTranslatef(512, 160, 1);
	glScalef(1024/80.0, 1024/80.0, 0.001);
	glRotatef(90, 0, 0, 1);
	glRotatef(-90, 0, 1, 0);
	player->display();
	glPopMatrix();
	
	// draw torpedo programming buttons
	draw_turnswitch(gm,   0, 256, 142, 17, player->get_trp_primaryrange(), 175, 138);
	draw_turnswitch(gm, 256, 256, 159, 2, player->get_trp_secondaryrange(), 0, 139);
	draw_turnswitch(gm, 512, 256, 161, 2, player->get_trp_initialturn(), 0, 140);
	draw_turnswitch(gm, 768, 256, 163, 2, player->get_trp_searchpattern(), 176, 141);

	// tube handling. compute coordinates for display and mouse use	
	const vector<submarine::stored_torpedo>& torpedoes = player->get_torpedoes();
	vector<pair<int, int> > tubecoords(torpedoes.size());
	pair<unsigned, unsigned> bow_tube_indices = player->get_bow_tube_indices();
	pair<unsigned, unsigned> stern_tube_indices = player->get_stern_tube_indices();
	pair<unsigned, unsigned> bow_storage_indices = player->get_bow_storage_indices();
	pair<unsigned, unsigned> stern_storage_indices = player->get_stern_storage_indices();
	pair<unsigned, unsigned> bow_top_storage_indices = player->get_bow_top_storage_indices();
	pair<unsigned, unsigned> stern_top_storage_indices = player->get_stern_top_storage_indices();
	unsigned k = bow_tube_indices.second - bow_tube_indices.first;
	for (unsigned i = bow_tube_indices.first; i < bow_tube_indices.second; ++i) {
		tubecoords[i].first = 0;
		tubecoords[i].second = 192+(i-k/2)*16;	
	}
	for (unsigned i = bow_storage_indices.first; i < bow_storage_indices.second; ++i) {
		unsigned j = i - bow_storage_indices.first;
		tubecoords[i].first = 192+(j/k)*128;
		tubecoords[i].second = 192+(j%k-k/2)*16;	
	}
	for (unsigned i = bow_top_storage_indices.first; i < bow_top_storage_indices.second; ++i) {
		unsigned j = i - bow_top_storage_indices.first;
		tubecoords[i].first = 192+(j/2)*128;
		tubecoords[i].second = 96+(j%2)*16;	
	}
	for (unsigned i = stern_tube_indices.first; i < stern_tube_indices.second; ++i) {
		unsigned j = i - stern_tube_indices.first;
		tubecoords[i].first = 896;
		tubecoords[i].second = 160+j*16;	
	}
	for (unsigned i = stern_storage_indices.first; i < stern_storage_indices.second; ++i) {
		unsigned j = i - stern_storage_indices.first;
		tubecoords[i].first = 704;
		tubecoords[i].second = 160+j*16;	
	}
	for (unsigned i = stern_top_storage_indices.first; i < stern_top_storage_indices.second; ++i) {
		unsigned j = i - stern_top_storage_indices.first;
		tubecoords[i].first = 704-(j/2)*128;
		tubecoords[i].second = 96+(j%2)*16;
	}

	// draw tubes
	for (unsigned i = bow_tube_indices.first; i < bow_tube_indices.second; ++i)
		draw_torpedo(gm, true, tubecoords[i].first, tubecoords[i].second, torpedoes[i]);
	for (unsigned i = bow_storage_indices.first; i < bow_storage_indices.second; ++i)
		draw_torpedo(gm, true, tubecoords[i].first, tubecoords[i].second, torpedoes[i]);
	for (unsigned i = bow_top_storage_indices.first; i < bow_top_storage_indices.second; ++i)
		draw_torpedo(gm, true, tubecoords[i].first, tubecoords[i].second, torpedoes[i]);
	for (unsigned i = stern_tube_indices.first; i < stern_tube_indices.second; ++i)
		draw_torpedo(gm, false, tubecoords[i].first, tubecoords[i].second, torpedoes[i]);
	for (unsigned i = stern_storage_indices.first; i < stern_storage_indices.second; ++i)
		draw_torpedo(gm, false, tubecoords[i].first, tubecoords[i].second, torpedoes[i]);
	for (unsigned i = stern_top_storage_indices.first; i < stern_top_storage_indices.second; ++i)
		draw_torpedo(gm, false, tubecoords[i].first, tubecoords[i].second, torpedoes[i]);
	
	// collect and draw type info
	set<unsigned> torptypes;
	for (unsigned i = 0; i < torpedoes.size(); ++i)
		if (torpedoes[i].type != torpedo::none)
			torptypes.insert(torpedoes[i].type);
	unsigned px = (1024-torptypes.size()*256)/2;
	for (set<unsigned>::iterator it = torptypes.begin(); it != torptypes.end(); ++it) {
		color::white().set_gl_color();
		notepadsheet->draw(px, 512);
		torptex(*it)->draw(px+64, 548);
		font_arial->print(px+16, 576, texts::get(300+*it-1), color(0,0,128));
		px += 256;
	}
	
	// mouse handling
	int mx, my, mb = system::sys().get_mouse_buttons();
	system::sys().get_mouse_position(mx, my);

	unsigned mouseovertube = 0xffff;
	for (unsigned i = 0; i < torpedoes.size(); ++i) {
		if (mx >= tubecoords[i].first && mx < tubecoords[i].first+128 &&
				my >= tubecoords[i].second && my < tubecoords[i].second+16) {
			mouseovertube = i;
			break;
		}
	}

	if (mb & system::sys().left_button) {
		// button down
		if (	mouseovertube < torpedoes.size()
			&& torptranssrc >= torpedoes.size()
			&& torpedoes[mouseovertube].status == submarine::stored_torpedo::st_loaded)
		{
			torptranssrc = mouseovertube;
		} else {
			if (torpedoes[mouseovertube].status == submarine::stored_torpedo::st_reloading ||
					torpedoes[mouseovertube].status == submarine::stored_torpedo::st_unloading) {
				glColor4f(1,1,1,1);
				notepadsheet->draw(mx, my);
				unsigned seconds = unsigned(torpedoes[mouseovertube].remaining_time + 0.5);	
				unsigned hours = seconds / 3600;
				unsigned minutes = (seconds % 3600) / 60;
				seconds = seconds % 60;
				ostringstream oss;
				oss << texts::get(211) << hours << ":" << minutes << ":" << seconds;
				font_arial->print(mx+32, my+32, oss.str(), color(0,0,128));
			}
		}
		
		// torpedo programming buttons
		if (my >= 256 && my < 512) {
			if (mx < 256) gm.send(new command_set_trp_primaryrange(player, turnswitch_input(mx, my-256, 17)));
			else if (mx < 512) gm.send(new command_set_trp_secondaryrange(player, turnswitch_input(mx-256, my-256, 2)));
			else if (mx < 768) gm.send(new command_set_trp_initialturn(player, turnswitch_input(mx-512, my-256, 2)));
			else gm.send(new command_set_trp_searchpattern(player, turnswitch_input(mx-768, my-256, 2)));
		}
	
	} else {
		
		// button released
		if (	   mouseovertube < torpedoes.size()
			&& torptranssrc < torpedoes.size()
			&& torpedoes[mouseovertube].status == submarine::stored_torpedo::st_empty
			&& mouseovertube != torptranssrc)
		{
			// transport
			gm.send(new command_transfer_torpedo(player, torptranssrc, mouseovertube));
		}
		torptranssrc = 0xffff;
	}
		
	// draw line for torpedo transfer if button down
	if (mb != 0 && torptranssrc < torpedoes.size()) {
		glColor4f(1,1,1,0.5);
		torptex(torpedoes[torptranssrc].type)->draw(mx-64, my-8);
		glColor4f(1,1,1,1);
		system::sys().no_tex();
		glBegin(GL_LINES);
		glVertex2i(tubecoords[torptranssrc].first+64,
			tubecoords[torptranssrc].second+8);
		glVertex2i(mx, my);
		glEnd();
	}

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

void submarine_interface::display_damagestatus(game& gm)
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

void submarine_interface::draw_torpedo(class game& gm,
	bool usebow, int x, int y, const submarine::stored_torpedo& st)
{
	if (usebow) {
		if (st.status == 0) {	// empty
			torpempty->draw(x, y);
		} else if (st.status == 1) {	// reloading
			torptex(st.type)->draw(x, y);
			torpreload->draw(x, y);
		} else if (st.status == 2) {	// unloading
			torpempty->draw(x, y);
			torpunload->draw(x, y);
		} else {		// loaded
			torptex(st.type)->draw(x, y);
		}
	} else {
		if (st.status == 0) {	// empty
			torpempty->draw_hm(x, y);
		} else if (st.status == 1) {	// reloading
			torptex(st.type)->draw_hm(x, y);
			torpreload->draw_hm(x, y);
		} else if (st.status == 2) {	// unloading
			torpempty->draw_hm(x, y);
			torpunload->draw_hm(x, y);
		} else {		// loaded
			torptex(st.type)->draw_hm(x, y);
		}
	}
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

// fixme: this function is already in user_interface.cpp. are there differences and why?
void submarine_interface::display_gauges(class game& gm)
{
	submarine* player = dynamic_cast<submarine*> ( get_player () );
	system::sys().prepare_2d_drawing();
	set_display_color ( gm );
	for (int y = 0; y < 3; ++y)	// fixme: replace with gauges
		for (int x = 0; x < 4; ++x)
			psbackgr->draw(x*256, y*256, 256, 256);
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
	draw_infopanel(gm);
	system::sys().unprepare_2d_drawing();

	// mouse handling
	int mx, my, mb = system::sys().get_mouse_buttons();
	system::sys().get_mouse_position(mx, my);

	if (mb & 1) {
		int marea = (my/256)*4+(mx/256);
		int mareax = (mx/256)*256+128;
		int mareay = (my/256)*256+128;
		angle mang(vector2(mx - mareax, mareay - my));
		if ( marea == 0 )
		{
			gm.send(new command_head_to_ang(player, mang, mang.is_cw_nearer(
				player->get_heading())));
		}
		else if ( marea == 1 )
		{}
		else if ( marea == 2 )
		{
			submarine* sub = dynamic_cast<submarine*> ( player );
			if ( sub )
			{
				gm.send(new command_dive_to_depth(player, mang.ui_value()));
			}
		}
	}

	// keyboard processing
	int key = system::sys().get_key().sym;
	while (key != 0) {
		if (!keyboard_common(key, gm)) {
			// specific keyboard processing
		}
		key = system::sys().get_key().sym;
	}
}
