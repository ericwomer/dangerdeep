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

#ifndef TILE_CACHE_H
#define TILE_CACHE_H

#include <map>
#include <string>
#include <sstream>
#include "tile.h"
#include "vector2.h"

/* A simple tile cache.
 * 
 * The tiles are stored in a std::map atm. Maybe a std::unsorted_map should be used when TR1 goes 
 * into the C++ Standard.
 * It also provides two morton lookup tables that are used by the tiles. For more information about this
 * check "Is morton layout competive for large two-dimensional arrasy, yet?" by Jeyarajan Thiyagalingam, 
 * Olav Beckmann, Paul H.J. Kelly
 */
template<class T>
class tile_cache
{
public:
	
	/* A simple struct which holds all configuration related variables. Just gives a cleaner code. */
	struct config_type
	{
		std::string 	tile_folder;
		int 			overall_rows;
		int 			overall_cols;
		int 			tile_size;
		unsigned int 	slots;
		unsigned long 	expire;
		T 				no_data;
	};
	
	/* The compare function for the std::map */
	struct coord_compare
	{
 		bool operator()(const vector2l& lhs, const vector2l& rhs) const {
			return (lhs.x<rhs.x && lhs.y<rhs.y);
		}
	};
		
	/* Iterator type to iterate through the tile map */
	typedef typename std::map<vector2l, tile<T>, coord_compare>::iterator tile_list_iterator;
	
	/* Constructs a tile_cache object and generates the morton lookup tables
	 * 
	 *
	 * tile_folder: The absolute path to the folder the tiles reside in. Needs a file separator at the end.
	 * overall_rows: number of rows of the tiled image
	 * overall_cols: number of rows of the tiled image
	 * tile_size: edge length of one tile (tiles are square)
	 * slots: capacity of the cache. zero means infinite
	 * expire: number of millisecs after a tile expire when it wasn't accessed. zero means infinite
	 * no_data: the no_data value. for further documentation is in tile.h
	 */
	tile_cache(const std::string tile_folder, int overall_rows, int overall_cols, int tile_size,
				unsigned int slots, unsigned long expire, T no_data) 
	{
		configuration.tile_folder = tile_folder;
		configuration.overall_rows = overall_rows;
		configuration.overall_cols = overall_cols;
		configuration.tile_size = tile_size;
		configuration.slots = slots;
		configuration.expire = expire;
		configuration.no_data = no_data;

		generate_morton_tables(morton_table_x, morton_table_y);
	};
	
	/* Returns a value from the corresponding tile. If the tile isn't in the cache it's added to it.
	 * 
	 * coord: should be clear. Note that it takes global coordinates, no tile local coordinates!
	 */
  	T get_value(vector2l& coord);

	/* Removes all tiles from cache */
  	void flush();
	
protected:
	
	/* a map that holds all cached tiles */
	std::map<vector2l, tile<T>, coord_compare> tile_list;
	/* holds all configuration related variables */
	config_type configuration;
	/* The two lookup tables for morton order */
	std::vector<long> morton_table_x, morton_table_y;
	
	/* removes the least recently used tile from cache */
	inline void free_slot();
	/* removes all expired tiles from cache */
	inline void	erase_expired();
	/* computes the bottom left corner of correspondig tile to the given global coordinates */
  	inline vector2l coord_to_tile(vector2l& coord);
	/* generates the morton lookup tables */
	inline void generate_morton_tables(std::vector<long>&, std::vector<long>&);
};

template<class T>
inline void tile_cache<T>::generate_morton_tables(std::vector<long>& table_x, std::vector<long>& table_y) 
{
	table_x.resize(configuration.tile_size);
	table_y.resize(configuration.tile_size);

	long d0 = 0, d1 = 0, ones0 = 0, ones1 = 0;

	/* data type lengths vary from system to system */
	for (unsigned int i=0; i<sizeof(long); i++) {
		ones0 <<= 4;
		ones0 += 10; /* 1010 */
		ones1 <<= 4;
		ones1 += 5; /* 0101 */
	}

	table_x[0] = 0;
	table_y[0] = 0;	
	for (long i=1; i < configuration.tile_size; i++) {
		d0 = ((d0 | ones0)+1) & ones1;
		d1 = ((d1 | ones1)+1) & ones0;
		table_x[i] = d0;
		table_y[i] = d1;
	}
};

template<class T>
T tile_cache<T>::get_value(vector2l& coord) 
{
	T return_value;

	/* wrap coordinates if needed */
	if (coord.x >= configuration.overall_cols) coord.x-= configuration.overall_cols;
	if (coord.y >= configuration.overall_rows) coord.y-= configuration.overall_rows;
	if (coord.x < 0) coord.x+= configuration.overall_cols;
	if (coord.y < 0) coord.y+= configuration.overall_rows;

	vector2l tile_coord = coord_to_tile(coord);
	tile_list_iterator it = tile_list.find(tile_coord);

	if(it != tile_list.end()) return_value = it->second.get_value(coord-tile_coord);
	else {
		if (configuration.slots>0 && tile_list.size()>=configuration.slots) 
			free_slot();
		
		std::stringstream filename;
		filename << configuration.tile_folder;
		filename << tile_coord.y;
		filename << "_";
		filename << tile_coord.x;
		filename << ".raw";

		tile<T> new_tile(filename.str(), tile_coord, configuration.tile_size, configuration.no_data, morton_table_x, morton_table_y);
		tile_list.insert(std::pair<vector2l, tile<T> >(tile_coord, new_tile));

		return_value = new_tile.get_value(coord-tile_coord);
	}
	erase_expired();
	return return_value;
}

template<class T>
inline void tile_cache<T>::free_slot() 
{
	unsigned long min = clock() / (CLOCKS_PER_SEC/1000);
	vector2l min_key;
	for (tile_list_iterator it = tile_list.begin(); it != tile_list.end(); it++) {
		if (it->second.get_last_access()<=min) {
			min = it->second.get_last_access();
			min_key = it->first;
		}
	}
	tile_list.erase(min_key);
}

template<class T>
inline void tile_cache<T>::erase_expired() 
{
	if (configuration.expire>0) {
		long time = clock() / (CLOCKS_PER_SEC/1000);
		for (tile_list_iterator it = tile_list.begin(); it != tile_list.end(); it++)
			if (time-it->second.get_last_access() >= configuration.expire) {
				tile_list.erase(it);
			}
	}
}

template<class T>
void tile_cache<T>::flush() 
{
	tile_list.clear();
}

template<class T>
inline vector2l tile_cache<T>::coord_to_tile(vector2l& coord) 
{
	return vector2l((coord.x/configuration.tile_size)*configuration.tile_size, (coord.y/configuration.tile_size)*configuration.tile_size);
}
#endif // TILE_CACHE_H
