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
#include <map>
#include <iostream>
#include <sstream>
using namespace std;



logbook_display::logbook_display(class user_interface& ui_)
	: user_display(ui_),
	  page_left_offset(76, 118),
	  page_right_offset(554, 118),
	  page_size(400, 500),
	  current_page(0),
	  nr_of_pages(1)
{
}



void logbook_display::previous_page()
{
	if (current_page > 0)
		current_page -= 2;
}



void logbook_display::next_page()
{
	if (current_page + 2 < nr_of_pages)
		current_page += 2;
}



void logbook_display::display(class game& gm) const
{
	// compute size of entries (number of lines for each entry)
	const logbook& lb = gm.get_players_logbook();
	vector<unsigned> lines_per_entry;
	unsigned total_lines = 0;
	lines_per_entry.reserve(lb.size());
	for (list<string>::const_iterator it = lb.begin(); it != lb.end(); ++it) {
		unsigned l = font_jphsl->get_nr_of_lines_wrapped(page_size.x, *it).first;
		lines_per_entry.push_back(l);
		total_lines += l;
	}

	unsigned lines_per_page = page_size.y / font_jphsl->get_height();
	nr_of_pages = (total_lines + lines_per_page-1) / lines_per_page;

	// now compute distribution of entries over pages
	// for each entry compute starting page and line
	vector<pair<unsigned, unsigned> > entry_page_and_line;
	entry_page_and_line.reserve(lb.size());
	unsigned cur_page = 0, cur_line = 0;
	int first_entry_cp_left = -1, last_entry_cp_left = -1;
	int first_entry_cp_right = -1, last_entry_cp_right = -1;
	for (unsigned i = 0; i < lines_per_entry.size(); ++i) {
		if (cur_page == current_page) {
			if (first_entry_cp_left < 0)
				first_entry_cp_left = int(i);
			last_entry_cp_left = int(i) + 1;
		}
		if (cur_page == current_page + 1) {
			if (first_entry_cp_right < 0)
				first_entry_cp_right = int(i);
			last_entry_cp_right = int(i) + 1;
		}
		entry_page_and_line.push_back(make_pair(cur_page, cur_line));
		cur_line += lines_per_entry[i];
		if (cur_line >= lines_per_page) {
			cur_line -= lines_per_page;
			++cur_page;
		}
	}

	// check range of entries to render for currently displayed page
	// render [first_entry_cp_left/right...last_entry_cp_left/right[
	// for current page... note: older entries may wrap, argh!

	sys().prepare_2d_drawing();
	background->draw(0, 0);

	// print rest of part from previous double page, if available
	if ((first_entry_cp_left > 0 && entry_page_and_line[first_entry_cp_left].second > 0) ||
	    (lb.size() > 0 && first_entry_cp_left < 0)) {
		// there is one previous entry and this entry does not start on line 0
		// compute text pointer where to begin print
		unsigned i;
		if (first_entry_cp_left < 0) {
			// no entry matched current_page, so we must be on the last page,
			// and only the rest of the last entry is visible
			i = unsigned(lb.size()-1);
		} else {
			i = unsigned(first_entry_cp_left-1);
		}
		unsigned maxlines = lines_per_page - entry_page_and_line[i].second;
		const string& et = *(lb.get_entry(i));
		unsigned textoff = font_jphsl->get_nr_of_lines_wrapped(page_size.x, et, maxlines).second;
		font_jphsl->print_wrapped(page_left_offset.x,
					  page_left_offset.y,
					  page_size.x, 0, et.substr(textoff), color(10, 10, 10), false, 0);
	}

	for (int i = first_entry_cp_left; i < last_entry_cp_left; ++i) {
		const string& et = *(lb.get_entry(i));
		unsigned maxh = (lines_per_page - entry_page_and_line[i].second) * font_jphsl->get_height();
		unsigned textptr =
			font_jphsl->print_wrapped(page_left_offset.x,
						  page_left_offset.y + entry_page_and_line[i].second * font_jphsl->get_height(),
						  page_size.x, 0, et, color(10, 10, 10), false, maxh);
		if (textptr < et.length()) {
			// print rest of it on second page
			font_jphsl->print_wrapped(page_right_offset.x,
						  page_right_offset.y,
						  page_size.x, 0, et.substr(textptr),
						  color(10, 10, 10), false, 0);
		}
	}
	for (int i = first_entry_cp_right; i < last_entry_cp_right; ++i) {
		const string& et = *(lb.get_entry(i));
		unsigned maxh = (lines_per_page - entry_page_and_line[i].second) * font_jphsl->get_height();
		font_jphsl->print_wrapped(page_right_offset.x,
					  page_right_offset.y + entry_page_and_line[i].second * font_jphsl->get_height(),
					  page_size.x, 0, et, color(10, 10, 10), false, maxh);
	}

	// Display page number.
	ostringstream oss1;
	oss1 << current_page + 1;
	font_jphsl->print(260, 635, oss1.str(), color(10, 10, 10));
	ostringstream oss2;
	oss2 << current_page + 2;
	font_jphsl->print(760, 635, oss2.str(), color(10, 10, 10));

	// Display arrows.
	font_jphsl->print(160, 635, "<<", color(10, 10, 10));
	font_jphsl->print(860, 635, ">>", color(10, 10, 10));

	ui.draw_infopanel();

	sys().unprepare_2d_drawing();
}



void logbook_display::process_input(class game& gm, const SDL_Event& event)
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
		if (sys().translate_position_x(event) < 530)
			previous_page();
		else
			next_page();
		break;
	default:
		break;
	}
}



void logbook_display::enter(bool /*is_day*/)
{
	background.reset(new image(get_image_dir() + "shipslog_main_daylight.jpg"));
}



void logbook_display::leave()
{
	background.reset();
}
