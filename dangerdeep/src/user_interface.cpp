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
#include "model.h"
#include "airplane.h"
#include "depth_charge.h"
#include "gun_shell.h"
#include "vector3.h"
#include "widget.h"
#include "submarine_interface.h"
#include "submarine.h"	// needed for underwater sound reduction
//#include "ship_interface.h"
//#include "airplane_interface.h"
#include "sky.h"
#include "particle.h"
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

user_interface::user_interface(game& gm) :
	pause(false), time_scale(1),
	panel_visible(true),
	current_display(0), target(0), mysky(0), mywater(0),
	mycoastmap(get_map_dir() + "default.xml")
{
	add_loading_screen("coast map initialized");
	mysky = new sky();
	mywater = new class water(128, 128, 0.0);//fixme: make detail configureable
	panel = new widget(0, 768-128, 1024, 128, "", 0, panelbackgroundimg);
	panel_messages = new widget_list(8, 8, 512, 128 - 2*8);
	panel->add_child(panel_messages);
	panel->add_child(new widget_text(528, 8, 0, 0, texts::get(1)));
	panel->add_child(new widget_text(528, 8+24+5, 0, 0, texts::get(4)));
	panel->add_child(new widget_text(528, 8+48+10, 0, 0, texts::get(5)));
	panel->add_child(new widget_text(528, 8+72+15, 0, 0, texts::get(2)));
	panel->add_child(new widget_text(528+160, 8, 0, 0, texts::get(98)));
	panel->add_child(new widget_text(528+160, 8+24+5, 0, 0, texts::get(61)));
	panel_valuetexts[0] = new widget_text(528+100, 8, 0, 0, "000");
	panel_valuetexts[1] = new widget_text(528+100, 8+24+5, 0, 0, "000");
	panel_valuetexts[2] = new widget_text(528+100, 8+48+10, 0, 0, "000");
	panel_valuetexts[3] = new widget_text(528+100, 8+72+15, 0, 0, "000");
	panel_valuetexts[4] = new widget_text(528+160+100, 8, 0, 0, "000");
	panel_valuetexts[5] = new widget_text(528+160+100, 8+24+5, 0, 0, "00:00:00");
	for (unsigned i = 0; i < 6; ++i)
		panel->add_child(panel_valuetexts[i]);
	panel->add_child(new widget_caller_button<game, void (game::*)(void)>(&gm, &game::stop, 1024-128-8, 128-40, 128, 32, texts::get(177)));
	add_loading_screen("user interface initialized");

	// create weather effects textures

	// rain
#if 0
#define NR_OF_RAIN_FRAMES 16
#define NR_OF_RAIN_DROPS 800
#define RAIN_TEX_W 256
#define RAIN_TEX_H 256
	raintex.resize(NR_OF_RAIN_FRAMES);
	vector<Uint8> raintmptex(RAIN_TEX_W * RAIN_TEX_H * 2);

	for (unsigned j = 0; j < NR_OF_RAIN_FRAMES; ++j) {
		for (unsigned k = 0; k < RAIN_TEX_W * RAIN_TEX_H * 2; k += 2) {
			raintmptex[k + 0] = 128;
			raintmptex[k + 1] = 0;
		}
		for (unsigned i = 0; i < NR_OF_RAIN_DROPS; ++i) {
			vector2i pos(rnd(RAIN_TEX_W-2)+2, rnd(RAIN_TEX_H-2));
			Uint8 c = rnd(64)+128;
			raintmptex[(RAIN_TEX_W*pos.y + pos.x) * 2 + 0] = c;
			raintmptex[(RAIN_TEX_W*pos.y + pos.x) * 2 + 1] = 128;
			pos.x -= 1; pos.y += 1;
			raintmptex[(RAIN_TEX_W*pos.y + pos.x) * 2 + 0] = c;
			raintmptex[(RAIN_TEX_W*pos.y + pos.x) * 2 + 1] = 192;
			pos.x -= 1; pos.y += 1;
			raintmptex[(RAIN_TEX_W*pos.y + pos.x) * 2 + 0] = c;
			raintmptex[(RAIN_TEX_W*pos.y + pos.x) * 2 + 1] = 255;
		}
		raintex[j] = new texture(&raintmptex[0], RAIN_TEX_W, RAIN_TEX_H, GL_LUMINANCE_ALPHA, GL_LINEAR_MIPMAP_LINEAR, GL_REPEAT, false);
	}
#endif
	// snow
#if 0
#define NR_OF_SNOW_FRAMES 23
#define NR_OF_SNOW_FLAKES 2000
#define SNOW_TEX_W 256
#define SNOW_TEX_H 256
	snowtex.resize(NR_OF_SNOW_FRAMES);
	vector<Uint8> snowtmptex(SNOW_TEX_W * SNOW_TEX_H * 2, 255);
	vector<vector2i> snowflakepos(NR_OF_SNOW_FLAKES);
	vector<int> snowxrand(NR_OF_SNOW_FRAMES);

	// create random x coordinate sequence (perturbation)
	vector<unsigned> snowxtrans(SNOW_TEX_W);
	for (unsigned k = 0; k < SNOW_TEX_W; ++k) {
		snowxtrans[k] = k;
	}
	for (unsigned k = 0; k < SNOW_TEX_W * 20; ++k) {
		unsigned a = rnd(SNOW_TEX_W), b = rnd(SNOW_TEX_W);
		unsigned c = snowxtrans[a];
		snowxtrans[a] = snowxtrans[b];
		snowxtrans[b] = c;
	}

	for (unsigned j = 0; j < NR_OF_SNOW_FRAMES; ++j) {
		snowxrand[j] = rnd(3)-1;
	}
	snowflakepos[0] = vector2i(snowxtrans[0], 0);
	for (unsigned i = 1; i < NR_OF_SNOW_FLAKES; ++i) {
		vector2i oldpos = snowflakepos[i-1];
		for (unsigned j = 0; j < NR_OF_SNOW_FRAMES; ++j) {
			oldpos.x += snowxrand[(j+3*i)%NR_OF_SNOW_FRAMES];
			if (oldpos.x < 0) oldpos.x += SNOW_TEX_W;
			if (oldpos.x >= SNOW_TEX_W) oldpos.x -= SNOW_TEX_W;
			oldpos.y += 1;	// fixme add more complex "fall down" function
			if (oldpos.y >= SNOW_TEX_H) {
				oldpos.x = snowxtrans[oldpos.x];
				oldpos.y = 0;
			}
		}
		snowflakepos[i] = oldpos;
	}
	for (unsigned i = 0; i < NR_OF_SNOW_FRAMES; ++i) {
		for (unsigned k = 0; k < SNOW_TEX_W * SNOW_TEX_H * 2; k += 2)
			snowtmptex[k + 1] = 0;
		for (unsigned j = 0; j < NR_OF_SNOW_FLAKES; ++j) {
			snowtmptex[(SNOW_TEX_H*snowflakepos[j].y + snowflakepos[j].x) * 2 + 1] = 255;
			vector2i& oldpos = snowflakepos[j];
			oldpos.x += snowxrand[(j+3*i)%NR_OF_SNOW_FRAMES];
			if (oldpos.x < 0) oldpos.x += SNOW_TEX_W;
			if (oldpos.x >= SNOW_TEX_W) oldpos.x -= SNOW_TEX_W;
			oldpos.y += 1;	// fixme add more complex "fall down" function
			if (oldpos.y >= SNOW_TEX_H) {
				oldpos.x = snowxtrans[oldpos.x];
				oldpos.y = 0;
			}
		}
/*
		ostringstream oss;
		oss << "snowframe"<<i<<".pgm";
		ofstream osg(oss.str().c_str());
		osg << "P5\n"<<SNOW_TEX_W<<" "<<SNOW_TEX_H<<"\n255\n";
		osg.write((const char*)(&snowtmptex[0]), SNOW_TEX_W * SNOW_TEX_H);
*/
		snowtex[i] = new texture(&snowtmptex[0], SNOW_TEX_W, SNOW_TEX_H, GL_LUMINANCE_ALPHA, GL_LINEAR_MIPMAP_LINEAR, GL_REPEAT, false);
	}
#endif

	particle::init();
}

user_interface* user_interface::create(game& gm)
{
	sea_object* p = gm.get_player();
	submarine* su = dynamic_cast<submarine*>(p); if (su) return new submarine_interface(gm);
	//ship* sh = dynamic_cast<ship*>(p); if (sh) return new ship_interface(gm);
	//airplane* ap = dynamic_cast<airplane*>(p); if (ap) return new airplane_interface(gm);
	return 0;
}

user_interface::~user_interface ()
{
	for (vector<user_display*>::iterator it = displays.begin(); it != displays.end(); ++it)
		delete *it;

	delete panel;

	delete mysky;
	delete mywater;

	for (unsigned i = 0; i < raintex.size(); ++i)
		delete raintex[i];
	for (unsigned i = 0; i < snowtex.size(); ++i)
		delete snowtex[i];

	particle::deinit();
}



void user_interface::display(game& gm) const
{
	// fixme: brightness needs sun_pos, so compute_sun_pos() is called multiple times per frame
	// but is very costly. we could cache it.
	mywater->set_refraction_color(gm.compute_light_brightness(gm.get_player()->get_pos()));
	displays[current_display]->display(gm);
}



void user_interface::set_time(double tm)
{
	mysky->set_time(tm);
	mywater->set_time(tm);
}



void user_interface::process_input(class game& gm, const SDL_Event& event)
{
	displays[current_display]->process_input(gm, event);
}



void user_interface::process_input(class game& gm, const list<SDL_Event>& events)
{
	// a bit misplaced here...
	if (target && !target->is_alive()) target = 0;

	for (list<SDL_Event>::const_iterator it = events.begin();
	     it != events.end(); ++it)
		process_input(gm, *it);
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
#if 0	// terrain pulls fps down from 45 to 33...
	glPushMatrix();
	//fixme this should be removed...
//	glTranslatef(0, 0, -viewpos.z);
	terraintex->set_gl_texture();
	mycoastmap.render(viewpos.xy());
	glPopMatrix();
#endif
}



void user_interface::draw_weather_effects(game& gm) const
{
#if 0
	// draw layers of snow flakes or rain drops (test)
	// get projection from frustum to view
	matrix4 c2w = (matrix4::get_gl(GL_PROJECTION_MATRIX) * matrix4::get_gl(GL_MODELVIEW_MATRIX)).inverse();
	// draw planes between z-near and z-far with ascending distance and 2d texture with flakes/strains
	glDisable(GL_LIGHTING);
	unsigned sf = unsigned(gm.get_time() * NR_OF_RAIN_FRAMES) % NR_OF_RAIN_FRAMES;
//	unsigned sf = unsigned(gm.get_time() * NR_OF_SNOW_FRAMES) % NR_OF_SNOW_FRAMES;
	raintex[sf]->set_gl_texture();
//	snowtex[sf]->set_gl_texture();
	//pd.near_z,pd.far_z
	double zd[3] = { 0.3, 0.9, 0.7 };
	glBegin(GL_QUADS);
	//fixme: planes should be orthogonal to z=0 plane (xy billboarding)
	for (unsigned i = 0; i < 1; ++i) {
		vector3 p0 = c2w * vector3(-1,  1, zd[i]);
		vector3 p1 = c2w * vector3(-1, -1, zd[i]);
		vector3 p2 = c2w * vector3( 1, -1, zd[i]);
		vector3 p3 = c2w * vector3( 1,  1, zd[i]);
		glTexCoord2f(0, 0);	//fixme: uv size changes with depth
		glVertex3dv(&p0.x);
		glTexCoord2f(0, 3);
		glVertex3dv(&p1.x);
		glTexCoord2f(3, 3);
		glVertex3dv(&p2.x);
		glTexCoord2f(3, 0);
		glVertex3dv(&p3.x);
	}
	glEnd();
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
		os0 << setw(3) << left << gm.get_player()->get_heading().ui_value();
		panel_valuetexts[0]->set_text(os0.str());
		ostringstream os1;
		os1 << setw(3) << left << unsigned(fabs(round(sea_object::ms2kts(gm.get_player()->get_speed()))));
		panel_valuetexts[1]->set_text(os1.str());
		ostringstream os2;
		os2 << setw(3) << left << unsigned(round(-gm.get_player()->get_pos().z));
		panel_valuetexts[2]->set_text(os2.str());
		ostringstream os3;
		os3 << setw(3) << left << bearing.ui_value();
		panel_valuetexts[3]->set_text(os3.str());
		ostringstream os4;
		os4 << setw(3) << left << time_scale;
		panel_valuetexts[4]->set_text(os4.str());
		// compute time string
		panel_valuetexts[5]->set_text(get_time_string(gm.get_time()));

		panel->draw();
		// let aside the fact that we should divide DRAWING and INPUT HANDLING
		// the new process_input function eats SDL_Events which we don't have here
//		panel->process_input(true);
	}
}



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



void user_interface::add_rudder_message(game& gm)
{
	// this whole function should be replaced...seems ugly
	ship* s = dynamic_cast<ship*>(gm.get_player());
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

sound* user_interface::get_sound_effect ( game& gm, sound_effect se ) const
{
	sound* s = 0;

	switch ( se )
	{
		case se_submarine_torpedo_launch:
			s = torpedo_launch_sound;
			break;
		case se_torpedo_detonation:
			{
				submarine* sub = dynamic_cast<submarine*>(gm.get_player());

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

void user_interface::play_sound_effect ( game& gm, sound_effect se, double volume ) const
{
	sound* s = get_sound_effect ( gm, se );

	if ( s )
		s->play ( volume );
}



void user_interface::play_sound_effect_distance(game& gm, sound_effect se, double distance) const
{
	sound* s = get_sound_effect(gm, se);

	if ( s )
	{
		double h = 3000.0f;
		submarine* sub = dynamic_cast<submarine*> ( gm.get_player () );
		if ( sub && sub->is_submerged () )
			h = 10000.0f;

		s->play ( ( 1.0f - gm.get_player()->get_noise_factor () ) * exp ( - distance / h ) );
	}
}
