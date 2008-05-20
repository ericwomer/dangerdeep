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
public: 
	struct area
	{
		vector2l tl, br;

		area(const vector2l& a, const vector2l& b) : tl(a), br(b) {}

		long int width() { return br.x-tl.x; }
		long int height() { return tl.y-br.y; }
		void move(vector2l m) {
			tl.x+=m.x;
			tl.y+=m.y;
			br.x+=m.x;
			br.y+=m.y;
		}
	};	
protected: // map
	
	std::ifstream data_stream;
	long int max_lat, min_lat, max_lot, min_lot, resolution, min_height, max_height, square_size;
	vector2l cache_tl;
	std::vector<std::vector<float> > levels;
	
	rastered_map();
	
	double gauss(long*);
	double ran1(long*);
	void load(std::vector<float>&);
	void compute_square(std::vector<float>&, vector2i, unsigned long, unsigned long, unsigned int, unsigned, float, long);
	void sample_up(long int, float, long int, std::vector<float>&, std::vector<float>&);
	
public: // map

	rastered_map(const std::string&, const std::string&, vector2l, long int, unsigned);
	~rastered_map();
	
	color compute_color(int detail, const vector2i& coord);
	double get_sample_spacing() const { return 14.453125; }
	float compute_height(int detail, const vector2i& coord);
	void get_min_max_height(double& minh, double& maxh) const
	{
		minh = (double)min_height;
		maxh = (double)max_height;
	}
}; // map

#endif
