// OpenGL based font rendering
// (C)+(W) by Thorsten Jordan. See LICENSE

#ifndef FONT_H
#define FONT_H

#include <SDL.h>

#include <vector>
#include <string>
using namespace std;
#include "texture.h"
#include "color.h"



class font
{
private:
	struct character {
		unsigned width, height;	// real width/height
		int left;	// offset
		unsigned top;	// offset
		texture* tex;
		character() : width(0), height(0), left(0), top(0), tex(0) {}
		~character() { delete tex; }
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
