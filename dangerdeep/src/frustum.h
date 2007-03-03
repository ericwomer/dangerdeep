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

//
//  A generic frustum (C)+(W) 2005 Thorsten Jordan
//

#ifndef FRUSTUM_H
#define FRUSTUM_H

#include "polygon.h"
#include <vector>

/// model of a frustum for view calculations.
class frustum
{
	frustum();
public:
	/// frustum is modelled by planes, each frustum has 6.
	std::vector<plane> planes;
	polygon viewwindow;	// planes are constructed matching to this
	vector3 viewpos;
	vector3 viewdir;
	double znear;
	/// construct frustum from given data
	frustum(polygon poly, const vector3& viewp, const vector3& viewd, double znear);
	/// clip polygon to frustum and return intersecting polygon
	polygon clip(polygon p) const;
	/// render frustum as test
	void draw() const;
	/// print frumstum values for debugging
	void print() const;
	/// construct frustum from current OpenGL matrices.
	///@param z_near_distance - override for z-near value, give any value below zero to use OpenGL value
	static frustum from_opengl(double z_near_distance = 1.0, const vector3& viewpos = vector3());
};

#endif
