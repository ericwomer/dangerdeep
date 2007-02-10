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

//	model* mdlgear = new model(get_model_dir() + "gear.3ds", true, false);
//	bool ok = modelcache.ref("gear.3ds", mdlgear);
//	sys().myassert(ok, "weird error");
	
//	mdlgear->get_mesh(0).mymaterial->col = color(255,255,255);

	int lineheight = font_arial->get_height();
	int lines_per_page = (768+lineheight-1)/lineheight;
	int textpos = -lines_per_page;
	int textlines = 0;
	for ( ; credits[textlines] != 0; ++textlines);
	int textendpos = textlines;
	float lineoffset = 0.0f;

	float lposition[4] = {200, 0, 0, 1};

	bool quit = false;
	float ang = 0.0f, ang_per_sec = 10.0f, r = 78, lines_per_sec = 2, d = -150;
	unsigned tm = sys().millisec();
	while (!quit) {
		sys().poll_event_queue();
		if (sys().get_key().sym == SDLK_ESCAPE) quit = true;

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glPushMatrix();
		glLoadIdentity();
		glLightfv(GL_LIGHT0, GL_POSITION, lposition);
		glRotatef(ang/10, 0, 0, 1);

		glPushMatrix();
		glTranslatef(-r,r,d);
		glRotatef(ang, 0, 0, 1);
//		mdlgear->display();
		glPopMatrix();

		glPushMatrix();
		glTranslatef(r,-r,d);
		glRotatef(4.05-ang, 0, 0, 1);
//		mdlgear->display();
		glPopMatrix();
		
		glPopMatrix();

		sys().prepare_2d_drawing();
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
		ang += ang_per_sec*(tm2-tm)/1000.0f;
		tm = tm2;
		sys().swap_buffers();
	}
	
//	modelcache.unref("gear.3ds");
	
	glClearColor(0, 0, 1, 0);
}
