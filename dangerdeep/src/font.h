// OpenGL based font rendering
// (C)+(W) by Thorsten Jordan. See LICENSE

#ifndef FONT_H
#define FONT_H

#include <vector>
using namespace std;
#include "texture.h"
#include "color.h"
#include <SDL/SDL.h>

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
	
	void print_text(int x, int y, const char* text, bool ignore_colors = false) const;
		
public:
	font(const char* filename, unsigned char_spacing=1, unsigned blank_length=8,
		const char* font_mapping= "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.,!?;:()<>/\\-+*");
	~font();
	void print(int x, int y, const char* text, color col = color(255,255,255), bool with_shadow = false) const;
	void print_hc(int x, int y, const char* text, color col = color(255,255,255), bool with_shadow = false) const;
	void print_vc(int x, int y, const char* text, color col = color(255,255,255), bool with_shadow = false) const;
	void print_c(int x, int y, const char* text, color col = color(255,255,255), bool with_shadow = false) const;
	pair<unsigned, unsigned> get_size(const char* text) const;
	unsigned get_height(void) const { return char_height; };
};

#endif
