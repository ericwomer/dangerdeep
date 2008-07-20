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

// simple height gen for simple map

#include "height_generator_map.h"
#include "xml.h"
#include "datadirs.h"
#include "texture.h"
#include "error.h"
#include "global_data.h"

using std::string;

//fixme: use wireframe rendering for debugging
// - gaps between levels

/* the map covers ca. 15625000m width at 3200 pixels,
   so we have S := 4882.8125m per pixel (height sample).
   Visible area is ca. 60km*60km, so we have to chose
   a geoclipmap value of N so that N*S <= 60km,
   and so many subdivision levels, that N/2^level is a
   reasonable value. With 7 further subdivision levels
   the finest level would have 4882.8125/128 = 38.15m
   sample spacing.
   For N we would have 60000/4882.8125 = 12.288, so N
   would be 8. Thus the coarsest level covers an area
   of 8*4882.8125 = 39062.5m, which is enough.
   N as low as 8 is highly problematic for the geoclipmap
   renderer, the smooth level transitions work barely
   and not very well.
   The finest level thus covers 8*38.15 = 305.2m, which
   is enough as well.
*/

height_generator_map::height_generator_map(const std::string& filename)
	: subdivision_steps(7)
{
	xml_doc doc(get_map_dir() + filename);
	doc.load();
	xml_elem er = doc.child("dftd-map");
	xml_elem et = er.child("topology");
	realwidth = et.attrf("realwidth");
	realoffset.x = et.attrf("realoffsetx");
	realoffset.y = et.attrf("realoffsety");
	sdl_image surf(get_map_dir() + et.attr("heights"));
	mapw = surf->w;
	maph = surf->h;
	pixelw_real = realwidth/mapw;
	mapoff.x = realoffset.x/pixelw_real;
	mapoff.y = realoffset.y/pixelw_real;
	realheight = maph*realwidth/mapw;
	sample_spacing = pixelw_real / (1 << subdivision_steps);
	log2_color_res_factor = 0;
	height_data.resize(vector2i(mapw, maph));
	surf.lock();
	if (surf->format->BytesPerPixel != 1 || surf->format->palette == 0 || surf->format->palette->ncolors != 256)
		throw error(string("coastmap: image is no greyscale 8bpp paletted image, in ") + filename);
	Uint8* offset = (Uint8*)(surf->pixels);
	int mapoffy = maph*mapw;
	for (int yy = 0; yy < int(maph); yy++) {
		mapoffy -= mapw;
		for (int xx = 0; xx < int(mapw); ++xx) {
			Uint8 c = (*offset++);
			height_data.at(xx, maph-1-yy) = (float(c)-128) * 4;
		}
		offset += surf->pitch - mapw;
	}
	surf.unlock();

	static const char* texnames[8] = {
		"tex_stone.jpg",
		"tex_sand.jpg",
		"tex_mud.jpg",
		"tex_grass.jpg",
		"tex_grass2.jpg",
		"tex_grass3.jpg",
		"tex_grass4.jpg",
		"tex_grass5.jpg"
	};
	for (unsigned i = 0; i < 8; ++i) {
		sdl_image tmp(get_texture_dir() + texnames[i]);
		unsigned bpp = 0;
		ct[i] = tmp.get_plain_data(cw, ch, bpp);
		if (bpp != 3) throw error("color bpp != 3");
	}
	for (unsigned i = 0; i < subdivision_steps+3; ++i) {
		noisemaps[i] = bivector<float>(vector2i(256, 256), 0.f).add_gauss_noise(float(1<<i), rndgen);
	}
}

bivector<float> height_generator_map::generate_patch(int detail, const vector2i& coord_bl,
						     const vector2i& coord_sz)
{
	// without caching this is slow as hell, but we don't care for first test.
	if (detail < int(subdivision_steps)) {
		// with smooth upsampling we can create 2n+1 values from n+3 values, so if we
		// assume coord_sz.x/y = m = 2n+1, thus (m-1)/2+3 = n+3
		// so we need (m-1)/2+3 samples in the coarser level
		// that is one extra value all around.
		// we can compute the values simply: round down/up the lower/upper
		// cordinates, divide by 2, enlarge by 1 and generate a subdivision from this.
		vector2i coord_tr = coord_bl + coord_sz - vector2i(1, 1);
		vector2i coord2_bl((coord_bl.x >> 1) - 1, (coord_bl.y >> 1) - 1);
		vector2i coord2_tr(((coord_tr.x+1) >> 1) + 1, ((coord_tr.y+1) >> 1) + 1);
		vector2i coord2_sz = coord2_tr - coord2_bl + vector2i(1, 1);
		vector2i offset(coord_bl.x & 1, coord_bl.y & 1);
		bivector<float> v = generate_patch(detail + 1, coord2_bl, coord2_sz).smooth_upsampled().sub_area(offset, coord_sz);
		v.add_shifted(noisemaps[detail+3], coord_bl);
		return v;
	} else if (detail == int(subdivision_steps)) {
		return height_data.sub_area(coord_bl - mapoff, coord_sz);
	} else {
		throw error("invalid detail level requested");
	}
}

void height_generator_map::compute_heights(int detail, const vector2i& coord_bl,
					   const vector2i& coord_sz, float* dest, unsigned stride,
					   unsigned line_stride, bool /*noise*/)
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

void height_generator_map::gen_col(float h, Uint8* c)
{
	unsigned hh = (h + 512) / 128;
	static const Uint8 cols[8*3] = {
		32, 32, 32,
		48, 48, 48,
		48, 48, 64,
		64, 64, 96,
		220, 180, 128,
		64, 220, 64,
		180, 220, 64,
		180, 180, 180
	};
	c[0] = cols[hh*3+0];
	c[1] = cols[hh*3+1];
	c[2] = cols[hh*3+2];
}

void height_generator_map::compute_colors(int detail, const vector2i& coord_bl,
					  const vector2i& coord_sz, Uint8* dest)
{
	bivector<float> v = generate_patch(detail, coord_bl, coord_sz);
	for (int y = 0; y < coord_sz.y; ++y) {
		for (int x = 0; x < coord_sz.x; ++x) {
			float h = v[vector2i(x, y)];
			unsigned xc, yc;
			vector2i coord = coord_bl + vector2i(x, y);
			if (detail >= -2) {
				xc = coord.x << (detail+2);
				yc = coord.y << (detail+2);
			} else {
				xc = coord.x >> (-(detail+2));
				yc = coord.y >> (-(detail+2));
			}
			float zif = (h + 512) * 7 / 1024;
			if (zif < 0.0) zif = 0.0;
			if (zif >= 7.0) zif = 6.999;
			unsigned zi = unsigned(zif);
			zif = myfrac(zif);
			unsigned i = ((yc&(ch-1))*cw+(xc&(cw-1)));
			float zif2 = 1.0-zif;
			color c(uint8_t(ct[zi][3*i]*zif2 + ct[zi+1][3*i]*zif),
				uint8_t(ct[zi][3*i+1]*zif2 + ct[zi+1][3*i+1]*zif),
				uint8_t(ct[zi][3*i+2]*zif2 + ct[zi+1][3*i+2]*zif));
			dest[0] = c.r;
			dest[1] = c.g;
			dest[2] = c.b;
			dest += 3;
		}
	}
/*
	for (int y = 0; y < coord_sz.y; ++y) {
		for (int x = 0; x < coord_sz.x; ++x) {
			gen_col(v[vector2i(x, y)], dest);
			dest += 3;
		}
	}
*/
}

void height_generator_map::get_min_max_height(double& minh, double& maxh) const
{
	// because of the additional noise, z-range is doubled
	minh = -512.0*2; maxh = 512.0*2;
}
