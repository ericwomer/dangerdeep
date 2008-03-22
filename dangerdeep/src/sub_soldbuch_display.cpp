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

// Object to display soldbook
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "system.h"
#include "image.h"
#include "texture.h"
#include "game.h"
#include "font.h"
#include "sub_soldbuch_display.h"
#include "texts.h"
#include "submarine_interface.h"
#include "global_data.h"
#include <sstream>
#include <fstream>
using namespace std;

sub_soldbuch_display::sub_soldbuch_display(user_interface& ui_) :
	user_display(ui_)
{
}

void sub_soldbuch_display::display(class game& gm) const
{
	// just for simplier handling of coords
	int offset_x, offset_y;
	std::stringstream ss;

	// draw display without display color.
	glColor4f(1,1,1,1);
	// draw background
	sys().prepare_2d_drawing();

	background->draw(0, 0);

	// render player info here..
	const game::player_info& pi = gm.get_player_info();
	player_photo->draw(250, 200);

	// specify the primary ovberlay's coords
	offset_x = 600;
	offset_y = 170;

	primary_overlay->draw (offset_x, offset_y);

	// year stamps
	stamps->draw (offset_x, offset_y);
	// soldbuch nr
	font_jphsl->print(offset_x+140, offset_y+45, pi.soldbuch_nr, color(20, 20, 30));
	// rank
	font_jphsl->print(offset_x+30, offset_y+79, texts::get(700), color(20, 20, 30));
	// paygroup
	ss << "A" << 2;
	font_jphsl->print(offset_x+230, offset_y+81, ss.str(), color(20, 20, 30));
	
	//career
	if (pi.career.size() > 1) {
		ss.str("");
		ss << "A" << 2+1;
		font_jphsl->print(offset_x+25, offset_y+140, *pi.career.begin(), color(20, 20, 30));
		font_jphsl->print(offset_x+100, offset_y+140, texts::get(700+1), color(20, 20, 30));
		font_jphsl->print(offset_x+270, offset_y+140, ss.str(), color(20, 20, 30));
		if ( pi.career.size() >= 2 ) {
			unsigned i = 1;
			list<string>::const_iterator it = pi.career.begin();
			for (it++; it != pi.career.end(); ++it ) {
				ss.str("");
				ss << "A" << (2+1+i);
				font_jphsl->print(offset_x+25, offset_y+150+(i*20), *it, color(20, 20, 30));
				font_jphsl->print(offset_x+100, offset_y+150+(i*20), texts::get(700+1+i), color(20, 20, 30));
				font_jphsl->print(offset_x+270, offset_y+150+(i*20), ss.str(), color(20, 20, 30));
				i++;
			}
		}
	}
	// player name
	font_jphsl->print(offset_x+20, offset_y+270, pi.name, color(20, 20, 30));
	//bloodgroup
	font_jphsl->print(offset_x+70, offset_y+340, pi.bloodgroup, color(20, 20, 30));
	//gasmask_size
	font_jphsl->print(offset_x+90, offset_y+364, pi.gasmask_size, color(20, 20, 30));
	//marineroll
	font_jphsl->print(offset_x+125, offset_y+389, pi.marine_roll, color(20, 20, 30));
	//marinegroup
	ss.str("");
	ss << pi.flotilla;
	std::string flotname = texts::get(164);
	flotname.replace(flotname.find("#"), 1, ss.str());
	font_jphsl->print(offset_x+95, offset_y+438, flotname, color(20, 20, 30));
	//identification
	font_jphsl->print(offset_x+125, offset_y+313, pi.name + "/" + pi.soldbuch_nr, color(20, 20, 30));	

	ui.draw_infopanel();
	sys().unprepare_2d_drawing();
}

void sub_soldbuch_display::process_input(class game& gm, const SDL_Event& event)
{
}

void sub_soldbuch_display::enter(bool is_day)
{
	background.reset(new image(get_image_dir() + "soldbuchscreen_background.jpg"));
	primary_overlay.reset(new image(get_image_dir() + "soldbuch_primaryoverlay.png"));
	player_photo.reset(new image(get_image_dir() + "photo_stamp" + ui.get_game().get_player_info().photo + ".jpg|png"));
	stamps.reset(new image(get_image_dir() + "stamp" + str(ui.get_game().get_date().get_value(date::year)) + ".png"));
}

void sub_soldbuch_display::leave()
{
	background.reset();
	player_photo.reset();
	stamps.reset();
	primary_overlay.reset();
}
