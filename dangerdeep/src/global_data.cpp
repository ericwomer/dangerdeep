// global data
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "model.h"
#include "texture.h"
#include "font.h"
#include "datapath.h"
#include <SDL/SDL_image.h>

model *merchant_medium, *subVII, *subXXI, *destroyer_tribal, *troopship_medium, *sky,
	*battleship_malaya, *torpedo_g7, *depth_charge_mdl;
texture *water, *background, *titel[4], *periscope[4], *gauge1,
	*gauge2, *gauge3, *gauge4, *psbackgr,
	*addleadangle, *torpleft, *torp, *uzo, *metalbackgr,
	*torpt1, *torpt3, *torpt3fat, *torpt5, *torpt6lut, *torpt11;
font *font_arial, *font_arial2, *font_ellis, *font_logo, *font_panel, *font_tahoma;

void init_global_data(void)
{
	sky = new model(datafilename("skyhemisphere.mdl"));
	water = new texture(datafilename("water.png"), 1, false);
	background = new texture(datafilename("background.png"), 1, false);
	merchant_medium = new model(datafilename("merchant1.mdl"));
	troopship_medium = new model(datafilename("troopship1.mdl"));
	battleship_malaya = new model(datafilename("battleship_malaya.mdl"));
	subVII = new model(datafilename("subVII.mdl"));
	subXXI = new model(datafilename("subXXI.mdl"));
	destroyer_tribal = new model(datafilename("destroyer1.mdl"));
	font_arial = new font(datafilename("font_arial.png").c_str());
	font_arial2 = new font(datafilename("font_arial2.png").c_str());
	font_ellis = new font(datafilename("font_ellis.png").c_str());
	font_logo = new font(datafilename("font_logo.png").c_str(), 1, 8, "Dangerfomthp");
	font_panel = new font(datafilename("font_panel.png").c_str(), 0, 8, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ.0123456789");
	font_tahoma = new font(datafilename("font_tahoma.png").c_str());
	torpedo_g7 = new model(datafilename("torpedo.mdl"));
	depth_charge_mdl = new model(datafilename("depth_charge.mdl"));
	SDL_Surface* titelimg = IMG_Load(datafilename("titel.png").c_str());
	titel[0] = new texture(titelimg, 0, 0, 256, 256);
	titel[1] = new texture(titelimg, 256, 0, 256, 256);
	titel[2] = new texture(titelimg, 0, 256, 256, 128);
	titel[3] = new texture(titelimg, 256, 256, 256, 128);
	SDL_FreeSurface(titelimg);
	SDL_Surface* periscopeimg = IMG_Load(datafilename("periscope.png").c_str());
	periscope[0] = new texture(periscopeimg, 0, 0, 256, 256);
	periscope[1] = new texture(periscopeimg, 256, 0, 256, 256);
	periscope[2] = new texture(periscopeimg, 0, 256, 256, 256);
	periscope[3] = new texture(periscopeimg, 256, 256, 256, 256);
	SDL_FreeSurface(periscopeimg);
	gauge1 = new texture(datafilename("manometer1.png"));
	gauge2 = new texture(datafilename("manometer2.png"));
	gauge3 = new texture(datafilename("manometer3.png"));
	gauge4 = new texture(datafilename("manometer4.png"));
	psbackgr = new texture(datafilename("psbackgr.png"));
	addleadangle = new texture(datafilename("addleadangle.png"));
	torp = new texture(datafilename("torp.png"));
	uzo = new texture(datafilename("uzo.png"), 1, true, true);
	metalbackgr = new texture(datafilename("metalbackgr.png"), 1, false);
	torpt1 = new texture(datafilename("torpt1.png"));
	torpt3 = new texture(datafilename("torpt3.png"));
	torpt3fat = new texture(datafilename("torpt3fat.png"));
	torpt5 = new texture(datafilename("torpt5.png"));
	torpt6lut = new texture(datafilename("torpt6lut.png"));
	torpt11 = new texture(datafilename("torpt11.png"));
}

void deinit_global_data(void)
{
	delete sky;
	delete water;
	delete background;
	delete merchant_medium;
	delete troopship_medium;
	delete battleship_malaya;
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
	delete psbackgr;
	delete addleadangle;
	delete torp;
	delete uzo;
	delete metalbackgr;
	delete torpt1;
	delete torpt3;
	delete torpt3fat;
	delete torpt5;
	delete torpt6lut;
	delete torpt11;
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
	unsigned y = year-1939;
	unsigned m = (year-1939)*12 + month-1;
	unsigned d = 0;
	for (unsigned i = 0; i < m; ++i) d += month_lengths[i];
	d += day-1;
	return d*86400.0;
}
