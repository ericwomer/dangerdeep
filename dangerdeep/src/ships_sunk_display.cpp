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

// Object to create and display the number and tonnage of sunk ships.
// subsim (C)+(W) Markus Petermann and Thorsten Jordan. SEE LICENSE

#include "system.h"
#include "model.h"
#include "texts.h"
#include "game.h"
#include "font.h"
#include "global_data.h"
#include "ships_sunk_display.h"
#include "user_interface.h"
#include "log.h"
#include "primitives.h"
#include <string>
#include <iostream>
#include <sstream>
using namespace std;

#define FONT_SCALE_FACTOR 0.7f


void ships_sunk_display::next_page(unsigned nrships)
{
	if (first_displayed_object + 12 < nrships)
		first_displayed_object += 12;
}



void ships_sunk_display::previous_page(unsigned nrships)
{
	if (first_displayed_object >= 12)
		first_displayed_object -= 12;
}



ships_sunk_display::ships_sunk_display(user_interface& ui_) :
	user_display(ui_), first_displayed_object(0)
{
}



void ships_sunk_display::display ( class game& gm ) const
{
	sys().prepare_2d_drawing();

	// Draw background image.
	cloudsbackgr->draw_tiles(0, 0, 1024, 768);

	unsigned j = first_displayed_object;
	const list<game::sink_record>& sunken_ships = gm.get_sunken_ships();
	list<game::sink_record>::const_iterator it = sunken_ships.begin();
	while (j > 0 && it != sunken_ships.end()) {
		--j;
		++it;
	}
	for (unsigned i = 0; it != sunken_ships.end() && i < 12; ++it, ++i) {
		unsigned x = 35 + 250 * unsigned ( i / 3 );
		unsigned y = 40 + 200 * ( i % 3 );

		// Draw flag.
		primitives::quad(vector2f(x,y), vector2f(x+200,y+150), colorf(1,1,1)).render();

		// Print class name.
		glPushMatrix ();
		glScalef ( FONT_SCALE_FACTOR, FONT_SCALE_FACTOR, 1.0f );
		font_vtremington12->print (
			unsigned ( ( x + 10 ) / FONT_SCALE_FACTOR ),
			unsigned ( ( y + 10 ) / FONT_SCALE_FACTOR ),
			it->descr,
			color ( 0, 0, 0 ) );

		// Print tonnage of ship.
		ostringstream oss;
		oss << it->tons << " " << texts::get(99);
		font_vtremington12->print (
			unsigned ( ( x + 10 ) / FONT_SCALE_FACTOR ),
			unsigned ( ( y + 30 ) / FONT_SCALE_FACTOR ),
			oss.str (),
			color ( 0, 0, 0 ) );
		glPopMatrix ();

		// Draw ship.
		glPushMatrix ();
		glTranslatef ( x + 100, y + 100, 1);
		glScalef (1, 1, 0.001f);
		glRotatef (90, 0, 0, 1);
		glRotatef (-90, 0, 1, 0);
		model* mdl = modelcache().find(data_file().get_rel_path(it->specfilename) + it->mdlname);
		if (mdl) {
			mdl->set_layout(it->layoutname);
			mdl->display();
		} else {
			log_warning("can't find model for that name, BUG?!" <<
				    data_file().get_rel_path(it->specfilename) << it->mdlname);
		}
		glPopMatrix ();
	}

	ui.draw_infopanel();

	sys().unprepare_2d_drawing();
}



void ships_sunk_display::process_input(class game& gm, const SDL_Event& event)
{
	unsigned nrships = gm.get_sunken_ships().size();
	switch (event.type) {
	case SDL_KEYDOWN:
		if (event.key.keysym.sym == SDLK_LESS) {
			if (event.key.keysym.mod & (KMOD_LSHIFT | KMOD_RSHIFT))
				next_page(nrships);
			else
				previous_page(nrships);
		}
		break;
	case SDL_MOUSEBUTTONDOWN:
		if (event.button.x < 530)
			previous_page(nrships);
		else
			next_page(nrships);
		break;
	default:
		break;
	}
}



void ships_sunk_display::enter(bool /*is_day*/)
{
	cloudsbackgr.reset(new texture(get_texture_dir() + "cloudsbackgr.jpg"));
}



void ships_sunk_display::leave()
{
	cloudsbackgr.reset();
}
