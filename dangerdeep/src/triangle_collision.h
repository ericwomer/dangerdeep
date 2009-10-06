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
//  A triangle to triangle collision test (C)+(W) 2009 Thorsten Jordan
//

#ifndef TRIANGLE_COLLISION_H
#define TRIANGLE_COLLISION_H

#include "vector3.h"

/// an algorithm for computing triangle to triangle intersections
template <class T>
class triangle_collision_t
{
 public:
	static bool compute(const vector3t<T>& va0,
			    const vector3t<T>& va1,
			    const vector3t<T>& va2,
			    const vector3t<T>& vb0,
			    const vector3t<T>& vb1,
			    const vector3t<T>& vb2)
	{
		// To compute for intersection we have the two triangles as
		// parametric form: P + a0 * D0 + a1 * D1
		// Q + b0 * W0 + b1 * W1, we need to check for intersection
		// of every edge of triangle 1 with triangle 0,
		// thus P + a0 * D0 + a1 * D1 = Qi + bi * Wi
		// and then solve for a0, a1, bi where i in [0..2].
		// It must be true that 0 <= bi <= 1
		// and 0 <= a0, a1 abd a0 + a1 <= 1
		// Thus we have the equation
		// a0 * D0 + a1 * D1 - bi * Wi = Qi - P
		// This can be written in matrix form
		// ( D0 | D1 | Wi ) * (a0, a1, -bi)^T = Qi - P
		// this can be solved and checked for bi first.
		// Exactly two bi's must be legal, it is sufficient to check
		// two of them (and to compute only two).
		// If either one or both are legel, we compute a0, a1,
		// and check for their legality.
		const vector3t<T>& P = va0;
		vector3t<T> D0 = va1 - va0;
		vector3t<T> D1 = va2 - va0;
		const vector3t<T>& Q = vb0; // Qi == Q always
		vector3t<T> W0 = vb1 - vb0;
		vector3t<T> W1 = vb2 - vb0;
		vector3t<T> R = Q - P;
		T det = D0.determinant(D1, R);
		// fixme: determinant computation by reduction of minors, is faster
		T det0 = D0.determinant(D1, W0);
		T det1 = D0.determinant(D1, W1);
		T b0 = -det * det0;
		bool b0_legal = (b0 >= T(0.0) && b0 <= det0*det0 && det0 != T(0));
		T b1 = -det * det1;
		bool b1_legal = (b1 >= T(0.0) && b1 <= det1*det1 && det1 != T(0));
		if (det0 == T(0) && det1 == T(0)) {
			if (det != T(0)) {
				// triangles are on parallel planes
				return false;
			}
			// triangles are on same plane
			return false;
		}
		if (b0_legal) {
			if (b1_legal) {
			} else {
			}
		} else if (b1_legal) {
		} else {
			return false; // all vertices are on one side
		}

		// fixme
		return true;
	}
};

#endif
