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

#ifndef TERRAIN_H
#define TERRAIN_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <math.h>
#include <SDL.h>
#include "vector2.h"
#include "vector3.h"
#include "global_constants.h"
#include "xml.h"
#include "height_generator.h"
#include "datadirs.h"
#include "global_data.h"
#include "bivector.h"
#include "tile_cache.h"
#include "game.h"
#include "log.h"
#include "datadirs.h"
#include "fractal.h"
#include "simplex_noise.h"


template <class T>
class terrain : public height_generator
{
private:
	terrain();
	
protected:
	tile_cache<T> m_tile_cache;
	int num_levels;
	long int resolution, min_height, max_height, tile_size;
	vector2l bounds;
	vector2l origin;
	float noise_h, noise_lac, noise_off, noise_scale, noise_coord_factor;

	std::auto_ptr<hybrid_multifractal> fractal;
	std::fstream map_file;
		
public:

	terrain(const std::string&, const std::string&, unsigned);
	bivector<float> generate_patch(int detail, const vector2i& coord_bl, const vector2i& coord_sz);
	void compute_heights(int, const vector2i&, const vector2i&, float*, unsigned = 0, unsigned = 0, bool = true);
	void get_min_max_height(double& minh, double& maxh) const
	{
		minh = (double)min_height;
		maxh = (double)max_height;
	}
	
};


template <class T>
terrain<T>::terrain(const std::string& header_file, const std::string& data_dir, unsigned _num_levels) 
	: num_levels(_num_levels)
{
    // open header file
    xml_doc doc(header_file);
    doc.load();

    // read header file
    xml_elem root = doc.child("dftd-map");
    xml_elem elem = root.child("resolution");
    resolution = elem.attri();
    elem = root.child("height");
    max_height = elem.attri("max");
    min_height = elem.attri("min");
	elem = root.child("tile_size");
	tile_size = elem.attri("value");
	elem = root.child("bounds");
	bounds.x = elem.attri("x");
	bounds.y = elem.attri("y");
	elem = root.child("origin");
	origin.x = elem.attri("x");
	origin.y = elem.attri("y");
	elem = root.child("tex_stretch_factor");
	tex_stretch_factor = elem.attrf("value");
	elem = root.child("regions");
	for (xml_elem::iterator it = elem.iterate("region"); !it.end(); it.next()) {
		elem = it.elem();
		regions.push_back(vector2f(elem.attrf("max")-elem.attrf("min"), elem.attrf("max")));
		terrain_textures.push_back(std::auto_ptr<texture>(new texture(get_texture_dir() += elem.attr("texture"), texture::LINEAR_MIPMAP_LINEAR, texture::REPEAT)));
	}
	elem = root.child("slope");
	slope_texture = std::auto_ptr<texture>(new texture(get_texture_dir() += elem.attr("texture"), texture::LINEAR_MIPMAP_LINEAR, texture::REPEAT));
	elem = root.child("noise");
	noise_h = elem.attrf("h");
	noise_lac = elem.attrf("lacunarity");
	noise_off = elem.attrf("offset");
	noise_scale = elem.attrf("scale");
	noise_coord_factor = elem.attrf("coord_factor");
	
	fractal = std::auto_ptr<hybrid_multifractal>(new hybrid_multifractal(noise_h, noise_lac, _num_levels+1, noise_off));
    // heired from height_generator interface
	sample_spacing = 50.0;
	m_tile_cache = tile_cache<T>(data_dir, bounds.y, bounds.x, tile_size, 0, 300000);
}

template <class T>
void terrain<T>::compute_heights(int detail, const vector2i& coord_bl, const vector2i& coord_sz,
                                   float* dest, unsigned stride, unsigned line_stride, bool noise)
{
	if (!stride) stride = 1;
	if (!line_stride) line_stride = coord_sz.x * stride;
	
	bivector<float> v = generate_patch(detail, coord_bl, coord_sz);
	
	for (int y = 0; y < coord_sz.y; ++y) {
		float* dest2 = dest;
		for (int x = 0; x < coord_sz.x; ++x) {
			*dest2 = v[vector2i(x, y)];
			dest2 += stride;
		}
		dest += line_stride;
	}
}

template <class T>
bivector<float> terrain<T>::generate_patch(int detail, const vector2i& coord_bl, const vector2i& coord_sz)
{
	bivector<float> patch;
    if (detail < (num_levels - 1)) {
		// upsample from the next coarser level
		vector2i coord_tr = coord_bl + coord_sz - vector2i(1, 1);
		vector2i coord2_bl((coord_bl.x >> 1) - 1, (coord_bl.y >> 1) - 1);
		vector2i coord2_tr(((coord_tr.x+1) >> 1) + 1, ((coord_tr.y+1) >> 1) + 1);
		vector2i coord2_sz = coord2_tr - coord2_bl + vector2i(1, 1);
		vector2i offset(2 + (coord_bl.x & 1), 2 + (coord_bl.y & 1));

		patch = generate_patch(detail+1, coord2_bl, coord2_sz).smooth_upsampled(true).sub_area(offset, coord_sz);

	} else if (detail == (num_levels - 1)) { // coarsest level - read from file
		patch.resize(coord_sz);
        for (int y = 0; y < coord_sz.y; y++) {
			for (int x = 0; x < coord_sz.x; x++) {
        		vector2f coord = vector2f(((float)((coord_bl.x+x)<<detail))*sample_spacing, ((float)((coord_bl.y+y)<<detail))*sample_spacing);
				
				coord.x = (coord.x*1944000.0)/(M_PI*M_PI*EARTH_RADIUS);
				coord.y = (coord.y*1944000.0)/(M_PI*M_PI*EARTH_RADIUS);
				coord = vector2f(((float)coord.x)/(float)resolution, 
								 ((float)coord.y)/(float)resolution);

				vector2i coord_l = vector2i(origin.x+coord.x, origin.y+coord.y);
					
				patch[vector2i(x, y)] = (float)m_tile_cache.get_value(coord_l);
            }
        }
    } else throw("terrain::generate_patch(): invalid detail level requested.");

	for (int y = 0; y < coord_sz.y; ++y) {
		for (int x = 0; x < coord_sz.x; ++x) {
			vector2i coord = coord_bl + vector2i(x, y);
			patch.at(x,y) += fractal->get_value(vector3f((coord.x<<(detail+1))*noise_coord_factor, (coord.y<<(detail+1))*noise_coord_factor, patch.at(x,y)*noise_coord_factor),num_levels-detail)*noise_scale;
		}
	}
	return patch;
}

#endif
