// Binary Stream Tools
// (C) 2002 Thorsten Jordan

#ifndef BINSTREAM_H
#define BINSTREAM_H

#include <SDL_types.h>
#include <SDL_endian.h>
#include <iostream>
#include <string>
#include "quaternion.h"
using namespace std;

// Data is stored in little endian mode.
// On big endian machines the data is converted for reading and writing.
// We use the SDL_SwapLE functions for both operations,
// because they are identical to their inverse functions
// on both type of machines.

// So far we assume sizeof(float)==4 and sizeof(double)==8 (IEEE standard!)
// The only system dependent assumption is that sizeof(int) >= 4 (32 bits).
// C(++) guarantees that short has at least 16 bits, long has at least 32,
// and int has some bit number between short and long.

inline void write_i8(ostream& out, signed char i)
{
	Sint8 ii = i;	// no LE/BE swap needed.
	out.write((char*)&ii, 1);
}

inline void write_i16(ostream& out, short i)
{
	Uint16 ii = SDL_SwapLE16((Uint16)i);
	out.write((char*)&ii, 2);
}

inline void write_i32(ostream& out, int i)
{
	Uint32 ii = SDL_SwapLE32((Uint32)i);
	out.write((char*)&ii, 4);
}

inline void write_i64(ostream& out, Sint64 i)
{
	Uint64 ii = SDL_SwapLE64((Uint64)i);
	out.write((char*)&ii, 8);
}

inline void write_u8(ostream& out, unsigned char i)
{
	Uint8 ii = i;	// no LE/BE swap needed.
	out.write((char*)&ii, 1);
}

inline void write_u16(ostream& out, unsigned short i)
{
	Uint16 ii = SDL_SwapLE16((Uint16)i);
	out.write((char*)&ii, 2);
}

inline void write_u32(ostream& out, unsigned int i)
{
	Uint32 ii = SDL_SwapLE32((Uint32)i);
	out.write((char*)&ii, 4);
}

inline void write_u64(ostream& out, Uint64 i)
{
	Uint64 ii = SDL_SwapLE64((Uint64)i);
	out.write((char*)&ii, 8);
}

inline signed char read_i8(istream& in)
{
	Sint8 i;	// no LE/BE swap needed.
	in.read((char*)&i, 1);
	return i;
}

inline short read_i16(istream& in)
{
	Uint16 i;
	in.read((char*)&i, 2);
	return short(SDL_SwapLE16(i));
}

inline int read_i32(istream& in)
{
	Sint32 i;
	in.read((char*)&i, 4);
	return int(SDL_SwapLE32(i));
}

inline Sint64 read_i64(istream& in)
{
	Sint64 i;
	in.read((char*)&i, 8);
	return Sint64(SDL_SwapLE64(i));
}

inline unsigned char read_u8(istream& in)
{
	Uint8 i;	// no LE/BE swap needed.
	in.read((char*)&i, 1);
	return i;
}

inline unsigned short read_u16(istream& in)
{
	Uint16 i;
	in.read((char*)&i, 2);
	return (unsigned short)(SDL_SwapLE16(i));
}

inline unsigned int read_u32(istream& in)
{
	Uint32 i;
	in.read((char*)&i, 4);
	return (unsigned int)(SDL_SwapLE32(i));
}

inline Uint64 read_u64(istream& in)
{
	Uint64 i;
	in.read((char*)&i, 8);
	return Uint64(SDL_SwapLE64(i));
}

inline void write_bool(ostream& out, bool b)
{
	write_u8(out, b ? 1 : 0);
}

inline bool read_bool(istream& in)
{
	return (read_u8(in) != 0);
}

inline void write_float(ostream& out, float f)
{
	write_u32(out, *(Uint32*)(&f));
}

inline float read_float(istream& in)
{
	Uint32 u = read_u32(in);
	return *(float*)(&u);
}

inline void write_double(ostream& out, double d)
{
	write_u64(out, *(Uint64*)(&d));
}

inline double read_double(istream& in)
{
	Uint64 u = read_u64(in);
	return *(double*)(&u);
}

inline void write_string(ostream& out, const string& s)
{
	unsigned l = s.size();
	write_u32(out, l);
	if (l > 0)
		out.write((char*)&s[0], l);
}

inline string read_string(istream& in)
{
	unsigned l = read_u32(in);
	if (l > 0) {
		string s(l, 'x');
		in.read(&s[0], l);
		return s;
	} else {
		return string();
	}
}

inline vector2 read_vector2(istream& in)
{
	vector2 v;
	v.x = read_double(in);
	v.y = read_double(in);
	return v;
}

inline void write_vector2(ostream& out, const vector2& v)
{
	write_double(out, v.x);
	write_double(out, v.y);
}

inline vector3 read_vector3(istream& in)
{
	vector3 v;
	v.x = read_double(in);
	v.y = read_double(in);
	v.z = read_double(in);
	return v;
}

inline void write_vector3(ostream& out, const vector3& v)
{
	write_double(out, v.x);
	write_double(out, v.y);
	write_double(out, v.z);
}

inline quaternion read_quaternion(istream& in)
{
	double s = read_double(in);
	return quaternion(s, read_vector3(in));
}

inline void write_quaternion(ostream& out, const quaternion& q)
{
	write_double(out, q.s);
	write_vector3(out, q.v);
}

#endif
