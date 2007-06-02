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

#include "font.h"
#include "texture.h"
#include "system.h"
#include "oglext/OglExt.h"
#include <SDL.h>
#include <SDL_image.h>
#include <sstream>
#include <fstream>
using std::vector;
using std::string;
using std::ifstream;

/* fixme: loading all fonts eats 1.3mb of video ram.
   We are using the ram very wastefully, one texture per character.
   This is bad.
   Better unite many characters to a 256x256 texture and use this.
   We may need several textures for one character set, which makes
   things more complicated.
   However we have to store one empty one row and line between each character
   in that texmap, to avoid artifacts showing up when characters are scaled.
*/


font::character::~character()
{
	delete tex;
}

void font::print_text(int x, int y, const string& text, bool ignore_colors) const
{
	glActiveTexture(GL_TEXTURE1);	// fixme: disable units where they are used!
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);
	int xs = x;
	for (unsigned ti = 0; ti < text.length(); ti = character_right(text, ti)) {
		// read next unicode character
		unsigned c = read_character(text, ti);
		// if it is broken or illegal, skip it
		if (c == invalid_utf8_char) {
			continue;
		}
		if (c == ' ') {	// space
			x += blank_width;
		} else if (c == '\n') {	// return
			x = xs;
			y += height;
		} else if (c == '\t') {	// tab
			unsigned tw = height*4;
			x = int((x - xs + tw)/tw)*tw + xs;
		} else if (c == '$') { // color information
			unsigned nr[6];
			++ti;
			for (int i = 0; i < 6; ++i, ++ti) {
				if (ti >= text.length()) break;
				char c2 = text[ti];
				if (c2 >= '0' && c2 <= '9')
					nr[i] = c2 - '0';
				else if (c2 >= 'a' && c2 <= 'f')
					nr[i] = 10 + c2 - 'a';
				else
					nr[i] = 0;
			}
			--ti;	// compensate for(...++ti)
			if (!ignore_colors)
				glColor3ub(nr[0]*16+nr[1], nr[2]*16+nr[3], nr[4]*16+nr[5]);
		} else if (unsigned(c) >= first_char && unsigned(c) <= last_char) {
			unsigned t = unsigned(c) - first_char;
			// fixme: we could use display lists here
			float u1 = float(characters[t].width)/characters[t].tex->get_width();
			float v1 = float(characters[t].height)/characters[t].tex->get_height();
			//fixme: width in text is width+left, so advance x by that value
			//and draw at x+left. must be changed everywhere
			//int x2 = x + characters[t].left;
			int y2 = y + base_height - characters[t].top;
			glBindTexture(GL_TEXTURE_2D, characters[t].tex->get_opengl_name());
		        glBegin(GL_QUADS);
		        glTexCoord2f(0,0);
		        glVertex2i(x,y2); 
		        glTexCoord2f(0,v1);
	        	glVertex2i(x,y2+characters[t].height);
		        glTexCoord2f(u1,v1);
		        glVertex2i(x+characters[t].width,y2+characters[t].height);
		        glTexCoord2f(u1,0);
	        	glVertex2i(x+characters[t].width,y2);
		        glEnd();
			x += characters[t].width + spacing;
		} // else: just skip (unknown) character
	}
}

font::font(const string& basefilename, unsigned char_spacing)
{
	ifstream metricfile((basefilename + ".metric").c_str());
	metricfile >> base_height;
	metricfile >> first_char;
	metricfile >> last_char;
	characters.resize(last_char-first_char+1);
	for (unsigned i = first_char; i <= last_char; ++i) {
		if (!metricfile.good())
			throw error(string("error reading font metricfile for ")+basefilename);
		metricfile >> characters[i-first_char].width;
		metricfile >> characters[i-first_char].height;
		metricfile >> characters[i-first_char].left;
		metricfile >> characters[i-first_char].top;
	}
	
	spacing = char_spacing;
	unsigned codex = unsigned('x');
	blank_width = (codex >= first_char && codex <= last_char) ? characters[codex-first_char].width : 8;
	height = base_height*3/2;
	base_height = base_height*7/6;	// tiny trick to use the space a bit better.
	
	// calculate offsets
	vector<unsigned> offsets(characters.size());
	for (unsigned i = 0, curoffset = 0; i < characters.size(); ++i) {
		offsets[i] = curoffset;
		curoffset += characters[i].width;
	}

	// load image
	sdl_image fontimage(basefilename + ".png");

	// process image data, create textures
	fontimage.lock();
	if (fontimage->format->BytesPerPixel != 1) {
		fontimage.unlock();
		throw error(string("font: only grayscale images are supported! font ")+basefilename);
	}

//	measure waste of video ram. 2006/12/02, ca. 660kb wasted, 1.3mb totally used. so 50% loss.
//	unsigned waste = 0;
	for (unsigned i = 0; i < characters.size(); i++) {
		character& c = characters[i];
		unsigned w = next_p2(c.width);
		unsigned h = next_p2(c.height);
		vector<Uint8> tmpimage(w * h * 2);
//		waste += (w*h-(c.width+1)*(c.height+1))*2;
		
		unsigned char* ptr = ((unsigned char*)(fontimage->pixels)) + offsets[i];
	
		for (unsigned y = 0; y < c.height; y++) {
			for (unsigned x = 0; x < c.width; ++x) {
				tmpimage[2*(y*w+x)] = 255;
				tmpimage[2*(y*w+x)+1] = *(ptr+x);
			}
			ptr += fontimage->pitch;
		}
		
		characters[i].tex = new texture(tmpimage, w, h, GL_LUMINANCE_ALPHA,
						texture::LINEAR, texture::CLAMP_TO_EDGE);
	}
//	printf("wasted ca. %u bytes of video ram for font %s\n", waste, basefilename.c_str());
	
	fontimage.unlock();
}



void font::print(int x, int y, const string& text, color col, bool with_shadow) const
{
	if (with_shadow) {
		glColor4ub(0,0,0,col.a);
		print_text(x+2, y+2, text, true);
	}
	col.set_gl_color();
	print_text(x, y, text);
}

void font::print_hc(int x, int y, const string& text, color col, bool with_shadow) const
{
	print(x-get_size(text).x/2, y, text, col, with_shadow);
}
	
void font::print_vc(int x, int y, const string& text, color col, bool with_shadow) const
{
	print(x, y-get_size(text).y/2, text, col, with_shadow);
}

void font::print_c(int x, int y, const string& text, color col, bool with_shadow) const
{
	vector2i wh = get_size(text);
	print(x-wh.x/2, y-wh.y/2, text, col, with_shadow);
}

unsigned font::print_wrapped(int x, int y, unsigned w, unsigned lineheight, const string& text,
			     color col, bool with_shadow, unsigned maxheight) const
{
	// loop over spaces
	unsigned currwidth = 0;
	unsigned textptr = 0, oldtextptr = 0;
	unsigned textlen = text.length();
	int ymax = maxheight ? int(maxheight) + y : 0x7fffffff /*intmax*/;
	while (true) {
		// remove spaces, treat returns
		while (textptr < textlen) {	// remove spaces at the beginning
			char c = text[textptr];
			if (c == '\n') {
				y += (lineheight == 0) ? get_height() : lineheight;
				currwidth = 0;
				++textptr;
				if (y >= ymax)
					return textptr;
			} else if (c == ' ') {
				++textptr;
			} else if (c == '\t') {
				++textptr;
			} else {
				break;
			}
		}
		oldtextptr = textptr;
		// collect characters for current word
		unsigned charw = 0;
		bool breaktext = false;
		while ((text[textptr] != '\n' && text[textptr] != ' ' && text[textptr] != '\t') && textptr < textlen) {
			unsigned c = read_character(text, textptr);
			charw += get_char_width(c);
			if (currwidth == 0 && charw >= w) {	// word is longer than line
				breaktext = true;
				break;
			}
			charw += spacing;
			textptr = character_right(text, textptr);
		}
		if (textlen > 0 && textptr == 0) {	// space is not enough to wrap first word. so disable wrapping
			w = 0xffffffff;
			continue;
		}
		if (breaktext) {
			print(x, y, text.substr(oldtextptr, textptr - oldtextptr), col, with_shadow);
			y += (lineheight == 0) ? get_height() : lineheight;
			if (y >= ymax)
				return textptr;
		} else {
			unsigned tw = get_size(text.substr(oldtextptr, textptr - oldtextptr)).x;
			if (currwidth + tw >= w) {
				y += (lineheight == 0) ? get_height() : lineheight;
				currwidth = 0;
				if (y >= ymax)
					return oldtextptr;
			}
			print(x+currwidth, y, text.substr(oldtextptr, textptr - oldtextptr), col, with_shadow);
			currwidth += tw + blank_width;
		}
		if (textptr == textlen)
			break;
	}
	return textlen;
}



vector2i font::get_size(const string& text) const
{
	unsigned x = 0, y = height;
	unsigned xmax = 0;
	for (unsigned ti = 0; ti < text.length(); ti = character_right(text, ti)) {
		// read next unicode character
		unsigned c = read_character(text, ti);
		// if it is broken or illegal, skip it
		if (c == invalid_utf8_char) {
			continue;
		}
		if (c == ' ') {	// space
			x += blank_width;
		} else if (c == '\n') {	// return
			x = 0;
			y += height;
		} else if (c == '\t') {	// tab
			unsigned tw = height*4;
			x = int((x + tw)/tw)*tw;
		} else if (c == '$') { // color information
			unsigned nr[6];
			++ti;
			for (int i = 0; i < 6; ++i, ++ti) {
				if (ti >= text.length()) break;
				char c2 = text[ti];
				if (c2 >= '0' && c2 <= '9')
					nr[i] = c2 - '0';
				else if (c2 >= 'a' && c2 <= 'f')
					nr[i] = 10 + c2 - 'a';
				else
					nr[i] = 0;
			}
			--ti;	// compensate for(...++ti)
		} else if (unsigned(c) >= first_char && unsigned(c) <= last_char) {
			unsigned t = unsigned(c) - first_char;
			x += characters[t].width + spacing;
		} // else: ignore (unknown) character
		if (x > xmax) xmax = x;
	}
	if (x == 0) y -= height;
	return vector2i(xmax, y);
}



std::pair<unsigned, unsigned> font::get_nr_of_lines_wrapped(unsigned w, const string& text, unsigned maxlines) const
{
	// loop over spaces
	unsigned currwidth = 0;
	unsigned textptr = 0, oldtextptr = 0;
	unsigned textlen = text.length();
	unsigned nrlines = 1;
	while (true) {
		// remove spaces, treat returns
		while (textptr < textlen) {	// remove spaces at the beginning
			char c = text[textptr];
			if (c == '\n') {
				++nrlines;
				currwidth = 0;
				++textptr;
				if (maxlines && nrlines > maxlines)
					return std::make_pair(nrlines, textptr);
			} else if (c == ' ') {
				++textptr;
			} else if (c == '\t') {
				++textptr;
			} else {
				break;
			}
		}
		oldtextptr = textptr;
		// collect characters for current word
		unsigned charw = 0;
		bool breaktext = false;
		while ((text[textptr] != '\n' && text[textptr] != ' ' && text[textptr] != '\t') && textptr < textlen) {
			unsigned c = read_character(text, textptr);
			charw += get_char_width(c);
			if (currwidth == 0 && charw >= w) {	// word is longer than line
				breaktext = true;
				break;
			}
			charw += spacing;
			textptr = character_right(text, textptr);
		}
		if (textlen > 0 && textptr == 0) {	// space is not enough to wrap first word. so disable wrapping
			w = 0xffffffff;
			continue;
		}
		if (breaktext) {
			++nrlines;
			if (maxlines && nrlines > maxlines) {
				textptr = oldtextptr;
				break;
			}
		} else {
			unsigned tw = get_size(text.substr(oldtextptr, textptr - oldtextptr)).x;
			if (currwidth + tw >= w) {
				++nrlines;
				if (maxlines && nrlines > maxlines) {
					textptr = oldtextptr;
					break;
				}
				currwidth = 0;
			}
			currwidth += tw + blank_width;
		}
		if (textptr == textlen)
			break;
	}
	return std::make_pair(nrlines, textptr);
}



unsigned font::get_char_width(unsigned c) const
{
	if (c == ' ') {
		return blank_width;
	} else if (c >= first_char && c <= last_char) {
		return characters[unsigned(c) - first_char].width;
	} else {
		return 0;
	}
}



unsigned font::character_left(const std::string& text, unsigned cp)
{
	if (cp > 0) {
		// move one left
		--cp;
		// check if we are on multibyte char
		if (is_byte_of_multibyte_char(text[cp])) {
			if (cp >= 2 && is_first_byte_of_threebyte_char(text[cp-2])) {
				cp -= 2;
			} else if (cp >= 1 && is_first_byte_of_twobyte_char(text[cp-1])) {
				--cp;
			}
			// else: 4-byte char or illegal char, skip
		}
	}
	return cp;
}



unsigned font::character_right(const std::string& text, unsigned cp)
{
	const unsigned l = text.size();
	if (cp < l) {
		// check if we are on multibyte char
		if (is_byte_of_multibyte_char(text[cp])) {
			if (is_first_byte_of_twobyte_char(text[cp])) {
				if (cp + 1 < l)
					cp += 2;
				else
					cp = l;
			} else if (is_first_byte_of_threebyte_char(text[cp])) {
				if (cp + 2 < l)
					cp += 3;
				else
					cp = l;
			} else {
				// four-byte char or other invalid char, skip
				++cp;
			}
		} else {
			++cp;
		}
	}
	return cp;
}



unsigned font::read_character(const std::string& text, unsigned cp)
{
	// Unicode (UTF-8) decoder:
	// In UTF-8 all characters from 0x00-0x7F are encoded as one byte.
	// Characters from 0x80-0x7FF are encoded as two bytes:
	// 0xC0 | (upper 5 bits of c), 0x80 | (lower 6 bits of c)
	// This is sufficient to encode 11bits of characters.
	// Characters from 0x0800-0xffff are encoded as three bytes:
	// 0xE0 | (upper 4 bits of c), 0x80 | (middle 6 bits of c), 0x80 | (lower 6 bits of c)
	// Characters from 0x010000-0x10FFFF are encoded as four bytes:
	// 0xF0 | (upper 3 bits), (0x80 | (further 6 bits)) * 3
	// We only use the lower 256 characters of Unicode, which map to
	// ISO-8859-1.
	// We could either copy the character value directly from the source
	// or translate it from UTF-8.
	unsigned char c = text[cp];
	if (is_byte_of_multibyte_char(c)) {
		// highest bit is set, so we have utf-8 multibyte character
		if (is_first_byte_of_twobyte_char(c)) {
			if (cp + 1 < text.length()) {
				unsigned c2 = text[cp + 1];
				// combine bytes to 8bit character
				return (unsigned(c & 0x1F) << 6) | (c2 & 0x3F);
			}
		} else if (is_first_byte_of_threebyte_char(c)) {
			if (cp + 2 < text.length()) {
				unsigned c2 = text[cp + 1];
				unsigned c3 = text[cp + 2];
				// combine bytes to 8bit character
				return (unsigned(c & 0x0F) << 12) | ((c2 & 0x3F) << 6)
					| (c3 & 0x3F);
			}
		}
		// fourbyte or invalid utf-8 character
		return invalid_utf8_char;
	} // else: normal character
	return c;
}



std::string font::to_utf8(Uint16 unicode)
{
	// input can have at max. 16bits, so range 0x0000-0xffff
	// that matches utf-8 1-byte characters until utf-8 3-byte characters.
	char tmp[4] = { 0, 0, 0, 0 };
	if (unicode <= 0x7F) {
		tmp[0] = unicode;
	} else if (unicode <= 0x7FF) {
		tmp[0] = 0xC0 | (unicode >> 6);
		tmp[1] = 0x80 | (unicode & 0x3F);
	} else {
		tmp[0] = 0xE0 | (unicode >> 12);
		tmp[1] = 0x80 | ((unicode >> 6) & 0x3F);
		tmp[2] = 0x80 | (unicode & 0x3F);
	}
	return std::string(tmp);
}
