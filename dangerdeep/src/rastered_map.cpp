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

// I know that this class is very ineffective but it's only a first test so just ignore it atm

#include "rastered_map.h"

rastered_map::rastered_map(const std::string& header_file, const std::string& data_file, vector2l _cache_tl, long int _square_size, unsigned num_levels) :
				min_height(-10800), max_height(8440), square_size(_square_size), cache_tl(_cache_tl), pn(64, 2, 16), pn2(128, 4, num_levels-4, true)
{
	extrah = pn.generate();
	sdl_image tmp(get_texture_dir() + "tex_grass.jpg");
	unsigned bpp = 0;
	texture = tmp.get_plain_data(cw, ch, bpp);
	if (bpp != 3) throw error("color bpp != 3");
	
	// open header file
	xml_doc doc(header_file);
	doc.load();

	// read header file
	xml_elem root = doc.child("dftd-map");
	xml_elem elem = root.child("resolution");
	resolution = elem.attri();
	elem = root.child("max");
	max_lat = elem.attri("latitude");
	max_lot = elem.attri("longitude");
	elem = root.child("min");
	min_lat = elem.attri("latitude");
	min_lot = elem.attri("longitude");	

	sample_spacing = (SECOND_IN_METERS*resolution)/pow(2, num_levels-1);
	
	// open data file
	data_stream.open(data_file.c_str(), std::ios::binary|std::ios::in);
	if (!data_stream.is_open()) throw std::ios::failure("Could not open file: "+data_file);
	
	levels.resize(num_levels);
	load(levels[levels.size()-1]);
	
	for (int i=levels.size()-2; i>=0; i--) {
		levels[i] = levels[i+1].smooth_upsampled(false);
	}
}

rastered_map::~rastered_map() {
	data_stream.close();
}

void rastered_map::load(bivector<float>& level)
{
	data_stream.clear();
	level.resize(vector2i(square_size/resolution, square_size/resolution));

	short signed int buf[(int)((square_size/resolution)*(square_size/resolution))];
	char *c_buf = (char*)buf;
	float *f_buf = level.data_ptr();
	
	long int file_width = ((max_lot+abs(min_lot))*3600)/resolution;
	long int file_center = (long int)((max_lat*3600/resolution)*file_width+(abs(min_lot)*3600/resolution));
	
	long int start = (long int)(file_center-(cache_tl.y/resolution)*file_width+cache_tl.x/resolution);
	long int end =   start+(file_width*((square_size-1)/resolution)+(square_size/resolution));
	
	int lines = 0;
	for (long int i=start; i<end; i+=file_width) 
	{
		lines++;
		data_stream.seekg(i*2);
		data_stream.read(c_buf, (int)(square_size/resolution)*2);
		for (int n=0; n < (int)(square_size/resolution); n++) {
			(*f_buf) = (float)buf[n];
			f_buf++;
		}
	}
}

void rastered_map::compute_heights(int detail, const vector2i& _coord_bl, const vector2i& coord_sz, 
								   float* dest, unsigned stride, unsigned line_stride)
{
	float level_res = (resolution/pow(2, levels.size()-1-detail));
	
	vector2f coord_bl;
	vector2i __coord_bl;
	
	if (!stride) stride = 1;
	if (!line_stride) line_stride = coord_sz.x * stride;

	if (detail>=0) {
		for (int y = 0; y < coord_sz.y; ++y) {
			float* dest2 = dest;
			for (int x = 0; x < coord_sz.x; ++x) {
				__coord_bl = _coord_bl+vector2i(x,y);
				coord_bl = vector2f((__coord_bl.x/SECOND_IN_METERS)/level_res, (__coord_bl.y/SECOND_IN_METERS)/level_res);
				(coord_bl.x<0)?coord_bl.x=floor(coord_bl.x):coord_bl.x=ceil(coord_bl.x);
				(coord_bl.y<0)?coord_bl.y=floor(coord_bl.y):coord_bl.y=ceil(coord_bl.y);
				
				int xc = __coord_bl.x * int(1 << detail);
				int yc = __coord_bl.y * int(1 << detail);				

				*dest2 = levels[detail].at(((cache_tl.y/level_res)-coord_bl.y), coord_bl.x-(cache_tl.x/level_res)) + pn2.value(xc, yc, levels.size()-detail);
				dest2 += stride;
			}
			dest += line_stride;
		}
	} else {
			// we need one value more to correctly upsample it
			bivector<float> d0h(vector2i((coord_sz.x>>-detail)+1, (coord_sz.y>>-detail)+1));
			compute_heights(0, vector2i(_coord_bl.x>>-detail, _coord_bl.y>>-detail),
					d0h.size(), d0h.data_ptr());
			for (int y = 0; y < coord_sz.y; ++y) {
				float* dest2 = dest;
				for (int x = 0; x < coord_sz.x; ++x) {
					vector2i coord = _coord_bl + vector2i(x, y);
					float baseh = d0h[vector2i(x,y)];
					//baseh += extrah[(coord.y&63)*64+(coord.x&63)];
					*dest2 = baseh;
					dest2 += stride;
				}
				dest += line_stride;
			}		
	}
}

void rastered_map::compute_colors(int detail, const vector2i& coord_bl,const vector2i& coord_sz, Uint8* dest) 
{
	std::vector<float> heights(1);
	
	for (int y = 0; y < coord_sz.y; ++y) {
		for (int x = 0; x < coord_sz.x; ++x) {
			vector2i coord = coord_bl + vector2i(x, y);
			color c;
			if (detail >= -2) {
				unsigned xc = coord.x << (detail+2);
				unsigned yc = coord.y << (detail+2);
				unsigned xc2,yc2;
				if (detail >= 0) {
					xc2 = coord.x << detail;
					yc2 = coord.y << detail;
				} else {
					xc2 = coord.x >> -detail;
					yc2 = coord.y >> -detail;
				}
				//fixme: replace compute_height!
				//float z = compute_height(0, vector2i(xc2,yc2));//coord);
				//compute_heights (detail, vector2i(xc2, yc2), vector2i(1,1), &heights[0], 0, 0);
				//float z = heights[0];
				float z = -4500.0;
				float zif = (z + 130) * 4 * 8 / 256;
				if (zif < 0.0) zif = 0.0;
				if (zif >= 7.0) zif = 6.999;
				zif = myfrac(zif);
				unsigned i = ((yc&(ch-1))*cw+(xc&(cw-1)));
				float zif2 = 1.0-zif;
				c = color(uint8_t(texture[3*i]*zif2 + texture[3*i]*zif),
					  uint8_t(texture[3*i+1]*zif2 + texture[3*i+1]*zif),
					  uint8_t(texture[3*i+2]*zif2 + texture[3*i+2]*zif));
			} else {
					unsigned xc = coord.x >> (-(detail+2));
					unsigned yc = coord.y >> (-(detail+2));
					//fixme: replace compute_height!
					//compute_heights (detail, vector2i(coord.x>>-detail,coord.y>>-detail), vector2i(1,1), &heights[0], 0, 0);
					//float z = heights[0];				
					//float z = compute_height(0, vector2i(coord.x>>-detail,coord.y>>-detail));
				float z = -4500.0;
					float zif = (z + 130) * 4 * 8 / 256;
					if (zif < 0.0) zif = 0.0;
					if (zif >= 7.0) zif = 6.999;
					zif = myfrac(zif);
					unsigned i = ((yc&(ch-1))*cw+(xc&(cw-1)));
					float zif2 = 1.0-zif;
					c = color(uint8_t(texture[3*i]*zif2 + texture[3*i]*zif),
						  uint8_t(texture[3*i+1]*zif2 + texture[3*i+1]*zif),
						  uint8_t(texture[3*i+2]*zif2 + texture[3*i+2]*zif));				
			}
			dest[0] = c.r;
			dest[1] = c.g;
			dest[2] = c.b;
			dest += 3;
		}
	}
}
