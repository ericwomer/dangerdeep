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
#include <stdexcept>


int framebufferobject::fbo_supported = -1;
bool framebufferobject::supported()
{
	if (fbo_supported < 0) {
		fbo_supported = sys().extension_supported("GL_EXT_frame_buffer_object") ? 1 : 0;
	}
	return fbo_supported == 1;
}



framebufferobject::framebufferobject(class texture* attachedtex)
	: id(0), mytex(attachedtex), bound(false)
{
	if (!attachedtex)
		throw std::invalid_argument("given null ptr to FBO c'tor");
	if (!supported())
		throw std::runtime_error("frame buffer objects are not supported!");
	glGenFramebuffersEXT(1, &id);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, id);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
				  GL_TEXTURE_2D, mytex->get_opengl_name(), 0);
	GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if(status != GL_FRAMEBUFFER_COMPLETE_EXT) {
		glDeleteFramebuffersEXT(1, &id);
		throw std::runtime_error("FBO initialization check failed");
	}
	// unbind for now
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}



framebufferobject::~framebufferobject()
{
	glDeleteFramebuffersEXT(1, &id);
}



void framebufferobject::bind()
{
	if (bound)
		throw std::runtime_error("FBO bind(): already bound!");
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, id);
	glPushAttrib(GL_VIEWPORT_BIT);
	glViewport(0, 0, mytex->get_gl_width(), mytex->get_gl_height());
	bound = true;
}



void framebufferobject::unbind()
{
	if (!bound)
		throw std::runtime_error("FBO unbind(): not bound yet!");
	glPopAttrib();
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	bound = false;
}
