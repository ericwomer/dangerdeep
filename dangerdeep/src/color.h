// a color
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef COLOR_H
#define COLOR_H

#include <GL/gl.h>

struct color {
	unsigned char r, g, b;
	color(unsigned char r_ = 0, unsigned char g_ = 0, unsigned char b_ = 0) : r(r_), g(g_), b(b_) {};
	void set_gl_color(void) const { glColor3ub(r, g, b); }
	void set_gl_color(unsigned char alpha) const { glColor4ub(r, g, b, alpha); }
	color(const color& c1, const color &c2, float scal) {
		r = (unsigned char)(c1.r*(1-scal) + c2.r*scal);
		g = (unsigned char)(c1.g*(1-scal) + c2.g*scal);
		b = (unsigned char)(c1.b*(1-scal) + c2.b*scal);
	}
};

#endif
