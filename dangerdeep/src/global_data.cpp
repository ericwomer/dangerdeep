// global data
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "model.h"
#include "texture.h"
#include "image.h"
#include "font.h"
#include "global_data.h"
#include "sound.h"
#include <sstream>
#ifdef WIN32
#include <SDL_image.h>
#else
#include <SDL/SDL_image.h>
#endif

#define TEXTURE_DIR "textures/"
#define FONT_DIR "fonts/"
#define MODEL_DIR "models/"
#define SOUND_DIR "sounds/"
#define IMAGES_DIR "images/"

// fixme: this could be replaced with an array of pointers using enum-names
// as indices. This would simplify destruction and possibly construction.

model	*merchant_large, *merchant_medium, *merchant_small,
	*subVII, *subXXI, *destroyer_tribal, *troopship_medium,
	*battleship_malaya, *carrier_bogue, *torpedo_g7, *depth_charge_mdl, *gun_shell_mdl,
	*skyhemisphere, *corvette_mdl, *freighter_medium, *freighter_large,
	*tanker_small,
	*conning_tower_typeVII;

texture *water, *background, *gauge1,
	*gauge2, *gauge3, *gauge4, *gauge5, *psbackgr, *panelbackgr,
	*addleadangle, *torpleft, *torpempty, *torpreload, *torpunload, *uzo, *metalbackgr,
	*torpt1, *torpt2, *torpt3, *torpt3a, *torpt4, *torpt5, *torpt11, *torpt1fat, *torpt3fat, *torpt6lut,
	*cloud_textures[NR_CLOUD_TEXTURES],
	*clock12, *clock24, *glasses, *torp_expl_water_splash[3],
	*woodbackgr, *smoke, *notepadsheet, *menuframe, *turnswitch, *turnswitchbackgr,
	*repairlight, *repairmedium, *repairheavy, *repaircritical, *repairwrecked;
	
font *font_arial, *font_arial2, *font_ellis, *font_panel, *font_tahoma,
	*font_nimbusrom;

sound *torpedo_launch_sound, *torpedo_detonation_submerged[2],
	*torpedo_detonation_surfaced[2];

image *titlebackgr, *periscope, *threesubsimg, *damage_screen_background, *sub_damage_scheme_all,
	*logbook_spiral, *killedimg, *scopewatcherimg, *depthchargeimg;

void init_global_data(void)
{
	skyhemisphere = new model((get_data_dir() + MODEL_DIR + "skyhemisphere.mdl"));
	water = new texture((get_data_dir() + TEXTURE_DIR + "water.png"), 1, false);
	background = new texture((get_data_dir() + TEXTURE_DIR + "background.png"), 1, false);
	merchant_large = new model((get_data_dir() + MODEL_DIR + "largemerchant.mdl"));
	merchant_medium = new model((get_data_dir() + MODEL_DIR + "mediummerchant.mdl"));
	merchant_small = new model((get_data_dir() + MODEL_DIR + "smallmerchant.mdl"));
	troopship_medium = new model((get_data_dir() + MODEL_DIR + "troopship1.mdl"));
	battleship_malaya = new model((get_data_dir() + MODEL_DIR + "battleship_malaya.mdl"));
	carrier_bogue = new model((get_data_dir() + MODEL_DIR + "carrier_bogue.mdl"));
	subVII = new model((get_data_dir() + MODEL_DIR + "subVII.mdl"));
	subXXI = new model((get_data_dir() + MODEL_DIR + "subXXI.mdl"));
	destroyer_tribal = new model((get_data_dir() + MODEL_DIR + "destroyer1.mdl"));
	corvette_mdl = new model((get_data_dir() + MODEL_DIR + "corvette.mdl"));
	freighter_large = new model((get_data_dir() + MODEL_DIR + "largefreighter.mdl"));
	freighter_medium = new model((get_data_dir() + MODEL_DIR + "mediumfreighter.mdl"));
	tanker_small = new model((get_data_dir() + MODEL_DIR + "smalltanker.mdl"));
	conning_tower_typeVII = new model((get_data_dir() + MODEL_DIR + "conning_tower_typeVII.mdl"));
	font_arial = new font(get_data_dir() + FONT_DIR + "font_arial.png");
	font_arial2 = new font(get_data_dir() + FONT_DIR + "font_arial2.png");
	font_ellis = new font(get_data_dir() + FONT_DIR + "font_ellis.png");
	font_panel = new font(get_data_dir() + FONT_DIR + "font_panel.png");
	font_tahoma = new font(get_data_dir() + FONT_DIR + "font_tahoma.png");
	font_nimbusrom = new font(get_data_dir() + FONT_DIR + "font_nimbusrom.png");
	torpedo_g7 = new model((get_data_dir() + MODEL_DIR + "torpedo.mdl"));
	depth_charge_mdl = new model((get_data_dir() + MODEL_DIR + "depth_charge.mdl"));
	gun_shell_mdl = new model((get_data_dir() + MODEL_DIR + "gun_shell.mdl"));
	titlebackgr = new image((get_data_dir() + IMAGES_DIR + "titlebackgr.jpg"));
	periscope = new image((get_data_dir() + TEXTURE_DIR + "periscope.png"));
	gauge1 = new texture((get_data_dir() + TEXTURE_DIR + "gauge1.png"));
	gauge2 = new texture((get_data_dir() + TEXTURE_DIR + "gauge2.png"));
	gauge3 = new texture((get_data_dir() + TEXTURE_DIR + "gauge3.png"));
	gauge4 = new texture((get_data_dir() + TEXTURE_DIR + "gauge4.png"));
	gauge5 = new texture((get_data_dir() + TEXTURE_DIR + "gauge5.png"));
	psbackgr = new texture((get_data_dir() + TEXTURE_DIR + "psbackgr.png"));
	panelbackgr = new texture((get_data_dir() + TEXTURE_DIR + "panelbackgr.png"), 1, false, true);
	addleadangle = new texture((get_data_dir() + TEXTURE_DIR + "addleadangle.png"));
	torpempty = new texture((get_data_dir() + TEXTURE_DIR + "torpempty.png"));
	torpreload = new texture((get_data_dir() + TEXTURE_DIR + "torpreload.png"));
	torpunload = new texture((get_data_dir() + TEXTURE_DIR + "torpunload.png"));
	uzo = new texture((get_data_dir() + TEXTURE_DIR + "uzo.png"), 1, true, true);
	metalbackgr = new texture((get_data_dir() + TEXTURE_DIR + "metalbackgr.png"), 1, false);
	torpt1 = new texture((get_data_dir() + TEXTURE_DIR + "torpt1.png"));
	torpt2 = new texture((get_data_dir() + TEXTURE_DIR + "torpt2.png"));
	torpt3 = new texture((get_data_dir() + TEXTURE_DIR + "torpt3.png"));
	torpt3a = new texture((get_data_dir() + TEXTURE_DIR + "torpt3a.png"));
	torpt4 = new texture((get_data_dir() + TEXTURE_DIR + "torpt4.png"));
	torpt5 = new texture((get_data_dir() + TEXTURE_DIR + "torpt5.png"));
	torpt11 = new texture((get_data_dir() + TEXTURE_DIR + "torpt11.png"));
	torpt1fat = new texture((get_data_dir() + TEXTURE_DIR + "torpt1fat.png"));
	torpt3fat = new texture((get_data_dir() + TEXTURE_DIR + "torpt3fat.png"));
	torpt6lut = new texture((get_data_dir() + TEXTURE_DIR + "torpt6lut.png"));
	for (unsigned i = 0; i < NR_CLOUD_TEXTURES; ++i) {
		ostringstream filename;
		filename << get_data_dir() << TEXTURE_DIR << "clouds" << (i+1) << ".png";
		cloud_textures[i] = new texture(filename.str(), 1, false, true);
	}
	clock12 = new texture((get_data_dir() + TEXTURE_DIR + "clock12.png"));
	clock24 = new texture((get_data_dir() + TEXTURE_DIR + "clock24.png"));
	glasses = new texture((get_data_dir() + TEXTURE_DIR + "glasses.png"), 1, true, true);
	threesubsimg = new image((get_data_dir() + IMAGES_DIR + "threesubs.jpg"));
	torp_expl_water_splash[0] = new texture ( ( get_data_dir () + TEXTURE_DIR + "torpedo_expl_water_splash.png" ), 1 );
	torp_expl_water_splash[1] = new texture ( ( get_data_dir () + TEXTURE_DIR + "torpedo_expl_water_splash_1.png" ), 1 );
	torp_expl_water_splash[2] = new texture ( ( get_data_dir () + TEXTURE_DIR + "torpedo_expl_water_splash_2.png" ), 1 );
	torpedo_launch_sound = new sound ( ( get_data_dir () + SOUND_DIR + "torpedo_launch.wav" ) );
	torpedo_detonation_submerged[0] = new sound ( ( get_data_dir () + SOUND_DIR + "torpedo_detonation_submerged_1.wav" ) );
	torpedo_detonation_submerged[1] = new sound ( ( get_data_dir () + SOUND_DIR + "torpedo_detonation_submerged_2.wav" ) );
	torpedo_detonation_surfaced[0] = new sound ( ( get_data_dir () + SOUND_DIR + "torpedo_detonation_surfaced_1.wav" ) );
	torpedo_detonation_surfaced[1] = new sound ( ( get_data_dir () + SOUND_DIR + "torpedo_detonation_surfaced_2.wav" ) );
	logbook_spiral = new image((get_data_dir () + TEXTURE_DIR + "logbook_spiral.png"));
	woodbackgr = new texture ( ( get_data_dir () + TEXTURE_DIR + "wooden_desk.png" ));
	smoke = new texture ( ( get_data_dir () + TEXTURE_DIR + "smoke.png" ), 1, true, true);
	notepadsheet = new texture ( ( get_data_dir () + TEXTURE_DIR + "notepadsheet.png" ), 0, true, true);
	menuframe = new texture ( ( get_data_dir () + TEXTURE_DIR + "menuframe.png" ), 0, true);
	turnswitch = new texture ( ( get_data_dir () + TEXTURE_DIR + "turnswitch.png" ), 0, true, true);
	turnswitchbackgr = new texture ( ( get_data_dir () + TEXTURE_DIR + "turnswitchbackgr.png" ), 0, true);
	damage_screen_background = new image((get_data_dir() + IMAGES_DIR + "damage_screen_backg.jpg"), 0, true, true, true);
	sub_damage_scheme_all = new image((get_data_dir() + IMAGES_DIR + "sub_damage_scheme_all.png"), 0, true, true, true);
	repairlight = new texture( ( get_data_dir () + TEXTURE_DIR + "repairlight.png" ), 0, true, true );
	repairmedium = new texture( ( get_data_dir () + TEXTURE_DIR + "repairmedium.png" ), 0, true, true );
	repairheavy = new texture( ( get_data_dir () + TEXTURE_DIR + "repairheavy.png" ), 0, true, true );
	repaircritical = new texture( ( get_data_dir () + TEXTURE_DIR + "repaircritical.png" ), 0, true, true );
	repairwrecked = new texture( ( get_data_dir () + TEXTURE_DIR + "repairwrecked.png" ), 0, true, true );
	killedimg = new image((get_data_dir() + IMAGES_DIR + "killed.jpg"), 0, true, true, true);
	scopewatcherimg = new image((get_data_dir() + IMAGES_DIR + "scopewatcher.jpg"), 0, true, true, true);
	depthchargeimg = new image((get_data_dir() + IMAGES_DIR + "depthcharge.jpg"), 0, true, true, true);
}

void deinit_global_data(void)
{
	delete skyhemisphere;
	delete water;
	delete background;
	delete merchant_large;
	delete merchant_medium;
	delete merchant_small;
	delete troopship_medium;
	delete battleship_malaya;
	delete carrier_bogue;
	delete subVII;
	delete subXXI;
	delete destroyer_tribal;
	delete corvette_mdl;
	delete tanker_small;
	delete conning_tower_typeVII;
	delete font_arial;
	delete font_arial2;
	delete font_ellis;
	delete font_panel;
	delete font_tahoma;
	delete font_nimbusrom;
	delete torpedo_g7;
	delete depth_charge_mdl;
	delete gun_shell_mdl;
	delete titlebackgr;
	delete periscope;
	delete gauge1;
	delete gauge2;
	delete gauge3;
	delete gauge4;
	delete gauge5;
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
	delete torpt2;
	delete torpt3;
	delete torpt3a;
	delete torpt4;
	delete torpt5;
	delete torpt11;
	delete torpt1fat;
	delete torpt3fat;
	delete torpt6lut;
	for (unsigned i = 0; i < NR_CLOUD_TEXTURES; ++i)
		delete cloud_textures[i];
	delete clock12;
	delete clock24;
	delete threesubsimg;
	delete torp_expl_water_splash[0];
	delete torp_expl_water_splash[1];
	delete torp_expl_water_splash[2];
	delete torpedo_launch_sound;
	delete torpedo_detonation_submerged[0];
	delete torpedo_detonation_submerged[1];
	delete torpedo_detonation_surfaced[0];
	delete torpedo_detonation_surfaced[1];
	delete logbook_spiral;
	delete woodbackgr;
	delete smoke;
	delete notepadsheet;
	delete menuframe;
	delete turnswitch;
	delete turnswitchbackgr;
	delete damage_screen_background;
	delete sub_damage_scheme_all;
	delete repairlight;
	delete repairmedium;
	delete repairheavy;
	delete repaircritical;
	delete repairwrecked;
	delete killedimg;
	delete scopewatcherimg;
	delete depthchargeimg;
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

void get_date ( double t, unsigned& year, unsigned& month, unsigned& day,
	unsigned& hour, unsigned& minute, unsigned& second )
{
	get_date ( t, year, month, day );

	double sd = fmod ( t, 86400 );
	hour   = unsigned ( sd / 3600.0f );
	sd -= hour * 3600.0f;
	minute = unsigned ( sd / 60.0f );
	sd -= minute * 60.0f;
	second = unsigned ( sd );
}

void get_date ( double t, date& d )
{
	unsigned year, month, day, hour, minute, second;
	get_date ( t, year, month, day, hour, minute, second );

	d = date ( year, month, day, hour, minute, second );
}
