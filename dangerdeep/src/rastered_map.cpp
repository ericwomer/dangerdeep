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

template <class A, class B> 
rastered_map<A,B>::rastered_map(const std::string& header_file, const std::string& data_file, long int _cache_dist, vector2l _center, long int _view_dist, unsigned num_levels) :
				view_dist(_view_dist), cache_dist(_cache_dist), center(_center), min_height(-10800), max_height(8440)
{
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
	data_stream.open(data_file.c_str(), std::ios::binary|std::ios::in);
	if (!data_stream.is_open()) throw std::ios::failure("Could not open file: "+data_file);

	levels.resize(num_levels);
	load(levels[levels.size()-1]);
	
	for (int i=levels.size()-2; i>=0; i--) {
		sample_up(levels[i+1], levels[i]);
	}
}

template <class A, class B> 
rastered_map<A,B>::~rastered_map() {
	data_stream.close();
}

template <class A, class B>
void rastered_map<A,B>::sample_up(unsigned newres, std::vector<B>& buf_in, std::vector<B>& buf_out)
// fixme: gap at right and bottom
{
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

template <class A, class B> 
void rastered_map<A,B>::load(std::vector<B>& level)
{
	data_stream.clear();

	A buf[(int)((view_dist*2)/resolution)];
	char *c_buf = (char*)buf;
	
	long int start = (long int)((((max_lat*3600)-top_left.y)/resolution)*((max_lot*2*3600)/resolution)+(((max_lot*3600)+top_left.x)/resolution));
	long int end = (long int)((((max_lat*2*3600)-bottom_right.y)/resolution)*((max_lot*2*3600)/resolution) + ((max_lot*3600)+bottom_right.x)/resolution); 
	long int step = (long int)((max_lot*2*3600)/resolution);

	for (long int i= start; i < end; i+=step) 
	{
		data_stream.seekg(i*2);
		data_stream.read(c_buf, (int)((view_dist*2)/resolution)*2);
		for (int n=0; n < (view_dist*2)/resolution; n++) {
			level.push_back(buf[n]);
		}
	}
}

template <class A, class B> 
void rastered_map<A,B>::update_center(vector2l _center)
{
	center = _center;
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
	
		for (int i=levels.size()-2; i>=0; i--) {
			sample_up(levels[i+1], levels[i]);
		}
	}
}

template <class A, class B> 
void rastered_map<A,B>::compute_height(unsigned detail, const height_generator::area region, std::vector<B>& target)
{
	long int start = (cache_dist/(resolution/(levels.size()+1-detail))) 
					 *((cache_dist-region.tl.y-center.y)/(resolution/(levels.size()+1-detail))) 
					 +((region.tl.x-center.x)/(resolution/(levels.size()+1-detail)));
	long int step  = (cache_dist*2 - region.width())/(resolution/(levels.size()+1-detail));
	long int end   = ((cache_dist+region.br.y-center.y)/(resolution/(levels.size()+1-detail))) 
				     *(cache_dist/(resolution/(levels.size()+1-detail))) 
				     +((cache_dist+region.br.x-center.x)/(resolution/(levels.size()+1-detail)));
					 
	
	// resize first is faster  (
	target.resize((region.width/(resolution/(levels.size()+1-detail)))*((region.height)/(resolution/(levels.size()+1-detail))));
	
	// copy values to buffer
	for (long int i=start; i<end; i+=step) {
		for (long int n=i; n<i+(region.width/(resolution/(levels.size()+1-detail))); n++) {
			target.push_back(levels[detail][i+n]);
		}
	}
}

template <class A, class B> 
void rastered_map<A,B>::compute_normal(unsigned detail, height_generator::area region, float zh, vector3t<B>& target)
{
	// resize first is faster  (
	target.resize((region.width/(resolution/(levels.size()+1-detail)))*((region.height)/(resolution/(levels.size()+1-detail))));
	
	region.tl.x+=(resolution/(levels.size()+1-detail));
	region.tl.y-=(resolution/(levels.size()+1-detail));
	region.br.x+=(resolution/(levels.size()+1-detail));
	region.br.y-=(resolution/(levels.size()+1-detail));	
	std::vector<B> height_buffer;
	compute_height(detail, region, height_buffer);
	
	for (long int y=1; y<region.height/(resolution/(levels.size()+1-detail))-1; y++) {
		for (long int x=1; x<region.width/(resolution/(levels.size()+1-detail))-1; x++) {
			target[y*(region.width/(resolution/(levels.size()+1-detail))-2)+x] = vector3t<B> (
				target[y*(region.width/(resolution/(levels.size()+1-detail))-2)+x+1] - target[y*(region.width/(resolution/(levels.size()+1-detail))-2)+x-1],
				target[(y-1)*(region.width/(resolution/(levels.size()+1-detail))-2)+x] - target[(y+1)*(region.width/(resolution/(levels.size()+1-detail))-2)+x],
				zh*2.0
				).normal();
		}
	}
}

template <class A, class B> 
void rastered_map<A,B>::compute_color(unsigned detail, height_generator::area region, std::vector<color>& target)
{
	std::vector<B> height_buffer;
	vector3t<B> normal_buffer;
	
	compute_height(detail, region, target);
	compute_normal(detail, region, 1.0*(1<<detail), normal_buffer);
	
	for (long int y=0; y<region.height/(resolution/(levels.size()+1-detail)); y++) {
		for (long int x=0; x<region.width/(resolution/(levels.size()+1-detail)); x++) {
			target[y*region.height/(resolution/(levels.size()+1-detail))+x] 
				= (normal_buffer[y*region.height/(resolution/(levels.size()+1-detail))+x].z > 0.9) 
				? (height_buffer[y*region.height/(resolution/(levels.size()+1-detail))+x] > 20 
				? color((Uint8)240, (Uint8)240, (Uint8)242) : color((Uint8)20,(Uint8)(75+50),(Uint8)20))
				: color(
						(Uint8)(64+height_buffer[y*region.height/(resolution/(levels.size()+1-detail))+x]*0.5+32), 
						(Uint8)(64+height_buffer[y*region.height/(resolution/(levels.size()+1-detail))+x]*0.5+32), 
						(Uint8)(64+height_buffer[y*region.height/(resolution/(levels.size()+1-detail))+x]*0.5+32)
				);
		}
	}
}
