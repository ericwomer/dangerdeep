// logbook
// subsim (C)+(W) Markus Petermann and Thorsten Jordan. SEE LICENSE

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL.h>

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
#include "logbook.h"
#include "font.h"
#include "texts.h"
#include "image.h"
#include "user_interface.h"

#define NUMBER_OF_LINES		27
#define FONT_SCALE_FACTOR	0.75f
#define CHARACTER_PER_LINE	50
#define LINE_INDENT "    "

//
// Class logbook
//
logbook::logbook ()
{}

logbook::~logbook ()
{
	logbook_entries.clear ();
}

ostream& operator << ( ostream& os, const logbook& lb )
{
	logbook::logbook_entry_map_const_iterator it;

	for ( it = lb.logbook_entries.begin (); it != lb.logbook_entries.end ();
		it++ )
	{
		os << (*it).second << endl;
	}
	return os;
}

void logbook::remove_entry ( unsigned i )
{
	logbook_entry_map_iterator it = logbook_entries.find ( i );

	if ( it != logbook_entries.end () )
	{
		logbook_entries.erase ( it );
	}
}



//
// Class logbook_display
//
void logbook_display::print_buffer ( unsigned i, const string& t ) const
{
	unsigned x = ( i < NUMBER_OF_LINES )? 42 : 550;
	x = unsigned ( x / FONT_SCALE_FACTOR );
	unsigned y = ( i < NUMBER_OF_LINES )? ( 40 + 20 * i ) :
		( 40 + 20 * ( i - NUMBER_OF_LINES ) );
	y = unsigned ( y / FONT_SCALE_FACTOR );

	font_arial->print ( x, y, t, color ( 0, 0, 0 ) );
}

void logbook_display::format_line ( list<string>& entries_list, const string& line )
{
	entries_list.clear ();

	if ( line.length () <= CHARACTER_PER_LINE )
	{
		entries_list.push_back ( line );
	}
	else
	{
		string cp_line = line;
		while ( cp_line.length () > CHARACTER_PER_LINE )
		{
			// Search for the first space BEFORE pos.
			int pos = cp_line.find_last_of ( " ", CHARACTER_PER_LINE );

			if ( pos >= 0 )
			{
				ostringstream oss;
				oss << LINE_INDENT << cp_line.substr ( 0, pos );
				entries_list.push_back ( oss.str () );
				cp_line = cp_line.substr ( pos + 1 );
			}
			else
			{
				ostringstream oss;
				oss << LINE_INDENT << cp_line;
				entries_list.push_back ( oss.str () );
				cp_line = "";
			}
		}

		// Pushl rest of line into list.
		if ( cp_line != "" )
		{
			ostringstream oss;
			oss << LINE_INDENT << cp_line;
			entries_list.push_back ( oss.str () );
		}
	}
}

void logbook_display::add_entry ( const date& d, const string& entry )
{
	// Remove initial value from logbook.
	if ( ( lb.size () == 1 ) && ( lb.get_entry ( 0 ) == texts::get(99) ) )
		lb.remove_entry ( 0 );

	// Create date entry.
	ostringstream oss;
	oss << d;
	lb.add_entry ( oss.str () );

	// Create text entry.
	list<string> new_entries;
	format_line ( new_entries, entry );
	for ( list<string>::iterator i = new_entries.begin (); i != new_entries.end (); i++ )
	{
		lb.add_entry ( *i );
	}
}

void logbook_display::next_page ()
{
	if ( actual_entry + 2 * NUMBER_OF_LINES <= lb.size () )
		actual_entry += 2 * NUMBER_OF_LINES;
}

void logbook_display::previous_page ()
{
	if ( actual_entry - 2 * NUMBER_OF_LINES >= 0 )
		actual_entry -= 2 * NUMBER_OF_LINES;
}



//
// Class captain_logbook_display.
//
captains_logbook_display::captains_logbook_display(user_interface& ui_) :
	logbook_display(ui_)
{
	init ();
}

void captains_logbook_display::init ()
{
	actual_entry = 0;
	lb.add_entry ( texts::get(99) );
}

void captains_logbook_display::display ( class game& gm ) const
{
	system::sys().prepare_2d_drawing();

	// Wooden background.
	glColor3f ( 1.0f, 1.0f, 1.0f );
	for ( int i = 0; i < 8; i++ )
	{
		for ( int j = 0; j < 6; j++ )
			woodbackgr->draw ( 128*i, 128*j, 128, 128 );
	}

	// Two white pages.
	glBindTexture ( GL_TEXTURE_2D, 0 );
	system::sys().draw_rectangle (  20, 20, 476, 600 );
	system::sys().draw_rectangle ( 528, 20, 476, 600 );

	// Draw lines.
	glColor3f ( 0.5f, 0.5f, 0.5f );
	for ( int i = 0; i < NUMBER_OF_LINES; i ++ )
	{
		unsigned y = 60 + 20 * i;
		glBegin ( GL_LINES );
		glVertex2i (  40, y );
		glVertex2i ( 476, y );
		glVertex2i ( 548, y );
		glVertex2i ( 984, y );
		glEnd ();
	}

	// Middle spiral.
	glColor3f ( 1.0f, 1.0f, 1.0f );
	glPushMatrix();		// a trick to enlarge spiral y from 512 to 600
	glTranslatef(0, 20, 0);
	glScalef(1, 600.0/512.0, 1);
	logbook_spiral->draw(496, 0);
	glPopMatrix();

	// Create entries.
	// Prepare OpenGL.
	glPushMatrix ();
	glScalef ( FONT_SCALE_FACTOR, FONT_SCALE_FACTOR, 1.0f );
	unsigned max_size = lb.size ();
	for ( unsigned j = 0, i = actual_entry; ( j < 2 * NUMBER_OF_LINES ) &&
		( i < max_size ); i++, j++ )
	{
		const string& entry = lb.get_entry ( i );
		print_buffer ( j, entry );
	}

	// Display page number.
	unsigned page_number = 1 + unsigned ( actual_entry / NUMBER_OF_LINES );
	ostringstream oss1;
	oss1 << page_number;
	font_arial->print (
		unsigned ( 260 / FONT_SCALE_FACTOR ),
		unsigned ( 595 / FONT_SCALE_FACTOR ),
		oss1.str (), color ( 0, 0, 0 ) );
	ostringstream oss2;
	oss2 << ( page_number + 1 );
	font_arial->print (
		unsigned ( 760 / FONT_SCALE_FACTOR ),
		unsigned ( 595 / FONT_SCALE_FACTOR ),
		oss2.str (), color ( 0, 0, 0 ) );

	// Display arrows.
	ostringstream left_arrow_oss;
	left_arrow_oss << "<<";
	font_arial->print (
		unsigned ( 160 / FONT_SCALE_FACTOR ),
		unsigned ( 595 / FONT_SCALE_FACTOR ),
		left_arrow_oss.str (), color ( 0, 0, 0 ) );
	ostringstream right_arrow_oss;
	right_arrow_oss << ">>";
	font_arial->print (
		unsigned ( 860 / FONT_SCALE_FACTOR ),
		unsigned ( 595 / FONT_SCALE_FACTOR ),
		right_arrow_oss.str (), color ( 0, 0, 0 ) );

	// Reset OpenGL matrix.
	glPopMatrix ();

	ui.draw_infopanel(gm);

	system::sys().unprepare_2d_drawing();
}

void captains_logbook_display::process_input(class game& gm, const SDL_Event& event)
{
	switch (event.type) {
	case SDL_KEYDOWN:
		if (event.key.keysym.sym == SDLK_LESS) {
			if (event.key.keysym.mod & (KMOD_LSHIFT | KMOD_RSHIFT))
				next_page();
			else
				previous_page();
		}
		break;
	case SDL_MOUSEBUTTONDOWN:
		if (event.button.x < 530)
			previous_page();
		else
			next_page();
		break;
	default:
		break;
	}
}
