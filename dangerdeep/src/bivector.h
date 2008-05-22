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

//
//  A two-dimensional general vector (C)+(W) 2001 Thorsten Jordan
//

#ifndef BIVECTOR_H
#define BIVECTOR_H

#include "vector2.h"
#include <vector>
#include <stdexcept>

//#include <iostream>

///\brief Template class for a two-dimensional generic vector.
template <class T>
class bivector
{
 public:
	bivector() {}
	bivector(const vector2i& sz, const T& v = T()) : datasize(sz), data(sz.x*sz.y, v) {}
	T& at(const vector2i& p) { return data[p.x + p.y*datasize.x]; }
	const T& at(const vector2i& p) const { return data[p.x + p.y*datasize.x]; }
	T& at(int x, int y) { return data[x + y*datasize.x]; }
	const T& at(int x, int y) const { return data[x + y*datasize.x]; }
	T& operator[](const vector2i& p) { return at(p); }
	const T& operator[](const vector2i& p) const { return at(p); }
	const vector2i& size() const { return datasize; }
	void resize(const vector2i& newsz, const T& v = T());
	bivector<T> sub_area(const vector2i& offset, const vector2i& sz) const;
	bivector<T> shifted(const vector2i& offset) const;
	bivector<T> transposed() const;

	// special operations
	bivector<T> upsampled(bool wrap = false) const;
	bivector<T> downsampled(bool force_even_size = false) const;
	T get_min() const;
	T get_max() const;
	bivector<T>& operator*= (const T& s);
	bivector<T>& operator+= (const T& v);

	// algebraic operations, omponent-wise add, sub, multiply (of same datasize)
	// sum of square of differences etc.

 protected:
	vector2i datasize;
	std::vector<T> data;
};

template <class T>
void bivector<T>::resize(const vector2i& newsz, const T& v)
{
	//fixme: not correct that way
	data.resize(newsz.x*newsz.y, v);
	datasize = newsz;
/*
	// if new size is greater than current size, get new vector
	if (newsz.x > datasize.x || newsz.y > datasize.y) {
		// copy values to new vector
	} else {
		// pack them
	}
	// fixme: maybe special case when newsz.x*newsz.y <= datasize.x*datasize.y,
	// although one component is greater...
*/
}

#define bivector_FOREACH(cmd) for (int z=0; z < int(data.size()); ++z) { cmd ; }
#define bivector_FOREACH_XY(cmd) for (int y=0; y < datasize.y; ++y) for (int x=0; x < datasize.x; ++x) { cmd ; }
#define bivector_FOREACH_XYZ(cmd) for (int y=0,z=0; y < datasize.y; ++y) for (int x=0; x < datasize.x; ++x,++z) { cmd ; }

template <class T>
T bivector<T>::get_min() const
{
	if (data.empty()) throw std::invalid_argument();
	T m = data[0];
	bivector_FOREACH(m = std::min(m, data[z]))
}

template <class T>
T bivector<T>::get_max() const
{
	if (data.empty()) throw std::invalid_argument();
	T m = data[0];
	bivector_FOREACH(m = std::max(m, data[z]))
}

template <class T>
bivector<T>& bivector<T>::operator*= (const T& s)
{
	bivector_FOREACH(data[z] *= s)
	return *this;
}

template <class T>
bivector<T>& bivector<T>::operator+= (const T& v)
{
	bivector_FOREACH(data[z] += v)
	return *this;
}

template <class T>
bivector<T> bivector<T>::sub_area(const vector2i& offset, const vector2i& sz) const
{
	if (offset.y + sz.y >= datasize.y) throw std::invalid_argument();
	if (offset.x + sz.x >= datasize.x) throw std::invalid_argument();
	bivector<T> result(sz);
	for (int y=0; y < result.datasize.y; ++y)
		for (int x=0; x < result.datasize.x; ++x)
			result.at(x,y) = at(offset.x+x, offset.y+y);
	return result;
}

template <class T>
bivector<T> bivector<T>::shifted(const vector2i& offset) const
{
	bivector<T> result(datasize);
	//fixme: if datasize.xy is power of two, use faster & instead of %
	bivector_FOREACH_XYZ(result.at((x+offset.x)%datasize.x, (y+offset.y)%datasize.y) = data[z])
	return result;
}

template <class T>
bivector<T> bivector<T>::transposed() const
{
	bivector<T> result(vector2i(datasize.y, datasize.x));
	bivector_FOREACH_XYZ(result.at(y,x)=data[z])
	return result;
}

template <class T>
bivector<T> bivector<T>::upsampled(bool wrap) const
{
	//fixme
	bivector<T> result(vector2i(datasize.x*2, datasize.y*2));
	// copy values that are kept and interpolate missing values on even rows
	for (int y=0; y < datasize.y; ++y) {
		for (int x=0; x < datasize.x; ++x) {
			result.at(2*x, 2*y) = at(x, y);
			result.at(2*x+1, 2*y) = T((at(x, y) + at(x+1, y)) * 0.5);
		}
	}
	// interpolate missing values on odd rows
	for (int y=0; y < datasize.y; ++y) {
		for (int x=0; x < datasize.x*2; ++x) {
			result.at(x, 2*y+1) = T((result.at(x, 2*y) + result.at(x, 2*y+2)) * 0.5);
		}
	}
	return result;
}

template <class T>
bivector<T> bivector<T>::downsampled(bool force_even_size) const
{
	vector2i newsize(datasize.x >> 1, datasize.y >> 1);
	vector2i resultsize = newsize;
	if (!force_even_size) {
		resultsize.x += datasize.x & 1;
		resultsize.y += datasize.x & 1;
	}
	bivector<T> result(resultsize);
	for (int y=0; y < newsize.y; ++y)
		for (int x=0; x < newsize.x; ++x)
			result.at(x,y) = T((at(2*x, 2*y) + at(2*x+1, 2*y) + at(2*x, 2*y+1) + at(2*x+1, 2*y+1)) * 0.25);
	if (!force_even_size) {
		// downsample last row or column if original size is odd
		if (datasize.x & 1) {
			for (int y=0; y < newsize.y; ++y)
				result.at(newsize.x, y) = T((at(datasize.x-1, 2*y) + at(datasize.x-1, 2*y+1)) * 0.5);
		}
		if (datasize.y & 1) {
			for (int x=0; x < newsize.x; ++x)
				result.at(x, newsize.y) = T((at(2*x, datasize.y-1) + at(2*x+1, datasize.y-1)) * 0.5);
		}
		if ((datasize.x & datasize.y) & 1) {
			// copy corner, hasn't been handled yet
			result.at(newsize.x, newsize.y) = at(datasize.x-1, datasize.y-1);
		}
	}
	return result;
}

#undef bivector_FOREACH
#undef bivector_FOREACH_XY
#undef bivector_FOREACH_XYZ

#endif
