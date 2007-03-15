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

// Frame Buffer Object
// (C)+(W) by Thorsten Jordan. See LICENSE

#ifndef FRAMEBUFFEROBJECT_H
#define FRAMEBUFFEROBJECT_H

#ifdef USE_NATIVE_GL
#include <gl.h>
#else
#include "oglext/OglExt.h"
#endif

/// this class handles a OpenGL Frame Buffer Object
///@note needs the GL_EXT_framebuffer_object extension, that is available on
///	newer cards (GeForce5+), or on all cards supporting OpenGL 2.0
class framebufferobject
{
 public:
	/// create buffer object.
	 framebufferobject(class texture& attachedtex, bool withdepthbuffer = false);

	/// free buffer object
	~framebufferobject();

	/// bind buffer and set up rendering
	void bind() const;

	/// unbind buffer
	void unbind() const;

	/// check if FBOs are supported
	static bool supported();

 protected:
	GLuint id;
	GLuint depthbuf_id;
	class texture& mytex;
	mutable bool bound;	// for extra error checks
	static int fbo_supported;

	void destroy();

 private:
	framebufferobject();
	framebufferobject(const framebufferobject& );	// no copy
	framebufferobject& operator= (const framebufferobject&);
};

#endif
