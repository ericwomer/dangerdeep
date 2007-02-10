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

#include "texture.h"
#include "font.h"
#include "system.h"
#include "shader.h"
#include "datadirs.h"

// instead of including global_data.h
extern font* font_arial;



const char* credits[] = {
"$80ffc0Project idea and initial code",
"$ffffffThorsten Jordan",
"", "", "", "", "",
"$80ffc0Program",
"$ffffffThorsten Jordan",
"Markus Petermann",
"Viktor Radnai",
"Andrew Rice",
"Alexandre Paes",
"Matt Lawrence",
"Michael Kieser",
"Renato Golin",
"Hiten Parmar",
"", "", "", "", "",
"$80ffc0Graphics",
"$ffffffLuis Barrancos",
"Markus Petermann",
"Christian Kolaß",
"Thorsten Jordan",
"", "", "", "", "",
"$80ffc0Models",
"$ffffffLuis Barrancos",
"Christian Kolaß",
"Thorsten Jordan",
"", "", "", "", "",
"$80ffc0Music",
"$ffffffMartin Alberstadt",
"", "", "", "", "",
"$80ffc0Operating system adaption",
"$ffffffNico Sakschewski (Win32)",
"Andrew Rice (MacOSX)",
"Jose Alonso Cardenas Marquez (acm) (FreeBSD)",
"", "", "", "", "",
"$80ffc0Web site administrator",
"$ffffffAlexandre Paes",
"$ffffffViktor Radnai",
"", "", "", "", "",
"$80ffc0Packaging",
"$ffffffMarkus Petermann (SuSE rpm)",
"Viktor Radnai (Debian)",
"Giuseppe Borzi (Mandrake rpm)",
"Michael Kieser (WIN32-Installer)",
"", "", "", "", "",
"$80ffc0Bug reporting and thanks",
"$ffffffRick McDaniel",
"Markus Petermann",
"Viktor Radnai",
"Christian Kolaß",
"Nico Sakschewski",
"Martin Butterweck",
"Bernhard Kaindl",
"Robert Obryk",
"Giuseppe Lipari",
"John Hopkin",
"Michael Wilkinson",
"Lee Close",
"Christopher Dean (Naval Warfare Simulations, Sponsoring)",
"Arthur Anker",
"Stefan Vilijoen",
"Luis Barrancos",
"Tony Becker",
"Frank Kaune",
"Paul Marks",
"Aaron Kulkis",
"Giuseppe Borzi",
"Andrew Rice",
"Alexandre Paes",
"...",
"...and all i may have forgotten here (write me!)",
"(no bockwursts were harmed in the making of this game).",
0
};



// ------------------------------------------- code ---------------------------------------

void show_credits()
{
	glClearColor(0.1,0.25,0.4,0);

	std::auto_ptr<texture> bkg;
	std::auto_ptr<glsl_shader_setup> glss;
	bool use_shaders = glsl_program::supported();
	if (use_shaders) {
		const unsigned sz = 16;
		std::vector<Uint8> data(sz*sz);
		for (unsigned y = 0; y < sz; ++y)
			for (unsigned x = 0; x < sz; ++x)
				data[y*sz+x] = rand() & 0xff;
		bkg.reset(new texture(data, sz, sz, GL_LUMINANCE, texture::LINEAR, texture::REPEAT));
		glss.reset(new glsl_shader_setup(get_shader_dir() + "credits.vshader",
						 get_shader_dir() + "credits.fshader"));
	}

	int lineheight = font_arial->get_height();
	int lines_per_page = (768+lineheight-1)/lineheight;
	int textpos = -lines_per_page;
	int textlines = 0;
	for ( ; credits[textlines] != 0; ++textlines);
	int textendpos = textlines;
	float lineoffset = 0.0f;

	float lposition[4] = {200, 0, 0, 1};

	bool quit = false;
	float lines_per_sec = 2;
	float ctr = 0.0, ctradd = 1.0/32.0;
	unsigned tm = sys().millisec();
	while (!quit) {
		sys().poll_event_queue();
		if (sys().get_key().sym == SDLK_ESCAPE) quit = true;

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		sys().prepare_2d_drawing();

		if (use_shaders) {
			glss->use();
			glColor4f(0.2,0.8,1,1);
			bkg->set_gl_texture();
			glBegin(GL_QUADS);
			glTexCoord2f(0+ctr,0);
			glMultiTexCoord2f(GL_TEXTURE1_ARB, ctr, ctr*0.5);
			glVertex2i(0,768);
			glTexCoord2f(1.33333+ctr,0);
			glMultiTexCoord2f(GL_TEXTURE1_ARB, ctr, ctr*0.5);
			glVertex2i(1024,768);
			glTexCoord2f(1.33333+ctr,1);
			glMultiTexCoord2f(GL_TEXTURE1_ARB, ctr, ctr*0.5);
			glVertex2i(1024,0);
			glTexCoord2f(0+ctr,1);
			glMultiTexCoord2f(GL_TEXTURE1_ARB, ctr, ctr*0.5);
			glVertex2i(0,0);
			glEnd();
			glBindTexture(GL_TEXTURE_2D, 0);
			glss->use_fixed();
		}

		for (int i = textpos; i <= textpos+lines_per_page; ++i) {
			if (i >= 0 && i < textlines) {
				font_arial->print_hc(512, (i-textpos)*lineheight+int(-lineoffset*lineheight), credits[i], color::white(), true);
			}
		}
		sys().unprepare_2d_drawing();

		unsigned tm2 = sys().millisec();
		lineoffset += lines_per_sec*(tm2-tm)/1000.0f;
		int tmp = int(lineoffset);
		lineoffset -= tmp;
		textpos += tmp;
		if (textpos >= textendpos) textpos = -lines_per_page;
		ctr += ctradd * (tm2-tm)/1000.0f;
		tm = tm2;
		sys().swap_buffers();
	}
	
	glClearColor(0, 0, 1, 0);
}
