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
//  A 2d vector (C)+(W) 2001 Thorsten Jordan
//

#ifndef VECTOR2_H
#define VECTOR2_H

#include <math.h>

#ifdef WIN32
#undef min
#undef max
#endif

#include <iostream>

template <class D2> class vector3t;

///\brief Template class for a mathematical vector with two coefficients.
template <class D>
class vector2t
{
 public:
	D x, y;

	vector2t() : x(0), y(0) {}
	vector2t(const D &x_, const D &y_) : x(x_), y(y_) {}
	vector2t<D> normal() const { D len = D(1.0)/length(); return vector2t(x * len, y * len); }
	void normalize() { D len = D(1.0)/length(); x *= len; y *= len; }
	vector2t<D> orthogonal() const { return vector2t(-y, x); }
	vector2t<D> operator* (const D &scalar) const { return vector2t(x * scalar, y * scalar); }
	vector2t<D> operator+ (const vector2t<D>& other) const { return vector2t(x + other.x, y + other.y); }
	vector2t<D> operator- (const vector2t<D>& other) const { return vector2t(x - other.x, y - other.y); }
	vector2t<D> operator- () const { return vector2t(-x, -y); }
	vector2t<D>& operator+= (const vector2t<D>& other) { x += other.x; y += other.y; return *this; }
	vector2t<D>& operator-= (const vector2t<D>& other) { x -= other.x; y -= other.y; return *this; }
	vector2t<D> min(const vector2t<D>& other) const { return vector2t(x < other.x ? x : other.x, y < other.y ? y : other.y); }
	vector2t<D> max(const vector2t<D>& other) const { return vector2t(x > other.x ? x : other.x, y > other.y ? y : other.y); }
	bool operator== (const vector2t<D>& other) const { return x == other.x && y == other.y; }
	D square_length() const { return x * x + y * y; }
	D length() const { return D(sqrt(square_length())); }
	D square_distance(const vector2t<D>& other) const { vector2t<D> n = *this - other; return n.square_length(); }
	D distance(const vector2t<D>& other) const { vector2t<D> n = *this - other; return n.length(); }
	D operator* (const vector2t<D>& other) const { return x * other.x + y * other.y; }
	bool solve(const vector2t<D>& o1, const vector2t<D>& o2, D& s1, D& s2) const;
	// multiplies 2x2 matrix (given in columns c0-c1) with *this.
	vector2t<D> matrixmul(const vector2t<D>& c0, const vector2t<D>& c1) const;
	vector2t<D> coeff_mul(const vector2t<D>& other) const { return vector2t(x * other.x, y * other.y); }
	vector3t<D> xy0() const { return vector3t<D>(x, y, 0); }
	vector3t<D> xyz(const D& z) const { return vector3t<D>(x, y, z); }
	template<class D2> friend std::ostream& operator<< ( std::ostream& os, const vector2t<D2>& v );
	template<class E> void assign(const vector2t<E>& other) { x = D(other.x); y = D(other.y); }
};

template <class D>
bool vector2t<D>::solve(const vector2t<D>& o1, const vector2t<D>& o2, D& s1, D& s2) const
{
	D det = o1.x*o2.y - o2.x*o1.y;
	if (!det) return false;
	s1 = (o2.y*x - o2.x*y) / det;
	s2 = (o1.x*y - o1.y*x) / det;
	return true;
}

template<class D>
vector2t<D> vector2t<D>::matrixmul(const vector2t<D>& c0, const vector2t<D>& c1) const
{
	return vector2t<D>(	c0.x * x + c1.x * y,
				c0.y * x + c1.y * y );
}

template<class D2> inline vector2t<D2> operator* (const D2& scalar, const vector2t<D2>& v)
{
	return v * scalar;
}

template<class D2> std::ostream& operator<< ( std::ostream& os, const vector2t<D2>& v )
{
	os << "x=" << v.x << "; y=" << v.y;
	return os;
}

typedef vector2t<double> vector2;
typedef vector2t<float> vector2f;
typedef vector2t<int> vector2i;
typedef vector2t<long int> vector2l;

#endif
