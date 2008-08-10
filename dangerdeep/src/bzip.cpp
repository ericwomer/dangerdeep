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

#include "bzip.h"
#include "keys.h"
#include <string.h>	//for memmove

bzip_streambuf::bzip_streambuf(std::ostream& os, int blocksize, int _workfactor, int _buffer_size)
: outstream(os), instream(), in_buffer(_buffer_size, 0), out_buffer(_buffer_size, 0), blocksize_100_k(blocksize), workfactor(_workfactor), buffer_size(_buffer_size), put_back(0), mode(COMPRESS)
{
    outstream.clear();

    bzstream.next_in = &in_buffer[0];
    bzstream.next_out = &out_buffer[0];
    bzstream.avail_in = 0;
    bzstream.avail_out = 0;
    bzstream.bzalloc = NULL;
    bzstream.bzfree = NULL;

    setp(&in_buffer.front(), &in_buffer[0] + buffer_size - 1);
    state = BZ2_bzCompressInit(&bzstream, blocksize, 0, workfactor);

    if (state < 0) throw bzip_failure(state);
}

bzip_streambuf::bzip_streambuf(std::istream& is, int _buffer_size, int _put_back, int small)
: outstream(), instream(is), in_buffer(_buffer_size, 0), out_buffer(_buffer_size + _put_back, 0), buffer_size(_buffer_size), put_back(_put_back), mode(DECOMPRESS)
{
    instream.clear();

    bzstream.next_in = &in_buffer[0];
    bzstream.next_out = &out_buffer[0];
    bzstream.avail_in = 0;
    bzstream.avail_out = 0;
    bzstream.bzalloc = NULL;
    bzstream.bzfree = NULL;

    state = BZ2_bzDecompressInit(&bzstream, 0, small);
    if (state < 0) throw bzip_failure(state);

    char *end = &out_buffer[0] + buffer_size + put_back;
    setg(end, end, end);
}

std::streambuf::int_type bzip_streambuf::overflow(int_type ch)
{
    if (ch != traits_type::eof()) {
        *pptr() = ch;
        pbump(1);

        bzstream.next_in = &in_buffer[0];
        bzstream.avail_in = buffer_size;

        do {
            bzstream.avail_out = out_buffer.size();
            bzstream.next_out = &out_buffer[0];
            state = BZ2_bzCompress(&bzstream, BZ_RUN);
            if (state < 0) throw bzip_failure(state);
            outstream.write(&(out_buffer[0]), out_buffer.size() - bzstream.avail_out);
        } while (state == BZ_RUN_OK && bzstream.avail_in != 0);
        
        setp(&in_buffer.front(), &in_buffer.front() + buffer_size - 1);
        return ch;
    }
    return traits_type::eof();
}

std::streambuf::int_type bzip_streambuf::underflow()
{
    if (gptr() < egptr()) // buffer not exhausted
        return traits_type::to_int_type(*gptr());

    char *base = &out_buffer[0];
    char *start = base;

    if (eback() == base) // true when this isn't the first fill
    {
        // Make arrangements for putback characters
        memmove(base, egptr() - put_back, put_back);
        start += put_back;
    }

    int num = bzip2stream(start, buffer_size + put_back - (start - base));
    
    if (num <= 0) return traits_type::eof();

    // Set buffer pointers
    setg(base, start, start + num);

    return traits_type::to_int_type(*gptr());

}

int bzip_streambuf::bzip2stream(char* start, int avail)
{
    bzstream.next_out = start;
    bzstream.avail_out = avail;
    
    do {
        
        if (bzstream.avail_in == 0) fill_buffer();
        state = BZ2_bzDecompress(&bzstream);
        if (state < 0) throw bzip_failure(state);

    } while (state == BZ_OK && bzstream.avail_out != 0);

    return avail-bzstream.avail_out;
}

int bzip_streambuf::fill_buffer()
{
    instream.read(&in_buffer[0], buffer_size);
    if (instream.fail() && !instream.eof()) throw std::ios_base::failure("read error");
   
    bzstream.next_in = &in_buffer[0];
    bzstream.avail_in = instream.gcount();
    
    return instream.gcount();
}

int bzip_streambuf::sync()
{
    if (pptr() && pptr() > pbase()) {
        int_type c = overflow(traits_type::eof());

        if (c == traits_type::eof())
            return -1;
    }
    return 0;
}

void bzip_streambuf::flush()
{
    bzstream.next_in = &in_buffer[0];
    bzstream.avail_in = pbase() - pptr();

    do {
        bzstream.avail_out = out_buffer.size();
        bzstream.next_out = &out_buffer[0];
        state = BZ2_bzCompress(&bzstream, BZ_FINISH);
        if (state < 0) throw bzip_failure(state);

        outstream.write(&(out_buffer[0]), out_buffer.size() - bzstream.avail_out);
    } while (state == BZ_FINISH_OK);

    outstream.flush();
}
