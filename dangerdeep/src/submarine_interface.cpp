// user interface for controlling a submarine
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL/SDL.h>
#include <sstream>
#include <map>
#include <list>
using namespace std;
#include "date.h"
#include "user_display.h"
#include "logbook.h"
#include "submarine_interface.h"
#include "system.h"
#include "game.h"
#include "texts.h"
#include "sound.h"

submarine_interface::submarine_interface(submarine* player_sub) : 
    	user_interface( player_sub )
{
}

submarine_interface::~submarine_interface()
{
}

bool submarine_interface::keyboard_common(int keycode, class system& sys, class game& gm)
{
	submarine* player = dynamic_cast<submarine*> ( get_player() );

	// handle common keys (fixme: make configureable?)
	if (sys.key_shift()) {
		switch (keycode) {
			// torpedo launching
			case SDLK_1:
			case SDLK_2:
			case SDLK_3:
			case SDLK_4:
			case SDLK_5:
			case SDLK_6:
				if ( player->fire_torpedo ( gm, keycode - SDLK_1, target, lead_angle ) )
				{
					add_message(TXT_Torpedofired[language]);
					ostringstream oss;
					oss << TXT_Torpedofired[language];
					if ( target )
						oss << " " << TXT_Target[language] << ": " << target->get_description ( 2 );
					add_captains_log_entry( gm, oss.str () );
					play_sound_effect ( se_submarine_torpedo_launch );
				}
				break;
			case SDLK_LEFT:
                player->rudder_hard_left();
                add_rudder_message();
                break;
			case SDLK_RIGHT:
                player->rudder_hard_right();
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
			case SDLK_F10: viewmode = display_mode_freeview; break;

			// time scaling fixme: too simple
			case SDLK_F11: if (time_scale_up()) { add_message(TXT_Timescaleup[language]); } break;
			case SDLK_F12: if (time_scale_down()) { add_message(TXT_Timescaledown[language]); } break;

			// control
			case SDLK_LEFT:
				player->rudder_left();
				add_rudder_message();
				break;
			case SDLK_RIGHT:
				player->rudder_right();
				add_rudder_message();
				break;
			case SDLK_UP: player->planes_up(1); add_message(TXT_Planesup[language]); break;
			case SDLK_DOWN: player->planes_down(1); add_message(TXT_Planesdown[language]); break;
			case SDLK_c:
				player->dive_to_depth(static_cast<unsigned>(player->get_max_depth()));
				add_message(TXT_Crashdive[language]);
				add_captains_log_entry ( gm, TXT_Crashdive[language] );
				break;
			case SDLK_d:
				if ( player->has_snorkel () )
				{
					player->dive_to_depth ( static_cast<unsigned> ( player->get_snorkel_depth () ) );
					add_message ( TXT_SnorkelDepth[language] );
					add_captains_log_entry ( gm, TXT_SnorkelDepth[language] );
				}
				break;
			case SDLK_f:
				if ( player->has_snorkel () )
				{
					if ( player->is_snorkel_up () )
					{
						if ( player->set_snorkel_up ( false ) )
						{
							add_message ( TXT_SnorkelDown[language] );
							add_captains_log_entry ( gm, TXT_SnorkelDown[language] );
						}
					}
					else
					{
						if ( player->set_snorkel_up ( true ) )
						{
							add_message ( TXT_SnorkelUp[language] );
							add_captains_log_entry ( gm, TXT_SnorkelUp[language] );
						}
					}
				}
				break;
			case SDLK_h:
				{
					bool turn_left = ( angle ( 180.0f ) <= bearing && bearing < angle ( 359.999f ) );
					player->head_to_ang ( player->get_heading () + bearing, turn_left );
				}
				break;
			case SDLK_p:
				player->dive_to_depth(static_cast<unsigned>(player->get_periscope_depth()));
				add_message(TXT_Periscopedepth[language]);
				add_captains_log_entry ( gm, TXT_Periscopedepth[language] );
				break;	//fixme
			case SDLK_s:
				player->dive_to_depth(0);
				add_message(TXT_Surface[language]);
				add_captains_log_entry ( gm, TXT_Surface[language] );
				break;
			case SDLK_v:
				bearing = 0.0f;
				break;
			case SDLK_RETURN :
                player->rudder_midships();
                player->planes_middle();
                add_message(TXT_Ruddermidships[language]);
                break;
			case SDLK_1: player->set_throttle(sea_object::aheadslow); add_message(TXT_Aheadslow[language]); break;
			case SDLK_2: player->set_throttle(sea_object::aheadhalf); add_message(TXT_Aheadhalf[language]); break;
			case SDLK_3: player->set_throttle(sea_object::aheadfull); add_message(TXT_Aheadfull[language]); break;
			case SDLK_4: player->set_throttle(sea_object::aheadflank); add_message(TXT_Aheadflank[language]); break;//flank/full change?
			case SDLK_5: player->set_throttle(sea_object::stop); add_message(TXT_Enginestop[language]); break;
			case SDLK_6: player->set_throttle(sea_object::reverse); add_message(TXT_Enginereverse[language]); break;
			case SDLK_0: if (player->is_scope_up()) {
				player->scope_down(); add_message(TXT_Scopedown[language]); } else {
				player->scope_up(); add_message(TXT_Scopeup[language]); }
				break;

			// view
			case SDLK_COMMA : bearing -= angle(1); break;
			case SDLK_PERIOD : bearing += angle(1); break;

			// weapons, fixme
			case SDLK_t:
				if (player->fire_torpedo(gm, -1, target, lead_angle))
					add_message(TXT_Torpedofired[language]);
				break;
			case SDLK_SPACE:
				target = gm.contact_in_direction(player, player->get_heading()+bearing);
				if (target)
				{
					add_message( TXT_Newtargetselected[language] );
					add_captains_log_entry ( gm, TXT_Newtargetselected[language] );
				}
				else
					add_message(TXT_Notargetindirection[language]);
				break;
			case SDLK_i:
				// calculate distance to target for identification detail
				if (target)
				{
					ostringstream oss;
					oss << TXT_Identifiedtargetas[language] << target->get_description(2); // fixme
					add_message( oss.str () );
					add_captains_log_entry ( gm, oss.str () );
				}
				else
				{
					add_message(TXT_Notargetselected[language]);
				}
				break;

			// quit, screenshot, pause etc.
			case SDLK_ESCAPE: quit = true; break;
			case SDLK_PRINT: sys.screenshot(); sys.add_console("screenshot taken."); break;
			case SDLK_PAUSE: pause = !pause;
				if (pause) add_message(TXT_Gamepaused[language]);
				else add_message(TXT_Gameunpaused[language]);
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
	
void submarine_interface::display(class system& sys, game& gm)
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
			display_gauges(sys, gm);
			break;
		case display_mode_periscope:
			display_periscope(sys, gm);
			break;
		case display_mode_uzo:
			display_UZO(sys, gm);
			break;
		case display_mode_glasses:
		case display_mode_bridge:
            if ( zoom_scope )
                display_glasses(sys, gm);
            else
                display_bridge(sys, gm);
            break;
		case display_mode_map:
			display_map(sys, gm);
			break;
		case display_mode_torpedoroom:
			display_torpedoroom(sys, gm);
			break;
		case display_mode_damagestatus:
			display_damagestatus(sys, gm);
			break;
		case display_mode_logbook:
			display_logbook(sys, gm);
			break;
		case display_mode_successes:
			display_successes(sys, gm);
			break;
		case display_mode_freeview:
		default:
			display_freeview(sys, gm);
			break;
	}
}

void submarine_interface::display_periscope(class system& sys, game& gm)
{
	submarine* player = dynamic_cast<submarine*> ( get_player() );

	glClear(GL_DEPTH_BUFFER_BIT);

	unsigned res_x = sys.get_res_x(), res_y = sys.get_res_y();

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

    double fov = 20.0f;

    if ( zoom_scope )
		fov = 5.0f;
	
	gluPerspective (fov, 1.0/1.0, 2.0, gm.get_max_view_distance());
	glViewport(res_x/2, res_y/3, res_x/2, res_x/2);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	vector3 viewpos = player->get_pos() + vector3(0, 0, 12+3);//fixme: +3 to be above waves
	// no torpedoes, no DCs, no player
	draw_view(sys, gm, viewpos, player->get_heading()+bearing, false, false);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glViewport(0, 0, res_x, res_y);
	glMatrixMode(GL_MODELVIEW);
	
	sys.prepare_2d_drawing();
	set_display_color ( gm );
	for (int x = 0; x < 3; ++x)
		sys.draw_image(x*256, 512, 256, 256, psbackgr);
	sys.draw_image(2*256, 0, 256, 256, periscope[0]);
	sys.draw_image(3*256, 0, 256, 256, periscope[1]);
	sys.draw_image(2*256, 256, 256, 256, periscope[2]);
	sys.draw_image(3*256, 256, 256, 256, periscope[3]);
	sys.draw_image(768, 512, 256, 256, addleadangle);

	// Draw lead angle value.
	double la = lead_angle.value ();

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
	draw_gauge(sys, gm, 1, 0, 0, 256, targetbearing, TXT_Targetbearing[language]);
	draw_gauge(sys, gm, 3, 256, 0, 256, targetrange, TXT_Targetrange[language]);
	draw_gauge(sys, gm, 2, 0, 256, 256, targetspeed, TXT_Targetspeed[language]);
	draw_gauge(sys, gm, 1, 256, 256, 256, targetheading, TXT_Targetcourse[language]);
	const vector<submarine::stored_torpedo>& torpedoes = player->get_torpedoes();
	pair<unsigned, unsigned> bow_tube_indices = player->get_bow_tube_indices();
	pair<unsigned, unsigned> stern_tube_indices = player->get_stern_tube_indices();
	for (unsigned i = bow_tube_indices.first; i < bow_tube_indices.second; ++i) {
		int j = i-bow_tube_indices.first;
		draw_torpedo(sys, gm, true, (j/4)*256, 512+(j%4)*32, torpedoes[i]);
	}
	for (unsigned i = stern_tube_indices.first; i < stern_tube_indices.second; ++i) {
		draw_torpedo(sys, gm, false, 512, 512+(i-stern_tube_indices.first)*32, torpedoes[i]);
	}
	glColor3f(1,1,1);
	draw_infopanel(sys, gm);
	sys.unprepare_2d_drawing();

	// mouse handling
	int mx;
	int my;
	int mb = sys.get_mouse_buttons();
	sys.get_mouse_position(mx, my);

	if (mb & sys.left_button) {
		// Evaluate lead angle box.
		if ( mx >= 776 && mx <= 1016 && my >= 520 && my <= 552 )
		{
			double lav = double ( mx - 896 ) / 10.8f;
			if ( lav < - 10.0f )
				lav = -10.0f;
			else if ( lav > 10.0f )
				lav = 10.0f;

			lead_angle = angle ( lav );
		}
	}

	// keyboard processing
	int key = sys.get_key();
	while (key != 0) {
		if (!keyboard_common(key, sys, gm)) {
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
		key = sys.get_key();
	}
}

void submarine_interface::display_UZO(class system& sys, game& gm)
{
	submarine* player = dynamic_cast<submarine*> ( get_player() );

	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	unsigned res_x = sys.get_res_x(), res_y = sys.get_res_y();

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluPerspective (5.0, 2.0/1.0, 2.0, gm.get_max_view_distance());
	glViewport(0, res_y/3, res_x, res_x/2);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	vector3 viewpos = player->get_pos() + vector3(0, 0, 6);
	// no torpedoes, no DCs, no player
	draw_view(sys, gm, viewpos, player->get_heading()+bearing, false, false);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glViewport(0, 0, res_x, res_y);
	glMatrixMode(GL_MODELVIEW);
	
	sys.prepare_2d_drawing();
	sys.draw_image(0, 0, 512, 512, uzo);
	sys.draw_hm_image(512, 0, 512, 512, uzo);
	draw_infopanel(sys, gm);
	sys.unprepare_2d_drawing();

	// keyboard processing
	int key = sys.get_key();
	while (key != 0) {
		if (!keyboard_common(key, sys, gm)) {
			// specific keyboard processing
		}
		key = sys.get_key();
	}
}

void submarine_interface::display_torpedoroom(class system& sys, game& gm)
{
	submarine* player = dynamic_cast<submarine*> ( get_player () );

	sys.prepare_2d_drawing();
	glBindTexture(GL_TEXTURE_2D, background->get_opengl_name());
	set_display_color ( gm );
	glBegin(GL_QUADS);
	glTexCoord2i(0,0);
	glVertex2i(0,0);
	glTexCoord2i(0,6);
	glVertex2i(0,768);
	glTexCoord2i(8,6);
	glVertex2i(1024,768);
	glTexCoord2i(8,0);
	glVertex2i(1024,0);
	glEnd();
	glClear(GL_DEPTH_BUFFER_BIT);
	
	glPushMatrix();
	glTranslatef(512, 192, 1);
	glScalef(1024/80.0, 1024/80.0, 0.001);
	glRotatef(90, 0, 0, 1);
	glRotatef(-90, 0, 1, 0);
	player->display();
	glPopMatrix();
	
	// draw tubes
	const vector<submarine::stored_torpedo>& torpedoes = player->get_torpedoes();
	pair<unsigned, unsigned> bow_tube_indices = player->get_bow_tube_indices();
	pair<unsigned, unsigned> stern_tube_indices = player->get_stern_tube_indices();
	pair<unsigned, unsigned> bow_storage_indices = player->get_bow_storage_indices();
	pair<unsigned, unsigned> stern_storage_indices = player->get_stern_storage_indices();
	pair<unsigned, unsigned> bow_top_storage_indices = player->get_bow_top_storage_indices();
	pair<unsigned, unsigned> stern_top_storage_indices = player->get_stern_top_storage_indices();
	for (unsigned i = bow_tube_indices.first; i < bow_tube_indices.second; ++i) {
		draw_torpedo(sys, gm, true, 0, 256+i*32, torpedoes[i]);
	}
	for (unsigned i = bow_storage_indices.first; i < bow_storage_indices.second; ++i) {
		unsigned j = i - bow_storage_indices.first;
		draw_torpedo(sys, gm, true, (1+j/6)*256, 256+(j%6)*32, torpedoes[i]);
	}
	for (unsigned i = bow_top_storage_indices.first; i < bow_top_storage_indices.second; ++i) {
		unsigned j = i - bow_top_storage_indices.first;
		draw_torpedo(sys, gm, true, 0, j*32, torpedoes[i]);
	}
	for (unsigned i = stern_tube_indices.first; i < stern_tube_indices.second; ++i) {
		unsigned j = i - stern_tube_indices.first;
		draw_torpedo(sys, gm, false, 768, 256+j*32, torpedoes[i]);
	}
	for (unsigned i = stern_storage_indices.first; i < stern_storage_indices.second; ++i) {
		unsigned j = i - stern_storage_indices.first;
		draw_torpedo(sys, gm, false, 512, 256+j*32, torpedoes[i]);
	}
	for (unsigned i = stern_top_storage_indices.first; i < stern_top_storage_indices.second; ++i) {
		unsigned j = i - stern_top_storage_indices.first;
		draw_torpedo(sys, gm, false, 768, j*32, torpedoes[i]);
	}

	draw_infopanel(sys, gm);
	sys.unprepare_2d_drawing();

	// mouse handling
	int mx, my; // mb = sys.get_mouse_buttons(); Unused variable
	sys.get_mouse_position(mx, my);

	// keyboard processing
	int key = sys.get_key();
	while (key != 0) {
		if (!keyboard_common(key, sys, gm)) {
			// specific keyboard processing
		}
		key = sys.get_key();
	}
}

void submarine_interface::draw_torpedo(class system& sys, class game& gm,
	bool usebow, int x, int y, const submarine::stored_torpedo& st)
{
	// Use a special color to display the torpedos.
	if ( gm.is_day_mode () )
		glColor3f ( 1.0f, 1.0f, 1.0f );
	else
		glColor3f ( 1.0f, 0.5f, 0.5f );

	if (usebow) {
		if (st.status == 0) {	// empty
			sys.draw_image(x, y, 256, 32, torpempty);
		} else if (st.status == 1) {	// reloading
			sys.draw_image(x, y, 256, 32, torptex(st.type));
			sys.draw_image(x, y, 256, 32, torpreload);
		} else if (st.status == 2) {	// unloading
			sys.draw_image(x, y, 256, 32, torpempty);
			sys.draw_image(x, y, 256, 32, torpunload);
		} else {		// loaded
			sys.draw_image(x, y, 256, 32, torptex(st.type));
		}
	} else {
		if (st.status == 0) {	// empty
			sys.draw_hm_image(x, y, 256, 32, torpempty);
		} else if (st.status == 1) {	// reloading
			sys.draw_hm_image(x, y, 256, 32, torptex(st.type));
			sys.draw_hm_image(x, y, 256, 32, torpreload);
		} else if (st.status == 2) {	// unloading
			sys.draw_hm_image(x, y, 256, 32, torpempty);
			sys.draw_hm_image(x, y, 256, 32, torpunload);
		} else {		// loaded
			sys.draw_hm_image(x, y, 256, 32, torptex(st.type));
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

void submarine_interface::display_gauges(class system& sys, class game& gm)
{
	submarine* player = dynamic_cast<submarine*> ( get_player () );
	sys.prepare_2d_drawing();
	set_display_color ( gm );
	for (int y = 0; y < 3; ++y)	// fixme: replace with gauges
		for (int x = 0; x < 4; ++x)
			sys.draw_image(x*256, y*256, 256, 256, psbackgr);
	angle player_speed = player->get_speed()*360.0/sea_object::kts2ms(36);
	angle player_depth = -player->get_pos().z;
	draw_gauge(sys, gm, 1, 0, 0, 256, player->get_heading(), TXT_Heading[language]);
	draw_gauge(sys, gm, 2, 256, 0, 256, player_speed, TXT_Speed[language]);
	draw_gauge(sys, gm, 4, 2*256, 0, 256, player_depth, TXT_Depth[language]);
	draw_clock(sys, gm, 3*256, 0, 256, gm.get_time(), TXT_Time[language]);
	draw_manometer_gauge ( sys, gm, 1, 0, 256, 256, player->get_fuel_level (),
		TXT_FuelGauge[language] );
	draw_manometer_gauge ( sys, gm, 1, 256, 256, 256, player->get_battery_level (),
		TXT_BatteriesGauge[language] );
	draw_infopanel(sys, gm);
	sys.unprepare_2d_drawing();

	// mouse handling
	int mx, my, mb = sys.get_mouse_buttons();
	sys.get_mouse_position(mx, my);

	if (mb & 1) {
		int marea = (my/256)*4+(mx/256);
		int mareax = (mx/256)*256+128;
		int mareay = (my/256)*256+128;
		angle mang(vector2(mx - mareax, mareay - my));
		if ( marea == 0 )
		{
			player->head_to_ang(mang, mang.is_cw_nearer(
				player->get_heading()));
		}
		else if ( marea == 1 )
		{}
		else if ( marea == 2 )
		{
			submarine* sub = dynamic_cast<submarine*> ( player );
			if ( sub )
			{
				sub->dive_to_depth(mang.ui_value());
			}
		}
	}

	// keyboard processing
	int key = sys.get_key();
	while (key != 0) {
		if (!keyboard_common(key, sys, gm)) {
			// specific keyboard processing
		}
		key = sys.get_key();
	}
}
