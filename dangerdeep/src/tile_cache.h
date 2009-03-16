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
#include "system.h"

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
	};
	
	/* The compare function for the std::map */
	struct coord_compare
	{
 		bool operator()(const vector2i& lhs, const vector2i& rhs) const {
			if(lhs.y>rhs.y) return false;
			if(lhs.y<rhs.y) return true;
			if(lhs.y==rhs.y) {
				if(lhs.x<rhs.x) return true;
				return false;
			}
			return false;
		}
	};
		
	/* Iterator type to iterate through the tile map */
	typedef typename std::map<vector2i, tile<T>, coord_compare>::iterator tile_list_iterator;
	
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
				unsigned int slots, unsigned long expire) 
	{
		configuration.tile_folder = tile_folder;
		configuration.overall_rows = overall_rows;
		configuration.overall_cols = overall_cols;
		configuration.tile_size = tile_size;
		configuration.slots = slots;
		configuration.expire = expire;

	};
	tile_cache() {};
	/* Returns a value from the corresponding tile. If the tile isn't in the cache it's added to it.
	 * 
	 * coord: should be clear. Note that it takes global coordinates, no tile local coordinates!
	 */
  	T get_value(vector2i& coord);

	/* Removes all tiles from cache */
  	void flush();
	
protected:
	
	/* a map that holds all cached tiles */
	std::map<vector2i, tile<T>, coord_compare> tile_list;
	/* holds all configuration related variables */
	config_type configuration;
	
	/* removes the least recently used tile from cache */
	inline void free_slot();
	/* removes all expired tiles from cache */
	inline void	erase_expired();
	/* computes the bottom left corner of correspondig tile to the given global coordinates */
  	inline vector2i coord_to_tile(vector2i& coord);
};

template<class T>
T tile_cache<T>::get_value(vector2i& coord) 
{
	T return_value;

	/* wrap coordinates if needed */
	if (coord.x >= configuration.overall_cols) coord.x-= configuration.overall_cols;
	if (coord.y >= configuration.overall_rows) coord.y-= configuration.overall_rows;
	if (coord.x < 0) coord.x+= configuration.overall_cols;
	if (coord.y < 0) coord.y+= configuration.overall_rows;

	vector2i tile_coord = coord_to_tile(coord);
	tile_list_iterator it = tile_list.find(tile_coord);

	if(it != tile_list.end()) return_value = it->second.get_value(coord-tile_coord);
	else {
		if (configuration.slots>0 && tile_list.size()>=configuration.slots) 
			free_slot();
		
			tile<T> new_tile(filename.str().c_str(), tile_coord, configuration.tile_size);
			tile_list.insert(std::pair<vector2i, tile<T> >(tile_coord, new_tile));

			return_value = new_tile.get_value(coord-tile_coord);
	}
	erase_expired();

	return return_value;
}

template<class T>
inline void tile_cache<T>::free_slot() 
{
	unsigned long min = sys().millisec();
	vector2i min_key;
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
		long time = sys().millisec();
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
inline vector2i tile_cache<T>::coord_to_tile(vector2i& coord) 
{
	return vector2i((coord.x/configuration.tile_size)*configuration.tile_size, (coord.y/configuration.tile_size)*configuration.tile_size);
}
#endif // TILE_CACHE_H
