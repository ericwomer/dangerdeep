// OpenGL based font rendering
// (C)+(W) by Thorsten Jordan. See LICENSE

#ifndef FONT_H
#define FONT_H

#ifdef WIN32
#define for if(0);else for
#endif

#include <SDL.h>

#include <vector>
#include <string>
using namespace std;
#include "texture.h"
#include "color.h"

// this are ASCII codes 33-126, additionally german umlauts and german quotation marks (may look different)
#define STANDARD_FONT_CHARACTERS "!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~ÄÖÜäöüß«»"

class font
{
private:
	struct character {
		unsigned char mapping;
		texture* tex;
		unsigned width;	// width in texels
	};
	font() {};
	font& operator=(const font& other);
	font(const font& other);
	vector<character> characters;

	unsigned nr_chars;
	unsigned char_height;
	unsigned spacing;
	unsigned blank_width;
	vector<unsigned char> translate;
	
	// returns RGBA value, Surface must be locked
	unsigned get_pixel(SDL_Surface* s, unsigned x, unsigned y) const;
	
	void print_text(int x, int y, const string& text, bool ignore_colors = false) const;
		
public:
	font(const string& filename, unsigned char_spacing=1, unsigned blank_length=8,
		const string& font_mapping = STANDARD_FONT_CHARACTERS);
	~font();
	void print(int x, int y, const string& text, color col = color(255,255,255), bool with_shadow = false) const;
	void print_hc(int x, int y, const string& text, color col = color(255,255,255), bool with_shadow = false) const;
	void print_vc(int x, int y, const string& text, color col = color(255,255,255), bool with_shadow = false) const;
	void print_c(int x, int y, const string& text, color col = color(255,255,255), bool with_shadow = false) const;
	// print text with wrap around, use lineheight 0 for automatic line height
	void print_wrapped(int x, int y, unsigned w, unsigned lineheight, const string& text, color col = color(255,255,255), bool with_shadow = false) const;
	pair<unsigned, unsigned> get_size(const string& text) const;
	unsigned get_char_width(char c) const;
	unsigned get_height(void) const { return char_height; };
};

#endif
