// OpenGL based font rendering
// (C)+(W) by Thorsten Jordan. See LICENSE

#include "font.h"
#include <GL/gl.h>
#include <cstring>
#include <SDL/SDL_image.h>
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

font::font(const char* filename, unsigned char_spacing, unsigned blank_length,
	const char* font_mapping)
{
	nr_chars = strlen(font_mapping);
	characters.resize(nr_chars);
	for (unsigned k = 0; k < nr_chars; ++k) characters[k].mapping = font_mapping[k];
	spacing = char_spacing;
	blank_width = blank_length;
	translate.resize(256, 0xff);

	SDL_Surface* fontimage = IMG_Load(filename);
	system::sys()->myassert(fontimage, string("font: failed to open")+filename);
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
		system::sys()->myassert(charnr < nr_chars, string("font: too many characters found ")+filename);
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

	ostringstream os;
	os << "font: detected " << charnr << " characters, expected " << nr_chars << " in " << filename;
	system::sys()->myassert(charnr == nr_chars, os.str());

	for (unsigned i = 0; i < nr_chars; i++) {
		characters[i].tex = new texture(fontimage, offsets[i], 0, characters[i].width, h, 1, true, true);
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

void font::print(int x, int y, const char* text) const
{
	int xs = x;
	while (*text) {
		unsigned c = (unsigned char)*text++;
		if (c == ' ') {	// space
			x += blank_width;
		} else if (c == '\n') {	// return
			x = xs;
			y += char_height;
		} else if (c == '$') { // color information
			unsigned nr[6];
			for (int i = 0; i < 6; i++) {
				if (*text >= '0' && *text <= '9')
					nr[i] = *text - '0';
				else if (*text >= 'a' && *text <= 'f')
					nr[i] = 10 + *text - 'a';
				else if (*text == 0)
					break;
				else
					nr[i] = 0;
				text++;
			}
			glColor3f(float(nr[0]*16+nr[1])/255.0, float(nr[2]*16+nr[3])/255.0,
				float(nr[4]*16+nr[5])/255.0);
		} else {
			unsigned t = translate[c];
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
	glColor3f(1,1,1);
}

pair<unsigned, unsigned> font::get_size(const char* text) const
{
	int x = 0, y = char_height;
	int xmax = 0;
	while (*text) {
		unsigned c = (unsigned char)*text++;
		if (c == ' ') {	// space
			x += blank_width;
		} else if (c == '\n') {	// return
			x = 0;
			y += char_height;
		} else if (c == '$') { // color information
			unsigned nr[6];
			for (int i = 0; i < 6; i++) {
				if (*text >= '0' && *text <= '9')
					nr[i] = *text - '0';
				else if (*text >= 'a' && *text <= 'f')
					nr[i] = 10 + *text - 'a';
				else if (*text == 0)
					break;
				else
					nr[i] = 0;
				text++;
			}
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
