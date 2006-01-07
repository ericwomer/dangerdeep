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
using namespace std;
#include "color.h"
#include "vector3.h"
#include "model.h"
class game;

class sky
{
protected:
	double mytime;					// store global time in seconds
	
	mutable	// test
	float skycolorfac;				// 0.0 sunny, 1.0 stormy
						//fixme: maybe rather use it for sunrise/fall colors
	
	model::ptr skyhemisphere;
	texture::ptr skycol;
	texture::ptr sunglow;
	texture::ptr clouds;
	texture::ptr suntex;
	texture::ptr moontex;
	double cloud_animphase;				// 0-1 phase of interpolation
	vector<vector<Uint8> > noisemaps_0, noisemaps_1;// interpolate to animate clouds
	unsigned clouds_dl;				// display list for clouds
	unsigned cloud_levels, cloud_coverage, cloud_sharpness;
	vector<Uint8> cloud_alpha;			// precomputed alpha texture
	vector<unsigned> cloud_interpolate_func;	// give fraction as Uint8
	unsigned skyhemisphere_dl;			// display list for sky (background)

	// the stars (positions in world space, constant, and their luminance)
	vector<vector3f> stars_pos;
	vector<Uint8> stars_lumin;
	
	sky& operator= (const sky& other);
	sky(const sky& other);

	void setup_textures(void) const;
	void cleanup_textures(void) const;

	// generate new clouds, fac (0-1) gives animation phase. animation is cyclic.
	void advance_cloud_animation(double fac);	// 0-1
	void compute_clouds(void);
	vector<vector<Uint8> > compute_noisemaps(void);
	Uint8 get_value_from_bytemap(unsigned x, unsigned y, unsigned level,
		const vector<Uint8>& nmap);
	void smooth_and_equalize_bytemap(unsigned s, vector<Uint8>& map1);

public:
	sky(double tm = 0.0);				// give day time in seconds
	void set_time(double tm);
	~sky();
	void display(const game& gm, const vector3& viewpos, double max_view_dist, bool isreflection) const;
	color get_horizon_color(const game& gm, const vector3& viewpos) const;
};

#endif
