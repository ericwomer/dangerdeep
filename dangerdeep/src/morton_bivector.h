
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

#ifndef MORTON_BIVECTOR_H
#define MORTON_BIVECTOR_H

#include "vector2.h"
#include "random_generator.h"
#include "bivector.h"
#include <vector>
#include <stdexcept>
#include <cstdlib>
#include <algorithm>
#include <sstream>

#define bivector_FOREACH(cmd) for (int z=0; z < int(data.size()); ++z) { cmd ; }
#define bivector_FOREACH_XY(cmd) for (int y=0; y < datasize; ++y) for (int x=0; x < datasize; ++x) { cmd ; }
#define bivector_FOREACH_XYZ(cmd) for (int y=0,z=0; y < datasize; ++y) for (int x=0; x < datasize; ++x,++z) { cmd ; }
#define bivector_abs(x) ((x>0)?x:-x)
#ifndef M_PI
#define M_PI 3.1415927
#endif

template <class T>
class morton_bivector
{
private:
	std::vector<long> morton_x, morton_y;
	long datasize;
	std::vector<T> data;

	template<class U> friend class bivector;
	
public:
	morton_bivector():datasize(0) {}
	
	morton_bivector(const bivector<T>& bv) {
		resize(std::max(pow(2, ceil(log2(bv.datasize.x))), pow(2, ceil(log2(bv.datasize.y)))));
		for(int y=0; y<bv.datasize.y; y++) {
			for(int x=0; x<bv.datasize.x; x++) {
				at(x,y) = bv.at(x,y);
			}
		}
	}
	
	morton_bivector(const morton_bivector<T>& bv) {
		resize(bv.size());

		for(long y=0; y<datasize; y++) {
			for(long x=0; x<datasize; x++) {
				at(x,y) = bv.at(x,y);
			}
		}
	}
	
	morton_bivector(const long& sz, const T& v = T()) : datasize(sz), data(sz*sz, v) {
		if(log2(sz) != (int)log2(sz)) throw "morton_bivector::morton_bivector() wrong datasize: size is not power of 2";
		morton_x.resize(sz);
		morton_y.resize(sz);
		generate_morton_tables(morton_x, morton_y);
	}
	
	T& at(const vector2i& p) { 
		if (p.x>=datasize || p.y>=datasize) {
			std::stringstream ss;
			ss << "morton_bivector::at " << p;
			throw std::out_of_range(ss.str());
		}
		return data[coord_to_morton(p)]; 
	}
	const T& at(const vector2i& p) const { 
		if (p.x>=datasize || p.y>=datasize) {
			std::stringstream ss;
			ss << "morton_bivector::at " << p;
			throw std::out_of_range(ss.str());
		}		
		return data[coord_to_morton(p)]; 
	}
	T& at(int x, int y) { 
		if (x>=datasize || y>=datasize) {
			std::stringstream ss;
			ss << "morton_bivector::at x=" << x << " y="<<x;
			throw std::out_of_range(ss.str());
		}		
		return data[coord_to_morton(vector2i(x,y))]; 
	}
	const T& at(int x, int y) const { 
		if (x>=datasize || y>=datasize) {
			std::stringstream ss;
			ss << "morton_bivector::at x=" << x << " y="<<x;
			throw std::out_of_range(ss.str());
		}
		return data[coord_to_morton(vector2i(x,y))]; 
	}
	
	T& operator[](const vector2i& p) { return at(p); }
	const T& operator[](const vector2i& p) const { return at(p); }
	const long& size() const { return datasize; }
	void resize(const long& newsz, const T& v = T());
	morton_bivector<T> sub_area(const vector2i& offset, const long& sz) const;

	morton_bivector<T> shifted(const vector2i& offset) const;
	morton_bivector<T> transposed() const;

	void swap(morton_bivector<T>& other) {
		std::swap(datasize, other.datasize);
		data.swap(other.data);
		morton_x.swap(other.morton_x);
		morton_y.swap(other.morton_y);
	}

	template<class U> morton_bivector<U> convert() const;
	template<class U> morton_bivector<U> convert(const T& minv, const T& maxv) const;

	// get pointer to storage, be very careful with that!
	T* data_ptr() { return &data[0]; }
	const T* data_ptr() const { return &data[0]; }
	
	// special operations
	morton_bivector<T> upsampled(bool wrap = false) const;
	morton_bivector<T> downsampled(bool force_even_size = false) const;
	T get_min() const;
	T get_max() const;
	T get_min_abs() const;
	T get_max_abs() const;
	morton_bivector<T>& operator*= (const T& s);
	morton_bivector<T>& operator+= (const T& v);
	morton_bivector<T>& operator+= (const bivector<T>& v);
	morton_bivector<T> smooth_upsampled(bool wrap = false) const;

	// algebraic operations, omponent-wise add, sub, multiply (of same datasize)
	// sum of square of differences etc.

	morton_bivector<T>& add_gauss_noise(const T& scal, random_generator& rg);
	morton_bivector<T>& add_tiled(const bivector<T>& other, const T& scal);
	morton_bivector<T>& add_shifted(const bivector<T>& other, const vector2i& offset);

	morton_bivector<T>& insert(const bivector<T>& other, const vector2i& offset);
protected:

	inline void generate_morton_tables(std::vector<long>&, std::vector<long>&);
	inline unsigned long coord_to_morton(vector2i& coord);
	inline unsigned long coord_to_morton(const vector2i& coord);
	inline unsigned long coord_to_morton(const vector2i& coord) const;
};

template<class T>
void morton_bivector<T>::resize(const long& newsz, const T& v)	{
	data.resize(newsz*newsz, v);
	morton_x.resize(newsz);
	morton_y.resize(newsz);

	if((newsz*newsz)>datasize) {
		datasize = newsz;
		generate_morton_tables(morton_x, morton_y);
	} else datasize = newsz;
}	

template<class T>
inline void morton_bivector<T>::generate_morton_tables(std::vector<long>& table_x, std::vector<long>& table_y) 
{
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
	for (long i=1; i < datasize; i++) {
		d0 = ((d0 | ones0)+1) & ones1;
		d1 = ((d1 | ones1)+1) & ones0;
		table_x[i] = d0;
		table_y[i] = d1;
	}
}

template<class T>
inline unsigned long morton_bivector<T>::coord_to_morton(vector2i& coord) 
{
	return morton_x[coord.x] + morton_y[coord.y];
}

template<class T>
inline unsigned long morton_bivector<T>::coord_to_morton(const vector2i& coord) const
{
	return morton_x[coord.x] + morton_y[coord.y];
}

template<class T>
inline unsigned long morton_bivector<T>::coord_to_morton(const vector2i& coord) 
{
	return morton_x[coord.x] + morton_y[coord.y];
}

template <class T>
T morton_bivector<T>::get_min() const
{
	if (data.empty()) throw std::invalid_argument("morton_bivector::get_min data empty");
	T m = data[0];
	bivector_FOREACH(m = std::min(m, data[z]))
	return m;
}

template <class T>
T morton_bivector<T>::get_max() const
{
	if (data.empty()) throw std::invalid_argument("morton_bivector::get_max data empty");
	T m = data[0];
	bivector_FOREACH(m = std::max(m, data[z]))
	return m;
}


template <class T>
T morton_bivector<T>::get_min_abs() const
{
	if (data.empty()) throw std::invalid_argument("morton_bivector::get_min_abs data empty");
	T m = bivector_abs(data[0]);
	bivector_FOREACH(m = std::min(m, abs(data[z])))
	return m;
}

template <class T>
T morton_bivector<T>::get_max_abs() const
{
	if (data.empty()) throw std::invalid_argument("morton_bivector::get_max_abs data empty");
	T m = bivector_abs(data[0]);
	bivector_FOREACH(m = std::max(m, (T)abs(data[z])))
	return m;
}


template <class T>
morton_bivector<T>& morton_bivector<T>::operator*= (const T& s)
{
	bivector_FOREACH(data[z] *= s)
	return *this;
}

template <class T>
morton_bivector<T>& morton_bivector<T>::operator+= (const T& v)
{
	bivector_FOREACH(data[z] += v)
	return *this;
}

template <class T>
morton_bivector<T>& morton_bivector<T>::operator+= (const bivector<T>& v)
{
	bivector_FOREACH(data[z] += v.data[z])
	return *this;
}

template <class T>
morton_bivector<T> morton_bivector<T>::sub_area(const vector2i& offset, const long& sz) const
{
	if (offset.y + sz > datasize) throw std::invalid_argument("morton_bivector::sub_area, offset.y invalid");
	if (offset.x + sz > datasize) throw std::invalid_argument("morton_bivector::sub_area, offset.x invalid");
	morton_bivector<T> result(sz);
	for (int y=0; y < result.datasize; ++y)
		for (int x=0; x < result.datasize; ++x)
			result.at(x,y) = at(offset.x+x, offset.y+y);
	return result;
}


template <class T>
morton_bivector<T> morton_bivector<T>::shifted(const vector2i& offset) const
{
	bivector<T> result(datasize);
	bivector_FOREACH_XYZ(result.at((x+offset.x) & (datasize-1), (y+offset.y) & (datasize-1)) = data[z])
	return result;
}

template <class T>
morton_bivector<T> morton_bivector<T>::transposed() const
{
	bivector<T> result(datasize);
	bivector_FOREACH_XYZ(result.at(y,x)=data[z])
	return result;
}

template<class T>
template<class U>
morton_bivector<U> morton_bivector<T>::convert() const
{
	bivector<U> result(datasize);
	bivector_FOREACH(result.data[z] = U(data[z]))
	return result;
}

template<class T>
template<class U>
morton_bivector<U> morton_bivector<T>::convert(const T& minv, const T& maxv) const
{
	bivector<U> result(datasize);
	bivector_FOREACH(result.data[z] = U(std::max(minv, std::min(maxv, data[z]))))
	return result;
}

template <class T>
morton_bivector<T> morton_bivector<T>::upsampled(bool wrap) const
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
	if (datasize) throw std::invalid_argument("bivector::upsampled base size invalid");
	long resultsize = wrap ? datasize*2 : datasize*2 - 1;
	morton_bivector<T> result(resultsize);
	// copy values that are kept and interpolate missing values on even rows
	for (int y=0; y < datasize; ++y) {
		for (int x=0; x < datasize-1; ++x) {
			result.at(2*x  , 2*y) = at(x, y);
			result.at(2*x+1, 2*y) = T((at(x, y) + at(x+1, y)) * 0.5);
		}
	}
	// handle special cases on last column
	if (wrap) {
		// copy/interpolate with wrap
		for (int y=0; y < datasize; ++y) {
			result.at(2*datasize-2, 2*y) = at(datasize-1, y);
			result.at(2*datasize-1, 2*y) = T((at(datasize-1, y) + at(0, y)) * 0.5);
		}
	} else {
		// copy last column
		for (int y=0; y < datasize; ++y) {
			result.at(2*datasize-2, 2*y) = at(datasize-1, y);
		}
	}
	// interpolate missing values on odd rows
	for (int y=0; y < datasize-1; ++y) {
		for (int x=0; x < resultsize; ++x) {
			result.at(x, 2*y+1) = T((result.at(x, 2*y) + result.at(x, 2*y+2)) * 0.5);
		}
	}
	// handle special cases on last row
	if (wrap) {
		// interpolate last row with first and second-to-last
		for (int x=0; x < resultsize; ++x) {
			result.at(x, resultsize-1) = T((result.at(x, resultsize-2) + result.at(x, 0)) * 0.5);
		}
	}
	return result;
}

template <class T>
morton_bivector<T> morton_bivector<T>::downsampled(bool force_even_size) const
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
	long newsize = datasize >> 1;
	long resultsize = newsize;
	if (!force_even_size) {
		resultsize += datasize & 1;
	}
	morton_bivector<T> result(resultsize);
	for (int y=0; y < newsize; ++y)
		for (int x=0; x < newsize; ++x)
			result.at(x,y) = T((at(2*x, 2*y) + at(2*x+1, 2*y) + at(2*x, 2*y+1) + at(2*x+1, 2*y+1)) * 0.25);
	if (!force_even_size) {
		// downsample last row or column if original size is odd
		if (datasize & 1) {
			for (int y=0; y < newsize; ++y)
				result.at(newsize, y) = T((at(datasize-1, 2*y) + at(datasize-1, 2*y+1)) * 0.5);
		}
		if (datasize & 1) {
			for (int x=0; x < newsize; ++x)
				result.at(x, newsize) = T((at(2*x, datasize-1) + at(2*x+1, datasize-1)) * 0.5);
		}
		// copy corner, hasn't been handled yet
		result.at(newsize, newsize) = at(datasize-1, datasize-1);
	}
	return result;
}

template <class T>
morton_bivector<T> morton_bivector<T>::smooth_upsampled(bool wrap) const
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
	if (datasize < 3) throw std::invalid_argument("morton_bivector::smooth_upsampled base size invalid");
	long resultsize = wrap ? datasize*2 : datasize*2 - 1;
	morton_bivector<T> result(resultsize);
	// copy values that are kept and interpolate missing values on even rows
	for (int y=0; y < datasize; ++y) {
		result.at(0, 2*y) = at(0, y);
		for (int x=1; x < datasize-2; ++x) {
			result.at(2*x  , 2*y) = at(x, y);
			result.at(2*x+1, 2*y) = T(at(x-1, y) * c1[0] +
						  at(x  , y) * c1[1] +
						  at(x+1, y) * c1[2] +
						  at(x+2, y) * c1[3]);
		}
		result.at(2*datasize-4, 2*y) = at(datasize-2, y);
		result.at(2*datasize-2, 2*y) = at(datasize-1, y);
	}
	if (wrap) {
		for (int y=0; y < datasize; ++y) {
			result.at(1, 2*y) = T(at(datasize-1, y) * c1[0] +
					      at(0, y) * c1[1] +
					      at(1, y) * c1[2] +
					      at(2, y) * c1[3]);
			result.at(2*datasize-3, 2*y) = T(at(datasize-3, y) * c1[0] +
							   at(datasize-2, y) * c1[1] +
							   at(datasize-1, y) * c1[2] +
							   at(           0, y) * c1[3]);
			result.at(2*datasize-1, 2*y) = T(at(datasize-2, y) * c1[0] +
							   at(datasize-1, y) * c1[1] +
							   at(           0, y) * c1[2] +
							   at(           1, y) * c1[3]);
		}
	} else {
		for (int y=0; y < datasize; ++y) {
			result.at(1, 2*y) = T(at(0, y) * c1[0] +
					      at(0, y) * c1[1] +
					      at(1, y) * c1[2] +
					      at(2, y) * c1[3]);
			result.at(2*datasize-3, 2*y) = T(at(datasize-3, y) * c1[0] +
							   at(datasize-2, y) * c1[1] +
							   at(datasize-1, y) * c1[2] +
							   at(datasize-1, y) * c1[3]);
		}
	}
	// interpolate missing values on odd rows
	for (int y=1; y < datasize-2; ++y) {
		for (int x=0; x < resultsize; ++x) {
			result.at(x, 2*y+1) = T(result.at(x, 2*y-2) * c1[0] +
						result.at(x, 2*y  ) * c1[1] +
						result.at(x, 2*y+2) * c1[2] +
						result.at(x, 2*y+4) * c1[3]);
		}
	}
	// handle special cases on last row
	if (wrap) {
		// interpolate 3 missing rows
		for (int x=0; x < resultsize; ++x) {
			result.at(x, 1) = T(result.at(x, 2*datasize-2) * c1[0] +
					    result.at(x,              0) * c1[1] +
					    result.at(x,              2) * c1[2] +
					    result.at(x,              4) * c1[3]);
			result.at(x, 2*datasize-3) = T(result.at(x, 2*datasize-6) * c1[0] +
							 result.at(x, 2*datasize-4) * c1[1] +
							 result.at(x, 2*datasize-2) * c1[2] +
							 result.at(x,              0) * c1[3]);
			result.at(x, 2*datasize-1) = T(result.at(x, 2*datasize-4) * c1[0] +
							 result.at(x, 2*datasize-2) * c1[1] +
							 result.at(x,              0) * c1[2] +
							 result.at(x,              2) * c1[3]);
		}
	} else {
		for (int x=0; x < resultsize; ++x) {
			result.at(x, 1) = T(result.at(x, 0) * c1[0] +
					    result.at(x, 0) * c1[1] +
					    result.at(x, 2) * c1[2] +
					    result.at(x, 4) * c1[3]);
			result.at(x, 2*datasize-3) = T(result.at(x, 2*datasize-6) * c1[0] +
							 result.at(x, 2*datasize-4) * c1[1] +
							 result.at(x, 2*datasize-2) * c1[2] +
							 result.at(x, 2*datasize-2) * c1[3]);
		}
	}
	return result;
}

template <class T>
morton_bivector<T>& morton_bivector<T>::add_gauss_noise(const T& scal, random_generator& rg)
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
morton_bivector<T>& morton_bivector<T>::add_tiled(const bivector<T>& other, const T& scal)
{
	bivector_FOREACH_XY(at(x,y) += other.at(x & (other.datasize-1), y & (other.datasize-1)) * scal;)
	return *this;
}

template <class T>
morton_bivector<T>& morton_bivector<T>::add_shifted(const bivector<T>& other, const vector2i& offset)
{
	bivector_FOREACH_XY(at(x,y) += other.at((x+offset.x) & (other.datasize-1), (y+offset.y) & (other.datasize-1));)
	return *this;
}

template <class T>
morton_bivector<T>& morton_bivector<T>::insert(const bivector<T>& other, const vector2i& offset)
{
	for (int y=0; y<other.size().y; y++) {
		for (int x=0; x<other.size().x; x++) {
			at(offset+vector2i(x,y)) = other.at(x,y);
		}
	}
	return *this;
}

#undef bivector_FOREACH
#undef bivector_FOREACH_XY
#undef bivector_FOREACH_XYZ
#undef bivector_abs
#endif