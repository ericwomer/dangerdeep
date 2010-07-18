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


#include <cstdio>
#include "shader.h"
#include "texture.h"
#include "postprocessor.h"
#include "datadirs.h"
#include "system.h"
#include "cfg.h"
#include "log.h"

#ifdef WIN32
// maybe one day...
#ifndef snprintf
#define snprintf _snprintf
#endif
#endif

using namespace std;

bool postprocessor::bloom_enabled = false;
bool postprocessor::hdr_enabled = false;
bool postprocessor::use_hqsfx = false;

postprocessor::postprocessor() :	filter( get_shader_dir() + "postp_null.vshader", get_shader_dir() + "postp_filter.fshader" ),
					combine( get_shader_dir() + "postp_null.vshader", get_shader_dir() + "postp_combine.fshader" ),
					combine2( get_shader_dir() + "postp_null.vshader", get_shader_dir() + "postp_combine2.fshader" ),
					hipass( get_shader_dir() + "postp_null.vshader", get_shader_dir() + "postp_highpass.fshader" )

{
	unsigned w = sys().get_res_x();
	unsigned h = sys().get_res_y();

	//unused//unsigned pp_fx = cfg::instance().geti("postprocessing");

	switch( cfg::instance().geti("postprocessing") )
	{
		case 1:
			bloom_enabled = true;
		break;

		case 2:
			hdr_enabled = true;
		break;

		default:
			// not really needed
			bloom_enabled = false;
			hdr_enabled = false;
	}

	use_hqsfx = (cfg::instance().geti("sfx_quality") >= 1);

	// only alloc buffers if needed
	if ( bloom_enabled || hdr_enabled )
	{
		for( unsigned ix=0; ix < PP_FILTERS; ix++ )
		{
			h_textures[ ix ] = new texture( w >> ix, h >> ix, GL_RGB, texture::LINEAR, texture::CLAMP, true );
			v_textures[ ix ] = new texture( w >> ix, h >> ix, GL_RGB, texture::LINEAR, texture::CLAMP, true );

			h_pass[ ix ] = new framebufferobject( *h_textures[ ix ], false );
			v_pass[ ix ] = new framebufferobject( *v_textures[ ix ], false );

		}

		// only use floats if doing HDR
		scene_t = new texture( w, h, ( hdr_enabled ) ? GL_RGB16F_ARB : GL_RGB, texture::LINEAR, texture::CLAMP, true );
		scene_fbo = new framebufferobject( *scene_t, true );
	}
}

postprocessor::~postprocessor()
{
	// only free buffers if alloc'd
	if ( bloom_enabled || hdr_enabled )
	{
		for( unsigned ix=0; ix < PP_FILTERS; ix++ )
		{
			delete h_textures[ix];
			delete v_textures[ix];
			delete h_pass[ ix ];
			delete v_pass[ ix ];
		}

		delete scene_t;
		delete scene_fbo;
	}
}

void postprocessor::render2texture()
{
	// only run if enabled otherwise take a 20% performance hit
	if ( !( bloom_enabled || hdr_enabled ) )
		return;

	scene_fbo->bind();
}

void postprocessor::render_quad()
{
	glBegin(GL_QUADS); 
		glTexCoord2i(0, 0); glVertex2i( -1, -1);
		glTexCoord2i(1, 0); glVertex2i( 1, -1);
		glTexCoord2i(1, 1); glVertex2i( 1, 1);
		glTexCoord2i(0, 1); glVertex2i( -1, 1);
	glEnd();
}

void postprocessor::do_2d()
{
//	using sys().prepare_2d_drawing() causes headaches (flipping/sizes), so we'll just stick with this for now.
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
}

void postprocessor::undo_2d()
{
	glFlush();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
}

void postprocessor::bloom()
{
	unsigned loc;

	// switch to 2D
	do_2d();

	glsl_shader_setup::default_tex->use();
	glsl_shader_setup::default_tex->set_uniform(glsl_shader_setup::loc_t_color, colorf( color::white() ) );
	glsl_shader_setup::default_tex->set_gl_texture( *scene_t, glsl_shader_setup::loc_t_tex, 0);
	
	// down-sample/size from initial fbo
	for( unsigned ix = 0; ix < PP_FILTERS; ix++ )
	{
		h_pass[ ix ]->bind();
		render_quad();
		h_pass[ ix ]->unbind();
	}

	// hqfx
	if ( use_hqsfx )
	{
		unsigned loc_off;
		filter.use();

		loc = filter.get_uniform_location( "source" );
		filter.set_uniform( loc, 0 );

		loc = filter.get_uniform_location( "coefficients" );
		filter.set_uniform( loc, vector3f( 5.0 / 16.0, 6.0 / 16.0, 5.0 / 16.0 ) ); // normalised kernel parms

		loc = filter.get_uniform_location( "offsety" );
		filter.set_uniform( loc, 0.0 );

		loc_off = filter.get_uniform_location( "offsetx" );
		filter.set_uniform( loc_off, 0.0 );

		for( unsigned ix = 0; ix < PP_FILTERS; ix++ )
		{
			float offset = 1.2f / (float)h_textures[ ix ]->get_width();
			filter.set_uniform( loc_off, offset );

			v_pass[ ix ]->bind();

			h_textures[ ix ]->set_gl_texture();
			render_quad();

			v_pass[ ix ]->unbind();
		}

		loc = filter.get_uniform_location( "offsetx" );
		filter.set_uniform( loc, 0.0 );

		loc_off = filter.get_uniform_location( "offsety" );
		filter.set_uniform( loc_off, 0.0 );

		for( unsigned ix = 0; ix < PP_FILTERS; ix++ )
		{
			float offset = 1.2f / (float)h_textures[ ix ]->get_height();
			filter.set_uniform( loc_off, offset );

			h_pass[ ix ]->bind();

			v_textures[ ix ]->set_gl_texture();
			render_quad();

			h_pass[ ix ]->unbind();
		}
	}

	// setup shader
	combine.use();
	
	loc = combine.get_uniform_location( "bkgd" );
	combine.set_uniform( loc, colorf( color::black() ) );

	char uniform[6] = "Pass?";

	// set texture values
	for( unsigned ix = 0; ix < PP_FILTERS; ix++ )
	{
		snprintf( uniform, 6, "Pass%d", ix );

		loc = combine.get_uniform_location( uniform );

		combine.set_gl_texture( *h_textures[ ix ], loc, ix );
	}

	// render final img
	render_quad();

	undo_2d();

}

void postprocessor::hdr()
{
	unsigned loc;

	// lqfx
	do_2d();

	// filter pass
	hipass.use();

	loc = hipass.get_uniform_location( "source" );
	hipass.set_gl_texture( *scene_t, loc, 0 );

	h_pass[0]->bind();

	render_quad();

	h_pass[0]->unbind();


	// down-sample/size from initial fbo
	glsl_shader_setup::default_tex->use();
	glsl_shader_setup::default_tex->set_uniform(glsl_shader_setup::loc_t_color, colorf( color::white() ) );
	glsl_shader_setup::default_tex->set_gl_texture( *h_textures[0], glsl_shader_setup::loc_t_tex, 0);
	
	for( unsigned ix = 1; ix < PP_FILTERS; ix++ )
	{
		h_pass[ ix ]->bind();
		render_quad();
		h_pass[ ix ]->unbind();
	}

	// hqfx
	if ( use_hqsfx )
	{
		unsigned loc_off;
		filter.use();

		loc = filter.get_uniform_location( "source" );
		filter.set_uniform( loc, 0 );

		loc = filter.get_uniform_location( "coefficients" );
		filter.set_uniform( loc, vector3f( 5.0 / 16.0, 6.0 / 16.0, 5.0 / 16.0 ) ); // normalised kernel parms

		loc = filter.get_uniform_location( "offsety" );
		filter.set_uniform( loc, 0.0 );

		loc_off = filter.get_uniform_location( "offsetx" );
		filter.set_uniform( loc_off, 0.0 );

		for( unsigned ix = 0; ix < PP_FILTERS; ix++ )
		{
			float offset = 1.2f / (float)h_textures[ ix ]->get_width();
			filter.set_uniform( loc_off, offset );

			v_pass[ ix ]->bind();
		
			h_textures[ ix ]->set_gl_texture();
			render_quad();

			v_pass[ ix ]->unbind();
		}

		loc = filter.get_uniform_location( "offsetx" );
		filter.set_uniform( loc, 0.0 );

		loc_off = filter.get_uniform_location( "offsety" );
		filter.set_uniform( loc_off, 0.0 );

		for( unsigned ix = 0; ix < PP_FILTERS; ix++ )
		{
			float offset = 1.2f / (float)h_textures[ ix ]->get_height();
			filter.set_uniform( loc_off, offset );

			h_pass[ ix ]->bind();

			v_textures[ ix ]->set_gl_texture();
			render_quad();

			h_pass[ ix ]->unbind();
		}
	}

	// setup shader
	combine2.use();
	
	loc = combine2.get_uniform_location( "bkgd" );
	combine2.set_uniform( loc, colorf( color::black() ) );

	char uniform[6] = "Pass?";

	// set texture values
	for( unsigned ix = 0; ix < PP_FILTERS; ix++ )
	{
		snprintf( uniform, 6, "Pass%d", ix );

		loc = combine2.get_uniform_location( uniform );

		combine2.set_gl_texture( *(h_textures[ ix ]), loc, ix );
	}

	// include the original
	loc = combine2.get_uniform_location( "hipass" );
	combine2.set_gl_texture( *scene_t, loc, PP_FILTERS );

	// render final img
	render_quad();

	undo_2d();

	// debug
#if 0
	system::sys().prepare_2d_drawing();
	h_textures[0]->draw(0, 0, 256, 256);
	system::sys().unprepare_2d_drawing();

#endif
}

void postprocessor::process()
{

	if ( !( bloom_enabled || hdr_enabled ) )
		return;
	
	// stop rendering into the scene buffer
	scene_fbo->unbind();

	// TODO: use a scalar instead of booleans ?
	if ( bloom_enabled )
		bloom();
	else
		hdr();

}

