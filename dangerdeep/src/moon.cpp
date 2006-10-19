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


#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <iostream>
using namespace std;

#include "oglext/OglExt.h"

#include "moon.h"
#include "matrix4.h"
#include "texture.h"
#include "datadirs.h"


#define RAD_TO_DEG(x) 360.0f *x / (2.0f*3.14159)


moon::moon()
{
	map_diffuse = texture::ptr(new texture(get_texture_dir()+"moon_d.png", texture::LINEAR));
	map_normal = texture::ptr(new texture(get_texture_dir()+"moon_n.png", texture::LINEAR));
}



void moon::display(const vector3 &moon_pos, const vector3 &sun_pos, double max_view_dist) const
{
	vector3 moon_dir = moon_pos.normal();
	double moon_size = max_view_dist/20;
	float moon_azimuth = atan2(-moon_dir.y, moon_dir.x);
	float moon_elevation = asin(moon_dir.z);

	//	bind normal map
	glActiveTextureARB(GL_TEXTURE0_ARB);
	glEnable(GL_TEXTURE_2D);
	map_normal.get()->set_gl_texture();
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_DOT3_RGB_ARB) ;
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_PRIMARY_COLOR_ARB) ;
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);

	//	bind diffuse map
	glActiveTextureARB(GL_TEXTURE1_ARB);
	glEnable(GL_TEXTURE_2D);
	map_diffuse.get()->set_gl_texture();
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	//	transform light into object space
	glPushMatrix();
	glLoadIdentity();
	glRotatef(RAD_TO_DEG(moon_azimuth), 0, 0, -1);
	glRotatef(RAD_TO_DEG(moon_elevation), 0, -1, 0);
	matrix4 invmodelview = (matrix4::get_gl(GL_MODELVIEW_MATRIX)).inverse();
	vector3 l = invmodelview * sun_pos;
	vector3 nl = vector3(-l.y, l.z, -l.x).normal();
	nl += vector3(1,1,1);
	nl = nl * (1.0f/2);
	glColor3f(nl.x, nl.y, nl.z);
	glPopMatrix();

	//	render moon
	glPushMatrix();
	glRotatef(RAD_TO_DEG(moon_azimuth), 0, 0, -1);
	glRotatef(RAD_TO_DEG(moon_elevation), 0, -1, 0);
	glTranslated(0.95*max_view_dist, 0, 0);

	glBegin(GL_QUADS);
	glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 1, 1);
	glMultiTexCoord2fARB(GL_TEXTURE1_ARB, 1, 1);
	glVertex3f( 0, -moon_size, -moon_size);
	glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 1, 0);
	glMultiTexCoord2fARB(GL_TEXTURE1_ARB, 1, 0);
	glVertex3f( 0, -moon_size, moon_size);
	glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0, 0);
	glMultiTexCoord2fARB(GL_TEXTURE1_ARB, 0, 0);
	glVertex3f( 0,  moon_size, moon_size);
	glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0, 1);
	glMultiTexCoord2fARB(GL_TEXTURE1_ARB, 0, 1);
	glVertex3f( 0,  moon_size, -moon_size);
	glEnd();
	glPopMatrix();

	//	reset textures
	glBindTexture(GL_TEXTURE_2D, 0);

	glActiveTextureARB(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
}
