// global data
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef GLOBAL_DATA_H
#define GLOBAL_DATA_H

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#define DATADIR "./data/"
#endif

#include <SDL.h>
#include <string>
#include <cmath>
using namespace std;
#include "date.h"



inline string get_data_dir(void) { return DATADIR; }
inline string get_texture_dir(void) { return get_data_dir() + "textures/"; }
inline string get_font_dir(void) { return get_data_dir() + "fonts/"; }
inline string get_model_dir(void) { return get_data_dir() + "models/"; }
inline string get_ship_dir(void) { return get_data_dir() + "ships/"; }
inline string get_sound_dir(void) { return get_data_dir() + "sounds/"; }
inline string get_image_dir(void) { return get_data_dir() + "images/"; }
inline string get_mission_dir(void) { return get_data_dir() + "missions/"; }
inline string get_map_dir(void) { return get_data_dir() + "maps/"; }

#define GRAVITY 9.806	// a very global constant
#define NR_CLOUD_TEXTURES 2

#include "objcache.h"
extern objcachet<class model> modelcache;
extern objcachet<class image> imagecache;
extern objcachet<class texture> texturecache;

// global models, textures, fonts
extern class model *merchant_large, *merchant_medium, *merchant_small,
	*subVII, *subIXc40, *subXXI, *destroyer_tribal, *troopship_medium,
	*battleship_malaya, *carrier_bogue, *torpedo_g7, *depth_charge_mdl, *gun_shell_mdl,
	*corvette_mdl, *freighter_medium, *freighter_large,
	*tanker_small, *conning_tower_typeVII, *std_plane;
extern class texture *background, *gauge1,
	*gauge2, *gauge3, *gauge4, *gauge5, *psbackgr, *panelbackgr,
	*addleadangle, *torpempty, *torpreload, *torpunload, *uzo, *metalbackgr,
	*torpt1, *torpt2, *torpt3, *torpt3a, *torpt4, *torpt5, *torpt11, *torpt1fat, *torpt3fat, *torpt6lut,
	*clock12, *clock24, *glasses, *torp_expl_water_splash[3],
	*woodbackgr, *smoke, *notepadsheet, *menuframe, *turnswitch, *turnswitchbackgr,
	*repairlight, *repairmedium, *repairheavy, *repaircritical, *repairwrecked,
	*terraintex, *cloudsbackgr, *atlanticmap;
extern class font *font_arial, *font_panel, *font_nimbusrom;
extern class sound *torpedo_launch_sound, *torpedo_detonation_submerged[2],
	*torpedo_detonation_surfaced[2];
extern class image *titlebackgrimg, *periscope, *threesubsimg, *damage_screen_background,
	*sub_damage_scheme_all, *logbook_spiral, *killedimg, *scopewatcherimg,
	*depthchargeimg, *sunkendestroyerimg, *kruppdocksimg, *rescuedestroyerimg, *sunderlandimg,
	*swordfishimg, *hedgehogimg, *panelbackgroundimg;

void init_global_data(void);
void deinit_global_data(void);

// fixme: move this to class date
void get_date(double t, unsigned& year, unsigned& month, unsigned& day);
void get_date ( double t, unsigned& year, unsigned& month, unsigned& day,
	unsigned& hour, unsigned& minute, unsigned& second );
void get_date ( double t, date& d );
double get_time(unsigned year, unsigned month, unsigned day);

// handle modulo calculation for negative values the way I need it
inline float myfmod(float a, float b) { return a-floorf(a/b)*b; }
inline float myfrac(float a) { return a-floorf(a); }
inline double myfmod(double a, double b) { return a-floor(a/b)*b; }
inline double myfrac(double a) { return a-floor(a); }
inline float mysgn(float a) { return (a < 0) ? -1.0f : ((a > 0) ? 1.0f : 0.0f); }
inline double mysgn(double a) { return (a < 0) ? -1.0 : ((a > 0) ? 1.0 : 0.0); }
template<class C> inline void add_saturated(C& sum, const C& add, const C& max) { sum += add; if (sum > max) sum = max; }
// return a random value in [0, 1(
inline double rnd(void) { return double(rand())/RAND_MAX; }
inline unsigned rnd(unsigned b) { return unsigned(b*rnd()); }

#endif
