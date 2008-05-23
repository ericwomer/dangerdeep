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
				min_height(-10800), max_height(8440), square_size(_square_size), cache_tl(_cache_tl), iset(0)
{
	sdl_image tmp(get_texture_dir() + "tex_stone.jpg");
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
	// open data file
	//data_stream.open(data_file.c_str(), std::ios::binary|std::ios::in);
	//if (!data_stream.is_open()) throw std::ios::failure("Could not open file: "+data_file);
	
	levels.resize(num_levels);
	load(levels[levels.size()-1]);
	
	long seed = 12234578;
	float scale = 200.0;
	float ratio = (float)pow(2.f, -0.80f);
	long int newres = square_size/resolution;
	for (int i=levels.size()-2; i>=0; i--) {
		newres*=2;
		scale*=ratio;
		std::cout << "sampling up to: " << newres << std::endl;
		sample_up(newres, scale, seed, levels[i+1], levels[i]);
	}
}

rastered_map::~rastered_map() {
	data_stream.close();
}

void rastered_map::load(std::vector<float>& level)
{
	data_stream.clear();

	short signed int buf[(int)((square_size/resolution)*(square_size/resolution))];
	char *c_buf = (char*)buf;
	
	long int file_width = ((max_lot+abs(min_lot))*3600)/resolution;
	long int file_center = (long int)((max_lat*3600/resolution)*file_width+(abs(min_lot)*3600/resolution));
	
	long int start = (long int)(file_center-(cache_tl.y/resolution)*file_width+cache_tl.x/resolution);
	long int end =   start+(file_width*((square_size-1)/resolution)+(square_size/resolution));
	
	int lines = 0;
	for (long int i=start; i<end; i+=file_width) 
	{
		lines++;
		//data_stream.seekg(i*2);
		//data_stream.read(c_buf, (int)(square_size/resolution)*2);
		for (int n=0; n < (int)(square_size/resolution); n++) {
			//level.push_back((float)buf[n]);
			level.push_back(-4500.0);
		}
	}

	std::cout << lines << " lines loaded." << std::endl;
}

void rastered_map::sample_up(long int newres, float scale, long seed, std::vector<float>& buf_in, std::vector<float>& buf_out)
// fixme: gap at right and bottom
{
	std::vector<float> result((newres+1) * (newres+1));
	
	// copy level-1 heights
	for (long int y = 0; y < newres/2; ++y) {
		for (long int x = 0; x < newres/2; ++x) {
			result[y*2*(newres+1)+x*2] = buf_in[y*(newres/2)+x]+gauss(&seed)*scale;
		}
	}
	// copy col 0 and row 0 to right and bottom border to avoid a gap of 0-values
	// fixme: real values for gap
	result[(newres+1)*(newres+1)-1] = result[0];
	for (long int x=0; x<newres; x++) {
		result[newres*(newres+1)+x] = result[x];
	}
	for (long int y=0; y<newres+1; y++) {
		result[y*(newres+1)+newres] = result[y*(newres+1)];
	}

	long int pos;
	// first row
	for (long int x=0; x<1; x++) {
		for (long int y=1; y<newres; y+=2) {
				pos = y*(newres+1)+x;
				result[y*(newres+1)+x] = ((result[(y-1)*(newres+1)+x]+result[(y+1)*(newres+1)+x+1]+result[y*(newres+1)+x+1])/3)+gauss(&pos)*scale;
		}
	}
	// last row
	for (long int x=newres; x<newres+1; x++) {
		for (long int y=1; y<newres; y+=2) {
			pos = y*(newres+1)+x;
			result[y*(newres+1)+x] = ((result[(y-1)*(newres+1)+x]+result[(y+1)*(newres+1)+x+1]+result[y*(newres+1)+x-1])/3)+gauss(&pos)*scale;
		}
	}
	
	// first col
	for (long int x=1; x<1; x+=2) {
		for (long int y=0; y<1; y++) {
			pos = y*(newres+1)+x;
			result[y*(newres+1)+x] = ((result[y*(newres+1)+x-1]+result[y*(newres+1)+x+1]+result[(y+1)*(newres+1)+x])/3)+gauss(&pos)*scale;
		}
	}
	// last col
	for (long int x=1; x<1; x+=2) {
		for (long int y=newres; y<newres+1; y++) {
			pos = y*(newres+1)+x;
			result[y*(newres+1)+x] = ((result[y*(newres+1)+x-1]+result[y*(newres+1)+x+1]+result[(y-1)*(newres+1)+x])/3)+gauss(&pos)*scale;
		}
	}
	
	//centers
	for (long int x=1; x<newres; x+=2) {
		for (long int y=1; y<newres; y+=2) {
			pos = y*(newres+1)+x;
			result[y*(newres+1)+x] = ((result[(y-1)*(newres+1)+x-1]+result[(y-1)*(newres+1)+x+1]+result[(y+1)*(newres+1)+x-1]+result[(y+1)*(newres+1)+x+1])/4)+gauss(&pos)*scale;
		}
	}
	
	// horizontal edges
	for (long int x=2; x<newres+1; x+=2) {
		for (long int y=1; y<newres+1; y+=2) {
			pos = y*(newres+1)+x;
			result[y*(newres+1)+x] = ((result[y*(newres+1)+x-1]+result[y*(newres+1)+x+1]+result[(y+1)*(newres+1)+x]+result[(y-1)*(newres+1)+x])/4)+gauss(&pos)*scale;
		}
	}
	
	// vertical edges
	for (long int x=1; x<newres+1; x+=2) {
		for (long int y=2; y<newres+1; y+=2) {
			pos = y*(newres+1)+x;
			result[y*(newres+1)+x] = ((result[y*(newres+1)+x-1]+result[y*(newres+1)+x+1]+result[(y+1)*(newres+1)+x]+result[(y-1)*(newres+1)+x])/4)+gauss(&pos)*scale;
		}
	}		
	buf_out.resize(newres*newres);
	for (long int y = 0; y < newres; ++y) {
		for (long int x = 0; x < newres; ++x) {
			buf_out[y*newres+x] = result[y*(newres+1)+x];
		}
	}
}

double rastered_map::ran1(long *idum)
{
	int j,k;
	static int iv[32],iy=0;
	static double NDIV = 1.0/(1.0+(2147483647-1.0)/32);
	static double RNMX = (1.0-(1.2E-07));
	static double AM = (1.0/2147483647);

	if ((*idum <= 0) || (iy == 0)) {
		*idum = (-*idum>*idum)?(-*idum):(*idum);
                for(j=32+7;j>=0;j--) {
			k = *idum/127773;
			*idum = 16807*(*idum-k*127773)-2836*k;
			if(*idum < 0) *idum += 2147483647;
			if(j < 32) iv[j] = *idum;
		}
		iy = iv[0];
	}
	k = *idum/127773;
	*idum = 16807*(*idum-k*127773)-2836*k;
	if(*idum<0) *idum += 2147483647;
	j = (int)(iy*NDIV);
	iy = iv[j];
	iv[j] = *idum;
	return ((AM*iy)<(RNMX))?(AM*iy):(RNMX);
}

double rastered_map::gauss(long *idum)
{
	float fac,rsq,v1,v2;

	if (iset == 0) {
    	do {
        	v1=2.0*ran1(idum)-1.0;
        	v2=2.0*ran1(idum)-1.0;
        	rsq=v1*v1+v2*v2;
    	} while	(rsq >= 1.0 || rsq == 0.0);
    	fac=sqrt(-2.0*log(rsq)/rsq);
    	gset=v1*fac;
    	iset=1;
    	return v2*fac;
	} else {
    	iset=0;
	    return gset;
	}
}

void rastered_map::sample_down(std::vector<float>& buf_in, std::vector<float>& buf_out, unsigned newres)
{
	buf_out.resize(newres * newres);
	std::cout << buf_in.size() << " "<< buf_out.size() <<std::endl;
	for (unsigned y = 0; y < newres; ++y) {
		for (unsigned x = 0; x < newres; ++x) {
			buf_out[y*newres+x] = (buf_in[2*y*newres*2+2*x] 
								   + buf_in[2*y*newres*2+2*x+1] 
								   + buf_in[(2*y+1)*newres*2+2*x]
								   + buf_in[(2*y+1)*newres*2+2*x+1]) / 4;
		}
	}
}

float rastered_map::compute_height(int detail, const vector2i& _coord) 
{
/*	
	if (detail<0) detail=0;
	float level_res = (resolution/pow(2, levels.size()-1-detail));
	long int level_width = square_size/level_res;
	
	vector2f coord = vector2f((_coord.x/SECOND_IN_METERS)/level_res, (_coord.y/SECOND_IN_METERS)/level_res);
	(coord.x<0)?coord.x=floor(coord.x):coord.x=ceil(coord.x);
	(coord.y<0)?coord.y=floor(coord.y):coord.y=ceil(coord.y);
	
	//std::cout << ((cache_tl.y/level_res)-coord.y)*level_width+coord.x-(cache_tl.x/level_res) << std::endl;
	return levels[detail][((cache_tl.y/level_res)-coord.y)*level_width+coord.x-(cache_tl.x/level_res)];
*/
	return 0.0;
}

void rastered_map::compute_heights(int detail, const vector2i& _coord_bl, const vector2i& coord_sz, 
								   float* dest, unsigned stride, unsigned line_stride)
{
	if (detail<0) detail=0;
	float level_res = (resolution/pow(2, levels.size()-1-detail));
	long int level_width = square_size/level_res;	
	
	vector2f coord_bl;
	vector2i __coord_bl;
	
	if (!stride) stride = 1;
	if (!line_stride) line_stride = coord_sz.x * stride;
	for (int y = 0; y < coord_sz.y; ++y) {
		float* dest2 = dest;
		for (int x = 0; x < coord_sz.x; ++x) {
			__coord_bl = _coord_bl+vector2i(x,y);
			coord_bl = vector2f((__coord_bl.x/SECOND_IN_METERS)/level_res, (__coord_bl.y/SECOND_IN_METERS)/level_res);
			(coord_bl.x<0)?coord_bl.x=floor(coord_bl.x):coord_bl.x=ceil(coord_bl.x);
			(coord_bl.y<0)?coord_bl.y=floor(coord_bl.y):coord_bl.y=ceil(coord_bl.y);

			*dest2 = levels[detail][((cache_tl.y/level_res)-coord_bl.y)*level_width+coord_bl.x-(cache_tl.x/level_res)];
			dest2 += stride;
		}
		dest += line_stride;
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
				//float z = compute_height(0/*detail*/, vector2i(xc2,yc2));//coord);
				compute_heights (detail, vector2i(xc2, yc2), vector2i(1,1), &heights[0], 0, 0);
				float z = heights[0];
				float zif = (z + 130) * 4 * 8 / 256;
				if (zif < 0.0) zif = 0.0;
				if (zif >= 7.0) zif = 6.999;
				unsigned zi = unsigned(zif);
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
compute_heights (detail, vector2i(coord.x>>-detail,coord.y>>-detail), vector2i(1,1), &heights[0], 0, 0);
				float z = heights[0];				
					//float z = compute_height(0, vector2i(coord.x>>-detail,coord.y>>-detail));
					float zif = (z + 130) * 4 * 8 / 256;
					if (zif < 0.0) zif = 0.0;
					if (zif >= 7.0) zif = 6.999;
					unsigned zi = unsigned(zif);
					zif = myfrac(zif);
					//if (zi <= 4) zi = ((xc/256)*(xc/256)*3+(yc/256)*(yc/256)*(yc/256)*2+(xc/256)*(yc/256)*7)%5;
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


/*

color rastered_map::compute_color(unsigned detail, const vector2i& coord)
{
	if (detail<0) detail=0;
		float h = compute_height(detail, coord);
		vector3f n = compute_normal(detail, coord, 1.0);
		return (n.z > 0.9) ? (h > 20 ? color(240, 240, 242) : color(20,75+50,20))
				: color(64+h*0.5+32, 64+h*0.5+32, 64+h*0.5+32);
}
color rastered_map::compute_color_extra(unsigned detail, const vector2i& coord) 
{ 
	return compute_color(0, coord);
}
vector3f rastered_map::compute_normal(unsigned detail, const vector2i& coord, float zh) {
		float hr = compute_height(detail, coord + vector2i(1, 0));
		float hu = compute_height(detail, coord + vector2i(0, 1));
		float hl = compute_height(detail, coord + vector2i(-1, 0));
		float hd = compute_height(detail, coord + vector2i(0, -1));
		return vector3f(hl-hr, hd-hu, zh*2).normal();
	}

*/
