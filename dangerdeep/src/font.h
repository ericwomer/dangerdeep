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

// OpenGL based font rendering
// (C)+(W) by Thorsten Jordan. See LICENSE

#ifndef FONT_H
#define FONT_H

#include <SDL.h>

#include <vector>
#include <string>
using namespace std;
#include "color.h"


class texture;

///\brief Represents a character font set for OpenGL rendering.
class font
{
private:
	struct character {
		unsigned width, height;	// real width/height
		int left;	// offset
		int top;	// offset
		texture* tex;
		character() : width(0), height(0), left(0), top(0), tex(0) {}
		~character();
	};
	font() {};
	font& operator=(const font& other);
	font(const font& other);
	vector<character> characters;

	unsigned first_char, last_char;	// codes
	unsigned base_height, height;	// base height and real height
	unsigned spacing;		// additional character spacing
	unsigned blank_width;		// width of blank character

	static unsigned next_p2(unsigned i) { unsigned p = 1; while (p < i) p <<= 1; return p; }
	
	void print_text(int x, int y, const string& text, bool ignore_colors = false) const;
		
public:
	font(const string& basefilename, unsigned char_spacing = 1);
	~font();
	void print(int x, int y, const string& text, color col = color(255,255,255), bool with_shadow = false) const;
	void print_hc(int x, int y, const string& text, color col = color(255,255,255), bool with_shadow = false) const;
	void print_vc(int x, int y, const string& text, color col = color(255,255,255), bool with_shadow = false) const;
	void print_c(int x, int y, const string& text, color col = color(255,255,255), bool with_shadow = false) const;
	// print text with wrap around, use lineheight 0 for automatic line height
	void print_wrapped(int x, int y, unsigned w, unsigned lineheight, const string& text, color col = color(255,255,255), bool with_shadow = false) const;
	pair<unsigned, unsigned> get_size(const string& text) const;
	unsigned get_char_width(unsigned char c) const;
	unsigned get_height(void) const { return height; };
};

#endif
