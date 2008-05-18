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

rastered_map::rastered_map(const std::string& header_file, const std::string& data_file, long int _cache_dist, vector2i _center, long int _view_dist, unsigned num_levels) :
				view_dist(_view_dist), cache_dist(_cache_dist), min_height(-10800), max_height(8440)
{
	std::cout << "Creating Terrain" << std::endl;
	std::cout << "Loading Header..." << std::endl;
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
	std::cout << "done" << std::endl;
	std::cout << "Loading data..." << std::endl;
	// open data file
	data_stream.open(data_file.c_str(), std::ios::binary|std::ios::in);
	if (!data_stream.is_open()) throw std::ios::failure("Could not open file: "+data_file);
	
	levels.resize(num_levels);
	update_center (_center);
}

rastered_map::~rastered_map() {
	data_stream.close();
}

void rastered_map::sample_up(long int newres, std::vector<float>& buf_in, std::vector<float>& buf_out)
// fixme: gap at right and bottom
{
	std::cout << "Sampling up from " << newres/2 << " to " << newres << std::endl;
	buf_out.resize(newres*newres);
	// copy
	for (long int y = 0; y < newres/2; ++y) {
		for (long int x = 0; x < newres/2; ++x) {
			buf_out[y*2*newres+x*2] = buf_in[y*(newres/2)+x];
		}
	}
	//first the centers
	for (long int y=1; y<newres;y+=2) {
		for (long int x=1;x<newres;x+=2) {
			buf_out[y*newres+x] = (buf_out[(y-1)*newres+x-1]+buf_out[(y-1)*newres+x+1]+buf_out[(y+1)*newres+x-1]+buf_out[(y+1)*newres+x+1])/4;
		}
	}
	//top edge
	long int y=0;
	for (long int x=1;x<newres;x+=2) {
		buf_out[y*newres+x] = (buf_out[y*newres+x-1]+buf_out[y*newres+x+1]+buf_out[(y+1)*newres+x])/3;
	}
	//bottom edge
	y=newres-1;
	for (long int x=1;x<newres;x+=2) {
		buf_out[y*newres+x] = (buf_out[y*newres+x-1]+buf_out[y*newres+x+1]+buf_out[(y-1)*newres+x])/3;
	}
	//left edge
	long int x=0;
	for (long int y=1;y<newres;y+=2) {
		buf_out[y*newres+x] = (buf_out[(y-1)*newres+x]+buf_out[y*newres+x+1]+buf_out[(y+1)*newres+x])/3;
	}
	//right edge
	x=newres-1;
	for (long int y=1;y<newres;y+=2) {
		buf_out[y*newres+x] = (buf_out[(y-1)*newres+x]+buf_out[y*newres+x-1]+buf_out[(y+1)*newres+x])/3;
	}
	//horizontal edges
	for (long int y=1; y<newres;y+=2) {
		for (long int x=2;x<newres;x+=2) {
			buf_out[y*newres+x] = (buf_out[(y-1)*newres+x]+buf_out[(y+1)*newres+x]+buf_out[y*newres+x-1]+buf_out[y*newres+x+1])/4;
		}
	}
	//vertical edges
	for (long int y=2; y<newres;y+=2) {
		for (long int x=1;x<newres;x+=2) {
			buf_out[y*newres+x] = (buf_out[(y-1)*newres+x]+buf_out[(y+1)*newres+x]+buf_out[y*newres+x-1]+buf_out[y*newres+x+1])/4;
		}
	}	
}

void rastered_map::load(std::vector<float>& level)
{
	data_stream.clear();

	short signed int buf[(int)((view_dist*2)/resolution)];
	char *c_buf = (char*)buf;
	
	long int start = (long int)((((max_lat*3600)-top_left.y)/resolution)*((max_lot*2*3600)/resolution)+(((max_lot*3600)+top_left.x)/resolution));
	long int end = (long int)((((max_lat*2*3600)-bottom_right.y)/resolution)*((max_lot*2*3600)/resolution) + ((max_lot*3600)+bottom_right.x)/resolution); 
	long int step = (long int)((max_lot*2*3600)/resolution);

	for (long int i= start; i < end; i+=step) 
	{
		data_stream.seekg(i*2);
		data_stream.read(c_buf, (int)((view_dist*2)/resolution)*2);
		for (int n=0; n < (view_dist*2)/resolution; n++) {
			level.push_back((float)buf[n]);
		}
	}
}

void rastered_map::update_center(vector2i _center)
{
	vector2l center(_center.x/SECOND_IN_METERS, _center.y/SECOND_IN_METERS);
	
	// check if view rect is outside of cache
	if ((center.x-view_dist)<top_left.x || (center.x+view_dist)>bottom_right.x || 
		(center.y+view_dist)>bottom_right.y || (center.y-view_dist)<top_left.y) 
	{
		// set new bounds and reload cache
		top_left.x = center.x-cache_dist;
		top_left.y = center.y-cache_dist;
		bottom_right.x = center.x+cache_dist;
		bottom_right.y = center.y+cache_dist;		
		
		load(levels[levels.size()-1]);
		
		long int newres = (cache_dist*2)/resolution;
		for (int i=levels.size()-2; i>=0; i--) {
			std::cout << "Sampling up " << i << std::endl;
			newres*=2;
			sample_up(newres, levels[i+1], levels[i]);
		}
	}
}

float rastered_map::compute_height(int detail, const vector2i& _coord)
{
	vector2l coord(_coord.x/SECOND_IN_METERS, _coord.y/SECOND_IN_METERS);
	unsigned level_res = (resolution/(levels.size()+1-detail));
	//std::cout << "Computing Height for (in seconds) " << coord.x << " " << coord.y << " level " << detail << std::endl;
	if (detail>=0) {
		long int pos = (((cache_dist*2*cache_dist)+cache_dist)-((coord.y-center.y)*cache_dist*2)+(coord.x-center.x))/level_res;
		//std::cout << "pos=" << pos <<std::endl;
		return levels[detail][pos];
	} else return 0.0;
}

vector3f rastered_map::compute_normal(int detail, const vector2i& coord)
{
	//std::cout << "Computing Normal for " << coord.x << " " << coord.y << std::endl;
	if (detail >= 0) {
		const float zh = sample_spacing * 0.5f * (detail >= 0 ? (1<<detail) : 1.0f/(1<<-detail));
		float hr = compute_height(detail, coord + vector2i(1, 0));
		float hu = compute_height(detail, coord + vector2i(0, 1));
		float hl = compute_height(detail, coord + vector2i(-1, 0));
		float hd = compute_height(detail, coord + vector2i(0, -1));
		return vector3f(hl-hr, hd-hu, zh*2).normal();
	} else return vector3f(0,0,1);
}

color rastered_map::compute_color(int detail, const vector2i& coord)
{
	//std::cout << "Computing Color for " << coord.x << " " << coord.y << std::endl;
	if (detail>=0) {
		float h = compute_height(detail, coord);
		vector3f n = compute_normal(detail, coord);
		return (n.z > 0.9) ? (h > 20 ? color(240, 240, 242) : color(20,75+50,20))
				: color(64+h*0.5+32, 64+h*0.5+32, 64+h*0.5+32);
	} else return color(255, 255, 255);
}
