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

#include "framebufferobject.h"
#include "oglext/OglExt.h"
#include "texture.h"
#include "system.h"
#include "log.h"
#include <stdexcept>


int framebufferobject::fbo_supported = -1;
bool framebufferobject::supported()
{
	if (fbo_supported < 0) {
		fbo_supported = sys().extension_supported("GL_EXT_framebuffer_object") ? 1 : 0;
	}
	return fbo_supported == 1;
}



framebufferobject::framebufferobject(class texture& attachedtex, bool withdepthbuffer)
	: id(0), depthbuf_id(0), mytex(attachedtex), bound(false)
{
	if (!supported())
		throw std::runtime_error("frame buffer objects are not supported!");

	// create and bind depth buffer if requested
	if (withdepthbuffer) {
		glGenRenderbuffersEXT(1, &depthbuf_id);
		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, depthbuf_id);
		glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT,
					 mytex.get_gl_width(), mytex.get_gl_height());
	}

	// create and bind FBO
	glGenFramebuffersEXT(1, &id);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, id);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
				  GL_TEXTURE_2D, mytex.get_opengl_name(), 0);

	// attach depth buffer if requested
	if (withdepthbuffer) {
		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
					     GL_RENDERBUFFER_EXT, depthbuf_id);
	}

	GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if(status != GL_FRAMEBUFFER_COMPLETE_EXT) {
		destroy();
		log_warning( "FBO initialization check failed: " << init_failure_reason( status ) );
		throw std::runtime_error("FBO initialization check failed");
	}
	// unbind for now
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);
}

const char* framebufferobject::init_failure_reason( int status )
{
	switch( status )
	{
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
			return "Attachment";

		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
			return "Missing attachment";

		case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
			return "Incorrect dimensions";

		case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
			return "Incorrect formats";

		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
			return "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT";

		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
			return "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT";

		case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
			return "GL_FRAMEBUFFER_UNSUPPORTED_EXT";

		default:
			return "Unknown";
	}
}


framebufferobject::~framebufferobject()
{
	destroy();
}



void framebufferobject::destroy()
{
	if (id)
		glDeleteFramebuffersEXT(1, &id);
	if (depthbuf_id)
		glDeleteRenderbuffersEXT(1, &depthbuf_id);
}



void framebufferobject::bind() const
{
	if (bound)
		throw std::runtime_error("FBO bind(): already bound!");
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, id);
	glPushAttrib(GL_VIEWPORT_BIT);
	glViewport(0, 0, mytex.get_gl_width(), mytex.get_gl_height());
	bound = true;
}



void framebufferobject::unbind() const
{
	if (!bound)
		throw std::runtime_error("FBO unbind(): not bound yet!");
	glPopAttrib();
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	bound = false;
}
