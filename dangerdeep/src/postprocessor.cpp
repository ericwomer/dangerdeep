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


#include "shader.h"
#include "texture.h"
#include "postprocessor.h"
#include "datadirs.h"
#include "system.h"

#include "log.h"

#define HIPASS_THRESH 0.99

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

	use_hqsfx = cfg::instance().getb("use_hqsfx");
	bloom_enabled = cfg::instance().getb("bloom_enabled");
	hdr_enabled = cfg::instance().getb("hdr_enabled");

	if ( bloom_enabled && hdr_enabled )
		throw runtime_error("can't have both bloom AND hdr");


	// only alloc buffers if needed
	if ( bloom_enabled || hdr_enabled )
	for( unsigned ix=0; ix < PP_FILTERS; ix++ )
	{
		h_textures[ ix ] =  new texture( w >> ix, h >> ix, GL_RGB, texture::LINEAR, texture::CLAMP );
		v_textures[ ix ] =  new texture( w >> ix, h >> ix, GL_RGB, texture::LINEAR, texture::CLAMP );

		h_pass[ ix ] = new framebufferobject( *h_textures[ ix ], 0 == ix ? true : false );
		v_pass[ ix ] = new framebufferobject( *v_textures[ ix ], false );

		if ( hdr_enabled )
		{
			hipass_t = new texture( w, h, GL_RGB, texture::LINEAR, texture::CLAMP );
			hipass_fbo = new framebufferobject( *hipass_t, false );
		}
	}
}

postprocessor::~postprocessor()
{
	// only free buffers if alloc'd
	if ( bloom_enabled || hdr_enabled )
	for( unsigned ix=0; ix < PP_FILTERS; ix++ )
	{
		delete h_textures[ix];
		delete v_textures[ix];
		delete h_pass[ ix ];
		delete v_pass[ ix ];

		if ( hdr_enabled )
		{
			delete hipass_t;
			delete hipass_fbo;
		}
	}
}

void postprocessor::render2texture()
{
	// only run if enabled otherwise take a 20% performance hit
	if ( !( bloom_enabled || hdr_enabled ) )
		return;
	
	// TODO: do we need to clear here? // yes for temp debugging
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	h_pass[0]->bind();
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
	glsl_shader_setup::default_tex->set_gl_texture( *h_textures[0], glsl_shader_setup::loc_t_tex, 0);
	
	// down-sample/size from initial fbo
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
	hipass.set_gl_texture( *h_textures[0], loc, 0 );
	
	loc = hipass.get_uniform_location( "thresh" );
	hipass.set_uniform( loc, HIPASS_THRESH );

	hipass_fbo->bind();

	render_quad();

	hipass_fbo->unbind();

	// down-sample/size from initial fbo
	glsl_shader_setup::default_tex->use();
	glsl_shader_setup::default_tex->set_uniform(glsl_shader_setup::loc_t_color, colorf( color::white() ) );
	glsl_shader_setup::default_tex->set_gl_texture( *hipass_t, glsl_shader_setup::loc_t_tex, 0);
	
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
		
			// hack: first render target is h_text0 so we want out hipass tex0 not the h_tex0
			if ( 0 == ix )
				hipass_t->set_gl_texture();
			else
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

			// hack: make sure we don't destroy our original render
			if ( 0 == ix )
				hipass_fbo->bind();
			else
				h_pass[ ix ]->bind();

			v_textures[ ix ]->set_gl_texture();
			render_quad();

			// hack: see above
			if ( 0 == ix )
				hipass_fbo->unbind();
			else
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
	combine2.set_gl_texture( *hipass_t, loc, PP_FILTERS );

	// render final img
	render_quad();

	undo_2d();
}

void postprocessor::process()
{

	if ( !( bloom_enabled || hdr_enabled ) )
		return;
	
	// stop rendering into the first buffer
	h_pass[0]->unbind();


	if ( bloom_enabled )
		bloom();
	else
		hdr();

}

