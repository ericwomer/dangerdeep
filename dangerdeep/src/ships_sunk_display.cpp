// Object to create and display the number and tonnage of sunk ships.
// subsim (C)+(W) Markus Petermann. SEE LICENSE

#include <map>
#include <string>
#include <iostream>
#include <sstream>
using namespace std;
#include "system.h"
#include "model.h"
#include "texts.h"
#include "global_data.h"
#include "sea_object.h"
#include "ship.h"
#include "user_display.h"
#include "ships_sunk_display.h"

#define FONT_SCALE_FACTOR 0.7f

ships_sunk_display::ships_sunk_display () : first_displayed_object ( 0 )
{
	dom.clear ();
}

ships_sunk_display::~ships_sunk_display ()
{
	dom.clear ();
}

void ships_sunk_display::add_sunk_ship ( const ship* so )
{
	unsigned size = dom.size ();
	dom[size] = destroyed_object ( so->get_model (), so->get_tonnage (),
		so->get_description ( 2 ) );
}

void ships_sunk_display::display ( class system& sys, class game& gm )
{
	glColor3f ( 1.0f, 1.0f, 1.0f );

	// Draw background image.
	for ( int i = 0; i < 4; i++ )
		for ( int j = 0; j < 3; j++ )
			sys.draw_image ( i*256, j*256, 256, 256, cloud_textures[0] );

	for ( unsigned i = first_displayed_object; i < first_displayed_object + 12; i ++ )
	{
		destroyed_object_map_c_it it = dom.find ( i );

		if ( it == dom.end () )
			break;

		int j = i % 12;
		unsigned x = 35 + 250 * unsigned ( j / 3 );
		unsigned y = 40 + 200 * ( j % 3 );

		// Draw flag.
		glColor3f ( 1.0f, 1.0f, 1.0f );
		glBindTexture ( GL_TEXTURE_2D, 0 );
		glBegin ( GL_QUADS );
		sys.draw_rectangle ( x, y, 200, 150 );

		// Print class name.
		glPushMatrix ();
		glScalef ( FONT_SCALE_FACTOR, FONT_SCALE_FACTOR, 1.0f );
		font_arial2->print (
			unsigned ( ( x + 10 ) / FONT_SCALE_FACTOR ),
			unsigned ( ( y + 10 ) / FONT_SCALE_FACTOR ),
			it->second.get_class_name(),
			color ( 0, 0, 0 ) );

		// Print tonnage of ship.
		ostringstream oss;
		oss << (*it).second.get_tonnage () << " " << texts::get(99);
		font_arial2->print (
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
		(*it).second.get_model ()->display ();
		glPopMatrix ();
	}
	glEnd ();
}

void ships_sunk_display::check_key ( int keycode, class system& sys, class game& gm )
{
	if ( sys.key_shift () )
	{
		switch ( keycode )
		{
			case SDLK_LESS:
				next_page ();
				break;
		}
	}
	else
	{
		switch ( keycode )
		{
			case SDLK_LESS:
				previous_page ();
				break;
		}
	}
}

void ships_sunk_display::check_mouse ( int x, int y, int mb )
{}

void ships_sunk_display::next_page ()
{
	if ( first_displayed_object + 12 < dom.size () )
		first_displayed_object += 12;
}

void ships_sunk_display::previous_page ()
{
	if ( first_displayed_object - 12 >= 0 )
		first_displayed_object -= 12;
}
