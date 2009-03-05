/*
 * Danger from the Deep - Open source submarine simulation
 * Copyright (C) 2003-2006  Thorsten Jordan, Luis Barrancos and others.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
 
#ifndef BITSTREAM_H
#define BITSTREAM_H

#include <SDL.h>
#include <iostream>
#include <vector>

#ifndef IS_BENDIAN
const int ENDIAN_TEST = 1;
#define IS_BENDIAN ((*(char*)&ENDIAN_TEST)==0)
#endif

class obitstream
{
private:
	static Uint8 bitmask[];
	
	unsigned long byte_pos, bit_pos;
	std::vector<Uint8> buffer;
	std::ostream* outstream;
	
	inline void to_buffer(Uint8 bits, Uint8 len);
	
public:
	obitstream(std::ostream* os, long bufsize = 128) : byte_pos(0), bit_pos(0), buffer(bufsize, 0), outstream(os) {
		if(buffer.size()<4) buffer.resize(4, 0);
	}
	~obitstream() { last_write(); }
	
	bool write(Uint8 bits, Uint8 len = 8);
	bool write(Uint16 bits, Uint8 len = 16);
	void flush();
	void last_write();
};

class ibitstream
{
private:
	static Uint8 bitmask[8];
	
	unsigned long byte_pos, bit_pos, end_pos;
	std::vector<Uint8> buffer;
	std::istream* instream;

	Uint8 read_byte(Uint8 len = 8);
	void inline fill_buffer();
	void inline update_position(Uint8& len);
	
public:
	ibitstream(std::istream* is, long bufsize = 128);
	~ibitstream() {}
	
	Uint16 read(Uint8 len = 16);
};
#endif