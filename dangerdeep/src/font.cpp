// OpenGL based font rendering
// (C)+(W) by Thorsten Jordan. See LICENSE

#include "font.h"
#include <cstring>
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <GL/gl.h>
#include <SDL_image.h>
#else
#include <GL/gl.h>
#include <SDL/SDL_image.h>
#endif
#include "system.h"
#include <sstream>

unsigned font::get_pixel(SDL_Surface* s, unsigned x, unsigned y) const
{
	unsigned bpp = s->format->BytesPerPixel;
	system::sys()->myassert(bpp == 1 || bpp == 4, "font: get_pixel: bpp not allowed");
	unsigned offset = s->pitch * y + bpp * x;
	if (s->format->palette != 0) {
		unsigned char c = *((unsigned char*)(s->pixels) + offset);
		unsigned col = *((unsigned*)&(s->format->palette->colors[c]));
		if ((s->flags & SDL_SRCCOLORKEY) != 0 && c == (s->format->colorkey & 0xff))
			col &= 0x00ffffff;
		else
			col |= 0xff000000;
		return col;
	} else {
		return *(unsigned*)((unsigned char*)(s->pixels) + offset);
	}
}

void font::print_text(int x, int y, const string& text, bool ignore_colors) const
{
	int xs = x;
	for (unsigned ti = 0; ti < text.length(); ++ti) {
		unsigned char c = text[ti];
		if (c == ' ') {	// space
			x += blank_width;
		} else if (c == '\n') {	// return
			x = xs;
			y += char_height;
		} else if (c == '\t') {	// tab
			unsigned tw = char_height*4;
			x = int((x - xs + tw)/tw)*tw + xs;
		} else if (c == '$') { // color information, fixme: do we need that anymore?
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
		} else {
			unsigned t = translate[c];	// fixme: compile and use display lists?
			if (t == 0xff) {
				x += blank_width;
			} else {
				float u1 = float(characters[t].width)/characters[t].tex->get_width();
				float v1 = float(char_height)/characters[t].tex->get_height();
				glBindTexture(GL_TEXTURE_2D, characters[t].tex->get_opengl_name());
			        glBegin(GL_QUADS);
			        glTexCoord2f(0,0);
			        glVertex2i(x,y); 
			        glTexCoord2f(0,v1);
		        	glVertex2i(x,y+char_height);
			        glTexCoord2f(u1,v1);
			        glVertex2i(x+characters[t].width,y+char_height);
			        glTexCoord2f(u1,0);
		        	glVertex2i(x+characters[t].width,y);
			        glEnd();
				x += characters[t].width + spacing;
			}
		}
	}
}

font::font(const string& filename, unsigned char_spacing, unsigned blank_length,
	const string& font_mapping)
{
	nr_chars = font_mapping.length();
	characters.resize(nr_chars);
	for (unsigned k = 0; k < nr_chars; ++k) characters[k].mapping = font_mapping[k];
	spacing = char_spacing;
	blank_width = blank_length;
	translate.resize(256, 0xff);

	SDL_Surface* fontimage = IMG_Load(filename.c_str());
	system::sys()->myassert(fontimage != 0, string("font: failed to open")+filename);
	unsigned w = fontimage->w;
	unsigned h = fontimage->h;

	// calculate widths and offsets
	vector<unsigned> offsets(nr_chars);
	bool onchar = false;
	unsigned charnr = 0;
	unsigned start = 0;
	unsigned widthtotal = 0;
	char_height = h;
	SDL_LockSurface(fontimage);
	for (unsigned i = 0; i < w; i++) {
		if (onchar && !(charnr < nr_chars)) {
			ostringstream os;
			os << "font: too many (" << charnr << "/" << nr_chars << ") found in " << filename;
			system::sys()->myassert(false, os.str());
		}
		bool emptycolumn = true;
		for (unsigned j = 0; j < h; j++) {
			unsigned c = get_pixel(fontimage, i, j);
			if ((c & 0xff000000) != 0) {	// use alpha value !
				emptycolumn = false;
				break;
			}
		}
		if (!emptycolumn && !onchar) {
			onchar = true;
			start = i;
			continue;
		}
		if (onchar && emptycolumn) {
			characters[charnr].width = i-start;
			widthtotal += characters[charnr].width;
			offsets[charnr] = start;
			charnr++;
			onchar = false;
		}
	}
	if (onchar) {	// last column wasn't empty
		characters[charnr].width = w-start;
		widthtotal += characters[charnr].width;
		offsets[charnr] = start;
		charnr++;
	}
	SDL_UnlockSurface(fontimage);

	if (charnr != nr_chars) {
		ostringstream os;
		os << "font: detected " << charnr << " characters, expected " << nr_chars << " in " << filename;
		system::sys()->myassert(false, os.str());
	}

	for (unsigned i = 0; i < nr_chars; i++) {
		characters[i].tex = new texture(fontimage, offsets[i], 0, characters[i].width, h, GL_LINEAR, GL_CLAMP);
	}
		
	for (unsigned i = 0; i < nr_chars; i++)
		translate[characters[i].mapping] = i;
}

font::~font()
{
	for (unsigned i = 0; i < nr_chars; i++) {
		delete characters[i].tex;
	}
}

void font::print(int x, int y, const string& text, color col, bool with_shadow) const
{
	if (with_shadow) {
		glColor3f(0,0,0);
		print_text(x+2, y+2, text, true);
	}
	col.set_gl_color();
	print_text(x, y, text);
}

void font::print_hc(int x, int y, const string& text, color col, bool with_shadow) const
{
	print(x-get_size(text).first/2, y, text, col, with_shadow);
}
	
void font::print_vc(int x, int y, const string& text, color col, bool with_shadow) const
{
	print(x, y-get_size(text).second/2, text, col, with_shadow);
}

void font::print_c(int x, int y, const string& text, color col, bool with_shadow) const
{
	pair<unsigned, unsigned> wh = get_size(text);
	print(x-wh.first/2, y-wh.second/2, text, col, with_shadow);
}

void font::print_wrapped(int x, int y, unsigned w, unsigned lineheight, const string& text, color col, bool with_shadow) const
{
	// loop over spaces
	unsigned currwidth = 0;
	unsigned textptr = 0, oldtextptr = 0;
	unsigned textlen = text.length();
	while (true) {
		// remove spaces, treat returns
		while (textptr < textlen) {	// remove spaces at the beginning
			char c = text[textptr];
			if (c == '\n') {
				y += (lineheight == 0) ? get_height() : lineheight;
				currwidth = 0;
				++textptr;
			} else if (c == ' ') {
				++textptr;
			} else {
				break;
			}
		}
		oldtextptr = textptr;
		// collect characters for current word
		while ((text[textptr] != '\n' && text[textptr] != ' ') && textptr < textlen) {
			++textptr;
		}
		if (textptr == textlen) {	// print remaining text
			print(x, y, text.substr(oldtextptr), col, with_shadow);
			break;
		} else {
			pair<unsigned, unsigned> wh = get_size(text.substr(oldtextptr, textptr - oldtextptr));
			if (currwidth + wh.first >= w) {
				y += (lineheight == 0) ? wh.second : lineheight;
				currwidth = 0;
			}
			print(x+currwidth, y, text.substr(oldtextptr, textptr - oldtextptr), col, with_shadow);
			currwidth += wh.first;
			currwidth += blank_width;
		}
	}
}

pair<unsigned, unsigned> font::get_size(const string& text) const
{
	int x = 0, y = char_height;
	int xmax = 0;
	for (unsigned ti = 0; ti < text.length(); ++ti) {
		unsigned char c = text[ti];
		if (c == ' ') {	// space
			x += blank_width;
		} else if (c == '\n') {	// return
			x = 0;
			y += char_height;
		} else if (c == '\t') {	// tab
			unsigned tw = char_height*4;
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
		} else {
			unsigned t = translate[c];
			if (t == 0xff) {
				x += blank_width;
			} else {
				x += characters[t].width + spacing;
			}
		}
		if (x > xmax) xmax = x;
	}
	if (x == 0) y -= char_height;
	return make_pair(xmax, y);
}
