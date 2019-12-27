/*
Danger from the Deep - Open source submarine simulation
Copyright (C) 2009  Matthew Lawrence, Thorsten Jordan, 
Luis Barrancos and others.

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

#ifndef POSTPROCESSING_H
#define POSTPROCESSING_H

#include "singleton.h"
#include "framebufferobject.h"
//#include "texture.h"
//#include "shader.h"

#define PP_FILTERS 4
#define PP_BUFFER_WIDTH 128
#define PP_BUFFER_HEIGHT PP_BUFFER_WIDTH

class postprocessor : public singleton<class postprocessor>
{
	friend class singleton<postprocessor>;

	public:
		postprocessor();
		~postprocessor();

		static bool bloom_enabled;
		static bool hdr_enabled;

		void render2texture();
		void process();

	private:
		glsl_shader_setup filter, combine, combine2, hipass;

		framebufferobject *h_pass[ PP_FILTERS ], *v_pass[ PP_FILTERS ], *scene_fbo;
		texture *h_textures[ PP_FILTERS ], *v_textures[ PP_FILTERS ], *scene_t;

		static bool use_hqsfx;

		void bloom();
		void hdr();

		void render_quad();
		void do_2d();
		void undo_2d();

};

#endif
