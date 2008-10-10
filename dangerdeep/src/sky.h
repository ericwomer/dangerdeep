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

// sky simulation and display (OpenGL)
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef SKY_H
#define SKY_H

/*
	This class simulates and displays the sky.
	Stars, atmospheric color, sunglow, sun, moon, clouds, etc.
*/

#include <vector>
#include "color.h"
#include "vector3.h"
#include "model.h"
#include "moon.h"
#include "stars.h"
#include "shader.h"
#include "vertexbufferobject.h"
#include "primitives.h"

class game;


///\brief Rendering of sky and atmospheric effects.
class sky
{
protected:
	double mytime;					// store global time in seconds

	texture::ptr sunglow;
	texture::ptr clouds;
	texture::ptr suntex;
	double cloud_animphase;				// 0-1 phase of interpolation
	std::vector<std::vector<Uint8> > noisemaps_0, noisemaps_1;// interpolate to animate clouds
	vertexbufferobject clouds_texcoords;
	unsigned cloud_levels, cloud_coverage, cloud_sharpness;
	std::vector<unsigned> cloud_interpolate_func;	// give fraction as Uint8

	sky& operator= (const sky& other);
	sky(const sky& other);

	// generate new clouds, fac (0-1) gives animation phase. animation is cyclic.
	void advance_cloud_animation(double fac);	// 0-1
	void compute_clouds();
	std::vector<std::vector<Uint8> > compute_noisemaps();
	Uint8 get_value_from_bytemap(unsigned x, unsigned y, unsigned level,
		const std::vector<Uint8>& nmap);
	void smooth_and_equalize_bytemap(unsigned s, std::vector<Uint8>& map1);

	stars _stars;
	moon _moon;
	vertexbufferobject sky_vertices, sky_indices;
	mutable vertexbufferobject sky_colors;
	unsigned nr_sky_vertices, nr_sky_indices;
	std::vector<vector2f> skyangles;
	mutable std::vector<color> skycolors;	// needed as lookup
	mutable vector3 sunpos, moonpos;
	mutable vector3 sundir, moondir;
	mutable float sun_azimuth, sun_elevation;
	float turbidity;

	std::auto_ptr<glsl_shader_setup> glsl_clouds;
	unsigned loc_cloudstex;

	void build_dome(const unsigned int sectors_h, const unsigned int sectors_v);

public:
	sky(const double tm = 0.0,
	    const unsigned int sectors_h = 64,
	    const unsigned int sectors_v = 16);
	//fixme: this should recompute sky color! not display...
	void set_time(double tm);

	// call this whenever time or viewpos has changed, it will modify the mutable variables.
	void rebuild_colors(const vector3& sunpos_, const vector3& moonpos_, const vector3& viewpos) const;

	void display(const colorf& lightcolor, const vector3& viewpos, double max_view_dist, bool isreflection) const;
	color get_horizon_color(const game& gm, const vector3& viewpos) const;
};

#endif
