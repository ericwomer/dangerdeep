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
//  A 3d plane (C)+(W) 2005 Thorsten Jordan
//

#ifndef PLANE_H
#define PLANE_H

#include "vector3.h"

template<class D>
class plane_t
{
// static const variables must not be double/float, some compilers are picky.
#define plane_t__EPSILON 0.001
public:
	vector3t<D> N;
	D d;

	plane_t() : d(0) {}
	plane_t(const vector3t<D>& N_, const D& d_) : N(N_), d(d_) {}
	plane_t(const vector3t<D>& N_, const vector3t<D>& pivot) : N(N_), d(-N_ * pivot) {}
	plane_t(const D& a, const D& b, const D& c, const D& d_) : N(a, b, c), d(d_) {}
	/// construct from three points.
	plane_t(const vector3t<D>& a, const vector3t<D>& b, const vector3t<D>& c)
		: N((b-a).cross(c-a).normal())
	{
		d = -(N * a);
	}
	/// determine if point is left of plane (on side that normal points to).
	bool is_left(const vector3t<D>& a) const {
		return (N * a >= -d);
	}
	/// determine if point is left of/right of/in plane (>0,<0,==0)
	int test_side(const vector3t<D>& a) const {
		D r = N * a + d;
		return (r > plane_t__EPSILON) ? 1 : ((r < -plane_t__EPSILON) ? -1 : 0);
	}
	/// determine distance of point to plane
	D distance(const vector3t<D>& a) const {
		return N * a + d;
	}
	/// compute intersection point of line a->b, assumes that a->b intersects the plane
	vector3t<D> intersection(const vector3t<D>& a, const vector3t<D>& b) const {
		D divi = N * (b - a);
		// if abs(divi) is near zero then a,b are both on the plane
		D t = - (d + N * a) / divi;
		return a + (b - a) * t;
	}
	/// compute intersection point of line a->b, returns true if intersection is valid
	bool test_intersection(const vector3t<D>& a, const vector3t<D>& b, vector3t<D>& result) const {
		bool a_left = is_left(a), b_left = is_left(b);
		// both are left or not left? no intersection
		if (a_left == b_left)
			return false;
		result = intersection(a, b);
		return true;
	}
	/// compute intersection point of line a->b, returns true if intersection is valid
	bool test_intersection_no_touch(const vector3t<D>& a, const vector3t<D>& b, vector3t<D>& result) const {
		int a_left = test_side(a), b_left = test_side(b);
		// both are left or not left? no intersection
		if (a_left * b_left >= 0)
			return false;
		result = intersection(a, b);
		return true;
	}
	/// translate
	void translate(const vector3t<D>& delta) {
		d -= delta * N;
	}
	/// compute pivot point
	vector3t<D> get_pivot() const {
		return N * -d;
	}
};

typedef plane_t<double> plane;
typedef plane_t<float> planef;

#endif
