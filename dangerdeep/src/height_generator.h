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

// interface class to compute heights

#ifndef HEIGHT_GENERATOR_H
#define HEIGHT_GENERATOR_H

#include "color.h"

class height_generator
{
public:
	/// destructor
	virtual ~height_generator() {}
	/// compute a rectangle of height information (z and z_c)
	///@note coordinates are relative to detail! so detail=0 coord=2,2 matches detail=1 coord=1,1 etc.
	///@param detail - accuracy of height (level of detail)
	///@param coord - coordinate to compute z/z_c for
	///@param dest - pointer to first z value to write to
	virtual float compute_height(unsigned detail, const vector2i& coord) = 0;
	virtual vector3f compute_normal(unsigned detail, const vector2i& coord, float zh) {
		float hr = compute_height(detail, coord + vector2i(1, 0));
		float hu = compute_height(detail, coord + vector2i(0, 1));
		float hl = compute_height(detail, coord + vector2i(-1, 0));
		float hd = compute_height(detail, coord + vector2i(0, -1));
		return vector3f(hl-hr, hd-hu, zh*2).normal();
	}
	virtual vector3f compute_normal_extra(unsigned detail, const vector2i& coord, float zh) { return vector3(0,0,1); }
	virtual color compute_color(unsigned detail, const vector2i& coord) { return color(128, 128, 255); }
	virtual color compute_color_extra(unsigned detail, const vector2i& coord) { return color(128, 128, 128); }
	// deprecated?
	virtual float compute_height_extra(unsigned detail, const vector2i& coord) { return 0.0; }
	virtual void get_min_max_height(double& minh, double& maxh) const = 0;
};

#endif
