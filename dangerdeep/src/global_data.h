// global data
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef GLOBAL_DATA_H
#define GLOBAL_DATA_H

#include <string>
using namespace std;

#define GRAVITY 9.806	// a very global constant

// global models, textures, fonts
extern class model *merchant_medium, *subVII, *subXXI, *destroyer_tribal, *troopship_medium,
	*battleship_malaya, *carrier_bogue, *torpedo_g7, *depth_charge_mdl, *gun_shell_mdl, *skyhemisphere;
extern class texture *water, *background, *titel[4], *periscope[4], *gauge1,
	*gauge2, *gauge3, *gauge4, *psbackgr, *panelbackgr,
	*addleadangle, *torpempty, *torpreload, *torpunload, *uzo, *metalbackgr,
	*torpt1, *torpt3, *torpt3fat, *torpt5, *torpt6lut, *torpt11, *clouds,
	*clock12, *clock24, *threesubs[4], *glasses, *torp_expl_water_splash[3];
extern class font *font_arial, *font_arial2, *font_ellis, *font_logo, *font_panel, *font_tahoma;
extern class sound *torpedo_launch_sound, *torpedo_detonation[2];

void init_global_data(void);
void deinit_global_data(void);

void get_date(double t, unsigned& year, unsigned& month, unsigned& day);
double get_time(unsigned year, unsigned month, unsigned day);
// computes part of day (0 night, 1 sunrise, 2 day, 3 sunfall)
// with fractional part as "position" in that part
double get_day_time(double t);

// handle modulo calculation for negative values the way I need it
inline float myfmod(float a, float b) { return a-floor(a/b)*b; }
// return a random value in [0, 1(
inline double rnd(void) { return double(rand())/RAND_MAX; }
inline unsigned rnd(unsigned b) { return unsigned(b*rnd()); }

inline string get_data_dir(void) { return DATADIR; }

#endif
