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
#include "vector2.h"
#include "vector3.h"
#include "simplex_noise.h"
#include <vector>
#include <math.h>


class fractal_noise {

protected:
	float H, lacunarity, offset, gain;
	int octaves;
	std::vector<float> exponent_array;

public:
	fractal_noise(float _H, float _lacunarity, int _octaves, float _offset, float _gain) 
		: H(_H), lacunarity(_lacunarity), offset(_offset), gain(_gain), octaves(_octaves), exponent_array(octaves)
	{   
		float frequency = 1.0;
		for (int i = 0; i < octaves; i++) {
			/* compute weight for each frequency */
    	exponent_array[i] = pow(frequency, -H);
    	frequency *= lacunarity;
		}
	}
	
	float get_value_hybrid(vector3f point, int octave) {

		int i;
		float weight, result = 0.0, signal;

		/* get first octave of function */
		result = (simplex_noise::noise(point, 1)+offset) * exponent_array[0];
		weight = result;

		/* increase frequency */
		point *= lacunarity;
	
		/* spectral construction inner loop, where the fractal is built */
		for (i = 1; i < octave; i++) {
			/* prevent divergence */
	    if (weight > 1.0) weight = 1.0;

		    /* get next higher frequency */
    		signal = (simplex_noise::noise(point, i+1)+offset) * exponent_array[i];
				result += weight * signal;

    		/* update the (monotonically decreasing) weighting value */
        /* (this is why H must specify a high fractal dimension) */
		    weight *= signal;

    		/* increase frequency */
        point *= lacunarity;
		} /* for */

		/* take care of remainder in “octaves” */
		float remainder = octaves - (int)octaves;
		if (remainder != 0.0) {
    	/* “i” and spatial freq. are preset in loop above */
	    result += remainder * simplex_noise::noise(point, i+1) * exponent_array[i];
		}
		return result;
	}

	float get_value_ridged(vector3f point, int octave) {

		int i;
		float weight, result = 0.0, signal;

		/* get first octave of function */
		signal = simplex_noise::noise(point);

		/* get absolute value of signal (this creates the ridges) */
		if (signal < 0.0) signal = -signal;

		/* invert and translate (note that “offset” should be ϳ = 1.0) */
		signal = offset - signal;

		/* square the signal, to increase “sharpness” of ridges */
		signal *= signal;
		/* assign initial values */
		result = signal;
		weight = 1.0;

	
		/* spectral construction inner loop, where the fractal is built */
		for (i = 1; i < octave; i++) {
			point *= lacunarity;

			/* weight successive contributions by previous signal */
			weight = signal * gain;
			if (weight > 1.0) weight = 1.0;
			if (weight < 0.0) weight = 0.0;
   		signal = simplex_noise::noise(point, i+1);
			if (signal < 0.0) signal = -signal;
			signal = offset - signal;
			signal *= signal;

			/* weight the contribution */
			signal *= weight;
			result += signal *exponent_array[i];
		} /* for */

		return result;
	}

	float get_value_fbm2(vector2f point, int octave) {

		int i;
		float result = 0.0;

		/* inner loop of fractal construction */
		for (i=0; i<octave; i++) {
			result += simplex_noise::noise(point, i+1) * exponent_array[i];
     	point *= lacunarity;
		}

		float remainder = octaves - (int)octaves;
		if (remainder != 0.0) {
    	/* ‘i’ and spatial freq. are preset in loop above */
    	result += remainder * simplex_noise::noise(point, i+1) * exponent_array[i];
		}
		return result;
	}

	float get_value_fbm3(vector3f point, int octave) {

		int i;
		float result = 0.0;

		/* inner loop of fractal construction */
		for (i=0; i<octave; i++) {
			result += simplex_noise::noise(point, i+1) * exponent_array[i];
     	point *= lacunarity;
		}

		float remainder = octaves - (int)octaves;
		if (remainder != 0.0) {
    	/* ‘i’ and spatial freq. are preset in loop above */
    	result += remainder * simplex_noise::noise(point, i+1) * exponent_array[i];
		}
		return result;
	}

	std::vector<Uint8> get_map_fbm(const vector2i& bl, const vector2i& sz) {

		float min = 1.0, max = 0.0, scale = 0.0;;
		std::vector<float> values(sz.x*sz.y);
		std::vector<Uint8> map(sz.x*sz.y);
		
		
		for(int y=0; y<sz.y; y++) {
			for(int x=0; x<sz.x; x++) {
				values[y*sz.x+x] = get_value_fbm2(vector2f((bl.x+x)*0.001, (bl.y+y)*0.001), octaves);
				if(values[y*sz.x+x]>max)max=values[y*sz.x+x];
				if(values[y*sz.x+x]<min)min=values[y*sz.x+x];
			}
		}

		scale = 255.0/(max-min);
		for(int y=0; y<sz.y; y++) for(int x=0; x<sz.x; x++)
		{
			map[y*sz.x+x] = (values[y*sz.x+x]-min)*scale;
		}

		return map;
	}
};
#endif
