// global data
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef GLOBAL_DATA_H
#define GLOBAL_DATA_H

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <SDL.h>
#include <string>
#include <cmath>
using namespace std;
#include "date.h"



string get_data_dir(void);
string get_program_version(void);
inline string get_texture_dir(void) { return get_data_dir() + "textures/"; }
inline string get_font_dir(void) { return get_data_dir() + "fonts/"; }
inline string get_model_dir(void) { return get_data_dir() + "models/"; }
inline string get_ship_dir(void) { return get_data_dir() + "ships/"; }
inline string get_submarine_dir(void) { return get_data_dir() + "submarines/"; }
inline string get_airplane_dir(void) { return get_data_dir() + "airplanes/"; }
inline string get_sound_dir(void) { return get_data_dir() + "sounds/"; }
inline string get_image_dir(void) { return get_data_dir() + "images/"; }
inline string get_mission_dir(void) { return get_data_dir() + "missions/"; }
inline string get_map_dir(void) { return get_data_dir() + "maps/"; }

string XmlAttrib(class TiXmlElement* elem, const char* attrname);
unsigned XmlAttribu(class TiXmlElement* elem, const char* attrname);

#define GRAVITY 9.806	// a very global constant

#include "objcache.h"
extern objcachet<class model> modelcache;
extern objcachet<class image> imagecache;
extern objcachet<class texture> texturecache;

// global models, textures, fonts
//fixme: get rid of this, instead use caches, maybe even for fonts.
extern class model *torpedo_g7, *depth_charge_mdl, *gun_shell_mdl, *conning_tower_typeVII;
extern class texture *background, *gauge1,
	*gauge2, *gauge3, *gauge4, *gauge5, *psbackgr, *panelbackgr,
	*addleadangle, *metalbackgr,
	*clock12, *clock24,
	*woodbackgr, *notepadsheet, *menuframe, *turnswitch, *turnswitchbackgr,
	*repairlight, *repairmedium, *repairheavy, *repaircritical, *repairwrecked,
	*terraintex, *cloudsbackgr, *atlanticmap;
extern class font *font_arial, *font_arialbd, *font_times, *font_timesbd, *font_verdana, *font_verdanabd;
extern class sound *torpedo_launch_sound, *torpedo_detonation_submerged[2],
	*torpedo_detonation_surfaced[2];
extern class image *titlebackgrimg, *periscope, *threesubsimg, *damage_screen_background,
	*sub_damage_scheme_all, *killedimg, *scopewatcherimg,
	*depthchargeimg, *sunkendestroyerimg, *kruppdocksimg, *rescuedestroyerimg, *sunderlandimg,
	*swordfishimg, *hedgehogimg, *panelbackgroundimg;

void init_global_data(void);
void deinit_global_data(void);

// display loading progress
void reset_loading_screen(void);
void add_loading_screen(const string& msg);

// transform time in seconds to 24h time of clock string (takes remainder of 86400 seconds first = 1 day)
string get_time_string(double tm);

// handle modulo calculation for negative values the way I need it
inline float myfmod(float a, float b) { return a-floorf(a/b)*b; }//fmod is different for negative a/b
inline float myfrac(float a) { return a-floorf(a); }
inline double myfmod(double a, double b) { return a-floor(a/b)*b; }//fmod is different for negative a/b
inline double myfrac(double a) { return a-floor(a); }
inline float mysgn(float a) { return (a < 0) ? -1.0f : ((a > 0) ? 1.0f : 0.0f); }
inline double mysgn(double a) { return (a < 0) ? -1.0 : ((a > 0) ? 1.0 : 0.0); }
template<class C> inline void add_saturated(C& sum, const C& add, const C& max) { sum += add; if (sum > max) sum = max; }
// return a random value in [0, 1(
inline double rnd(void) { return double(rand())/RAND_MAX; }
inline unsigned rnd(unsigned b) { return unsigned(b*rnd()); }

#endif
