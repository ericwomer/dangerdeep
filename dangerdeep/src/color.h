// a color
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef COLOR_H
#define COLOR_H

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <GL/gl.h>
#pragma warning (disable:4786)
#else
#include <GL/gl.h>
#endif

#include "binstream.h"

#include <iostream>
using namespace std;

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

	// transform color to grey value (model of human vision, 11% to 59% to 30% RGB)
	color grey_value(void) const { unsigned char c = (unsigned char)(r*0.11+g*0.59+b*0.3); return color(c, c, c); }
	
	color(istream& in) { r = read_u8(in); g = read_u8(in); b = read_u8(in); }
	void save(ostream& out) const { write_u8(out, r); write_u8(out, g); write_u8(out, b); }
	
	// some useful standard colors
	static color black(void) { return color(0,0,0); }
	static color blue(void) { return color(0,0,255); }
	static color green(void) { return color(0,255,0); }
	static color red(void) { return color(255,0,0); }
	static color magenta(void) { return color(255,0,255); }
	static color cyan(void) { return color(0,255,255); }
	static color yellow(void) { return color(255,255,0); }
	static color orange(void) { return color(255,128,0); }
	static color lightgrey(void) { return color(192,192,192); }
	static color grey(void) { return color(128,128,128); }
	static color darkgrey(void) { return color(64,64,64); }
	static color white(void) { return color(255,255,255); }
};

#endif
