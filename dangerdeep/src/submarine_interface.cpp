// user interface for controlling a submarine
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL/SDL.h>
#include <sstream>
#include "submarine_interface.h"
#include "system.h"
#include "game.h"
#include "texts.h"

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
				if (player->fire_torpedo(gm, 0, target))
					add_message(TXT_Torpedofired[language]);
				break;
			case SDLK_2:
				if (player->fire_torpedo(gm, 1, target))
					add_message(TXT_Torpedofired[language]);
				break;
			case SDLK_3:
				if (player->fire_torpedo(gm, 2, target))
					add_message(TXT_Torpedofired[language]);
				break;
			case SDLK_4:
				if (player->fire_torpedo(gm, 3, target))
					add_message(TXT_Torpedofired[language]);
				break;
			case SDLK_5:
				if (player->fire_torpedo(gm, 4, target))
					add_message(TXT_Torpedofired[language]);
				break;
			case SDLK_6:
				if (player->fire_torpedo(gm, 5, target))
					add_message(TXT_Torpedofired[language]);
				break;
			
			// control
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
			case SDLK_F7: viewmode = display_mode_damagecontrol; break;
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
				break;
			case SDLK_d:
				if ( player->has_snorkel () )
				{
					player->dive_to_depth ( static_cast<unsigned> ( player->get_snorkel_depth () ) );
					add_message ( TXT_SnorkelDepth[language] );
				}
				break;
			case SDLK_f:
				if ( player->has_snorkel () )
				{
					if ( player->is_snorkel_up () )
					{
						if ( player->set_snorkel_up ( false ) )
							add_message ( TXT_SnorkelDown[language] );
					}
					else
					{
						if ( player->set_snorkel_up ( true ) )
							add_message ( TXT_SnorkelUp[language] );
					}
				}
				break;
			case SDLK_h:
				{
					bool turn_left = ( angle ( 180.0f ) <= bearing && bearing < angle ( 359.999f ) );
					player->head_to_ang ( bearing, turn_left );
				}
				break;
			case SDLK_p:
				player->dive_to_depth(static_cast<unsigned>(player->get_periscope_depth()));
				add_message(TXT_Periscopedepth[language]);
				break;	//fixme
			case SDLK_s:
				player->dive_to_depth(0);
				add_message(TXT_Surface[language]);
				break;
			case SDLK_v:
				bearing = player->get_heading ();
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
				if (player->fire_torpedo(gm, -1, target))
					add_message(TXT_Torpedofired[language]);
				break;
			case SDLK_SPACE:
				target = gm.contact_in_direction(player, player->get_heading()+bearing);
				if (target)
					add_message(TXT_Newtargetselected[language]);
				else
					add_message(TXT_Notargetindirection[language]);
				break;
			case SDLK_i: {
				// calculate distance to target for identification detail
				if (target)
					add_message(TXT_Identifiedtargetas[language] + target->get_description(2));//fixme
				else
					add_message(TXT_Notargetselected[language]);
				break; }

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
		case display_mode_damagecontrol:
			display_damagecontrol(sys, gm);
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
