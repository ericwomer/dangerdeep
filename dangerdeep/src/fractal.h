/*
 * Danger from the Deep - Open source submarine simulation
 * Copyright (C) 2003-2006  Thorsten Jordan, Luis Barrancos and others.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef FRACTAL_H
#define FRACTAL_H
#include "vector3.h"
#include "simplex_noise.h"
#include <vector>
#include <math.h>


class hybrid_multifractal {
public:
	hybrid_multifractal(float _H, float _lacunarity, int _octaves, float _offset) 
		: H(_H), lacunarity(_lacunarity), offset(_offset), octaves(_octaves), exponent_array(octaves)
	{   
		float frequency = 1.0;
		for (int i = 0; i < octaves; i++) {
			/* compute weight for each frequency */
    		exponent_array[i] = pow(frequency, -H);
    		frequency *= lacunarity;
		}
	}
	
	float get_value(vector3f point, int octave) {
		float weight, result = 0.0, signal;
		/* get first octave of function */
		result = (simplex_noise::noise(point, 1)+offset) * exponent_array[0];
		weight = result;
		/* increase frequency */
		point.x *= lacunarity;
		point.y *= lacunarity;
		point.z *= lacunarity;
	
		/* spectral construction inner loop, where the fractal is built */
		for (int i = 1; i < octave; i++) {
			/* prevent divergence */
	        if (weight > 1.0) weight = 1.0;
		    /* get next higher frequency */
    		signal = (simplex_noise::noise(point, i+1)+offset) * exponent_array[i];
			result += weight * signal;
    		/* update the (monotonically decreasing) weighting value */
	        /* (this is why H must specify a high fractal dimension) */
		    weight *= signal;
    		/* increase frequency */
	        point.x *= lacunarity;
		    point.y *= lacunarity;
			point.z *= lacunarity;
		} /* for */
		/* “i” and spatial freq. are preset in loop above */
		return result;
	}
protected:
	float H, lacunarity, offset;
	int octaves;
	std::vector<float> exponent_array;
private:
};
#endif
