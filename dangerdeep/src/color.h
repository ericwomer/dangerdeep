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
// 2006-11-30 doc1972 added check to prevent double definition. 
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif /* WIN32_LEAN_AND_MEAN */
#include <windows.h>
#endif

#include "oglext/OglExt.h"//DEPRECATED!
#include <SDL_types.h>

//#include <iostream>

///\brief Color representation with some basic transformations and OpenGL usage.
struct color {
	Uint8 r, g, b, a;
	color(Uint8 r_ = 0, Uint8 g_ = 0, Uint8 b_ = 0, Uint8 a_ = 255) : r(r_), g(g_), b(b_), a(a_) {};
	void set_gl_color() const { glColor4ub(r, g, b, a); }//DEPRECATED!
	void set_gl_color(Uint8 alpha) const { glColor4ub(r, g, b, alpha); }//DEPRECATED!
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
	float brightness() const { return (r*0.299+g*0.587+b*0.114)/255; }
	color grey_value() const { Uint8 c = (Uint8)(r*0.299+g*0.587+b*0.114); return color(c, c, c, a); }
	
	//color(istream& in) { r = read_u8(in); g = read_u8(in); b = read_u8(in); a = read_u8(in); }
	//void save(ostream& out) const { write_u8(out, r); write_u8(out, g); write_u8(out, b); write_u8(out, a); }

	color more_contrast(unsigned fac) const {
		int rr = (int(r)-128)*fac+128;
		int gg = (int(g)-128)*fac+128;
		int bb = (int(b)-128)*fac+128;
		return color(Uint8(rr > 255 ? 255 : rr < 0 ? 0 : rr),
			     Uint8(gg > 255 ? 255 : gg < 0 ? 0 : gg),
			     Uint8(bb > 255 ? 255 : bb < 0 ? 0 : bb));
	}
	
	// some useful standard colors
	static color black() { return color(0,0,0); }
	static color blue() { return color(0,0,255); }
	static color green() { return color(0,255,0); }
	static color red() { return color(255,0,0); }
	static color magenta() { return color(255,0,255); }
	static color cyan() { return color(0,255,255); }
	static color yellow() { return color(255,255,0); }
	static color orange() { return color(255,128,0); }
	static color lightgrey() { return color(192,192,192); }
	static color grey() { return color(128,128,128); }
	static color darkgrey() { return color(64,64,64); }
	static color white() { return color(255,255,255); }
};

///\brief Color representation with some basic transformations and OpenGL usage. Float values!
struct colorf {
	float r, g, b, a;
	colorf(float r_ = 0, float g_ = 0, float b_ = 0, float a_ = 1.0f) : r(r_), g(g_), b(b_), a(a_) {};
	colorf(const color& c)
		: r(c.r * 0.003921569f), g(c.g * 0.003921569f),
		  b(c.b * 0.003921569f), a(c.a * 0.003921569f) {} // 0.003921569 = 1/255
	void set_gl_color() const { glColor4f(r, g, b, a); }//DEPRECATED!
	void set_gl_color(float alpha) const { glColor4f(r, g, b, alpha); }//DEPRECATED!
	colorf(const colorf& c1, const colorf& c2, float scal) {
		r = (c1.r*(1-scal) + c2.r*scal);
		g = (c1.g*(1-scal) + c2.g*scal);
		b = (c1.b*(1-scal) + c2.b*scal);
		a = (c1.a*(1-scal) + c2.a*scal);
	}

	colorf operator* (const colorf& c) const {
		return colorf(r*c.r, g*c.g, b*c.b, a*c.a);
	}
	
	///> component wise linear interpolation
	colorf lerp(const colorf& c1, const colorf& c2) const {
		return colorf(c1.r*(1-r) + c2.r*r,
			      c1.g*(1-g) + c2.g*g,
			      c1.b*(1-b) + c2.b*b,
			      c1.a*(1-a) + c2.a*a);
	}
	
	void store_rgb(Uint8* ptr) const { ptr[0] = Uint8(r*255); ptr[1] = Uint8(g*255); ptr[2] = Uint8(b*255); }
	void store_rgba(Uint8* ptr) const { ptr[0] = Uint8(r*255); ptr[1] = Uint8(g*255); ptr[2] = Uint8(b*255); ptr[3] = Uint8(a*255); }
	void store_rgb(float* ptr) const { ptr[0] = r; ptr[1] = g; ptr[2] = b; }
	void store_rgba(float* ptr) const { ptr[0] = r; ptr[1] = g; ptr[2] = b; ptr[3] = a; }

	// transform color to grey value (model of human vision, 29.9% to 58.7% to 11.4% RGB)
	float brightness() const { return (r*0.299+g*0.587+b*0.114); }
	colorf grey_value() const { float c = (r*0.299+g*0.587+b*0.114); return colorf(c, c, c, a); }

	color to_uint8() const { return color(Uint8(r*255), Uint8(g*255), Uint8(b*255), Uint8(a*255)); };
};

#endif
