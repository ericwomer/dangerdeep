// Binary Stream Tools
// (C) 2002 Thorsten Jordan

#ifndef BINSTREAM_H
#define BINSTREAM_H

#include <SDL/SDL_types.h>
#include <iostream>
#include <string>
using namespace std;

inline void write_i8(ostream& os, signed char i)
{
	Sint8 ii = i;
	os.write((char*)&ii, 1);
}

inline void write_i16(ostream& os, short i)
{
	Sint16 ii = i;
	os.write((char*)&ii, 2);
}

inline void write_i32(ostream& os, int i)
{
	Sint32 ii = i;
	os.write((char*)&ii, 4);
}

inline void write_u8(ostream& os, unsigned char i)
{
	Uint8 ii = i;
	os.write((char*)&ii, 1);
}

inline void write_u16(ostream& os, unsigned short i)
{
	Uint16 ii = i;
	os.write((char*)&ii, 2);
}

inline void write_u32(ostream& os, unsigned int i)
{
	Uint32 ii = i;
	os.write((char*)&ii, 4);
}

inline signed char read_i8(istream& is)
{
	Sint8 i;
	is.read((char*)&i, 1);
	return i;
}

inline short read_i16(istream& is)
{
	Sint16 i;
	is.read((char*)&i, 2);
	return i;
}

inline int read_i32(istream& is)
{
	Sint32 i;
	is.read((char*)&i, 4);
	return i;
}

inline unsigned char read_u8(istream& is)
{
	Uint8 i;
	is.read((char*)&i, 1);
	return i;
}

inline unsigned short read_u16(istream& is)
{
	Uint16 i;
	is.read((char*)&i, 2);
	return i;
}

inline unsigned int read_u32(istream& is)
{
	Uint32 i;
	is.read((char*)&i, 4);
	return i;
}

/*
inline void write_int64(ostream& os, __int64 i)
{
	os.write((char*)&i, sizeof(__int64));
}

inline __int64 read_int64(istream& is)
{
	__int64 i;
//	assert(is.good());
	is.read((char*)&i, sizeof(__int64));
	return i;
}
*/

inline void write_bool(ostream& os, bool b)
{
	write_u8(os, b ? 1 : 0);
}

inline bool read_bool(istream& is)
{
	return (read_u8(is) != 0);
}

// float/double are IEEE standard, size is always 32/64 bits.
inline void write_float(ostream& os, float f)
{
	os.write((char*)&f, sizeof(float));
}

inline float read_float(istream& is)
{
	float f;
	is.read((char*)&f, sizeof(float));
	return f;
}

inline void write_string(ostream& os, const string& s)
{
	int l = s.size();
	write_i32(os, l);
	os.write((char*)s.c_str(), l+1);
}

inline string read_string(istream& is)
{
	int l = read_i32(is);
	char* c = new char [l+1];
	is.read(c, l+1);
	string s(c);
	delete c;
	return s;
}

#endif
