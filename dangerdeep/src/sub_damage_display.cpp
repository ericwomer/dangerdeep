// Object to display the damage status of a submarine.
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include <string>
#include <iostream>
#include <sstream>
using namespace std;
#include "system.h"
#include "texts.h"
#include "global_data.h"
#include "user_display.h"
#include "sub_damage_display.h"
#include "image.h"

struct rect {
	int x, y, w, h;
	rect(int x1, int y1, int x2, int y2) : x(x1), y(y1), w(x2-x1), h(y2-y1) {};
};

static rect rect_data[] = {
	rect(108,115,128,143),	// rudder
	rect(150,121,164,146),	// screws
	rect(165,130,304,140),	// screw shaft
	rect(123,268,146,336),	// stern dive planes
	rect(0,0,0,0),	//       water pump
	rect(147,277,274,331),	//       pressure hull
	rect(275,290,300,312),	//       hatch
	rect(314,122,355,145),	// electric engines
	rect(0,0,0,0),	// air compressor
	rect(0,0,0,0),	// machine water pump
	rect(301,277,466,331),	//          pressure hull
	rect(557,123,628,145),	// aft battery
	rect(376,120,464,145),	// diesel engines
	rect(0,0,0,0),	// kitchen hatch
	rect(0,0,0,0),	// balance tank valves
	rect(645,123,721,145),	// forward battery
	rect(535,28,545,104),	// periscope
	rect(467,277,575,331),	// central pressure hull
	rect(0,0,0,0),	// bilge? water pump
	rect(517,50,532,62),	// conning tower hatch
	rect(0,0,0,0),	// listening device
	rect(0,0,0,0),	// radio device
	rect(808,103,825,132),	// inner bow tubes
	rect(905,103,944,132),	// outer
	rect(0,0,0,0),	// bow water pump
	rect(732,293,756,314),	//     hatch
	rect(576,277,731,331),	//     pressure hull
	rect(877,270,906,341),	//     dive planes
	rect(464,32,493,57),	// aa gun
	rect(458,66,495,80),	// ammo depot
	rect(323,261,673,277),	// outer fuel tanks (used left in image)

	rect(84,107,106,115),	// outer stern tubes
	rect(177,107,201,115),	// inner
	rect(0,0,0,0),	// snorkel
	rect(587,58,656,80),	// deck gun
	rect(0,0,0,0),	// radio detection device
};



sub_damage_display::sub_damage_display (submarine* s) : mysub(s)
{
}

sub_damage_display::~sub_damage_display ()
{
}

void sub_damage_display::display_popup (int x, int y, const string& text, bool atleft, bool atbottom) const
{
	int posx = atleft ? 100 : 604, posy = atbottom ? 480 : 30, width = 320, height = 140;
	glBindTexture ( GL_TEXTURE_2D, 0 );
	
	color::red().set_gl_color();
	glBegin(GL_LINES);
	glVertex2f(x, y);
	glVertex2f(posx+width/2, posy+height/2);
	glEnd();
	
	color(0,0,0).set_gl_color(128);
	system::sys()->draw_rectangle(posx+4,posy+4,width,height);
	color(255,255,128).set_gl_color(255);
	system::sys()->draw_rectangle(posx,posy,width,height);
	font_arial2->print(posx+4,posy+4,text.c_str(),color::black());
	color::white().set_gl_color();
}

void sub_damage_display::display ( class system& sys, class game& gm )
{
	int ydrawdiff = (damage_screen_background->get_height()-sub_damage_scheme_all->get_height())/2;
	damage_screen_background->draw(0, 0);
	sub_damage_scheme_all->draw(0, ydrawdiff);

	const vector<submarine::damage_status> damages = mysub->get_damage_status();
	for (unsigned i = 0; i < damages.size(); ++i) {
		rect r = rect_data[i];
			if (r.x == 0) continue;	// display test hack fixme
		int x = r.x + r.w/2 - 16, y = r.y + r.h/2 - 16 + ydrawdiff;
		texture* t = 0;
		switch (damages[i]) {
			case submarine::light: t = repairlight; break;
			case submarine::medium: t = repairmedium; break;
			case submarine::heavy: t = repairheavy; break;
			case submarine::critical: t = repaircritical; break;
			case submarine::wrecked: t = repairwrecked; break;
		}
		if (t)	sys.draw_image(x, y, 32, 32, t);
	}
	
	// fixme: clean up used textures of damage_screen_background & sub_damage_scheme_all
}

void sub_damage_display::check_key ( int keycode, class system& sys, class game& gm )
{
}

void sub_damage_display::check_mouse ( int x, int y, int mb )
{
	if (!mysub) return;
	// fixme
	const vector<submarine::damage_status> damages = mysub->get_damage_status();
	for (unsigned i = 0; i < damages.size(); ++i) {
		rect r = rect_data[i];
		r.y += (damage_screen_background->get_height()-sub_damage_scheme_all->get_height())/2;
		if (x >= r.x && x <= r.x+r.w && y >= r.y && y <= r.y+r.h) {
			// it is important, that texts are in correct order starting with 400.
			bool atleft = (r.x+r.w/2) < 1024/2;
			bool atbottom = (r.y+r.h/2) >= 768/2;
			display_popup(r.x+r.w/2, r.y+r.h/2, texts::get(400+i), atleft, atbottom);
		}
	}
}
