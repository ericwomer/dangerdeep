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
#ifndef TILE_H
#define TILE_H

#include <fstream>
#include <string>
#include <SDL.h>
#include "time.h"
#include "vector2.h"
#include "system.h"
#include "bzip.h"
#include "bitstream.h"
#include "morton_bivector.h"
#include "log.h"

#define read_bw(target, len, stream); {target = stream.read(len); if(stream.read(1)==1) {target=(-target);}}
#define read_bw_s(target, len, shift, stream); {target = stream.read(len); target<<=shift; if(stream.read(1)==1) {target=(-target);}}

/* A Class that caches tile data from a file and provide a method to access it's values.
 * 
 * As the tile data is stored in morton order (or Z order) it assumes that the file is stored in 
 * morton order too.
 * To avoid lags on construction time the file's data is read not until it's queried by the get_value()
 * method. For further information check the method's comment.
 */
template<class T>
class tile
{
public:

	/* Constructs the tile
	 * 
	 * filename: the absolute path to the tile file
	 * _bottom_left: global coordinates of the tile's bottom left corner
	 * size: the edge length of the square tile
	 * _no_data: a value which shows the get_value() member that a value for a given coordinate does
	 * not resides in memory atm and needs to read from the file. So this value has to be outside the
	 * range of the tile's values (f.e.: if the data contains values from -11000 to 8000, _no_data could be
	 * -11001 or -32768 or 32767 or ...)
	 */
  	tile(const char *filename, vector2i& _bottom_left, unsigned size);
	tile(const tile<T>&);
	tile() {};
	/* Returns the value correspondig to the given local coordinate.
	 * 
	 * If the value is in memory already it's just returned. 
	 * If the specified position in memory equals the no_data value it's read from the file.
	 * In order to increase cache hits not only the one given value is read but also the corresponding 
	 * morton sub-block. (f.e.: if coord.x==1 and coord.y==1 this results in a morton index of 3.
	 * Instead of just retrieving this one value, values 0,1,2,3 are read from file.)
	 * For further information you may check http://en.wikipedia.org/wiki/Image:Z-curve.svg or
	 * "Is morton layout competive for large two-dimensional arrasy, yet?" by Jeyarajan Thiyagalingam, 
 	 * Olav Beckmann, Paul H.J. Kelly
	 */
	T get_value(vector2i coord);

	/* simple getters */
  	unsigned long get_last_access() const { return last_access; };
	vector2i get_bottom_left() const { return bottom_left; };
	const morton_bivector<T>& get_data() const { return data; };

protected:
	morton_bivector<T>	data;
	vector2i 			bottom_left;
	unsigned long 		last_access;
	
};

template<class T>
tile<T>::tile(const char *filename, vector2i& _bottom_left, unsigned size) 
: data(size), bottom_left(_bottom_left), last_access(sys().millisec())
{
	std::ifstream file;
	file.open(filename);
	if(file.is_open()) {
		bzip_istream bin(&file);
		ibitstream in(&bin);

		T average;
		Uint8 shift, numbits;

		read_bw(average, 15, in);
		read_bw(numbits, 4, in);
		read_bw(shift, 4, in);

		T *ptr = data.data_ptr();
		for(unsigned long i=0; i<size*size; i++) {
			read_bw_s((*ptr), numbits, shift, in);
			*ptr +=average;
			ptr++;
		}

		bin.close();
		file.close();
	} else {
		std::stringstream msg;
		msg << "Cannot open file: ";
		msg << filename;
		log_warning(msg.str());
	}
}

template<class T>
tile<T>::tile(const tile<T>& other) : data(other.get_data()), bottom_left(other.get_bottom_left()), last_access(other.get_last_access())
{
}

template<class T>
T tile<T>::get_value(vector2i coord) 
{
	last_access = sys().millisec();
	coord.y = data.size()-coord.y-1;
	return data.at(coord);
}
#endif
