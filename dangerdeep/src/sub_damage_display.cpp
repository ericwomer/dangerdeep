/*
Danger from the Deep - Open source submarine simulation
Copyright (C) 2003-2006  Thorsten Jordan, Luis Barrancos and others.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

// Object to display the damage status of a submarine.
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include <string>
#include <iostream>
#include <sstream>
using namespace std;
#include "system.h"
#include "texts.h"
#include "font.h"
#include "global_data.h"
#include "user_display.h"
#include "user_interface.h"
#include "sub_damage_display.h"
#include "image.h"
#include "game.h"
#include "primitives.h"

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
	rect(323,261,673,277),	// outer fuel tanks left
	rect(323,330,673,347),	// outer fuel tanks right

	rect(84,107,106,115),	// outer stern tubes
	rect(177,107,201,115),	// inner
	rect(0,0,0,0),	// snorkel
	rect(587,58,656,80),	// deck gun
	rect(0,0,0,0),	// radio detection device
	rect(0,0,0,0),	// radar
};



sub_damage_display::sub_damage_display (user_interface& ui_) :
	user_display(ui_),
	mx(0), my(0),
	notepadsheet(texturecache(), "notepadsheet.png")
{
}



void sub_damage_display::display_popup (int x, int y, const string& text, bool atleft, bool atbottom) const
{
	int posx = atleft ? 100 : 604, posy = atbottom ? 480 : 30, width = 320, height = 140;

	primitives::line(vector2f(x, y), vector2f(posx+width/2, posy+height/2), color::red()).render();

	notepadsheet.get()->draw(posx, posy);
	font_vtremington12->print_wrapped(posx+8, posy+45, 256-16, 20, text, color(0,0,128));
}

void sub_damage_display::display ( class game& gm ) const
{
	sys().prepare_2d_drawing();

	int ydrawdiff = (damage_screen_background->get_height()-sub_damage_scheme_all->get_height())/2;
	glColor4f(1,1,1,1);
	damage_screen_background->draw(0, 0);
	sub_damage_scheme_all->draw(0, ydrawdiff);

	submarine* mysub = dynamic_cast<submarine*>(gm.get_player());

	const vector<submarine::part>& parts = mysub->get_damage_status();
	for (unsigned i = 0; i < parts.size(); ++i) {
		rect r = rect_data[i];
			if (r.x == 0) continue;	// display test hack fixme
		int x = r.x + r.w/2 - 16, y = r.y + r.h/2 - 16 + ydrawdiff;
		if (parts[i].status > 0.0) {
			const texture* t = 0;
			if (parts[i].status <= 0.25) t = repairlight.get();
			else if (parts[i].status <= 0.50) t = repairmedium.get();
			else if (parts[i].status <= 0.75) t = repairheavy.get();
			else if (parts[i].status < 1.00) t = repaircritical.get();
			else t = repairwrecked.get();
			t->draw(x, y, 32, 32);
		}
	}
	
	// draw popup if mouse is over any part
	for (unsigned i = 0; i < parts.size(); ++i) {
		if (parts[i].status < 0) continue;	// part does not exist
		rect r = rect_data[i];
		r.y += (damage_screen_background->get_height()-sub_damage_scheme_all->get_height())/2;
		if (mx >= r.x && mx <= r.x+r.w && my >= r.y && my <= r.y+r.h) {
			// it is important, that texts are in correct order starting with 400.
			bool atleft = (r.x+r.w/2) < 1024/2;
			bool atbottom = (r.y+r.h/2) >= 768/2;

			// determine amount of damage
			unsigned damcat = 0;	// damage category
			if (parts[i].status > 0) {
				if (parts[i].status <= 0.25) damcat = 1;
				else if (parts[i].status <= 0.50) damcat = 2;
				else if (parts[i].status <= 0.75) damcat = 3;
				else if (parts[i].status < 1.00) damcat = 4;
				else damcat = 5;
			}

			// display basic information			
			ostringstream dmgstr;
			dmgstr	<< texts::get(400+i) << "\n"	// name
				<< texts::get(165) << texts::get(130+damcat)
				<< " (" << unsigned(round(100*parts[i].status)) << " "
				<< texts::get(166) << ")\n";

			// if part is damages, display repair information
			if (damcat > 0) {
				if (mysub->damage_schemes[i].repairable) {
					if (mysub->damage_schemes[i].surfaced) {
						dmgstr << texts::get(168);
					} else {
						unsigned minutes = unsigned(round(parts[i].repairtime / 60.0));
						dmgstr << texts::get(167) << "\n"
						<< texts::get(170) << minutes << texts::get(minutes == 1 ? 171 : 172);
					}
				} else {
					dmgstr << texts::get(169);
				}
			}
			
			// display popup with all information. fixme automatic line breaks
			display_popup(r.x+r.w/2, r.y+r.h/2, dmgstr.str(), atleft, atbottom);
		}
	}

	ui.draw_infopanel();

	sys().unprepare_2d_drawing();
}

void sub_damage_display::process_input(class game& gm, const SDL_Event& event)
{
	switch (event.type) {
	case SDL_MOUSEMOTION:
		mx = event.motion.x;
		my = event.motion.y;
		break;
	default:
		break;
	}
}



void sub_damage_display::enter(bool /*is_day*/)
{
	damage_screen_background.reset(new image(get_image_dir() + "damage_screen_backg.jpg"));
	sub_damage_scheme_all.reset(new image(get_image_dir() + "sub_damage_scheme_all.png"));
	repairlight.reset(new texture(get_texture_dir() + "repairlight.png"));
	repairmedium.reset(new texture(get_texture_dir() + "repairmedium.png"));
	repairheavy.reset(new texture(get_texture_dir() + "repairheavy.png"));
	repaircritical.reset(new texture(get_texture_dir() + "repaircritical.png"));
	repairwrecked.reset(new texture(get_texture_dir() + "repairwrecked.png"));
}



void sub_damage_display::leave()
{
	damage_screen_background.reset();
	sub_damage_scheme_all.reset();
}
