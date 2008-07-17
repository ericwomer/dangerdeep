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

using std::string;

height_generator_map::height_generator_map(const std::string& filename)
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
	//fixme: this value is too high in combination with level resolution.
	//we use 2^8=256 there and this value is ca. 4850m, so one level
	//covers an area of 1241.6km! no wonder there are graphic errors.
	//we need to give much finer detail values and a smaller number here,
	//so that one level area is smaller than half screen space,
	//i.e. < +-16km, so this value must be < 125m. Use 7 levels of
	//sub-detail gives 4850/(2^7)=37.9m, fine enough.
	sample_spacing = pixelw_real;
	log2_color_res_factor = 0;
	hd.resize(vector2i(mapw, maph));
	surf.lock();
	if (surf->format->BytesPerPixel != 1 || surf->format->palette == 0 || surf->format->palette->ncolors != 256)
		throw error(string("coastmap: image is no greyscale 8bpp paletted image, in ") + filename);
	Uint8* offset = (Uint8*)(surf->pixels);
	int mapoffy = maph*mapw;
	for (int yy = 0; yy < int(maph); yy++) {
		mapoffy -= mapw;
		for (int xx = 0; xx < int(mapw); ++xx) {
			Uint8 c = (*offset++);
			hd.at(xx, maph-1-yy) = c;
		}
		offset += surf->pitch - mapw;
	}
	surf.unlock();
}

void height_generator_map::compute_heights(int detail, const vector2i& coord_bl,
					   const vector2i& coord_sz, float* dest, unsigned stride,
					   unsigned line_stride, bool noise)
{
	if (!stride) stride = 1;
	if (!line_stride) line_stride = coord_sz.x * stride;
	for (int y = 0; y < coord_sz.y; ++y) {
		float* dest2 = dest;
		for (int x = 0; x < coord_sz.x; ++x) {
			vector2i coord = coord_bl + vector2i(x, y);
			if (detail >= 0) {
				int xc = coord.x * int(1 << detail);
				int yc = coord.y * int(1 << detail);
				*dest2 = (hd_at(xc, yc) - 128.f) * 4;
			} else {
				float xc = coord.x / float(1 << -detail);
				float yc = coord.y / float(1 << -detail);
				*dest2 = (hd_at(xc, yc) - 128.f) * 4;
			}
			dest2 += stride;
		}
		dest += line_stride;
	}
}

void height_generator_map::gen_col(int x, int y, Uint8* c)
{
	Uint8 h = hd_at(x, y);
	h /= 32;
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
	c[0] = cols[h*3+0];
	c[1] = cols[h*3+1];
	c[2] = cols[h*3+2];
}

void height_generator_map::compute_colors(int detail, const vector2i& coord_bl,
					  const vector2i& coord_sz, Uint8* dest)
{
	for (int y = 0; y < coord_sz.y; ++y) {
		for (int x = 0; x < coord_sz.x; ++x) {
			vector2i coord = coord_bl + vector2i(x, y);
			if (detail >= 0) {
				int xc = coord.x * int(1 << detail);
				int yc = coord.y * int(1 << detail);
				gen_col(xc, yc, dest);
			} else {
				float xc = coord.x / float(1 << -detail);
				float yc = coord.y / float(1 << -detail);
				gen_col(xc, yc, dest);
			}
			dest += 3;
		}
	}
}

void height_generator_map::get_min_max_height(double& minh, double& maxh) const
{
	minh = -512.0; maxh = 512.0;
}
