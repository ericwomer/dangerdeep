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

// Binary Stream Tools
// (C) 2002 Thorsten Jordan

#ifndef BINSTREAM_H
#define BINSTREAM_H

#include <SDL_types.h>
#include <SDL_endian.h>
#include <iostream>
#include <string>
#include "quaternion.h"

// Data is stored in little endian mode.
// On big endian machines the data is converted for reading and writing.
// We use the SDL_SwapLE functions for both operations,
// because they are identical to their inverse functions
// on both type of machines.

// So far we assume sizeof(float)==4 and sizeof(double)==8 (IEEE standard!)
// The only system dependent assumption is that sizeof(int) >= 4 (32 bits).
// C(++) guarantees that short has at least 16 bits, long has at least 32,
// and int has some bit number between short and long.

// these unions are a way to handle C99 strict aliasing rules.
// float f = *(float*)&u;   is a bad idea with C99, instructions
// can be reordered that shouldn't because u is hidden.
union float_u32_shared
{
	float f;
	Uint32 u;
};

union double_u64_shared
{
	double d;
	Uint64 u;
};



inline void write_i8(std::ostream& out, Sint8 i)
{
	// no LE/BE swap needed.
	out.write((char*)&i, 1);
}

inline void write_i16(std::ostream& out, Sint16 i)
{
	Uint16 ii = SDL_SwapLE16((Uint16)i);
	out.write((char*)&ii, 2);
}

inline void write_i32(std::ostream& out, Sint32 i)
{
	Uint32 ii = SDL_SwapLE32((Uint32)i);
	out.write((char*)&ii, 4);
}

inline void write_i64(std::ostream& out, Sint64 i)
{
	Uint64 ii = SDL_SwapLE64((Uint64)i);
	out.write((char*)&ii, 8);
}

inline void write_u8(std::ostream& out, Uint8 i)
{
	// no LE/BE swap needed.
	out.write((char*)&i, 1);
}

inline void write_u16(std::ostream& out, Uint16 i)
{
	Uint16 ii = SDL_SwapLE16(i);
	out.write((char*)&ii, 2);
}

inline void write_u32(std::ostream& out, Uint32 i)
{
	Uint32 ii = SDL_SwapLE32(i);
	out.write((char*)&ii, 4);
}

inline void write_u64(std::ostream& out, Uint64 i)
{
	Uint64 ii = SDL_SwapLE64(i);
	out.write((char*)&ii, 8);
}

inline Sint8 read_i8(std::istream& in)
{
	Sint8 i = 0;	// no LE/BE swap needed.
	in.read((char*)&i, 1);
	return i;
}

inline Sint16 read_i16(std::istream& in)
{
	Uint16 i = 0;
	in.read((char*)&i, 2);
	return Sint16(SDL_SwapLE16(i));
}

inline Sint32 read_i32(std::istream& in)
{
	Uint32 i = 0;
	in.read((char*)&i, 4);
	return Sint32(SDL_SwapLE32(i));
}

inline Sint64 read_i64(std::istream& in)
{
	Uint64 i = 0;
	in.read((char*)&i, 8);
	return Sint64(SDL_SwapLE64(i));
}

inline Uint8 read_u8(std::istream& in)
{
	Uint8 i = 0;	// no LE/BE swap needed.
	in.read((char*)&i, 1);
	return i;
}

inline Uint16 read_u16(std::istream& in)
{
	Uint16 i = 0;
	in.read((char*)&i, 2);
	return Uint16(SDL_SwapLE16(i));
}

inline Uint32 read_u32(std::istream& in)
{
	Uint32 i = 0;
	in.read((char*)&i, 4);
	return Uint32(SDL_SwapLE32(i));
}

inline Uint64 read_u64(std::istream& in)
{
	Uint64 i = 0;
	in.read((char*)&i, 8);
	return Uint64(SDL_SwapLE64(i));
}

inline void write_bool(std::ostream& out, bool b)
{
	write_u8(out, b ? 1 : 0);
}

inline bool read_bool(std::istream& in)
{
	return (read_u8(in) != 0);
}

inline void write_float(std::ostream& out, float f)
{
	float_u32_shared s;
	s.f = f;
	write_u32(out, s.u);
}

inline float read_float(std::istream& in)
{
	float_u32_shared s;
	s.u = read_u32(in);
	return s.f;
}

inline void write_double(std::ostream& out, double d)
{
	double_u64_shared s;
	s.d = d;
	write_u64(out, s.u);
}

inline double read_double(std::istream& in)
{
	double_u64_shared s;
	s.u = read_u64(in);
	return s.d;
}

inline void write_string(std::ostream& out, const std::string& s)
{
	unsigned l = s.size();
	write_u32(out, l);
	if (l > 0)
		out.write(s.c_str(), l);
}

inline std::string read_string(std::istream& in)
{
	unsigned l = read_u32(in);
	if (l > 0) {
		std::string s(l, 'x');
		in.read(&s[0], l);
		return s;
	} else {
		return std::string();
	}
}

inline vector2 read_vector2(std::istream& in)
{
	vector2 v;
	v.x = read_double(in);
	v.y = read_double(in);
	return v;
}

inline void write_vector2(std::ostream& out, const vector2& v)
{
	write_double(out, v.x);
	write_double(out, v.y);
}

inline vector3 read_vector3(std::istream& in)
{
	vector3 v;
	v.x = read_double(in);
	v.y = read_double(in);
	v.z = read_double(in);
	return v;
}

inline void write_vector3(std::ostream& out, const vector3& v)
{
	write_double(out, v.x);
	write_double(out, v.y);
	write_double(out, v.z);
}

inline quaternion read_quaternion(std::istream& in)
{
	double s = read_double(in);
	return quaternion(s, read_vector3(in));
}

inline void write_quaternion(std::ostream& out, const quaternion& q)
{
	write_double(out, q.s);
	write_vector3(out, q.v);
}

#endif
