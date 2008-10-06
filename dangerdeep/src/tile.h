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

#include <vector>
#include <fstream>
#include <string>
#include "time.h"
#include "vector2.h"

/* A simple macro that determines if we are on a Big-Endian system */
#ifndef IS_BENDIAN
const int ENDIAN_TEST = 1;
#define IS_BENDIAN ((*(char*)&ENDIAN_TEST)==0)
#endif

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
  	tile(std::string filename, vector2l& _bottom_left, int size, T _no_data, std::vector<long>& _morton_x, std::vector<long>& _morton_y);

	/* copy constructor */
	tile(const tile<T> &rhs);

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
	T get_value(vector2l coord);

	/* simple getters */
  	unsigned long get_last_access() const { return last_access; };		
	vector2l get_bottom_left() const { return bottom_left; };
	unsigned int get_tile_size() const { return tile_size; };
	T get_no_data() const { return no_data; };
	std::string get_filename() const { return filename; };
	std::vector<long>& get_morton_x() const { return morton_x; };
	std::vector<long>& get_morton_y() const { return morton_y; };

protected:
	/* Reverses the byte order of a given variable. Transforms between Big-Endian and Little-Endian and vice versa */
	inline void reverse_byte_order(unsigned char * b, int n);
	/* Computes the morton index in 1D space from a given coordinate in 2D space. */
	inline unsigned long coord_to_morton(vector2l& coord);

	/* The file stream for the data file */
	std::auto_ptr<std::ifstream>	instream;

	/* Check the constructor's comment */
	std::string			filename;
	std::vector<T>		data_vector;
	vector2l 			bottom_left;
	unsigned int 		tile_size;
	unsigned long 		last_access;
	T 					no_data;
	/* References(!) to the both morton lookup tables. */
	std::vector<long>&	morton_x;
	std::vector<long>&	morton_y;
		
};

template<class T>
tile<T>::tile(std::string _filename, vector2l& _bottom_left, int size, T _no_data, std::vector<long>& _morton_x, std::vector<long>& _morton_y) 
: instream(std::auto_ptr<std::ifstream>(new std::ifstream(_filename.c_str(), std::ios::binary | std::ios::in))), filename(_filename), data_vector(size*size, _no_data), 
	bottom_left(_bottom_left), tile_size(size), last_access(clock() / (CLOCKS_PER_SEC/1000)), no_data(_no_data), morton_x(_morton_x), morton_y(_morton_y)
{
	if (!instream->is_open()) throw std::ios::failure("Could not open file: " + std::string(filename));	
}

template<class T>
tile<T>::tile(const tile<T> &rhs)
: instream(std::auto_ptr<std::ifstream>(new std::ifstream(rhs.get_filename().c_str(), std::ios::binary | std::ios::in))), 
filename(rhs.get_filename()), data_vector(rhs.get_tile_size()*rhs.get_tile_size(), rhs.get_no_data()), 
bottom_left(rhs.get_bottom_left()), tile_size(rhs.get_tile_size()), last_access(rhs.get_last_access()), 
no_data(rhs.get_no_data()), morton_x(rhs.get_morton_x()), morton_y(rhs.get_morton_y())
{
	if (!instream->is_open()) throw std::ios::failure("Could not open file: " + std::string(filename));	
}

template<class T>
T tile<T>::get_value(vector2l coord) 
{
	last_access = clock() / (CLOCKS_PER_SEC/1000);
	coord.y = tile_size-coord.y-1;

	long morton_coord = coord_to_morton(coord);

	if (data_vector[morton_coord] != no_data)
		return data_vector[morton_coord];

	vector2l start_coord((coord.x%2)==0?coord.x:coord.x-1, (coord.y%2)==0?coord.y:coord.y-1);
	long start_morton = coord_to_morton(start_coord);
	
	instream->seekg(start_morton*sizeof(T));
	instream->read((char*)&data_vector[start_morton], 4*sizeof(T));
  	
	if (IS_BENDIAN)
		for (int i=0; i<4; i++) reverse_byte_order((unsigned char*)&data_vector[start_morton+i], sizeof(T));

	return data_vector[morton_coord];
}

template<class T>
inline unsigned long tile<T>::coord_to_morton(vector2l& coord) 
{
	return morton_x[coord.x] + morton_y[coord.y];
}

template<class T>
inline void tile<T>::reverse_byte_order(unsigned char * b, int n)
{
   register int i = 0;
   register int j = n-1;
   while (i<j)
   {
      std::swap(b[i], b[j]);
      i++, j--;
   }
}
#endif
