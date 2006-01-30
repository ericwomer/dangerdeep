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

// a color
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef COLOR_H
#define COLOR_H

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#ifdef USE_NATIVE_GL
#include <gl.h>
#else
#include "oglext/OglExt.h"
#endif

//#include "binstream.h"

#include <iostream>
using namespace std;

///\brief Color representation with some basic transformations and OpenGL usage.
struct color {
	Uint8 r, g, b, a;
	color(Uint8 r_ = 0, Uint8 g_ = 0, Uint8 b_ = 0, Uint8 a_ = 255) : r(r_), g(g_), b(b_), a(a_) {};
	void set_gl_color(void) const { glColor4ub(r, g, b, a); }
	void set_gl_color(Uint8 alpha) const { glColor4ub(r, g, b, alpha); }
	color(const color& c1, const color &c2, float scal) {
		r = (Uint8)(c1.r*(1-scal) + c2.r*scal);
		g = (Uint8)(c1.g*(1-scal) + c2.g*scal);
		b = (Uint8)(c1.b*(1-scal) + c2.b*scal);
		a = (Uint8)(c1.a*(1-scal) + c2.a*scal);
	}

	color operator* (const color& c) const {
		return color(
			(Uint8)(unsigned(r)*unsigned(c.r)/255),
			(Uint8)(unsigned(g)*unsigned(c.g)/255),
			(Uint8)(unsigned(b)*unsigned(c.b)/255),
			(Uint8)(unsigned(a)*unsigned(c.a)/255)
		);
	}
	
	void store_rgb(Uint8* ptr) const { ptr[0] = r; ptr[1] = g; ptr[2] = b; }
	void store_rgba(Uint8* ptr) const { ptr[0] = r; ptr[1] = g; ptr[2] = b; ptr[3] = a; }
	void store_rgb(float* ptr) const { ptr[0] = float(r)/255; ptr[1] = float(g)/255; ptr[2] = float(b)/255; }
	void store_rgba(float* ptr) const { ptr[0] = float(r)/255; ptr[1] = float(g)/255; ptr[2] = float(b)/255; ptr[3] = float(a)/255; }

	// transform color to grey value (model of human vision, 29.9% to 58.7% to 11.4% RGB)
	float brightness(void) const { return (r*0.299+g*0.587+b*0.114)/255; }
	color grey_value(void) const { Uint8 c = (Uint8)(r*0.299+g*0.587+b*0.114); return color(c, c, c, a); }
	
	//color(istream& in) { r = read_u8(in); g = read_u8(in); b = read_u8(in); a = read_u8(in); }
	//void save(ostream& out) const { write_u8(out, r); write_u8(out, g); write_u8(out, b); write_u8(out, a); }
	
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
