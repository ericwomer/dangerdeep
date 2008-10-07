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

#include "oglext/OglExt.h"

#include "moon.h"
#include "matrix4.h"
#include "texture.h"
#include "datadirs.h"

#include "global_data.h"


#define RAD_TO_DEG(x) 360.0f *x / (2.0f*3.14159)


moon::moon()
{
	map_diffuse = texture::ptr(new texture(get_texture_dir()+"moon_d.png", texture::LINEAR));
	// compute moon normal map
	const unsigned mns = 256; // moon normal map size
	std::vector<Uint8> mnp(3*mns*mns);
	const double mid = (mns-1) * 0.5;
	unsigned mnpc = 0;
	for (unsigned k = 0; k < mns; ++k) {
		double y = -(k - mid)/mid;
		for (unsigned j = 0; j < mns; ++j) {
			double x = (j - mid)/mid;
			double l = x*x + y*y;
			double y2 = y;
			if (l > 0.999) {
				l = 1.0/sqrt(l);
				y2 = y*l;
				x = x*l;
			}
			double cosbetha2 = std::max(1.0 - y2*y2, 0.0);
			double sinalpha2 = std::max(1.0 - x*x / cosbetha2, 0.0);
			double z = sqrt(sinalpha2 * cosbetha2);
			mnp[mnpc+0] = Uint8(x * 127 + 127.5);
			mnp[mnpc+1] = Uint8(y2 * 127 + 127.5);
			mnp[mnpc+2] = Uint8(z * 127 + 127.5);
			mnpc += 3;
		}
	}
	map_normal = texture::ptr(new texture(mnp, mns, mns, GL_RGB, texture::LINEAR, texture::CLAMP_TO_EDGE));

#if 0
	// Test code:
 	ofstream oss("moon.ppm");
 	oss << "P6\n" << mns << " " << mns << "\n255\n";
 	oss.write((const char*)(&mnp[0]), mns*mns*3);
#endif
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
	//fixme: remove all this stuff and replace it by a shader!!!
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_DOT3_RGB_ARB) ;
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_PRIMARY_COLOR_ARB) ;
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
	glTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE, 2);	//	make moon brighter

	//	bind diffuse map
	glActiveTextureARB(GL_TEXTURE1_ARB);
	glEnable(GL_TEXTURE_2D);
	map_diffuse.get()->set_gl_texture();
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_MODULATE) ;	
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_PREVIOUS);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_TEXTURE);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
	//	transform light into object space
	matrix4 roth = matrix4::rot_z(-RAD_TO_DEG(moon_azimuth));
	matrix4 rotv = matrix4::rot_y(-RAD_TO_DEG(moon_elevation));
	matrix4 model_mat = roth*rotv;
	vector3 l = model_mat.inverse() * sun_pos;
	vector3 nl = vector3(-l.y, l.z, -l.x).normal();	//	OpenGL coordinates
	nl += vector3(1,1,1);
	nl = nl*0.5;
	glColor3f(nl.x, nl.y, nl.z);

	//	render moon
	glPushMatrix();
	model_mat.multiply_gl();
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

	// reset textures
	glBindTexture(GL_TEXTURE_2D, 0);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE, 1); // clean up!
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}
