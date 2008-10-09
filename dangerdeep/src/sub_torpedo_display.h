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

// user display: submarine's torpedo room
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef SUB_TORPEDO_DISPLAY_H
#define SUB_TORPEDO_DISPLAY_H

#include "user_display.h"
#include "submarine.h"
#include "vector2.h"
#include "texture.h"
#include "objcache.h"
#include "log.h"
#include "primitives.h"

class sub_torpedo_display : public user_display
{
	// source tube nr for manual torpedo transfer, used for drag & drop drawing
	unsigned torptranssrc;

	// textures for drawing the screen
	std::auto_ptr<texture> torpempty;
	std::auto_ptr<texture> torpload;
	std::auto_ptr<texture> torpunload;
	std::auto_ptr<texture> torp1fat1;
	std::auto_ptr<texture> torp1lut1;
	std::auto_ptr<texture> torp1lut2;
	std::auto_ptr<texture> torp1;
	std::auto_ptr<texture> torp1practice;
	std::auto_ptr<texture> torp2;
	std::auto_ptr<texture> torp3afat2;
	std::auto_ptr<texture> torp3alut1;
	std::auto_ptr<texture> torp3alut2;
	std::auto_ptr<texture> torp3fat2;
	std::auto_ptr<texture> torp3;
	std::auto_ptr<texture> torp4;
	std::auto_ptr<texture> torp5b;
	std::auto_ptr<texture> torp5;
	std::auto_ptr<texture> torp6lut1;
//	std::auto_ptr<texture> torp11;
	std::auto_ptr<texture> submodelVIIc;
	std::auto_ptr<image> background;
	std::auto_ptr<image> subtopsideview;
	rotat_tex pointer_seconds;
	rotat_tex pointer_minutes;
	rotat_tex pointer_hours;

	class desc_text
	{
		std::vector<std::string> txtlines;
		desc_text();
	public:
		desc_text(const std::string& filename);
		// give startline and number of lines to fetch (nr=0: all).
		std::string str(unsigned startline = 0, unsigned nrlines = 0) const;
		unsigned nr_of_lines() const { return txtlines.size(); }
	};

	mutable objcachet<desc_text> desc_texts;

	void draw_torpedo(class game& gm, bool usebow, const vector2i& pos, const submarine::stored_torpedo& st) const;

	int mx, my, mb;

	mutable unsigned torp_desc_line;

	objcachet<texture>::reference notepadsheet;

	const texture& torptex(const std::string& torpname) const;

	std::vector<vector2i> get_tubecoords(class submarine* sub) const;

	unsigned get_tube_below_mouse(const std::vector<vector2i>& tubecoords) const;

public:
	sub_torpedo_display(class user_interface& ui_);

	virtual void display(class game& gm) const;
	virtual void process_input(class game& gm, const SDL_Event& event);

	void enter(bool is_day);
	void leave();
};

#endif
