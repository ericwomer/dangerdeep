// global data
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "model.h"
#include "texture.h"
#include "image.h"
#include "font.h"
#include "global_data.h"
#include "sound.h"
#include <sstream>
#include "oglext/OglExt.h"
#include "tinyxml/tinyxml.h"
#include <SDL_image.h>



// fixme: this could be replaced with an array of pointers using enum-names
// as indices. This would simplify destruction and possibly construction.

model	*torpedo_g7, *depth_charge_mdl, *gun_shell_mdl, *conning_tower_typeVII;

texture *background, *gauge1,
	*gauge2, *gauge3, *gauge4, *gauge5, *psbackgr, *panelbackgr,
	*addleadangle, *torpleft, *torpempty, *torpreload, *torpunload, *uzo, *metalbackgr,
	*torpt1, *torpt2, *torpt3, *torpt3a, *torpt4, *torpt5, *torpt11, *torpt1fat, *torpt3fat, *torpt6lut,
	*clock12, *clock24, *glasses, *torp_expl_water_splash[3],
	*woodbackgr, *smoke, *notepadsheet, *menuframe, *turnswitch, *turnswitchbackgr,
	*repairlight, *repairmedium, *repairheavy, *repaircritical, *repairwrecked,
	*terraintex, *cloudsbackgr, *atlanticmap;
	
font *font_arial, *font_panel, *font_nimbusrom;

sound *torpedo_launch_sound, *torpedo_detonation_submerged[2],
	*torpedo_detonation_surfaced[2];

image *titlebackgrimg, *periscope, *threesubsimg, *damage_screen_background,
	*sub_damage_scheme_all, *logbook_spiral, *killedimg, *scopewatcherimg,
	*depthchargeimg, *sunkendestroyerimg, *kruppdocksimg, *rescuedestroyerimg, *sunderlandimg,
	*swordfishimg, *hedgehogimg, *panelbackgroundimg;

objcachet<class model> modelcache(get_model_dir());
objcachet<class image> imagecache(get_image_dir());
objcachet<class texture> texturecache(get_texture_dir());

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

void init_global_data(void)
{
	background = new texture(get_texture_dir() + "background.png", GL_LINEAR);
	conning_tower_typeVII = new model(get_model_dir() + "conning_tower_typeVIIc.3ds");
	font_arial = new font(get_font_dir() + "font_arial.png");
	font_panel = new font(get_font_dir() + "font_panel.png");
	font_nimbusrom = new font(get_font_dir() + "font_nimbusrom.png");
	torpedo_g7 = new model(get_model_dir() + "torpedo_g7.3ds");
	depth_charge_mdl = new model(get_model_dir() + "depth_charge.3ds");
	gun_shell_mdl = new model(get_model_dir() + "gun_shell.3ds");
	gauge1 = new texture(get_texture_dir() + "gauge1.png");
	gauge2 = new texture(get_texture_dir() + "gauge2.png");
	gauge3 = new texture(get_texture_dir() + "gauge3.png");
	gauge4 = new texture(get_texture_dir() + "gauge4.png");
	gauge5 = new texture(get_texture_dir() + "gauge5.png");
	psbackgr = new texture(get_texture_dir() + "psbackgr.png");
	panelbackgr = new texture(get_texture_dir() + "panelbackgr.png", GL_LINEAR);
	addleadangle = new texture(get_texture_dir() + "addleadangle.png");
	torpempty = new texture(get_texture_dir() + "torpempty.png");
	torpreload = new texture(get_texture_dir() + "torpreload.png");
	torpunload = new texture(get_texture_dir() + "torpunload.png");
	uzo = new texture(get_texture_dir() + "uzo.png", GL_LINEAR, GL_CLAMP_TO_EDGE);
	metalbackgr = new texture(get_texture_dir() + "metalbackgr.png", GL_LINEAR);
	torpt1 = new texture(get_texture_dir() + "torpt1.png");
	torpt2 = new texture(get_texture_dir() + "torpt2.png");
	torpt3 = new texture(get_texture_dir() + "torpt3.png");
	torpt3a = new texture(get_texture_dir() + "torpt3a.png");
	torpt4 = new texture(get_texture_dir() + "torpt4.png");
	torpt5 = new texture(get_texture_dir() + "torpt5.png");
	torpt11 = new texture(get_texture_dir() + "torpt11.png");
	torpt1fat = new texture(get_texture_dir() + "torpt1fat.png");
	torpt3fat = new texture(get_texture_dir() + "torpt3fat.png");
	torpt6lut = new texture(get_texture_dir() + "torpt6lut.png");
	clock12 = new texture(get_texture_dir() + "clock12.png");
	clock24 = new texture(get_texture_dir() + "clock24.png");
	glasses = new texture(get_texture_dir() + "glasses.png", GL_LINEAR, GL_CLAMP_TO_EDGE);
	torp_expl_water_splash[0] = new texture(get_texture_dir() + "torpedo_expl_water_splash.png" , GL_LINEAR);
	torp_expl_water_splash[1] = new texture(get_texture_dir() + "torpedo_expl_water_splash_1.png" , GL_LINEAR);
	torp_expl_water_splash[2] = new texture(get_texture_dir() + "torpedo_expl_water_splash_2.png" , GL_LINEAR);
	torpedo_launch_sound = new sound(get_sound_dir() + "torpedo_launch.wav" );
	torpedo_detonation_submerged[0] = new sound(get_sound_dir() + "torpedo_detonation_submerged_1.wav" );
	torpedo_detonation_submerged[1] = new sound(get_sound_dir() + "torpedo_detonation_submerged_2.wav" );
	torpedo_detonation_surfaced[0] = new sound(get_sound_dir() + "torpedo_detonation_surfaced_1.wav" );
	torpedo_detonation_surfaced[1] = new sound(get_sound_dir() + "torpedo_detonation_surfaced_2.wav" );
	woodbackgr = new texture(get_texture_dir() + "wooden_desk.png" );
	smoke = new texture(get_texture_dir() + "smoke.png" , GL_LINEAR, GL_CLAMP_TO_EDGE);
	notepadsheet = new texture(get_texture_dir() + "notepadsheet.png" /*, GL_CLAMP_TO_EDGE*/);
	menuframe = new texture(get_texture_dir() + "menuframe.png" );
	turnswitch = new texture(get_texture_dir() + "turnswitch.png" );
	turnswitchbackgr = new texture(get_texture_dir() + "turnswitchbackgr.png" );
	repairlight = new texture(get_texture_dir() + "repairlight.png" );
	repairmedium = new texture(get_texture_dir() + "repairmedium.png" );
	repairheavy = new texture(get_texture_dir() + "repairheavy.png" );
	repaircritical = new texture(get_texture_dir() + "repaircritical.png" );
	repairwrecked = new texture(get_texture_dir() + "repairwrecked.png" );
	terraintex = new texture(get_texture_dir() + "terrain2.jpg", GL_LINEAR);
	cloudsbackgr = new texture(get_texture_dir() + "cloudsbackgr.jpg" );
	atlanticmap = new texture(get_texture_dir() + "atlanticmap.jpg", GL_LINEAR, GL_CLAMP_TO_EDGE);

	titlebackgrimg = new image(get_image_dir() + "titlebackgr.jpg");
	periscope = new image(get_texture_dir() + "periscope.png", true);
	threesubsimg = new image(get_image_dir() + "threesubs.jpg");
	logbook_spiral = new image(get_texture_dir() + "logbook_spiral.png", true);
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
	panelbackgroundimg = new image(get_image_dir() + "panelbackground.jpg", true);
}

void deinit_global_data(void)
{
	delete background;
	delete conning_tower_typeVII;
	delete font_arial;
	delete font_panel;
	delete font_nimbusrom;
	delete torpedo_g7;
	delete depth_charge_mdl;
	delete gun_shell_mdl;
	delete gauge1;
	delete gauge2;
	delete gauge3;
	delete gauge4;
	delete gauge5;
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
	delete clock12;
	delete clock24;
	delete glasses;
	delete torp_expl_water_splash[0];
	delete torp_expl_water_splash[1];
	delete torp_expl_water_splash[2];
	delete torpedo_launch_sound;
	delete torpedo_detonation_submerged[0];
	delete torpedo_detonation_submerged[1];
	delete torpedo_detonation_surfaced[0];
	delete torpedo_detonation_surfaced[1];
	delete woodbackgr;
	delete smoke;
	delete notepadsheet;
	delete menuframe;
	delete turnswitch;
	delete turnswitchbackgr;
	delete repairlight;
	delete repairmedium;
	delete repairheavy;
	delete repaircritical;
	delete repairwrecked;
	delete terraintex;
	delete cloudsbackgr;
	delete atlanticmap;

	delete titlebackgrimg;
	delete periscope;
	delete threesubsimg;
	delete logbook_spiral;
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
