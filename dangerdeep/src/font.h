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

#include <vector>
#include <string>
#include "vector2.h"
#include "color.h"
#include "objcache.h"


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
	font();
	font& operator=(const font& other);
	font(const font& other);
	std::vector<character> characters;

	unsigned first_char, last_char;	// codes
	unsigned base_height, height;	// base height and real height
	unsigned spacing;		// additional character spacing
	unsigned blank_width;		// width of blank character

	static unsigned next_p2(unsigned i) { unsigned p = 1; while (p < i) p <<= 1; return p; }
	
	void print_text(int x, int y, const std::string& text, bool ignore_colors = false) const;

public:
	font(const std::string& basefilename, unsigned char_spacing = 1);
	void print(int x, int y, const std::string& text, color col = color(255,255,255), bool with_shadow = false) const;
	void print_hc(int x, int y, const std::string& text, color col = color(255,255,255), bool with_shadow = false) const;
	void print_vc(int x, int y, const std::string& text, color col = color(255,255,255), bool with_shadow = false) const;
	void print_c(int x, int y, const std::string& text, color col = color(255,255,255), bool with_shadow = false) const;
	// print text with wrap around, use lineheight 0 for automatic line height
	// maxheight=0 disables height limit.
	// returns pointer where text processing stopped, when max. height reached
	unsigned print_wrapped(int x, int y, unsigned w, unsigned lineheight,
			       const std::string& text, color col = color(255,255,255),
			       bool with_shadow = false,
			       unsigned maxheight = 0 /* 0 means no limit */) const;
	vector2i get_size(const std::string& text) const;
	unsigned get_char_width(unsigned c) const;
	unsigned get_height() const { return height; }

	// common functions for working with UTF-8 strings
	static unsigned character_left(const std::string& text, unsigned cp);
	static unsigned character_right(const std::string& text, unsigned cp);
	static bool is_byte_of_multibyte_char(unsigned char c) { return (c & 0x80); }
	static bool is_first_byte_of_twobyte_char(unsigned char c) { return (c & 0xE0) == 0xC0; }
	static bool is_first_byte_of_threebyte_char(unsigned char c) { return (c & 0xF0) == 0xE0; }
	static bool is_first_byte_of_fourbyte_char(unsigned char c) { return (c & 0xF8) == 0xF0; }
	static unsigned read_character(const std::string& text, unsigned cp);
	static std::string to_utf8(Uint16 unicode);
	static const unsigned invalid_utf8_char = 0xffffffff;
};

#endif
