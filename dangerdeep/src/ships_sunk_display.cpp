// Object to create and display the number and tonnage of sunk ships.
// subsim (C)+(W) Markus Petermann and Thorsten Jordan. SEE LICENSE

#include <string>
#include <iostream>
#include <sstream>
using namespace std;
#include "system.h"
#include "model.h"
#include "texts.h"
#include "game.h"
#include "global_data.h"
#include "ships_sunk_display.h"
#include "user_interface.h"

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



ships_sunk_display::~ships_sunk_display()
{
}



void ships_sunk_display::display ( class game& gm ) const
{
	system::sys().prepare_2d_drawing();

	glColor3f ( 1.0f, 1.0f, 1.0f );

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
		glColor3f ( 1.0f, 1.0f, 1.0f );
		glBindTexture ( GL_TEXTURE_2D, 0 );
		glBegin ( GL_QUADS );
		system::sys().draw_rectangle ( x, y, 200, 150 );

		// Print class name.
		glPushMatrix ();
		glScalef ( FONT_SCALE_FACTOR, FONT_SCALE_FACTOR, 1.0f );
		font_arial->print (
			unsigned ( ( x + 10 ) / FONT_SCALE_FACTOR ),
			unsigned ( ( y + 10 ) / FONT_SCALE_FACTOR ),
			it->descr,
			color ( 0, 0, 0 ) );

		// Print tonnage of ship.
		ostringstream oss;
		oss << it->tons << " " << texts::get(99);
		font_arial->print (
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
		modelcache.find(it->mdlname)->display();
		glPopMatrix ();
	}
	glEnd ();

	ui.draw_infopanel(gm);

	system::sys().unprepare_2d_drawing();
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
