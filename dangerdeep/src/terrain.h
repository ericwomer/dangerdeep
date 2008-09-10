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
#include "color.h"
#include "height_generator.h"
#include "image.h"
#include "datadirs.h"
#include "global_data.h"
#include "bivector.h"
#include "perlinnoise.h"
#include "tile_cache.h"


#define rad2deg(x) ((x*180.0)/M_PI)

#define MF_H 0.0

/* 0.7 for hybrid, 1.0 ridged */
#define MF_OFFSET 1.0

/* 2.0 for ridged (only used for ridged) */
#define MF_GAIN 1.0

#define MF_LACUNARITY .5


template <class T>
class terrain : public height_generator
{
protected:
	std::auto_ptr<tile_cache<T> > m_tile_cache;
	int num_levels;
	long int resolution, min_height, max_height;
	vector2l bounds;
	vector2l origin;
    bool byteorder;

	terrain();

	std::vector<uint8_t> texture;
	unsigned cw, ch;
	perlinnoise pn;
	perlinnoise pn2;
	std::vector<uint8_t> ct[8];
	std::vector<float> extrah;

public:

	terrain(const std::string&, const std::string&, unsigned);
	void compute_heights(int, const vector2i&, const vector2i&, float*, unsigned = 0, unsigned = 0, bool = true);
	void compute_colors(int, const vector2i&, const vector2i&, Uint8*);
	void get_min_max_height(double& minh, double& maxh) const
	{
		minh = (double)min_height;
		maxh = (double)max_height;
	}
};


template <class T>
terrain<T>::terrain(const std::string& header_file, const std::string& data_dir, unsigned _num_levels) 
	: num_levels(_num_levels), pn(64, 2, 16), pn2(64, 4, num_levels - 4, true)
{
    // proxy stuff for textures and detail <0
    extrah.resize(64 * 64);
    for (unsigned y = 0; y < 64; ++y)
        for (unsigned x = 0; x < 64; ++x)
            // FIXME: use real gaussion noise function
            extrah[64 * y + x] = rnd() - 0.5; //extrah2[64*y+x]/256.0-0.5;
    const char* texnames[8] = {
        "tex_grass.jpg",
        "tex_grass2.jpg",
        "tex_grass3.jpg",
        "tex_grass4.jpg",
        "tex_grass5.jpg",
        "tex_mud.jpg",
        "tex_stone.jpg",
        "tex_sand.jpg"
    };
    for (unsigned i = 0; i < 8; ++i) {
        sdl_image tmp(get_texture_dir() + texnames[i]);
        unsigned bpp = 0;
        ct[i] = tmp.get_plain_data(cw, ch, bpp);
        if (bpp != 3) throw error("color bpp != 3");
    }
    // --------------------------------------

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
	elem = root.child("bounds");
	bounds.x = elem.attri("x");
	bounds.y = elem.attri("y");
	elem = root.child("origin");
	origin.x = elem.attri("x");
	origin.y = elem.attri("y");

    // heired from height_generator interface
    sample_spacing = (SECOND_IN_METERS*resolution)/pow(2, num_levels-1);

	m_tile_cache = std::auto_ptr<tile_cache<T> >(new tile_cache<T>(data_dir, bounds.y, bounds.x, 512, 4, 0, min_height-1));

 }

template <class T>
void terrain<T>::compute_heights(int detail, const vector2i& coord_bl, const vector2i& coord_sz,
                                   float* dest, unsigned stride, unsigned line_stride, bool noise)
{
    float scale = 1.0;
    if (!stride) stride = 1;
    if (!line_stride) line_stride = coord_sz.x * stride;

    if (detail >= 0) {
        if (detail < (num_levels - 1)) {
            // upsample from the next coarser level
            bivector<float> lower(vector2i((coord_sz.x >> 1) + 1, (coord_sz.y >> 1) + 1));
            compute_heights(detail + 1, vector2i((coord_bl.x >> 1) + 1, (coord_bl.y >> 1) + 1), vector2i((coord_sz.x >> 1) + 1, (coord_sz.y >> 1) + 1), lower.data_ptr(), 0, 0, false);
            bivector<float> heights = lower.upsampled(false);

            // copy the upsampled vector to dest
            for (int y = 0; y < coord_sz.y; ++y) {
                float* dest2 = dest;
                for (int x = 0; x < coord_sz.x; ++x) {
                    vector2i coord = coord_bl + vector2i(x, y);

                    *dest2 = heights.at(x, y);
                    if (noise) *dest2 += pn2.valuef(coord.x * int(1 << detail), coord.y * int(1 << detail), num_levels - detail) * scale;
                    dest2 += stride;
                }
                dest += line_stride;
            }

        } else if (detail == (num_levels - 1)) { // coarsest level - read from file

            for (int y = 0; y < coord_sz.y; ++y) {
                float* dest2 = dest;
                for (int x = 0; x < coord_sz.x; ++x) {
                    vector2i coord = coord_bl + vector2i(x, y);

                    // dftd uses no real geographic coordinates atm (it uses a simple cartesian coordinate system) while the map uses real
                    // gps coordines. but instead doing a real conversion from dftd coords to gps coords, we just use the
                    // formula for arc length (l = r * alpha -> alpha = l / r)
                    // with this, one meter in coord means one meter ON the earth surface, starting at 0°/0°
                    vector2f coord_f = vector2f(rad2deg(coord.x/EARTH_RADIUS)*3600/resolution, rad2deg(coord.x/EARTH_RADIUS)*3600/resolution);
                    (coord_f.x < 0)?coord_f.x = floor(coord_f.x):coord_f.x = ceil(coord_f.x);
                    (coord_f.y < 0)?coord_f.y = floor(coord_f.y):coord_f.y = ceil(coord_f.y);
					
					vector2l coord_l = vector2l(origin.x+coord_f.x, origin.y+coord_f.y);

                    *dest2 = (float) m_tile_cache->get_value(coord_l);

                    if (noise) *dest2 += pn2.valuef(coord.x * int(1 << detail), coord.y * int(1 << detail), num_levels - detail) * scale;
                    dest2 += stride;
                }
                dest += line_stride;
            }
        } else throw error("invalid detail level requested");

    } else { // detail < 0
        bivector<float> d0h(vector2i((coord_sz.x >> -detail) + 1, (coord_sz.y >> -detail) + 1));
        compute_heights(0, vector2i(coord_bl.x >> -detail, coord_bl.y >> -detail),
                        d0h.size(), d0h.data_ptr());
        for (int z = detail; z < 0; ++z) {
            bivector<float> tmp = d0h.upsampled(false);
            tmp.swap(d0h);
        }
        for (int y = 0; y < coord_sz.y; ++y) {
            float* dest2 = dest;
            for (int x = 0; x < coord_sz.x; ++x) {
                vector2i coord = coord_bl + vector2i(x, y);
                float baseh = d0h[vector2i(x, y)];
                baseh += extrah[(coord.y & 63)*64 + (coord.x & 63)]*0.25;
                *dest2 = baseh;
                dest2 += stride;
            }
            dest += line_stride;
        }
    }
}

template <class T>
void terrain<T>::compute_colors(int detail, const vector2i& coord_bl, const vector2i& coord_sz, Uint8* dest)
{
    for (int y = 0; y < coord_sz.y; ++y) {
        for (int x = 0; x < coord_sz.x; ++x) {
            vector2i coord = coord_bl + vector2i(x, y);
            color c;
            if (detail >= -2) {
                unsigned xc = coord.x << (detail + 2);
                unsigned yc = coord.y << (detail + 2);
                unsigned xc2, yc2;
                if (detail >= 0) {
                    xc2 = coord.x << detail;
                    yc2 = coord.y << detail;
                } else {
                    xc2 = coord.x >> -detail;
                    yc2 = coord.y >> -detail;
                }
                float z = -4500;
                if (detail <= 6) {
                    float h = pn.value(xc2, yc2, 6) / 255.0f;
                    z = -4500 + h * h * h * 0.5 * 256;
                }
                float zif = (z + 130) * 4 * 8 / 256;
                if (zif < 0.0) zif = 0.0;
                if (zif >= 7.0) zif = 6.999;
                unsigned zi = unsigned(zif);
                zif = myfrac(zif);
                //if (zi <= 4) zi = ((xc/256)*(xc/256)*3+(yc/256)*(yc/256)*(yc/256)*2+(xc/256)*(yc/256)*7)%5;
                unsigned i = ((yc & (ch - 1)) * cw + (xc & (cw - 1)));
                float zif2 = 1.0 - zif;
                c = color(uint8_t(ct[zi][3 * i] * zif2 + ct[zi + 1][3 * i] * zif),
                          uint8_t(ct[zi][3 * i + 1] * zif2 + ct[zi + 1][3 * i + 1] * zif),
                          uint8_t(ct[zi][3 * i + 2] * zif2 + ct[zi + 1][3 * i + 2] * zif));
            } else {
                unsigned xc = coord.x >> (-(detail + 2));
                unsigned yc = coord.y >> (-(detail + 2));
                float z = -130;
                if (detail <= 6) {
                    float h = pn.value(xc, yc, 6) / 255.0f;
                    z = -4500 + h * h * h * 0.5 * 256;
                }
                float zif = (z + 130) * 4 * 8 / 256;
                if (zif < 0.0) zif = 0.0;
                if (zif >= 7.0) zif = 6.999;
                unsigned zi = unsigned(zif);
                zif = myfrac(zif);
                //if (zi <= 4) zi = ((xc/256)*(xc/256)*3+(yc/256)*(yc/256)*(yc/256)*2+(xc/256)*(yc/256)*7)%5;
                unsigned i = ((yc & (ch - 1)) * cw + (xc & (cw - 1)));
                float zif2 = 1.0 - zif;
                c = color(uint8_t(ct[zi][3 * i] * zif2 + ct[zi + 1][3 * i] * zif),
                          uint8_t(ct[zi][3 * i + 1] * zif2 + ct[zi + 1][3 * i + 1] * zif),
                          uint8_t(ct[zi][3 * i + 2] * zif2 + ct[zi + 1][3 * i + 2] * zif));
            }
            dest[0] = c.r;
            dest[1] = c.g;
            dest[2] = c.b;
            dest += 3;
        }
    }
}
#endif
