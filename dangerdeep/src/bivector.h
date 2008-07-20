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
#include "random_generator.h"
#include <vector>
#include <stdexcept>
#include <cstdlib>

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
	///@note bivector must have power of two dimensions for this!
	bivector<T> shifted(const vector2i& offset) const;
	bivector<T> transposed() const;

	void swap(bivector<T>& other) {
		data.swap(other.data);
		std::swap(datasize, other.datasize);
	}

	template<class U> bivector<U> convert() const;
	template<class U> bivector<U> convert(const T& minv, const T& maxv) const;

	// get pointer to storage, be very careful with that!
	T* data_ptr() { return &data[0]; }
	const T* data_ptr() const { return &data[0]; }

	// special operations
	bivector<T> upsampled(bool wrap = false) const;
	bivector<T> downsampled(bool force_even_size = false) const;
	T get_min() const;
	T get_max() const;
	bivector<T>& operator*= (const T& s);
	bivector<T>& operator+= (const T& v);
	bivector<T>& operator+= (const bivector<T>& v);
	bivector<T> smooth_upsampled(bool wrap = false) const;

	// algebraic operations, omponent-wise add, sub, multiply (of same datasize)
	// sum of square of differences etc.

	bivector<T>& add_gauss_noise(const T& scal, random_generator& rg);
	///@note other bivector must have power of two dimensions for this!
	bivector<T>& add_tiled(const bivector<T>& other, const T& scal);
	///@note other bivector must have power of two dimensions for this!
	bivector<T>& add_shifted(const bivector<T>& other, const vector2i& offset);

 protected:
	vector2i datasize;
	std::vector<T> data;

	template<class U> friend class bivector;
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
	if (data.empty()) throw std::invalid_argument("bivector::get_min data empty");
	T m = data[0];
	bivector_FOREACH(m = std::min(m, data[z]))
}

template <class T>
T bivector<T>::get_max() const
{
	if (data.empty()) throw std::invalid_argument("bivector::get_max data empty");
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
bivector<T>& bivector<T>::operator+= (const bivector<T>& v)
{
	bivector_FOREACH(data[z] += v.data[z])
	return *this;
}

template <class T>
bivector<T> bivector<T>::sub_area(const vector2i& offset, const vector2i& sz) const
{
	if (offset.y + sz.y >= datasize.y) throw std::invalid_argument("bivector::sub_area, offset.y invalid");
	if (offset.x + sz.x >= datasize.x) throw std::invalid_argument("bivector::sub_area, offset.x invalid");
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
	bivector_FOREACH_XYZ(result.at((x+offset.x) & (datasize.x-1), (y+offset.y) & (datasize.y-1)) = data[z])
	return result;
}

template <class T>
bivector<T> bivector<T>::transposed() const
{
	bivector<T> result(vector2i(datasize.y, datasize.x));
	bivector_FOREACH_XYZ(result.at(y,x)=data[z])
	return result;
}

template<class T>
template<class U>
bivector<U> bivector<T>::convert() const
{
	bivector<U> result(datasize);
	bivector_FOREACH(result.data[z] = U(data[z]))
	return result;
}

template<class T>
template<class U>
bivector<U> bivector<T>::convert(const T& minv, const T& maxv) const
{
	bivector<U> result(datasize);
	bivector_FOREACH(result.data[z] = U(std::max(minv, std::min(maxv, data[z]))))
	return result;
}

template <class T>
bivector<T> bivector<T>::upsampled(bool wrap) const
{
	/* upsampling generates 3 new values out of the 4 surrounding
	   values like this: (x - surrounding values, numbers: generated)
	   x1x
	   23-
	   x-x
	   So 1x1 pixels are upsampled to 2x2 using the neighbourhood.
	   This means we can't generate samples beyond the last column/row,
	   thus n+1 samples generate 2n+1 resulting samples.
	   With wrapping we can compute one more sample.
	   So we have:
	   n+1 -> 2n+1
	   n   -> 2n    with wrapping
	*/
	if (datasize.x < 1 || datasize.y < 1) throw std::invalid_argument("bivector::upsampled base size invalid");
	vector2i resultsize = wrap ? datasize*2 : datasize*2 - vector2i(1,1);
	bivector<T> result(resultsize);
	// copy values that are kept and interpolate missing values on even rows
	for (int y=0; y < datasize.y; ++y) {
		for (int x=0; x < datasize.x-1; ++x) {
			result.at(2*x  , 2*y) = at(x, y);
			result.at(2*x+1, 2*y) = T((at(x, y) + at(x+1, y)) * 0.5);
		}
	}
	// handle special cases on last column
	if (wrap) {
		// copy/interpolate with wrap
		for (int y=0; y < datasize.y; ++y) {
			result.at(2*datasize.x-2, 2*y) = at(datasize.x-1, y);
			result.at(2*datasize.x-1, 2*y) = T((at(datasize.x-1, y) + at(0, y)) * 0.5);
		}
	} else {
		// copy last column
		for (int y=0; y < datasize.y; ++y) {
			result.at(2*datasize.x-2, 2*y) = at(datasize.x-1, y);
		}
	}
	// interpolate missing values on odd rows
	for (int y=0; y < datasize.y-1; ++y) {
		for (int x=0; x < resultsize.x; ++x) {
			result.at(x, 2*y+1) = T((result.at(x, 2*y) + result.at(x, 2*y+2)) * 0.5);
		}
	}
	// handle special cases on last row
	if (wrap) {
		// interpolate last row with first and second-to-last
		for (int x=0; x < resultsize.x; ++x) {
			result.at(x, resultsize.y-1) = T((result.at(x, resultsize.y-2) + result.at(x, 0)) * 0.5);
		}
	}
	return result;
}

template <class T>
bivector<T> bivector<T>::downsampled(bool force_even_size) const
{
	/* downsampling builds the average of 2x2 pixels.
	   if "force_even_size" is false:
	   If the width/height is odd the last column/row is
	   handled specially, here 1x2 or 2x1 pixels are averaged.
	   If width and height are odd, the last pixel is kept,
	   it can't be averaged.
	   We could add wrapping for the odd case here though...
	   if "force_even_size" is true:
	   If the width/height is odd the last column/row is
	   skipped and the remaining data averaged.
	*/
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

template <class T>
bivector<T> bivector<T>::smooth_upsampled(bool wrap) const
{
	/* interpolate one new value out of four neighbours,
	   or out of 4x4 neighbours with a coefficient matrix:
	   -1/16 9/16 9/16 -1/16 along one axis,
	*/
	static const float c1[4] = { -1.0f/16, 9.0f/16, 9.0f/16, -1.0f/16 };
	/* upsampling generates 3 new values out of the 16 surrounding
	   values like this: (x - surrounding values, numbers: generated)
	   x-x-x-x
	   -------
	   x-x1x-x
	   --23---
	   x-x-x-x
	   -------
	   x-x-x-x
	   So 1x1 pixels are upsampled to 2x2 using the neighbourhood.
	   This means we can't generate samples beyond the last two columns/rows,
	   and before the second column/row.
	   thus n+3 samples generate 2n+1 resulting samples (4->3, 5->5, 6->7, 7->9, ...)
	   With wrap we can get 2n samples out of n.
	   So we have:
	   n+3  -> 2n+1
	   n    -> 2n    with wrapping
	*/
	if (datasize.x < 3 || datasize.y < 3) throw std::invalid_argument("bivector::smooth_upsampled base size invalid");
	vector2i resultsize = wrap ? datasize*2 : datasize*2 - vector2i(1,1);
	bivector<T> result(resultsize);
	// copy values that are kept and interpolate missing values on even rows
	for (int y=0; y < datasize.y; ++y) {
		result.at(0, 2*y) = at(0, y);
		for (int x=1; x < datasize.x-2; ++x) {
			result.at(2*x  , 2*y) = at(x, y);
			result.at(2*x+1, 2*y) = T(at(x-1, y) * c1[0] +
						  at(x  , y) * c1[1] +
						  at(x+1, y) * c1[2] +
						  at(x+2, y) * c1[3]);
		}
		result.at(2*datasize.x-4, 2*y) = at(datasize.x-2, y);
		result.at(2*datasize.x-2, 2*y) = at(datasize.x-1, y);
	}
	if (wrap) {
		for (int y=0; y < datasize.y; ++y) {
			result.at(1, 2*y) = T(at(datasize.x-1, y) * c1[0] +
					      at(0, y) * c1[1] +
					      at(1, y) * c1[2] +
					      at(2, y) * c1[3]);
			result.at(2*datasize.x-3, 2*y) = T(at(datasize.x-3, y) * c1[0] +
							   at(datasize.x-2, y) * c1[1] +
							   at(datasize.x-1, y) * c1[2] +
							   at(           0, y) * c1[3]);
			result.at(2*datasize.x-1, 2*y) = T(at(datasize.x-2, y) * c1[0] +
							   at(datasize.x-1, y) * c1[1] +
							   at(           0, y) * c1[2] +
							   at(           1, y) * c1[3]);
		}
	} else {
		for (int y=0; y < datasize.y; ++y) {
			result.at(1, 2*y) = T(at(0, y) * c1[0] +
					      at(0, y) * c1[1] +
					      at(1, y) * c1[2] +
					      at(2, y) * c1[3]);
			result.at(2*datasize.x-3, 2*y) = T(at(datasize.x-3, y) * c1[0] +
							   at(datasize.x-2, y) * c1[1] +
							   at(datasize.x-1, y) * c1[2] +
							   at(datasize.x-1, y) * c1[3]);
		}
	}
	// interpolate missing values on odd rows
	for (int y=1; y < datasize.y-2; ++y) {
		for (int x=0; x < resultsize.x; ++x) {
			result.at(x, 2*y+1) = T(result.at(x, 2*y-2) * c1[0] +
						result.at(x, 2*y  ) * c1[1] + 
						result.at(x, 2*y+2) * c1[2] + 
						result.at(x, 2*y+4) * c1[3]);
		}
	}
	// handle special cases on last row
	if (wrap) {
		// interpolate 3 missing rows
		for (int x=0; x < resultsize.x; ++x) {
			result.at(x, 1) = T(result.at(x, 2*datasize.y-2) * c1[0] +
					    result.at(x,              0) * c1[1] +
					    result.at(x,              2) * c1[2] +
					    result.at(x,              4) * c1[3]);
			result.at(x, 2*datasize.y-3) = T(result.at(x, 2*datasize.y-6) * c1[0] +
							 result.at(x, 2*datasize.y-4) * c1[1] +
							 result.at(x, 2*datasize.y-2) * c1[2] +
							 result.at(x,              0) * c1[3]);
			result.at(x, 2*datasize.y-1) = T(result.at(x, 2*datasize.y-4) * c1[0] +
							 result.at(x, 2*datasize.y-2) * c1[1] +
							 result.at(x,              0) * c1[2] +
							 result.at(x,              2) * c1[3]);
		}
	} else {
		for (int x=0; x < resultsize.x; ++x) {
			result.at(x, 1) = T(result.at(x, 0) * c1[0] +
					    result.at(x, 0) * c1[1] +
					    result.at(x, 2) * c1[2] +
					    result.at(x, 4) * c1[3]);
			result.at(x, 2*datasize.y-3) = T(result.at(x, 2*datasize.y-6) * c1[0] +
							 result.at(x, 2*datasize.y-4) * c1[1] +
							 result.at(x, 2*datasize.y-2) * c1[2] +
							 result.at(x, 2*datasize.y-2) * c1[3]);
		}
	}
	return result;
}

#ifndef M_PI
#define M_PI 3.1415927
#endif

template <class T>
bivector<T>& bivector<T>::add_gauss_noise(const T& scal, random_generator& rg)
{
	for (int z=0; z < int(data.size()); ++z) {
		double r, q, s;
		do {
			r=rg.rndf()*2-1;
			s=r*M_PI;
			s=exp(-0.5*s*s);
			q=rg.rndf();
		} while (q > s);
		data[z] += T(r * scal);
	}
	//bivector_FOREACH(data[z] += T((rg.rndf()*2.0-1.0) * scal))
	return *this;
}

template <class T>
bivector<T>& bivector<T>::add_tiled(const bivector<T>& other, const T& scal)
{
	bivector_FOREACH_XY(at(x,y) += other.at(x & (other.datasize.x-1), y & (other.datasize.y-1)) * scal;)
	return *this;
}

template <class T>
bivector<T>& bivector<T>::add_shifted(const bivector<T>& other, const vector2i& offset)
{
	bivector_FOREACH_XY(at(x,y) += other.at((x+offset.x) & (other.datasize.x-1), (y+offset.y) & (other.datasize.y-1));)
	return *this;
}

#undef bivector_FOREACH
#undef bivector_FOREACH_XY
#undef bivector_FOREACH_XYZ

#endif
