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
#include <SDL_image.h>


// return global data directory. all other directories depend on this.
// by defining it only here, we need to recompile only one object file
string get_data_dir(void)
{
	return string(DATADIR);
}



// same with version string
string get_program_version(void)
{
	return string(VERSION);
}



// fixme: this could be replaced with an array of pointers using enum-names
// as indices. This would simplify destruction and possibly construction.

// fixme2: forget enums. use auto_ptr's instead. MUCH easier!
// normal data should get shrinked as much as possible, only fonts should remain or so.

model	*torpedo_g7, *depth_charge_mdl, *gun_shell_mdl, *conning_tower_typeVII;

texture *background,
	*psbackgr, *panelbackgr,
	*addleadangle, *torpleft, *metalbackgr,
	*woodbackgr, *notepadsheet, *menuframe, *turnswitch, *turnswitchbackgr,
	*repairlight, *repairmedium, *repairheavy, *repaircritical, *repairwrecked,
	*terraintex, *cloudsbackgr, *atlanticmap;
	
font *font_arial, *font_arialbd, *font_times, *font_timesbd, *font_verdana, *font_verdanabd, *font_olympiaworn;

image *titlebackgrimg, *threesubsimg, *damage_screen_background,
	*sub_damage_scheme_all, *killedimg, *scopewatcherimg,
	*depthchargeimg, *sunkendestroyerimg, *kruppdocksimg, *rescuedestroyerimg, *sunderlandimg,
	*swordfishimg, *hedgehogimg, *panelbackgroundimg;

bool loading_screen_usable = false;

objcachet<class model> modelcache(get_model_dir());
objcachet<class image> imagecache(get_image_dir());
objcachet<class texture> texturecache(get_texture_dir());
objcachet<class sound> soundcache(get_sound_dir());

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

void init_global_data(void)
{
	background = new texture(get_texture_dir() + "background.png", texture::LINEAR);
	conning_tower_typeVII = new model(get_model_dir() + "conning_tower_typeVIIc.3ds");
	font_arial = new font(get_font_dir() + "font_arial");
	loading_screen_usable = true;
	font_arialbd = new font(get_font_dir() + "font_arialbd");
	font_times = new font(get_font_dir() + "font_times");
	font_timesbd = new font(get_font_dir() + "font_timesbd");
	font_verdana = new font(get_font_dir() + "font_verdana");
	font_verdanabd = new font(get_font_dir() + "font_verdanabd");
	font_olympiaworn = new font(get_font_dir() + "font_olympiaworn");
	add_loading_screen("fonts loaded");
	torpedo_g7 = new model(get_model_dir() + "torpedo_g7.3ds");
	depth_charge_mdl = new model(get_model_dir() + "depth_charge.3ds");
	gun_shell_mdl = new model(get_model_dir() + "gun_shell.3ds");
	psbackgr = new texture(get_texture_dir() + "psbackgr.png");
	panelbackgr = new texture(get_texture_dir() + "panelbackgr.png", texture::LINEAR);
	addleadangle = new texture(get_texture_dir() + "addleadangle.png");
	metalbackgr = new texture(get_texture_dir() + "metalbackgr.png", texture::LINEAR);
	woodbackgr = new texture(get_texture_dir() + "wooden_desk.png" );
	notepadsheet = new texture(get_texture_dir() + "notepadsheet.png" /*, GL_CLAMP_TO_EDGE*/);
	menuframe = new texture(get_texture_dir() + "menuframe.png" );
	turnswitch = new texture(get_texture_dir() + "turnswitch.png" );
	turnswitchbackgr = new texture(get_texture_dir() + "turnswitchbackgr.png" );
	terraintex = new texture(get_texture_dir() + "terrain.jpg", texture::LINEAR);
	cloudsbackgr = new texture(get_texture_dir() + "cloudsbackgr.jpg" );
	atlanticmap = new texture(get_texture_dir() + "atlanticmap.jpg", texture::LINEAR, texture::CLAMP_TO_EDGE);
	add_loading_screen("textures loaded");

	soundcache.ref(se_submarine_torpedo_launch);
	soundcache.ref(se_torpedo_detonation);
	soundcache.ref(se_small_gun_firing);
	soundcache.ref(se_medium_gun_firing);
	soundcache.ref(se_large_gun_firing);
	soundcache.ref(se_depth_charge_firing);
	soundcache.ref(se_depth_charge_exploding);
	soundcache.ref(se_ping);
	soundcache.ref(se_shell_exploding);
	soundcache.ref(se_shell_splash);		
	soundcache.ref(se_sub_screws_slow);
	soundcache.ref(se_sub_screws_normal);
	soundcache.ref(se_sub_screws_fast);
	soundcache.ref(se_sub_screws_very_fast);	
	add_loading_screen("sounds loaded");

	titlebackgrimg = new image(get_image_dir() + "titlebackgr.jpg");

	threesubsimg = new image(get_image_dir() + "threesubs.jpg");

	damage_screen_background = new image(get_image_dir() + "damage_screen_backg.jpg");

	sub_damage_scheme_all = new image(get_image_dir() + "sub_damage_scheme_all.png");

	killedimg = new image(get_image_dir() + "killed.jpg");
	scopewatcherimg = new image(get_image_dir() + "scopewatcher.jpg");
	depthchargeimg = new image(get_image_dir() + "depthcharge.jpg");
	sunkendestroyerimg = new image(get_image_dir() + "sunken_destroyer.jpg");
	kruppdocksimg = new image(get_image_dir() + "krupp_docks.jpg");
	rescuedestroyerimg = new image(get_image_dir() + "rescue_destroyer.jpg");
	sunderlandimg = new image(get_image_dir() + "sunderland.jpg");
	swordfishimg = new image(get_image_dir() + "swordfish.jpg");
	hedgehogimg = new image(get_image_dir() + "hedgehog.jpg");
	panelbackgroundimg = new image(get_image_dir() + "panelbackground.jpg");
}

void deinit_global_data(void)
{
	delete background;
	delete conning_tower_typeVII;
	delete font_arial;
	delete font_arialbd;
	delete font_times;
	delete font_timesbd;
	delete font_verdana;
	delete font_verdanabd;
	delete torpedo_g7;
	delete depth_charge_mdl;
	delete gun_shell_mdl;
	delete psbackgr;
	delete panelbackgr;
	delete addleadangle;
	delete metalbackgr;
	delete woodbackgr;
	delete notepadsheet;
	delete menuframe;
	delete turnswitch;
	delete turnswitchbackgr;
	delete terraintex;
	delete cloudsbackgr;
	delete atlanticmap;

	delete titlebackgrimg;
	delete threesubsimg;
	delete damage_screen_background;
	delete sub_damage_scheme_all;
	delete killedimg;
	delete scopewatcherimg;
	delete depthchargeimg;
	delete sunkendestroyerimg;
	delete kruppdocksimg;
	delete rescuedestroyerimg;
	delete sunderlandimg;
	delete swordfishimg;
	delete hedgehogimg;
	delete panelbackgroundimg;
}

// display loading progress
list<string> loading_screen_messages;
unsigned starttime;
void display_loading_screen(void)
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

void reset_loading_screen(void)
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
