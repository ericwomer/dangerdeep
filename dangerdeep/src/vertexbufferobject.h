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

// Vertex Buffer Object
// (C)+(W) by Thorsten Jordan. See LICENSE

#ifndef VERTEXBUFFER_H
#define VERTEXBUFFER_H

#ifdef USE_NATIVE_GL
#include <gl.h>
#else
#include "oglext/OglExt.h"
#endif

///> this class handles a OpenGL Vertex Buffer Object
///> Note! older cards support at max 64k vertices (Geforce4MX), but it doesn't make sense
///> to use more than 64k vertices per buffer in most cases. One could use GL_UNSIGNED_SHORT
///> as index format then, which saves memory and copy bandwidth.
class vertexbufferobject
{
	GLuint id;
	unsigned size;
	bool mapped;
	int target;
 public:
	///> create buffer. Tell the handler if you wish to store indices or other data.
	vertexbufferobject(bool indexbuffer = false);
	///> free buffer
	~vertexbufferobject();
	///> call ONCE after creation, to set data size and optionally data.
	///> pointer to data can be NULL to just (re)size the buffer.
	///> usage can be one of GL_STREAM_DRAW_ARB, GL_STREAM_READ_ARB,
	///> GL_STREAM_COPY_ARB, GL_STATIC_DRAW_ARB, GL_STATIC_READ_ARB,
	///> GL_STATIC_COPY_ARB, GL_DYNAMIC_DRAW_ARB, GL_DYNAMIC_READ_ARB,
	///> GL_DYNAMIC_COPY_ARB
	void init_data(unsigned size, const void* data, int usage);
	///> init/set sub data
	void init_sub_data(unsigned offset, unsigned subsize, const void* data);
	///> bind buffer
	void bind() const;
	///> unbind buffer
	void unbind() const;
	///> map buffer to address, access is one of GL_READ_ONLY_ARB, GL_WRITE_ONLY_ARB,
	///> GL_ERAD_WRITE_ARB.
	void* map(int access);
	///> get size of mapped space
	unsigned get_map_size() const { return size; }
	///> unmap buffer
	void unmap();
};

#endif
