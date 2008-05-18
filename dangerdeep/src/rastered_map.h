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
#include <vector>
#include <math.h>
#include "vector2.h"
#include "global_constants.h"
#include "xml.h"
#include "color.h"
#include "height_generator.h"
#include "log.h"

class rastered_map : public height_generator
{
protected: // map
	
	std::ifstream data_stream;
	long int max_lat, min_lat, max_lot, min_lot, resolution, view_dist, cache_dist, min_height, max_height;
	vector2l center, top_left, bottom_right;
	std::vector<std::vector<float> > levels;
	
	rastered_map();
	
	void load(std::vector<float>&);
		
public: // map
	
	rastered_map(const std::string&, const std::string&, long int, vector2i, long int, unsigned);
	~rastered_map();
	
	void sample_up(long int, std::vector<float>&, std::vector<float>&);
	void update_center(vector2i center);
	
	float compute_height(int detail, const vector2i& coord);
	vector3f compute_normal(int detail, const vector2i& coord);
	color compute_color(int detail, const vector2i& coord);
	void get_min_max_height(double& minh, double& maxh) const
	{
		minh = (double)min_height;
		maxh = (double)max_height;
	}
	double get_sample_spacing() const { return 7.22; }
}; // map

#endif
