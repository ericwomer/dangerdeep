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
	//fixme out of date
	rect(105,116,25,10),	// fixme: measure directly in the program.
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



sub_damage_display::sub_damage_display (submarine* s) : mysub(s)  // fixme: give sub type
{
}

sub_damage_display::~sub_damage_display ()
{
}

void sub_damage_display::display_popup (int x, int y, const string& text, bool atbottom) const
{
	int posx = 100, posy = atbottom ? 480 : 30, width = 320, height = 140;
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
	if (!mysub) return;
	// fixme
	const vector<submarine::damage_status> damages = mysub->get_damage_status();
	for (unsigned i = 0; i < damages.size(); ++i) {
		rect r = rect_data[i];
		r.y += (damage_screen_background->h-sub_damage_scheme_all->h)/2;
		if (x >= r.x && x <= r.x+r.w && y >= r.y && y <= r.y+r.h) {
			// it is important, that texts are in correct order starting with 103.fixme
			bool atbottom = (r.y+r.h/2) >= 768/2;
			display_popup(r.x+r.w/2, r.y+r.h/2, texts::get(103+i), atbottom);
		}
	}
}
