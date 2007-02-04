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

// logbook display
// subsim (C)+(W) Markus Petermann and Thorsten Jordan. SEE LICENSE

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

//#include <gl.h>
//#include <glu.h>
//#include <SDL.h>

#include <map>
#include <iostream>
#include <sstream>
using namespace std;
#include "vector2.h"
#include "angle.h"
#include "date.h"
#include "system.h"
#include "game.h"
#include "global_data.h"
#include "user_display.h"
#include "logbook_display.h"
#include "font.h"
#include "texts.h"
#include "image.h"
#include "texture.h"
#include "user_interface.h"

#define NUMBER_OF_LINES		27
#define CHARACTER_PER_LINE	50
#define LINE_INDENT "    "



void logbook_display::print_buffer(unsigned i, const string& t) const
{
	unsigned x = ( i < NUMBER_OF_LINES )? 42 : 550;
	unsigned y = ( i < NUMBER_OF_LINES )? ( 80 + 20 * i ) :
		( 80 + 20 * ( i - NUMBER_OF_LINES ) );

	font_arial->print(x, y, t, color(0, 0, 128));
}



void logbook_display::next_page(unsigned nrentries)
{
	if (actual_entry + 2 * NUMBER_OF_LINES <= nrentries)
		actual_entry += 2 * NUMBER_OF_LINES;
}



void logbook_display::previous_page(unsigned nrentries)
{
	if (actual_entry >= 2 * NUMBER_OF_LINES)
		actual_entry -= 2 * NUMBER_OF_LINES;
}



logbook_display::logbook_display(class user_interface& ui_) : user_display(ui_), actual_entry(0)
{
}



void logbook_display::display(class game& gm) const
{
	// fixme: old code wrapped text when entry was too long. this is missing here
	sys().prepare_2d_drawing();

	glColor4f(1,1,1,1);
	background->draw(0, 0);

	// Draw lines.
	glColor3f ( 0.5f, 0.5f, 0.5f );
	for ( int i = 0; i < NUMBER_OF_LINES; i ++ )
	{
		unsigned y = 100 + 20 * i;
		glBegin ( GL_LINES );
		glVertex2i (  40, y );
		glVertex2i ( 476, y );
		glVertex2i ( 548, y );
		glVertex2i ( 984, y );
		glEnd ();
	}

	// Create entries.
	// Prepare OpenGL.
	const logbook& lb = gm.get_players_logbook();
	list<string>::const_iterator it = lb.get_entry(actual_entry);
	for ( unsigned j = 0; ( j < 2 * NUMBER_OF_LINES ) && it != lb.end(); ++j, ++it) {
		print_buffer(j, *it);
	}

	// Display page number.
	unsigned page_number = 1 + unsigned ( actual_entry / NUMBER_OF_LINES );
	ostringstream oss1;
	oss1 << page_number;
	font_arial->print (
		260,
		635,
		oss1.str (), color ( 0, 0, 0 ) );
	ostringstream oss2;
	oss2 << ( page_number + 1 );
	font_arial->print (
		760,
		635,
		oss2.str (), color ( 0, 0, 0 ) );

	// Display arrows.
	ostringstream left_arrow_oss;
	left_arrow_oss << "<<";
	font_arial->print (
		160,
		635,
		left_arrow_oss.str (), color ( 0, 0, 0 ) );
	ostringstream right_arrow_oss;
	right_arrow_oss << ">>";
	font_arial->print (
		860,
		635,
		right_arrow_oss.str (), color ( 0, 0, 0 ) );

	ui.draw_infopanel();

	sys().unprepare_2d_drawing();
}



void logbook_display::process_input(class game& gm, const SDL_Event& event)
{
	unsigned nrentries = gm.get_players_logbook().size();
	switch (event.type) {
	case SDL_KEYDOWN:
		if (event.key.keysym.sym == SDLK_LESS) {
			if (event.key.keysym.mod & (KMOD_LSHIFT | KMOD_RSHIFT))
				next_page(nrentries);
			else
				previous_page(nrentries);
		}
		break;
	case SDL_MOUSEBUTTONDOWN:
		if (event.button.x < 530)
			previous_page(nrentries);
		else
			next_page(nrentries);
		break;
	default:
		break;
	}
}



void logbook_display::enter(bool /*is_day*/)
{
	background.reset(new image(get_image_dir() + "logbook_background.jpg"));
}



void logbook_display::leave()
{
	background.reset();
}
