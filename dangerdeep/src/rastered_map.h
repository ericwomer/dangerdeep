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

#ifndef RASTERED_MAP_H
#define RASTERED_MAP_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <math.h>
#include <SDL.h>
#include "vector2.h"
#include "global_constants.h"
#include "xml.h"
#include "color.h"
#include "height_generator.h"
#include "image.h"
#include "datadirs.h"
#include "global_data.h"
#include "bivector.h"
#include "perlinnoise.h"

const int ENDIAN_TEST = 1;

#define is_bigendian() ( ((char*)&ENDIAN_TEST) == 0)
#define LSB 0
#define MSB 1

class rastered_map : public height_generator
{
public: 

protected:
	std::ifstream data_stream;
	int num_levels;
	long int max_lat, min_lat, max_lot, min_lot, resolution, min_height, max_height;
        bool byteorder;
        
	rastered_map();

	void load(bivector<float>&);
	
	std::vector<uint8_t> texture;
	unsigned cw, ch;		
	perlinnoise pn, pn2;
	std::vector<uint8_t> ct[8];
	std::vector<float> extrah;
	

public:

	rastered_map(const std::string&, const std::string&, unsigned);
	~rastered_map();
	void compute_heights(int, const vector2i&, const vector2i&, float*, unsigned = 0, unsigned = 0, bool = true);
	void compute_colors(int, const vector2i&, const vector2i&, Uint8*);
	void get_min_max_height(double& minh, double& maxh) const
	{
		minh = (double)min_height;
		maxh = (double)max_height;
	}
};

#endif
