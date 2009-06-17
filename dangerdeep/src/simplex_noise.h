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

#ifndef SIMPLEX_NOISE_H
#define SIMPLEX_NOISE_H

#include "vector2.h"
#include "vector3.h"
#include "vector4.h"
#include "morton_bivector.h"
#include <math.h>
#include <vector>
#include <SDL_types.h>

class simplex_noise {
protected:
	static const int grad3[12][3];
	static const int grad4[36][4];
	static const int perm[512];
	static const int simplex4D[64][4];

	static int fastfloor(const double& x) {
		return x>0 ? (int)x : (int)x-1;
	}
	static double dot(const int* g, double& x, double& y) {
		return g[0]*x + g[1]*y; 
	}
	static double dot(const int* g, double& x, double& y, double& z) {
		return g[0]*x + g[1]*y + g[2]*z; 
	}
	static double dot(const int* g, double& x, double& y, double& z, double& w) {
		return g[0]*x + g[1]*y + g[2]*z + g[3]*w; 
	}

	static double interpolate2D(const vector2& coord);
	static double interpolate3D(const vector3& coord);
	static double interpolate4D(const vector4& coord);

public:
	
	static double noise(vector2 coord, unsigned ocatves = 1, float persistence = 1.0);
	static double noise(vector3 coord, unsigned ocatves = 1, float persistence = 1.0);
	static double noise(vector4 coord, unsigned ocatves = 1, float persistence = 1.0);
	
	static std::vector<Uint8> noise_map2D(vector2i size, unsigned ocatves = 1, float persistence = 1.0, float coord_factor = 0.01);

};
#endif
