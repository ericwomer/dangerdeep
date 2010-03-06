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
//  A 4d (homogenous) vector (C)+(W) 2005 Thorsten Jordan
//

#ifndef VECTOR4_H
#define VECTOR4_H

#include "vector3.h"

///\brief Template class for a mathematical vector with four coefficients.
template<class D>
class vector4t
{
	public:
	D x, y, z, w;

	vector4t() : x(0), y(0), z(0), w(0) {}
	vector4t(const D& x_, const D& y_, const D& z_, const D& w_) : x(x_), y(y_), z(z_), w(w_) {}
	// not all of the following operations are sensible for homogenous vectors.
	vector4t<D> normal() const { D len = D(1.0)/length(); return vector4t(x * len, y * len, z * len, w * len); }
	void normalize() { D len = D(1.0)/length(); x *= len; y *= len; z *= len; w *= len; }
	vector4t<D> operator* (const D &scalar) const { return vector4t(x * scalar, y * scalar, z * scalar, w * scalar); }
	vector4t<D> operator+ (const vector4t<D>& other) const { return vector4t(x + other.x, y + other.y, z + other.z, w + other.w); }
	vector4t<D> operator- (const vector4t<D>& other) const { return vector4t(x - other.x, y - other.y, z - other.z, w - other.w); }
	vector4t<D> operator- () const { return vector4t(-x, -y, -z, -w); };
	vector4t<D>& operator+= (const vector4t<D>& other) { x += other.x; y += other.y; z += other.z; w += other.w; return *this; }
	vector4t<D>& operator-= (const vector4t<D>& other) { x -= other.x; y -= other.y; z -= other.z; w -= other.w; return *this; }
	bool operator== (const vector4t<D>& other) const { return x == other.x && y == other.y && z == other.z && w == other.w; }
	D square_length() const { return x * x + y * y + z * z + w * w; }
	D length() const { return D(sqrt(square_length())); }
	D square_distance(const vector4t<D>& other) const { vector4t<D> n = *this - other; return n.square_length(); }
	D distance(const vector4t<D>& other) const { vector4t<D> n = *this - other; return n.length(); }
	D operator* (const vector4t<D>& other) const { return x * other.x + y * other.y + z * other.z + w * other.w; }
	vector2t<D> xy() const { return vector2t<D>(x, y); }
	vector3t<D> xyz() const { return vector3t<D>(x, y, z); }
	vector3t<D> to_real() const { return (w == 0) ? vector3t<D>() : vector3t<D>(x/w, y/w, z/w); }
	template<class D2> friend std::ostream& operator<< ( std::ostream& os, const vector4t<D2>& v );
	template<class E> void assign(const vector4t<E>& other) { x = D(other.x); y = D(other.y); z = D(other.z); w = D(other.w); }
};

template<class D2> inline vector4t<D2> operator* (const D2& scalar, const vector4t<D2>& v) { return v * scalar; }

template<class D>
std::ostream& operator<< ( std::ostream& os, const vector4t<D>& v )
{
	os << "x=" << v.x << "; y=" << v.y << "; z=" << v.z << "; w=" << v.w;
	return os;
}

typedef vector4t<double> vector4;
typedef vector4t<float> vector4f;
typedef vector4t<int> vector4i;

#endif
