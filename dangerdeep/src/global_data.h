// global data
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef GLOBAL_DATA_H
#define GLOBAL_DATA_H

#include "global.h"
#include <vector>
using namespace std;

// global models, textures, fonts
extern class model *merchant_medium, *subVII, *subXXI, *destroyer_tribal, *troopship_medium, *sky,
	*battleship_malaya, *torpedo_g7, *depth_charge_mdl;
extern class texture *water, *background, *titel[4], *periscope[4], *manometer1,
	*manometer2, *manometer3, *manometer4, *psbackgr,
	*addleadangle, *torp, *uzo, *metalbackgr,
	*torpt1, *torpt3, *torpt3fat, *torpt5, *torpt6lut, *torpt11;
extern class font *font_arial, *font_arial2, *font_ellis, *font_logo, *font_panel, *font_tahoma;

void init_global_data(void);
void deinit_global_data(void);

void get_date(double t, unsigned& year, unsigned& month, unsigned& day);
double get_time(unsigned year, unsigned month, unsigned day);

#endif
