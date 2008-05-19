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

/// interface class to generate heights, normals and texture data for the geoclipmap renderer
class height_generator
{
 public:
	/// destructor
	virtual ~height_generator() {}

	/// compute height value of given detail and coordinates.
	///@param detail - detail level to be generated and also coordinate domain,
	///                0 means a sample spacing of "L", the basic geometry clipmap spacing,
	///                higher values mean coarser levels, values < 0 mean extra detail,
	///                finer than basic resolution.
	///@param coord - xy coordinates for the value to generate, scaled to match detail level
	virtual float compute_height(int detail, const vector2i& coord) = 0;

	/// compute normal value of given detail and coordinates.
	///@note here is some reasonable implementation, normally it should be overloaded
	///@param detail - detail level to be generated and also coordinate domain,
	///@param coord - xy coordinates for the value to generate, scaled to match detail level
	virtual vector3f compute_normal(int detail, const vector2i& coord) {
		const float zh = sample_spacing * 0.5f * (detail >= 0 ? (1<<detail) : 1.0f/(1<<-detail));
		float hr = compute_height(detail, coord + vector2i(1, 0));
		float hu = compute_height(detail, coord + vector2i(0, 1));
		float hl = compute_height(detail, coord + vector2i(-1, 0));
		float hd = compute_height(detail, coord + vector2i(0, -1));
		return vector3f(hl-hr, hd-hu, zh*2).normal();
	}

	/// compute color value of given detail and coordinates.
	///@note here is some test implementation, normally it should be overloaded
	///@param detail - detail level to be generated and also coordinate domain,
	///@param coord - xy coordinates for the value to generate, scaled to match detail level
	virtual color compute_color(int detail, const vector2i& coord) {
		return color((detail & 1) * 255, 128/*coord.x & 255*/, 128/*coord.y & 255*/);
	}

	/// get absolute minimum and maximum height of all levels, used for clipping
	///@param minh - minimum height values of all levels and samples
	///@param maxh - maximum height values of all levels and samples
	virtual void get_min_max_height(double& minh, double& maxh) const = 0;

	/// get sample spacing of detail level 0 (geometry)
	double get_sample_spacing() const { return sample_spacing; }

	/// get color res factor (log2 of it)
	unsigned get_log2_color_res_factor() const { return log2_color_res_factor; }

 protected:
	height_generator(double L = 1.0, unsigned l2crf = 1)
		: sample_spacing(L), log2_color_res_factor(l2crf) {}
	const double sample_spacing;	// equal to "L" value of geoclipmap renderer
	const unsigned log2_color_res_factor; // colors have 2^x more values as vertices
};

#endif
