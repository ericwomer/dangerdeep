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

struct rect {
	int x, y, w, h;
	// values measured with gimp
	rect(int xr, int yb, int wi, int he) : x(xr-wi), y(yb-he), w(wi), h(he) {};
};

static rect rect_data[] = {
	rect(105,116,25,10),
	rect(129,144,22,30),
	rect(169,148,20,28),
	rect(286,141,85,10),
	rect(148,336,26,68),
	rect(201,116,46,10),
	rect(0,0,0,0),
	rect(0,0,0,0),
	rect(300,313,24,22),
	rect(362,141,56,24),
	rect(0,0,0,0),
	rect(0,0,0,0),
	rect(0,0,0,0),
	rect(628,144,72,24),
	rect(467,141,95,24),
	rect(0,0,0,0),
	rect(0,0,0,0),
	rect(0,0,0,0),
	rect(720,144,76,24),
	rect(545,106,10,80),
	rect(0,0,0,0),
	rect(0,0,0,0),
	rect(0,0,0,0),
	rect(0,0,0,0),
	rect(0,0,0,0),
	rect(0,0,0,0),
	rect(0,0,0,0),
	rect(0,0,0,0),
	rect(0,0,0,0),
	rect(0,0,0,0),
	rect(0,0,0,0),
	rect(0,0,0,0),
	rect(0,0,0,0),
	rect(0,0,0,0),
	rect(0,0,0,0),
	rect(0,0,0,0)
};



sub_damage_display::sub_damage_display ()  // fixme: give sub type
{
	unsigned s = unsigned(nr_of_damageable_parts);
	all_parts.resize(s, none);
}

sub_damage_display::~sub_damage_display ()
{
}

void sub_damage_display::add_damage ( const vector3& fromwhere, float amount )
{
}

void sub_damage_display::display_popup (int x, int y, const string& text) const
{
	glBindTexture ( GL_TEXTURE_2D, 0 );
	color(255,255,128).set_gl_color();
	system::sys()->draw_rectangle(x-32,y-32,64,64);	//fixme
	font_arial->print(x-30,y-30,text.c_str(),color::black());
	color::white().set_gl_color();
}

void sub_damage_display::display ( class system& sys, class game& gm )
{
	// the source image is 8bpp, paletted. It is transformed to
	// 16bpp plain on the fly by SDL, that is slow! fixme
	// This could be done by copying it once to the auxiliary buffer.
	// Maybe we don't need OpenGL auxiliary buffers, if we use a
	// SDL Hardware surface instead.
	SDL_BlitSurface(damage_screen_background, 0, SDL_GetVideoSurface(), 0);
	SDL_Rect dstr;
	dstr.x = 0;
	dstr.y = (damage_screen_background->h-sub_damage_scheme_all->h)/2;
	SDL_BlitSurface(sub_damage_scheme_all, 0, SDL_GetVideoSurface(), &dstr);
	SDL_UpdateRect(SDL_GetVideoSurface(), 0, 0, 1024, 640);
}

void sub_damage_display::check_key ( int keycode, class system& sys, class game& gm )
{
}

void sub_damage_display::check_mouse ( int x, int y, int mb )
{
	// fixme
	unsigned s = unsigned(nr_of_damageable_parts);
	for (unsigned i = 0; i < s; ++i) {
		rect r = rect_data[i];
		r.y += (damage_screen_background->h-sub_damage_scheme_all->h)/2;
		if (x >= r.x && x <= r.x+r.w && y >= r.y && y <= r.y+r.h) {
			ostringstream os;
			os << "Part No." << i ;	//fixme
			display_popup(r.x, r.y, os.str());
//			display_popup(x, y, os.str());
		}
	}
}
