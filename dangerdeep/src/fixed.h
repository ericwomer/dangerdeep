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

// a 32bit fixed point data type, handles only positive values
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef FIXED_H
#define FIXED_H

#include <SDL_types.h>

///\brief Implementation of a fixed point number.
class fixed32
{
	static const Sint32 ONE = 0x10000;
	static const Sint32 HALF = 0x08000;
	static const unsigned SHIFT = 16;
	Sint32 x;
 public:
	fixed32() : x(0) {}
	fixed32(const fixed32& f) : x(f.x) {}
	fixed32& operator= (const fixed32& f) { x = f.x; return *this; }
	fixed32(Sint32 n) : x(n) {}
	fixed32(float f) : x(Sint32(f * ONE)) {}
	fixed32 frac() const { return fixed32(x & (ONE-1)); }
	fixed32 floor() const { return fixed32(x & (~(ONE-1))); }
	fixed32 ceil() const { return fixed32((x + (ONE-1)) & (~(ONE-1))); }
	fixed32 operator+ (const fixed32& f) const { return fixed32(x + f.x); }
	fixed32 operator- (const fixed32& f) const { return fixed32(x - f.x); }
	fixed32 operator- () const { return fixed32(-x); }
	fixed32 operator* (const fixed32& f) const { return fixed32(Sint32(Sint64(x)*f.x >> SHIFT)); }
	fixed32 operator* (int n) const { return fixed32(x * n); }
	fixed32& operator+= (const fixed32& f) { x += f.x; return *this; }
	fixed32& operator-= (const fixed32& f) { x -= f.x; return *this; }
	fixed32& operator*= (const fixed32& f) { x = Sint32(Sint64(x)*f.x >> SHIFT); return *this; }
	bool operator== (const fixed32& f) const { return x == f.x; }
	bool operator!= (const fixed32& f) const { return x != f.x; }
	bool operator<= (const fixed32& f) const { return x <= f.x; }
	bool operator>= (const fixed32& f) const { return x >= f.x; }
	bool operator< (const fixed32& f) const { return x < f.x; }
	bool operator> (const fixed32& f) const { return x > f.x; }
	static fixed32 one() { return fixed32(ONE); }
	fixed32 operator/ (const fixed32& f) const { return fixed32(Sint32((Sint64(x)<<SHIFT)/f.x)); }
	fixed32 operator/ (int n) const { return fixed32(x/n); }
	int intpart() const { return int(x>>SHIFT); }
	int round() const { return int((x+HALF)>>SHIFT); }
	Sint32 value() const { return x; }
};

#endif
