// global data
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef GLOBAL_DATA_H
#define GLOBAL_DATA_H

#include <string>
#include <SDL/SDL.h>
using namespace std;
#include "date.h"

#define GRAVITY 9.806	// a very global constant

// global models, textures, fonts
extern class model *merchant_medium, *subVII, *subXXI, *destroyer_tribal, *troopship_medium,
	*battleship_malaya, *carrier_bogue, *torpedo_g7, *depth_charge_mdl, *gun_shell_mdl, *skyhemisphere;
extern class texture *water, *background, *gauge1,
	*gauge2, *gauge3, *gauge4, *gauge5, *psbackgr, *panelbackgr,
	*addleadangle, *torpempty, *torpreload, *torpunload, *uzo, *metalbackgr,
	*torpt1, *torpt3, *torpt3fat, *torpt5, *torpt6lut, *torpt11, *clouds,
	*clock12, *clock24, *glasses, *torp_expl_water_splash[3],
	*woodbackgr,
	*repairlight, *repairmedium, *repairheavy, *repaircritical, *repairwrecked;
extern class font *font_arial, *font_arial2, *font_ellis, *font_panel, *font_tahoma,
	*font_nimbusrom;
extern class sound *torpedo_launch_sound, *torpedo_detonation_submerged[2],
	*torpedo_detonation_surfaced[2];
extern class image *titlebackgr, *periscope, *threesubsimg, *damage_screen_background,
	*sub_damage_scheme_all, *logbook_spiral, *killedimg, *scopewatcherimg,
	*depthchargeimg;

void init_global_data(void);
void deinit_global_data(void);

void get_date(double t, unsigned& year, unsigned& month, unsigned& day);
void get_date ( double t, unsigned& year, unsigned& month, unsigned& day,
	unsigned& hour, unsigned& minute, unsigned& second );
void get_date ( double t, date& d );
double get_time(unsigned year, unsigned month, unsigned day);
// computes part of day (0 night, 1 sunrise, 2 day, 3 sunfall)
// with fractional part as "position" in that part
double get_day_time(double t);

// handle modulo calculation for negative values the way I need it
inline float myfmod(float a, float b) { return a-floor(a/b)*b; }
inline float myfrac(float a) { return a-floor(a); }
// return a random value in [0, 1(
inline double rnd(void) { return double(rand())/RAND_MAX; }
inline unsigned rnd(unsigned b) { return unsigned(b*rnd()); }

inline string get_data_dir(void) { return DATADIR; }

#endif
