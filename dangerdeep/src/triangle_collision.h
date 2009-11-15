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
#include "vector2.h"

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
		// parametric form: P + a0 * p0 + a1 * p1
		// Q + b0 * q0 + b1 * q1, we need to check for intersection
		// of every edge of triangle 1 with triangle 0,
		// thus P + a0 * p0 + a1 * p1 = Qi + bi * qi
		// where Q0 = Q1 = Q and Q2 = Q + q0 and q2 = q1 - q0
		// and then solve for a0, a1, bi where i in [0..2].
		// It must be true that 0 <= bi <= 1 if edges cut the plane
		// of triangle 1.
		// Thus we have the equation
		// a0 * p0 + a1 * p1 - bi * qi = Qi - P =: Ri
		// This can be written in matrix form
		// ( p0 | p1 | qi ) * (a0, a1, -bi)^T = Qi - P
		// this can be solved and checked for bi first.
		// Exactly two bi's must be legal, it is sufficient to check
		// two of them (and to compute only two).
		// We define the matrix A(v) as matrix ( p0 | p1 | v )
		// and then we can rewrite the equation above as
		// A(qi) * (a0, a1, -bi)^T = Ri
		// this can be solved with determinates to bi
		// bi = - |A(Ri)| / |A(qi)|
		// we solve for b0 and b1:
		// b0 = - |A(R0)| / |A(q0)|
		// b1 = - |A(R1)| / |A(q1)|
		// note that R0 = R1 and to avoid divisions we compute
		// b0 * |A(q0)|^2 = - |A(R)| * |A(q1)|
		// b1 * |A(q1)|^2 = - |A(R)| * |A(q1)|
		// we need to multiply by squares to keep result of same sign.
		// determinates can be computed cheaper by computing the left two
		// columns 2x2 matrix determinates first, where the two left
		// columns are equal on all of these matrices.
		// After having found two intersections from b0,b1,b2
		// we compute the matching a0,a1 pairs for these two
		// and run a 2d line segment to triangle intersection test with them.
		const T nullT = T(0);
		const vector3t<T>& P = va0;
		vector3t<T> p0 = va1 - va0;
		vector3t<T> p1 = va2 - va0;
		const vector3t<T>& Q = vb0;
		vector3t<T> q0 = vb1 - vb0;
		vector3t<T> q1 = vb2 - vb0;
		vector3t<T> R = Q - P;
		T dp0 = p0.y * p1.z - p1.y * p0.z;
		T dp1 = p0.x * p1.z - p1.x * p0.z;
		T dp2 = p0.x * p1.y - p1.x * p0.y;
		T detAq0 = q0.x * dp0 - q0.y * dp1 + q0.z * dp2;
		T detAq1 = q1.x * dp0 - q1.y * dp1 + q1.z * dp2;
		if (detAq0 == nullT && detAq1 == nullT) {
			// either triangles are on parallel planes when detAr != 0
			// or triangles are on the same plane
			return false;
		}
		T detAr  =  R.x * dp0 -  R.y * dp1 +  R.z * dp2; // = detAr0 = detAr1
		T b0 = - detAr * detAq0;
		T b1 = - detAr * detAq1;
		// fixme use bit-wise AND here, faster!
		bool b0_legal = (b0 >= nullT) && (b0 <= detAq0*detAq0) && (detAq0 != nullT);
		bool b1_legal = (b1 >= nullT) && (b1 <= detAq1*detAq1) && (detAq1 != nullT);

		// this can be computed because of linearity of determinant computing
		T detAq2 = detAq1 - detAq0;

		// not at least one intersection (there must be totally two), then no
		// intersection at all
		if (!b0_legal && !b1_legal) {
			return false;
		}

		vector2t<T> aone, atwo;
		if (b0_legal) {
			// compute a0, a1 for b0
			aone.x = R.determinant(p1, q0) / detAq0;
			aone.y = p0.determinant(R, q0) / detAq0;
			if (b1_legal) {
				atwo.x = R.determinant(p1, q1) / detAq1;
				atwo.y = p0.determinant(R, q1) / detAq1;
			} else {
				vector3t<T> q2 = q1 - q0;
				vector3t<T> R2 = R + q0;
				// det(R2 | p1 | q2) = det(R | p1 | q2) + det(q0 | p1 | q2)
				// and q2 = q1 - q0 so
				// = det(R | p1 | q1) - det(R | p1 | q0)
				//  + det(q0 | p1 | q1) - det(q0 | p1 | q0)
				// but we can't reuse one of them nor make use
				// of linearity here
				atwo.x = R2.determinant(p1, q2) / detAq2;
				atwo.y = p0.determinant(R2, q2) / detAq2;
			}
		} else {
			// b1_legal must be true here
			aone.x = R.determinant(p1, q1) / detAq1;
			aone.y = p0.determinant(R, q1) / detAq1;
			vector3t<T> q2 = q1 - q0;
			vector3t<T> R2 = R + q0;
			atwo.x = R2.determinant(p1, q2) / detAq2;
			atwo.y = p0.determinant(R2, q2) / detAq2;
		}

		// now we solve for intersections of line aone, atwo-aone
		// with triangle (0,0) + delta0 * (1,0) + delta1 * (0,1)
		const vector2t<T>& t = aone;
		vector2t<T> d = atwo - aone;
		T dtd = t.x * d.y - t.y * d.x;
		T delta0 = dtd * d.y; // scaled by d.y^2
		T delta1 = -dtd * d.x; // scaled by d.x^2
		T dx2 = d.x * d.x;
		T dy2 = d.y * d.y;
		// fixme: use bitwise and here
		bool delta0_legal = (d.y != nullT) && (delta0 >= nullT) && (delta0 <= dy2);
		bool delta1_legal = (d.x != nullT) && (delta1 >= nullT) && (delta1 <= dx2);
		// there are either two deltas legal or none (if one of delta0, delta1 is
		// legal we assume delta2 would be legal as well)
		if (delta0_legal) {
			T gamma0 = -t.y * d.y; // scaled by d.y^2
			if (delta1_legal) {
				// this case is most common
				T gamma1 = -t.x * d.x; // scaled by d.x^2
				//printf("gamma0 1 %f %f dy2 x2 %f %f\n",gamma0,gamma1,dy2,dx2);
				return	((gamma0 >= nullT) && (gamma0 <= dy2)) ||
					((gamma1 >= nullT) && (gamma1 <= dx2)) ||
					gamma0 * gamma1 < 0;
			} else {
				T dxpdy = d.x + d.y;
				T gamma2 = (T(1) - t.x - t.y) * dxpdy; // scaled by dxpdy^2
				//printf("gamma0 2 %f %f dy2 dxpdy*dxpdy %f %f\n",gamma0,gamma2,dy2,dxpdy*dxpdy);
				return	((gamma0 >= nullT) && (gamma0 <= dy2)) ||
					((gamma2 >= nullT) && (gamma2 <= dxpdy*dxpdy)) ||
					gamma0 * gamma2 < 0;
			}
		} else if (delta1_legal) {
			T gamma1 = -t.x * d.x; // scaled by d.x^2
			T dxpdy = d.x + d.y;
			T gamma2 = (T(1) - t.x - t.y) * dxpdy; // scaled by dxpdy^2
			//printf("d1l %i  gamma1 2 %f %f dx2 dxpdy*dxpdy %f %f\n",delta1_legal,gamma1,gamma2,dx2,dxpdy*dxpdy);
			return	((gamma1 >= nullT) && (gamma1 <= dx2)) ||
				((gamma2 >= nullT) && (gamma2 <= dxpdy*dxpdy)) ||
				gamma1 * gamma2 < 0;
		}
		return false;
	}
};

typedef triangle_collision_t<double> triangle_collision;
typedef triangle_collision_t<float> triangle_collisionf;

#endif
