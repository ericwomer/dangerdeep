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

// a model viewer
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#define DATADIR "./data/"
#endif

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "system.h"
#include "cfg.h"
#include "vector3.h"
#include "datadirs.h"
#include "font.h"
#include "model.h"
#include "texture.h"
#include "image.h"
#include "make_mesh.h"
#include "xml.h"
#include "filehelper.h"
#include "widget.h"
#include "objcache.h"
#include "log.h"
#include "primitives.h"
#include <glu.h>
#include <SDL.h>

#include "mymain.cpp"

using std::vector;

const char* glenum2str( int fmt )
{
	switch( fmt )
	{
		case GL_ALPHA: return "GL_ALPHA:";
		case GL_ALPHA4: return "GL_ALPHA4:";
		case GL_ALPHA8: return "GL_ALPHA8:";
		case GL_ALPHA12: return "GL_ALPHA12:";
		case GL_ALPHA16: return "GL_ALPHA16:";
		case GL_LUMINANCE: return "GL_LUMINANCE:";
		case GL_LUMINANCE4: return "GL_LUMINANCE4:";
		case GL_LUMINANCE8: return "GL_LUMINANCE8:";
		case GL_LUMINANCE12: return "GL_LUMINANCE12:";
		case GL_LUMINANCE16: return "GL_LUMINANCE16:";
		case GL_LUMINANCE_ALPHA: return "GL_LUMINANCE_ALPHA:";
		case GL_LUMINANCE4_ALPHA4: return "GL_LUMINANCE4_ALPHA4:";
		case GL_LUMINANCE6_ALPHA2: return "GL_LUMINANCE6_ALPHA2:";
		case GL_LUMINANCE8_ALPHA8: return "GL_LUMINANCE8_ALPHA8:";
		case GL_LUMINANCE12_ALPHA4: return "GL_LUMINANCE12_ALPHA4:";
		case GL_LUMINANCE12_ALPHA12: return "GL_LUMINANCE12_ALPHA12:";
		case GL_LUMINANCE16_ALPHA16: return "GL_LUMINANCE16_ALPHA16:";
		case GL_INTENSITY: return "GL_INTENSITY:";
		case GL_INTENSITY4: return "GL_INTENSITY4:";
		case GL_INTENSITY8: return "GL_INTENSITY8:";
		case GL_INTENSITY12: return "GL_INTENSITY12:";
		case GL_INTENSITY16: return "GL_INTENSITY16:";
		case GL_R3_G3_B2: return "GL_R3_G3_B2:";
		case GL_RGB: return "GL_RGB:";
		case GL_RGB4: return "GL_RGB4:";
		case GL_RGB5: return "GL_RGB5:";
		case GL_RGB8: return "GL_RGB8:";
		case GL_RGB10: return "GL_RGB10:";
		case GL_RGB12: return "GL_RGB12:";
		case GL_RGB16: return "GL_RGB16:";
		case GL_RGBA: return "GL_RGBA:";
		case GL_RGBA2: return "GL_RGBA2:";
		case GL_RGBA4: return "GL_RGBA4:";
		case GL_RGB5_A1: return "GL_RGB5_A1:";
		case GL_RGBA8: return "GL_RGBA8:";
		case GL_RGB10_A2: return "GL_RGB10_A2:";
		case GL_RGBA12: return "GL_RGBA12:";
		case GL_RGBA16: return "GL_RGBA16:";
		case GL_COMPRESSED_ALPHA_ARB: return "GL_COMPRESSED_ALPHA_ARB:";
		case GL_COMPRESSED_LUMINANCE_ARB: return "GL_COMPRESSED_LUMINANCE_ARB:";
		case GL_COMPRESSED_LUMINANCE_ALPHA_ARB: return "GL_COMPRESSED_LUMINANCE_ALPHA_ARB:";
		case GL_COMPRESSED_INTENSITY_ARB: return "GL_COMPRESSED_INTENSITY_ARB:";
		case GL_COMPRESSED_RGB_ARB: return "GL_COMPRESSED_RGB_ARB:";
		case GL_COMPRESSED_RGBA_ARB: return "GL_COMPRESSED_RGBA_ARB:";

		default: return "Unknown";
	}
}

int mymain(list<string>& args)
{
	int res_x, res_y, res_area_2d_w, res_area_2d_h, res_area_2d_x, res_area_2d_y;
//	font* font_arial = 0;

	res_x = 640;
	res_y = res_x*3/4;

	// compute 2d area and resolution. it must be 4:3 always.
	if (res_x * 3 >= res_y * 4) {
		// screen is wider than high
		res_area_2d_w = res_y * 4 / 3;
		res_area_2d_h = res_y;
		res_area_2d_x = (res_x - res_area_2d_w) / 2;
		res_area_2d_y = 0;
	} else {
		// screen is higher than wide
		res_area_2d_w = res_x;
		res_area_2d_h = res_x * 3 / 4;
		res_area_2d_x = 0;
		res_area_2d_y = (res_y - res_area_2d_h) / 2;
		// maybe limit y to even lines for interlaced displays?
		//res_area_2d_y &= ~1U;
	}

	// parse configuration
	cfg& mycfg = cfg::instance();
	mycfg.register_option("screen_res_x", 1024);
	mycfg.register_option("screen_res_y", 768);
	mycfg.register_option("fullscreen", true);
	mycfg.register_option("debug", false);
	mycfg.register_option("sound", true);
	mycfg.register_option("use_hqsfx", true);
	mycfg.register_option("use_ani_filtering", false);
	mycfg.register_option("anisotropic_level", 1.0f);
	mycfg.register_option("use_compressed_textures", false);
	mycfg.register_option("multisampling_level", 0);
	mycfg.register_option("use_multisampling", false);
	mycfg.register_option("bloom_enabled", false);
	mycfg.register_option("hdr_enabled", false);
	mycfg.register_option("hint_multisampling", 0);
	mycfg.register_option("hint_fog", 0);
	mycfg.register_option("hint_mipmap", 0);
	mycfg.register_option("hint_texture_compression", 0);
	mycfg.register_option("vsync", false);
	mycfg.register_option("water_detail", 128);
	mycfg.register_option("wave_fft_res", 128);
	mycfg.register_option("wave_phases", 256);
	mycfg.register_option("wavetile_length", 256.0f);
	mycfg.register_option("wave_tidecycle_time", 10.24f);
	mycfg.register_option("usex86sse", true);
	mycfg.register_option("language", 0);
	mycfg.register_option("cpucores", 1);
	mycfg.register_option("terrain_texture_resolution", 0.1f);

	system::parameters params(1.0, 1000.0, res_x, res_y, false);
	system::create_instance(new class system(params));
	sys().set_res_2d(640, 480);
	sys().set_max_fps(25);
	
	log_info("A simple 2D test for ATI cards");

	GLfloat lambient[4] = {0.1,0.1,0.09,1};
	GLfloat ldiffuse[4] = {1,1,0.9,1};
	GLfloat lspecular[4] = {1,1,0.9,1};
	glLightfv(GL_LIGHT0, GL_AMBIENT, lambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, ldiffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, lspecular);
	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);

	glClearColor( 0.0, 1.0, 0.0, 1.0 );
	glClearDepth( 1.0 );

	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	sdl_image test_img( get_image_dir() + "sv_r2c2_red_pos8.jpg" );
	colorf col(1,1,1,1);
	texture *test_texture = new texture( test_img, 0, 0, test_img->w, test_img->h, texture::NEAREST, texture::CLAMP );
	texture *test_texture2 = new texture( test_img, 0, 0, test_img->w, test_img->h, texture::LINEAR, texture::CLAMP );
	texture *test_texture3 = new texture( test_img, 0, 0, test_img->w, test_img->h, texture::NEAREST_MIPMAP_NEAREST, texture::CLAMP );

	log_warning("T1 FMT = " << glenum2str( test_texture->get_format()));
	log_warning("T2 FMT = " << glenum2str( test_texture2->get_format()));
	log_warning("T3 FMT = " << glenum2str( test_texture3->get_format()));

	static const char* vs1 =
		"void main(){\n"
		"gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;\n"
		"gl_Position = ftransform();\n"
		"}\n";

	static const char* fs1 =
		"uniform sampler2D tex;\n"
		"void main(){\n"
		"gl_FragColor = texture2D(tex, gl_TexCoord[0].st);\n"
		"}\n";

	std::string vss1(vs1);
	std::string fss1(fs1);

	glsl_shader::defines_list dl;
	glsl_shader_setup *glsl1 = new glsl_shader_setup(vss1, fss1, dl, true);

	glsl1->use();
	unsigned loc_tex = glsl1->get_uniform_location("tex");



	{
		glFlush();
		glViewport(res_area_2d_x, res_area_2d_y, res_area_2d_w, res_area_2d_h);
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		gluOrtho2D(0, res_area_2d_w, 0, res_area_2d_h);
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();
		glTranslatef(0, res_area_2d_h, 0);
		glScalef(1, -1, 1);
		glDisable(GL_DEPTH_TEST);
		glCullFace(GL_FRONT);
		glPixelZoom(float(res_area_2d_w)/res_area_2d_w, -float(res_area_2d_h)/res_area_2d_h);	// flip images
		glDisable(GL_LIGHTING);
		glDisable(GL_CULL_FACE);

		test_texture->draw( 0, 0, col );
//		test_texture2->draw( 200, 200, col );
		test_texture3->draw( 0, 200, col );
//
//
//
		vector3f vertices[4];
		vector2f texcoords[4];

		vertices[0].x = 300;
		vertices[0].y = 300;
		vertices[1].x = 493;
		vertices[1].y = 300;
		vertices[2].x = 493;
		vertices[2].y = 450;
		vertices[3].x = 300;
		vertices[3].y = 450;

		texcoords[0].x = 0;
		texcoords[0].y = 0;
		texcoords[1].x = 1;
		texcoords[1].y = 0;
		texcoords[2].x = 1;
		texcoords[2].y = 1;
		texcoords[3].x = 0;
		texcoords[3].y = 1;

		glVertexPointer(3, GL_FLOAT, sizeof(vector3f), &vertices[0]);




		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2, GL_FLOAT, sizeof(vector2f), &texcoords[0]);

		glsl1->use();
		glsl1->set_gl_texture(*test_texture2, loc_tex, 0);

		glDrawArrays(GL_QUADS, 0, 4);

		glDisableClientState(GL_TEXTURE_COORD_ARRAY);

		primitives::quad( vector2f( 20, 20 ), vector2f( 40, 40 ), colorf( 1, 0, 0 ) ).render();
		primitives::quad( vector2f( 40, 20 ), vector2f( 60, 40 ), colorf( 0, 1, 0 ) ).render();
		primitives::quad( vector2f( 60, 20 ), vector2f( 80, 40 ), colorf( 0, 0, 1 ) ).render();

		glFlush();
		glPixelZoom(1.0f, 1.0f);
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
		glEnable(GL_DEPTH_TEST);
		glCullFace(GL_BACK);
		glEnable(GL_LIGHTING);

		sys().swap_buffers();

		SDL_Delay( 5000 );
	}

	delete test_texture;
	delete glsl1;

	system::destroy_instance();

	return 0;
}
