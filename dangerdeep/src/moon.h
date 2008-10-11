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

#ifndef MOON_H
#define MOON_H


#include "vector3.h"
#include "texture.h"
#include "shader.h"


///\brief Moon rendering.
class moon
{
 private:
	texture::ptr map_diffuse;
	texture::ptr map_normal;
	std::auto_ptr<glsl_shader_setup> glsl_moon;
	unsigned loc_diffcol, loc_nrml, loc_lightdir;

 public:
	moon();

	void display(const vector3 &moon_pos, const vector3 &sun_pos, double max_view_dist) const;
};

#endif
