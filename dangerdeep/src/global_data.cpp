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

// global data
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "model.h"
#include "texture.h"
#include "image.h"
#include "font.h"
#include "global_data.h"
#include "sound.h"
#include <sstream>
#include <list>
#include <iomanip>
#include "oglext/OglExt.h"
#include "tinyxml/tinyxml.h"
#include "system.h"
#include "datadirs.h"
#include "sound_effect_names.h"
#include <SDL_image.h>
#include <stdexcept>

using namespace std;

// computed with an earth radius of 40030.17359km
const double DEGREE_IN_METERS = 111194.9266388889;
const double MINUTE_IN_METERS = 1853.248777315;


// same with version string
string get_program_version()
{
	return string(VERSION);
}



global_data* global_data::inst = 0;

global_data::global_data()
	: modelcache(get_data_dir()),
	  imagecache(get_image_dir()),
	  texturecache(get_texture_dir()),
	  soundcache(get_sound_dir()),
	  fontcache(get_font_dir())
{
	if (inst != 0)
		throw std::runtime_error("must not initialize global_data object twice");
	inst = this;
}



global_data::~global_data()
{
	inst = 0;
}


// remove this ASAP.
texture *notepadsheet,
	*terraintex, *panelbackground;
	
font *font_arial, *font_arialbd, *font_times, *font_timesbd, *font_verdana, *font_verdanabd, *font_olympiaworn, *font_damagedtypewriter, *font_king, *font_typenr, *font_jphsl, *font_janeaust, *font_vtportable;

bool loading_screen_usable = false;

// later: remove that crap when tinyxml is not used directly any longer
string XmlAttrib(class TiXmlElement* elem, const char* attrname)
{
	const char* tmp = elem->Attribute(attrname);
	if (tmp) return string(tmp);
	return string();
}
unsigned XmlAttribu(class TiXmlElement* elem, const char* attrname)
{
	return unsigned(atoi(XmlAttrib(elem, attrname).c_str()));
}
float XmlAttribf(class TiXmlElement* elem, const char* attrname)
{
	return float(atof(XmlAttrib(elem, attrname).c_str()));
}

void init_global_data()
{
	font_arial = fontcache().ref("font_arial");
	loading_screen_usable = true;
	font_arialbd = fontcache().ref("font_arialbd");
	font_times = fontcache().ref("font_times");
	font_timesbd = fontcache().ref("font_timesbd");
	font_verdana = fontcache().ref("font_verdana");
	font_verdanabd = fontcache().ref("font_verdanabd");
	font_olympiaworn = fontcache().ref("font_olympiaworn");
	font_damagedtypewriter = fontcache().ref("font_damagedtypewriter");
	font_king = fontcache().ref("font_king");
	font_typenr = fontcache().ref("font_typenr");
	font_jphsl = fontcache().ref("font_jphsl");
	font_janeaust = fontcache().ref("font_janeaust");
	font_vtportable = fontcache().ref("font_vtportable");
	add_loading_screen("fonts loaded");
	// later: they should get loaded by environmental classes (sky/water/user_interface)
	panelbackground = 0; // not used with new ingame gui
	notepadsheet = new texture(get_texture_dir() + "notepadsheet.png" /*, GL_CLAMP_TO_EDGE*/);
	terraintex = new texture(get_texture_dir() + "terrain.jpg", texture::LINEAR);
	add_loading_screen("textures loaded");

	// later: this makes the use of the cache senseless. load on demand, and only ingame
	soundcache().ref(se_submarine_torpedo_launch);
	soundcache().ref(se_torpedo_detonation);
	soundcache().ref(se_small_gun_firing);
	soundcache().ref(se_medium_gun_firing);
	soundcache().ref(se_large_gun_firing);
	soundcache().ref(se_depth_charge_firing);
	soundcache().ref(se_depth_charge_exploding);
	soundcache().ref(se_ping);
	soundcache().ref(se_shell_exploding);
	soundcache().ref(se_shell_splash);		
	soundcache().ref(se_sub_screws_slow);
	soundcache().ref(se_sub_screws_normal);
	soundcache().ref(se_sub_screws_fast);
	soundcache().ref(se_sub_screws_very_fast);	
	add_loading_screen("sounds loaded");
}

void deinit_global_data()
{
	delete panelbackground;
	delete notepadsheet;
	delete terraintex;
}

// display loading progress
list<string> loading_screen_messages;
unsigned starttime;
void display_loading_screen()
{
	if (!loading_screen_usable) return;
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);
	sys().prepare_2d_drawing();
	glColor4f(1, 1, 1, 1);
	unsigned fh = font_arial->get_height();
	unsigned y = 0;
	for (list<string>::const_iterator it = loading_screen_messages.begin();
	     it != loading_screen_messages.end(); ++it) {
		font_arial->print(0, y, *it);
		y += fh;
	}
	sys().unprepare_2d_drawing();
	sys().swap_buffers();
}

void reset_loading_screen()
{
	loading_screen_messages.clear();
	loading_screen_messages.push_back("Loading...");
	sys().add_console("Loading...");
	display_loading_screen();
	starttime = sys().millisec();
}

void add_loading_screen(const string& msg)
{
	unsigned tm = sys().millisec();
	unsigned deltatime = tm - starttime;
	starttime = tm;
	ostringstream oss;
	oss << msg << " (" << deltatime << "ms)";
	loading_screen_messages.push_back(oss.str());
	sys().add_console(oss.str());
	display_loading_screen();
}

string get_time_string(double tm)
{
	unsigned seconds = unsigned(floor(myfmod(tm, 86400)));
	unsigned hours = seconds / 3600;
	unsigned minutes = (seconds % 3600) / 60;
	seconds = seconds % 60;
	ostringstream oss;
	oss << setw(2) << setfill('0') << hours << ":" 
	    << setw(2) << setfill('0') << minutes << ":"
	    << setw(2) << setfill('0') << seconds;
	return oss.str();
}


static double transform_nautic_coord_to_real(const string& s, char minus, char plus, int degmax)
{
	if (s.length() < 2)
		throw error(string("nautic coordinate invalid ") + s);
	char sign = s[s.length() - 1];
	if (sign != minus && sign != plus)
		throw error(string("nautic coordinate (direction sign) invalid ") + s);
	// find separator
	string::size_type st = s.find("/");
	if (st == string::npos)
		throw error(string("no separator in position string ") + s);
	string degrees = s.substr(0, st);
	string minutes = s.substr(st + 1, s.length() - st - 2);
	int deg = atoi(degrees.c_str());
	if (deg < 0 || deg > degmax)
		throw error(string("degrees are not in range [0...180/360] in position string ") + s);
	int mts = atoi(minutes.c_str());
	if (mts < 0 || mts > 59)
		throw error(string("minutes are not in [0...59] in position string ") + s);
	return (sign == minus ? -1 : 1) * ((DEGREE_IN_METERS * deg) + (MINUTE_IN_METERS * mts));
}

double transform_nautic_posx_to_real(const string& s)
{
	return transform_nautic_coord_to_real(s, 'W', 'E', 180);
}

double transform_nautic_posy_to_real(const string& s)
{
	return transform_nautic_coord_to_real(s, 'S', 'N', 90);
}

std::list<std::string> string_split(const std::string& src, char splitter)
{
	std::list<std::string> result;
	std::string::size_type where = 0, st = 0;
	do {
		st = src.find(splitter, where);
		if (st == std::string::npos) {
			// rest of string
			result.push_back(src.substr(where));
		} else {
			result.push_back(src.substr(where, st - where));
			where = st + 1;
		}
	} while (st != std::string::npos);
	return result;
}
