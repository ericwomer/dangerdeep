// global data
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "model.h"
#include "texture.h"
#include "font.h"
#include "global_data.h"
#include <SDL/SDL_image.h>

#define TEXTURE_DIR "textures/"
#define FONT_DIR "fonts/"
#define MODEL_DIR "models/"

model *merchant_medium, *subVII, *subXXI, *destroyer_tribal, *troopship_medium,
	*battleship_malaya, *carrier_bogue, *torpedo_g7, *depth_charge_mdl, *gun_shell_mdl,
	*skyhemisphere;
texture *water, *background, *titel[4], *periscope[4], *gauge1,
	*gauge2, *gauge3, *gauge4, *psbackgr, *panelbackgr,
	*addleadangle, *torpleft, *torpempty, *torpreload, *torpunload, *uzo, *metalbackgr,
	*torpt1, *torpt3, *torpt3fat, *torpt5, *torpt6lut, *torpt11, *clouds,
	*clock12, *clock24, *threesubs[4], *glasses, *torp_expl_water_splash;
font *font_arial, *font_arial2, *font_ellis, *font_logo, *font_panel, *font_tahoma;

void init_global_data(void)
{
	skyhemisphere = new model((get_data_dir() + MODEL_DIR + "skyhemisphere.mdl"));
	water = new texture((get_data_dir() + TEXTURE_DIR + "water.png"), 1, false);
	background = new texture((get_data_dir() + TEXTURE_DIR + "background.png"), 1, false);
	merchant_medium = new model((get_data_dir() + MODEL_DIR + "merchant1.mdl"));
	troopship_medium = new model((get_data_dir() + MODEL_DIR + "troopship1.mdl"));
	battleship_malaya = new model((get_data_dir() + MODEL_DIR + "battleship_malaya.mdl"));
	carrier_bogue = new model((get_data_dir() + MODEL_DIR + "carrier_bogue.mdl"));
	subVII = new model((get_data_dir() + MODEL_DIR + "subVII.mdl"));
	subXXI = new model((get_data_dir() + MODEL_DIR + "subXXI.mdl"));
	destroyer_tribal = new model((get_data_dir() + MODEL_DIR + "destroyer1.mdl"));
	font_arial = new font((get_data_dir() + FONT_DIR + "font_arial.png").c_str());
	font_arial2 = new font((get_data_dir() + FONT_DIR + "font_arial2.png").c_str());
	font_ellis = new font((get_data_dir() + FONT_DIR + "font_ellis.png").c_str());
	font_logo = new font((get_data_dir() + FONT_DIR + "font_logo.png").c_str(), 1, 8, "Dangerfomthp");
	font_panel = new font((get_data_dir() + FONT_DIR + "font_panel.png").c_str(), 0, 8);
	font_tahoma = new font((get_data_dir() + FONT_DIR + "font_tahoma.png").c_str());
	torpedo_g7 = new model((get_data_dir() + MODEL_DIR + "torpedo.mdl"));
	depth_charge_mdl = new model((get_data_dir() + MODEL_DIR + "depth_charge.mdl"));
	gun_shell_mdl = new model((get_data_dir() + MODEL_DIR + "gun_shell.mdl"));
	SDL_Surface* titelimg = IMG_Load((get_data_dir() + TEXTURE_DIR + "titel.png").c_str());
	titel[0] = new texture(titelimg, 0, 0, 256, 256);
	titel[1] = new texture(titelimg, 256, 0, 256, 256);
	titel[2] = new texture(titelimg, 0, 256, 256, 128);
	titel[3] = new texture(titelimg, 256, 256, 256, 128);
	SDL_FreeSurface(titelimg);
	SDL_Surface* periscopeimg = IMG_Load((get_data_dir() + TEXTURE_DIR + "periscope.png").c_str());
	periscope[0] = new texture(periscopeimg, 0, 0, 256, 256);
	periscope[1] = new texture(periscopeimg, 256, 0, 256, 256);
	periscope[2] = new texture(periscopeimg, 0, 256, 256, 256);
	periscope[3] = new texture(periscopeimg, 256, 256, 256, 256);
	SDL_FreeSurface(periscopeimg);
	gauge1 = new texture((get_data_dir() + TEXTURE_DIR + "gauge1.png"));
	gauge2 = new texture((get_data_dir() + TEXTURE_DIR + "gauge2.png"));
	gauge3 = new texture((get_data_dir() + TEXTURE_DIR + "gauge3.png"));
	gauge4 = new texture((get_data_dir() + TEXTURE_DIR + "gauge4.png"));
	psbackgr = new texture((get_data_dir() + TEXTURE_DIR + "psbackgr.png"));
	panelbackgr = new texture((get_data_dir() + TEXTURE_DIR + "panelbackgr.png"), 1, false);
	addleadangle = new texture((get_data_dir() + TEXTURE_DIR + "addleadangle.png"));
	torpempty = new texture((get_data_dir() + TEXTURE_DIR + "torpempty.png"));
	torpreload = new texture((get_data_dir() + TEXTURE_DIR + "torpreload.png"));
	torpunload = new texture((get_data_dir() + TEXTURE_DIR + "torpunload.png"));
	uzo = new texture((get_data_dir() + TEXTURE_DIR + "uzo.png"), 1, true, true);
	metalbackgr = new texture((get_data_dir() + TEXTURE_DIR + "metalbackgr.png"), 1, false);
	torpt1 = new texture((get_data_dir() + TEXTURE_DIR + "torpt1.png"));
	torpt3 = new texture((get_data_dir() + TEXTURE_DIR + "torpt3.png"));
	torpt3fat = new texture((get_data_dir() + TEXTURE_DIR + "torpt3fat.png"));
	torpt5 = new texture((get_data_dir() + TEXTURE_DIR + "torpt5.png"));
	torpt6lut = new texture((get_data_dir() + TEXTURE_DIR + "torpt6lut.png"));
	torpt11 = new texture((get_data_dir() + TEXTURE_DIR + "torpt11.png"));
	clouds = new texture((get_data_dir() + TEXTURE_DIR + "clouds.png"), 1, false, true);
	clock12 = new texture((get_data_dir() + TEXTURE_DIR + "clock12.png"));
	clock24 = new texture((get_data_dir() + TEXTURE_DIR + "clock24.png"));
	glasses = new texture((get_data_dir() + TEXTURE_DIR + "glasses.png"), 1, true, true);
	SDL_Surface* threesubsimg = IMG_Load((get_data_dir() + TEXTURE_DIR + "3subs.png").c_str());
	threesubs[0] = new texture(threesubsimg, 0, 0, 256, 256);
	threesubs[1] = new texture(threesubsimg, 256, 0, 256, 256);
	threesubs[2] = new texture(threesubsimg, 0, 256, 256, 128);
	threesubs[3] = new texture(threesubsimg, 256, 256, 256, 128);
	SDL_FreeSurface(threesubsimg);
	torp_expl_water_splash = new texture ((get_data_dir() + TEXTURE_DIR + "torpedo_expl_water_splash.png"), 1, true, true);
}

void deinit_global_data(void)
{
	delete skyhemisphere;
	delete water;
	delete background;
	delete merchant_medium;
	delete troopship_medium;
	delete battleship_malaya;
	delete carrier_bogue;
	delete subVII;
	delete subXXI;
	delete destroyer_tribal;
	delete font_arial;
	delete font_arial2;
	delete font_ellis;
	delete font_logo;
	delete font_panel;
	delete font_tahoma;
	delete torpedo_g7;
	delete depth_charge_mdl;
	delete gun_shell_mdl;
	delete titel[0];
	delete titel[1];
	delete titel[2];
	delete titel[3];
	delete periscope[0];
	delete periscope[1];
	delete periscope[2];
	delete periscope[3];
	delete gauge1;
	delete gauge2;
	delete gauge3;
	delete gauge4;
	delete glasses;
	delete psbackgr;
	delete panelbackgr;
	delete addleadangle;
	delete torpempty;
	delete torpreload;
	delete torpunload;
	delete uzo;
	delete metalbackgr;
	delete torpt1;
	delete torpt3;
	delete torpt3fat;
	delete torpt5;
	delete torpt6lut;
	delete torpt11;
	delete clouds;
	delete clock12;
	delete clock24;
	delete threesubs[0];
	delete threesubs[1];
	delete threesubs[2];
	delete threesubs[3];
	delete torp_expl_water_splash;
}

// returns 1939-1945, 1-12, 1-31
static unsigned month_lengths[7*12] = {
	31,28,31,30,31,30,31,31,30,31,30,31,	// 1939
	31,29,31,30,31,30,31,31,30,31,30,31,	// 1940
	31,28,31,30,31,30,31,31,30,31,30,31,	// 1941
	31,28,31,30,31,30,31,31,30,31,30,31,	// 1942
	31,28,31,30,31,30,31,31,30,31,30,31,	// 1943
	31,29,31,30,31,30,31,31,30,31,30,31,	// 1944
	31,28,31,30,31,30,31,31,30,31,30,31 };	// 1945
	
void get_date(double t, unsigned& year, unsigned& month, unsigned& day)
{
	unsigned secs = unsigned(t);
	unsigned days = secs/86400;
	unsigned i, daysum;
	for (i = 0, daysum = 0; i < 7*12; ++i) {
		if (daysum + month_lengths[i] > days) break;
		daysum += month_lengths[i];
	}
	day = days - daysum + 1;
	unsigned j, monthsum;
	for (j = 0, monthsum = 0; j < 7; ++j) {
		if (monthsum + 12 > i) break;
		monthsum += 12;
	}
	month = i - monthsum + 1;
	year = 1939 + j;
}

double get_time(unsigned year, unsigned month, unsigned day)
{
	// unsigned y = year-1939; Unused variable
	unsigned m = (year-1939)*12 + month-1;
	unsigned d = 0;
	for (unsigned i = 0; i < m; ++i) d += month_lengths[i];
	d += day-1;
	return d*86400.0;
}

double get_day_time(double t)
{
	// fixme: calculate sunrise and fall etc.
	double d = fmod(t, 86400);
	if (d < 5*3600) return 0 + (d+5*3600)/(10*3600);	// sunrise at 5am
	if (d < 6*3600) return 1 + (d-5*3600)/3600;	// day at 6am
	if (d < 18*3600) return 2 + (d-6*3600)/(12*3600);	// sundown at 6pm
	if (d < 19*3600) return 3 + (d-18*3600)/3600;	// night at 7pm
	return 0 + (d-19*3600)/(10*3600);
}
