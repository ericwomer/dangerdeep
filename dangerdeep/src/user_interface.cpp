// user interface common code
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "oglext/OglExt.h"
#include <GL/glu.h>
#include <SDL.h>

#include <iostream>
#include <sstream>
#include <iomanip>
#include "user_interface.h"
#include "system.h"
#include "game.h"
#include "texts.h"
#include "sound.h"
#include "logbook.h"
#include "model.h"
#include "airplane.h"
#include "depth_charge.h"
#include "gun_shell.h"
#include "water_splash.h"
#include "ships_sunk_display.h"
#include "vector3.h"
#include "widget.h"
#include "tokencodes.h"
#include "command.h"
#include "submarine_interface.h"
//#include "ship_interface.h"
//#include "airplane_interface.h"
#include "sky.h"
#include "water.h"
#include "matrix4.h"
using namespace std;

#define MAX_PANEL_SIZE 256


/*
	a note on our coordinate system (11/10/2003):
	We simulate earth by projecting objects according to curvature from earth
	space to Euclidian space. This projection is yet a identity projection, that means
	we ignore curvature yet.
	The map forms a cylinder around the earth, that means x,y position on the map translates
	to longitude,latitude values. Hence valid coordinates go from -20000km...20000km in x
	direction and -10000km to 10000km in y direction. (we could use exact values, around
	20015km). The wrap around is a problem, but that's somewhere in the Pacific ocean, so
	we just ignore it. This mapping leads to some distorsion and wrong distance values
	when coming to far north or south on the globe. We just ignore this for simplicity's
	sake. The effect shouldn't be noticeable.
*/

user_interface::user_interface(sea_object* player, game& gm) :
	pause(false), time_scale(1), player_object ( player ),
	panel_visible(true), bearing(0), elevation(0),
	viewmode(4), target(0), zoom_scope(false), mapzoom(0.1), mysky(0), mywater(0),
	mycoastmap(get_map_dir() + "default.xml"), freeviewsideang(0), freeviewupang(0), freeviewpos()
{
	init(gm);
	panel = new widget(0, 768-128, 1024, 128, "", 0, panelbackgroundimg);
	panel_messages = new widget_list(8, 8, 512, 128 - 2*8);
	panel->add_child(panel_messages);
	panel->add_child(new widget_text(528, 8, 0, 0, texts::get(1)));
	panel->add_child(new widget_text(528, 8+24+5, 0, 0, texts::get(4)));
	panel->add_child(new widget_text(528, 8+48+10, 0, 0, texts::get(5)));
	panel->add_child(new widget_text(528, 8+72+15, 0, 0, texts::get(2)));
	panel->add_child(new widget_text(528+160, 8, 0, 0, texts::get(98)));
	panel_valuetexts[0] = new widget_text(528+100, 8, 0, 0, "000");
	panel_valuetexts[1] = new widget_text(528+100, 8+24+5, 0, 0, "000");
	panel_valuetexts[2] = new widget_text(528+100, 8+48+10, 0, 0, "000");
	panel_valuetexts[3] = new widget_text(528+100, 8+72+15, 0, 0, "000");
	panel_valuetexts[4] = new widget_text(528+160+100, 8, 0, 0, "000");
	for (unsigned i = 0; i < 5; ++i)
		panel->add_child(panel_valuetexts[i]);
}

user_interface* user_interface::create(game& gm)
{
	sea_object* p = gm.get_player();
	submarine* su = dynamic_cast<submarine*>(p); if (su) return new submarine_interface(su, gm);
	//ship* sh = dynamic_cast<ship*>(p); if (sh) return new ship_interface(sh, gm);
	//airplane* ap = dynamic_cast<airplane*>(p); if (ap) return new airplane_interface(ap, gm);
	return 0;
}

user_interface::~user_interface ()
{
	for (vector<user_display*>::iterator it = displays.begin(); it != displays.end(); ++it)
		delete *it;

	delete panel;

	delete captains_logbook;
	delete ships_sunk_disp;

	delete mysky;
	delete mywater;
}

void user_interface::init(game& gm)
{
	// if the constructors of these classes may ever fail, we should use C++ exceptions.
	captains_logbook = new captains_logbook_display(gm);
	ships_sunk_disp = new ships_sunk_display(gm);

	mysky = new sky();
	mywater = new class water(128, 128, 0.0);
}

/* 2003/07/04 idea.
   simulate earth curvature by drawing several horizon faces
   approximating the curvature.
   earth has medium radius of 6371km, that means 40030km around it.
   A ship with 15m height above the waterline disappears behind
   the horizon at ca. 13.825km distance (7.465 sm)
   
   exact value 40030.17359km. (u), earth radius (r)
   
   height difference in view: (h), distance (d). Formula:
   
   h = r * (1 - cos( 360deg * d / u ) )
   
   or
   
   d = arccos ( 1 - h / r ) * u / 360deg
   
   draw ships with height -h. so (dis)appearing of ships can be
   simulated properly.
   
   highest ships are battleships (approx. 30meters), they disappear
   at 19.551km (10.557 sm).
   
   That's much shorter than I thought! But there is a mistake:
   The viewer's height is not 0 but around 6-8m for submarines,
   so the formulas are more difficult:
   
   The real distance is twice the formula, once for the viewer's
   height, once for the object:
   
   d = (arccos(1 - myh/r) + arccos(1 - h/r)) * u / 360deg
   
   or for the watched object
   
   h = r * (1 - cos( 360deg * (d - (arccos(1 - myh/r)) / u ) )
   
   so for a watcher in 6m height and other ships we have
   arccos(1-myh/r) = 0.07863384deg
   15m in height -> dist: 22.569km (12.186sm)
   30m in height -> dist: 28.295km (15.278sm)
   
   This values are useful for computing "normal" simulation's
   maximum visibility.
   Waves are disturbing sight but are ignored here.
*/	   

void user_interface::rotate_by_pos_and_wave(const vector3& pos,
	double rollfac, bool inverse) const
{
	vector3f rz = mywater->get_normal(pos.xy(), rollfac);
	vector3f rx = vector3f(1, 0, -rz.x).normal();
	vector3f ry = vector3f(0, 1, -rz.y).normal();
	if (inverse) {
		float mat[16] = {
			rx.x, ry.x, rz.x, 0,
			rx.y, ry.y, rz.y, 0,
			rx.z, ry.z, rz.z, 0,
			0,    0,    0,    1 };
		glMultMatrixf(&mat[0]);
	} else {
		float mat[16] = {
			rx.x, rx.y, rx.z, 0,
			ry.x, ry.y, ry.z, 0,
			rz.x, rz.y, rz.z, 0,
			0,    0,    0,    1 };
		glMultMatrixf(&mat[0]);
	}
}

void user_interface::draw_terrain(const vector3& viewpos, angle dir,
	double max_view_dist) const
{
#if 1
	glPushMatrix();
	glTranslatef(0, 0, -viewpos.z);
	terraintex->set_gl_texture();
	mycoastmap.render(viewpos.xy());
	glPopMatrix();
#endif
}

bool user_interface::time_scale_up(void)
{
	if (time_scale < 4096) {
		time_scale *= 2;
		return true;
	}
	return false;
}

bool user_interface::time_scale_down(void)
{
	if (time_scale > 1) {
		time_scale /= 2;
		return true;
	}
	return false;
}

void user_interface::draw_infopanel(class game& gm) const
{
	if (panel_visible) {
		ostringstream os0;
		os0 << setw(3) << left << get_player()->get_heading().ui_value();
		panel_valuetexts[0]->set_text(os0.str());
		ostringstream os1;
		os1 << setw(3) << left << unsigned(fabs(round(sea_object::ms2kts(get_player()->get_speed()))));
		panel_valuetexts[1]->set_text(os1.str());
		ostringstream os2;
		os2 << setw(3) << left << unsigned(round(-get_player()->get_pos().z));
		panel_valuetexts[2]->set_text(os2.str());
		ostringstream os3;
		os3 << setw(3) << left << bearing.ui_value();
		panel_valuetexts[3]->set_text(os3.str());
		ostringstream os4;
		os4 << setw(3) << left << time_scale;
		panel_valuetexts[4]->set_text(os4.str());

		panel->draw();
		// let aside the fact that we should divide DRAWING and INPUT HANDLING
		// the new process_input function eats SDL_Events which we don't have here
//		panel->process_input(true);
	}
}


void user_interface::draw_gauge(class game& gm,
	unsigned nr, int x, int y, unsigned wh, angle a, const string& text, angle a2) const
{
	set_display_color ( gm );
	switch (nr) {
		case 1:	gauge1->draw(x, y, wh, wh); break;
		case 2:	gauge2->draw(x, y, wh, wh); break;
		case 3:	gauge3->draw(x, y, wh, wh); break;
		case 4:	gauge4->draw(x, y, wh, wh); break;
		default: return;
	}
	vector2 d = a.direction();
	int xx = x+wh/2, yy = y+wh/2;
	pair<unsigned, unsigned> twh = font_arial->get_size(text);

	color font_color ( 255, 255, 255 );
	if ( !gm.is_day_mode () )
		font_color = color ( 255, 127, 127 );

	font_arial->print(xx-twh.first/2, yy-twh.second/2, text, font_color);
	glBindTexture(GL_TEXTURE_2D, 0);
	if (a2 != a) {
		vector2 d2 = a2.direction();
		glColor3f(0.2,0.8,1);
		glBegin(GL_LINES);
		glVertex2i(xx, yy);
		glVertex2i(xx + int(d2.x*wh*3/8),yy - int(d2.y*wh*3/8));
		glEnd();
	}
	glColor3f(1,0,0);
	glBegin(GL_TRIANGLES);
	glVertex2i(xx - int(d.y*4),yy - int(d.x*4));
	glVertex2i(xx + int(d.y*4),yy + int(d.x*4));
	glVertex2i(xx + int(d.x*wh*3/8),yy - int(d.y*wh*3/8));
	glEnd();
	glColor3f(1,1,1);
}

void user_interface::draw_clock(class game& gm,
	int x, int y, unsigned wh, double t, const string& text) const
{
	unsigned seconds = unsigned(fmod(t, 86400));
	unsigned minutes = seconds / 60;
	bool is_day_mode = gm.is_day_mode ();

	set_display_color ( gm );
	if (minutes < 12*60)
		clock12->draw(x, y, wh, wh);
	else
		clock24->draw(x, y, wh, wh);
	minutes %= 12*60;
	int xx = x+wh/2, yy = y+wh/2;
	pair<unsigned, unsigned> twh = font_arial->get_size(text);

	color font_color ( 255, 255, 255 );
	if ( !is_day_mode )
		font_color = color ( 255, 127, 127 );

	font_arial->print(xx-twh.first/2, yy-twh.second/2, text, font_color);
	vector2 d;
	int l;

	glBindTexture(GL_TEXTURE_2D, 0);
	glBegin(GL_TRIANGLES);

	d = (angle(minutes * 360 / (12*60))).direction();
	l = wh/4;
	if ( is_day_mode )
		glColor3f(0,0,0.5);
	else
		glColor3f ( 0.5f, 0.0f, 0.5f );
	glVertex2i(xx - int(d.y*4),yy - int(d.x*4));
	glVertex2i(xx + int(d.y*4),yy + int(d.x*4));
	glVertex2i(xx + int(d.x*l),yy - int(d.y*l));

	d = (angle((minutes%60) * 360 / 60)).direction();
	l = wh*3/8;
	if ( is_day_mode )
		glColor3f(0,0,1);
	else
		glColor3f ( 0.5f, 0.0f, 1.0f );
	glVertex2i(xx - int(d.y*4),yy - int(d.x*4));
	glVertex2i(xx + int(d.y*4),yy + int(d.x*4));
	glVertex2i(xx + int(d.x*l),yy - int(d.y*l));

	d = (angle((seconds%60) * 360 / 60)).direction();
	l = wh*7/16;
	glColor3f(1,0,0);
	glVertex2i(xx - int(d.y*4),yy - int(d.x*4));
	glVertex2i(xx + int(d.y*4),yy + int(d.x*4));
	glVertex2i(xx + int(d.x*l),yy - int(d.y*l));

	glEnd();
	glColor3f(1,1,1);
}

void user_interface::display_logbook(game& gm)
{
	class system& sys = system::sys();

	// glClearColor ( 0.5f, 0.25f, 0.25f, 0 );
	glClearColor ( 0, 0, 0, 0 );
	glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	sys.prepare_2d_drawing ();
	captains_logbook->display ( gm );
	draw_infopanel ( gm );
	sys.unprepare_2d_drawing ();

	// mouse processing;
	int mx;
	int my;
	int mb = sys.get_mouse_buttons();
	sys.get_mouse_position(mx, my);
	if ( mb & sys.left_button )
		captains_logbook->check_mouse ( mx, my, mb );

	// keyboard processing
	int key = sys.get_key().sym;
	while (key != 0) {
		if (!keyboard_common(key, gm)) {
			// specific keyboard processing
			captains_logbook->check_key ( key, gm );
		}
		key = sys.get_key().sym;
	}
}

void user_interface::display_successes(game& gm)
{
	class system& sys = system::sys();

	// glClearColor ( 0, 0, 0, 0 );
	// glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	sys.prepare_2d_drawing ();
	ships_sunk_disp->display ( gm );
	draw_infopanel ( gm );
	sys.unprepare_2d_drawing ();

	// keyboard processing
	int key = sys.get_key ().sym;
	while ( key != 0 )
	{
		if ( !keyboard_common ( key, gm ) )
		{
			// specific keyboard processing
			ships_sunk_disp->check_key ( key, gm );
		}
		key = sys.get_key ().sym;
	}
}

#ifdef OLD
void user_interface::display_successes(game& gm)
{
	class system& sys = system::sys();

	glClearColor(0.25, 0.25, 0.25, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	sys.prepare_2d_drawing();
	font_arial->print(0, 0, "success records - fixme");
	font_arial->print(0, 100, "Ships sunk\n----------\n");
	unsigned ships = 0, tons = 0;
	for (list<unsigned>::const_iterator it = tonnage_sunk.begin(); it != tonnage_sunk.end(); ++it) {
		++ships;
		char tmp[20];
		sprintf(tmp, "%u BRT", *it);
		font_arial->print(0, 100+(ships+2)*font_arial->get_height(), tmp);
		tons += *it;
	}
	char tmp[40];
	sprintf(tmp, "total: %u BRT", tons);
	font_arial->print(0, 100+(ships+4)*font_arial->get_height(), tmp);
	sys.unprepare_2d_drawing();

	// keyboard processing
	int key = sys.get_key().sym;
	while (key != 0) {
		if (!keyboard_common(key, gm)) {
			// specific keyboard processing
		}
		key = sys.get_key().sym;
	}
}
#endif // OLD

void user_interface::add_message(const string& s)
{
	panel_messages->append_entry(s);
	if (panel_messages->get_listsize() > panel_messages->get_nr_of_visible_entries())
		panel_messages->delete_entry(0);
/*
	panel_texts.push_back(s);
	if (panel_texts.size() > 1+MAX_PANEL_SIZE/font_arial->get_height())
		panel_texts.pop_front();
*/		
}

void user_interface::add_rudder_message()
{
	ship* s = dynamic_cast<ship*>(player_object);
	if (!s) return;	// ugly hack to allow compilation
    switch (s->get_rudder_to())
    {
        case ship::rudderfullleft:
            add_message(texts::get(35));
            break;
        case ship::rudderleft:
            add_message(texts::get(33));
            break;
        case ship::ruddermidships:
            add_message(texts::get(42));
            break;
        case ship::rudderright:
            add_message(texts::get(34));
            break;
        case ship::rudderfullright:
            add_message(texts::get(36));
            break;
    }
}

#define DAY_MODE_COLOR() glColor3f ( 1.0f, 1.0f, 1.0f )

#define NIGHT_MODE_COLOR() glColor3f ( 1.0f, 0.4f, 0.4f )

void user_interface::set_display_color ( color_mode mode ) const
{
	switch ( mode )
	{
		case night_color_mode:
			NIGHT_MODE_COLOR ();
			break;
		default:
			DAY_MODE_COLOR ();
			break;
	}
}

void user_interface::set_display_color ( const class game& gm ) const
{
	if ( gm.is_day_mode () )
		DAY_MODE_COLOR ();
	else
		NIGHT_MODE_COLOR ();
}

sound* user_interface::get_sound_effect ( sound_effect se ) const
{
	sound* s = 0;

	switch ( se )
	{
		case se_submarine_torpedo_launch:
			s = torpedo_launch_sound;
			break;
		case se_torpedo_detonation:
			{
				submarine* sub = dynamic_cast<submarine*>( player_object );

				if ( sub && sub->is_submerged () )
				{
					double sid = rnd ( 2 );
					if ( sid < 1.0f )
						s = torpedo_detonation_submerged[0];
					else if ( sid < 2.0f )
						s = torpedo_detonation_submerged[1];
				}
				else
				{
					double sid = rnd ( 2 );
					if ( sid < 1.0f )
						s = torpedo_detonation_surfaced[0];
					else if ( sid < 2.0f )
						s = torpedo_detonation_surfaced[1];
				}
			}
			break;
	}

	return s;
}

void user_interface::play_sound_effect ( sound_effect se, double volume ) const
{
	sound* s = get_sound_effect ( se );

	if ( s )
		s->play ( volume );
}

void user_interface::play_sound_effect_distance ( sound_effect se, double distance ) const
{
	sound* s = get_sound_effect ( se );

	if ( s )
		s->play ( ( 1.0f - player_object->get_noise_factor () ) * exp ( - distance / 3000.0f ) );
}

void user_interface::add_captains_log_entry ( class game& gm, const string& s)
{
	date d(unsigned(gm.get_time()));

	if ( captains_logbook )
		captains_logbook->add_entry( d, s );
}

inline void user_interface::record_sunk_ship ( const ship* so )
{
	ships_sunk_disp->add_sunk_ship ( so );
}

void user_interface::draw_manometer_gauge ( class game& gm,
	unsigned nr, int x, int y, unsigned wh, float value, const string& text) const
{
	set_display_color ( gm );
	switch (nr)
	{
		case 1:
			gauge5->draw ( x, y, wh, wh / 2 );
			break;
		default:
			return;
	}
	angle a ( 292.5f + 135.0f * value );
	vector2 d = a.direction ();
	int xx = x + wh / 2, yy = y + wh / 2;
	pair<unsigned, unsigned> twh = font_arial->get_size(text);

	// Draw text.
	color font_color ( 0, 0, 0 );
	font_arial->print ( xx - twh.first / 2, yy - twh.second / 2 - wh / 6,
		text, font_color );

	// Draw pointer.
	glColor3f ( 0.0f, 0.0f, 0.0f );
	glBindTexture ( GL_TEXTURE_2D, 0 );
	glBegin ( GL_LINES );
	glVertex2i ( xx + int ( d.x * wh / 16 ), yy - int ( d.y * wh / 16 ) );
	glVertex2i ( xx + int ( d.x * wh * 3 / 8 ), yy - int ( d.y * wh * 3 / 8 ) );
	glEnd ();
	glColor3f ( 1.0f, 1.0f, 1.0f );
}
