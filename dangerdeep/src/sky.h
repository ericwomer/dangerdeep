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
	unsigned clouds_dl;				// display list for clouds
	unsigned cloud_levels, cloud_coverage, cloud_sharpness;
	std::vector<Uint8> cloud_alpha;			// precomputed alpha texture
	std::vector<unsigned> cloud_interpolate_func;	// give fraction as Uint8
	unsigned skyhemisphere_dl;			// display list for sky (background)

	// the stars (positions in world space, constant, and their luminance)
	std::vector<vector3f> stars_pos;
	std::vector<Uint8> stars_lumin;

	sky& operator= (const sky& other);
	sky(const sky& other);

	// generate new clouds, fac (0-1) gives animation phase. animation is cyclic.
	void advance_cloud_animation(double fac);	// 0-1
	void compute_clouds();
	std::vector<std::vector<Uint8> > compute_noisemaps();
	Uint8 get_value_from_bytemap(unsigned x, unsigned y, unsigned level,
		const std::vector<Uint8>& nmap);
	void smooth_and_equalize_bytemap(unsigned s, std::vector<Uint8>& map1);

	moon _moon;
	std::vector<vector3f> skyverts;
	std::vector<vector2f> skyangles;
	std::vector<unsigned int> skyindices;
	mutable std::vector<colorf> skycolors;
	mutable vector3 sunpos, moonpos;
	mutable vector3 sundir, moondir;
	mutable float sun_azimuth, sun_elevation;
	mutable float moon_azimuth, moon_elevation;
	float turbidity;

	void build_dome(const unsigned int sectors_h, const unsigned int sectors_v);

public:
	sky(const double tm = 0.0,
	    const unsigned int sectors_h = 100,
	    const unsigned int sectors_v = 25);
	//fixme: this should recompute sky color! not display...
	void set_time(double tm);
	~sky();

	// call this whenever time or viewpos has changed, it will modify the mutable variables.
	void rebuild_colors(const game& gm, const vector3& viewpos) const;

	void display(const game& gm, const vector3& viewpos, double max_view_dist, bool isreflection) const;
	color get_horizon_color(const game& gm, const vector3& viewpos) const;
};

#endif
