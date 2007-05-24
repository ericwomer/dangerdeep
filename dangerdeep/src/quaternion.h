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
//  A quaternion (C)+(W) 2001 Thorsten Jordan
//

#ifndef QUATERNION_H
#define QUATERNION_H

#include <math.h>
#include "vector3.h"
#include "matrix4.h"

/*
  3d rotations with quaternions:
  qr is rotation quaternion: qr = quaternion::rot(angle, axis)
  qp is 3d point quaternion: qp = quaternion::vec(point)
  rotation is: qp' = qr*qp*qr^-1, with qr = unit quaternion, and hence qr^-1 = qr.conj()
  therefore: qp' = qr*qp*conj(qr)
*/

///\brief This class represents quaternions. They're used for fast computation of rotations
///\note Angles given as parameters or returned angles are in degrees!
template<class D>
class quaterniont
{
	public:		// we use a spatial representation
	D s;		// r
	vector3t<D> v;	// i, j, k

	quaterniont() : s(1), v() {}	// note: not fully zero, but neutral rotation instead
	quaterniont(const D &r_, const D &i_, const D &j_, const D &k_) : s(r_), v(i_, j_, k_) {}
	quaterniont(const D &s_, const vector3t<D>& v_) : s(s_), v(v_) {}
	quaterniont<D> normal() const { D len = D(1.0)/length(); return quaterniont(s * len, v * len); }
	void normalize() { D len = D(1.0)/length(); s *= len; v *= len; }
	quaterniont<D> operator* (const D &scalar) const { return quaterniont(s * scalar, v * scalar); }
	quaterniont<D> operator+ (const quaterniont<D>& other) const { return quaterniont(s + other.s, v + other.v); }
	quaterniont<D> operator- (const quaterniont<D>& other) const { return quaterniont(s - other.s, v - other.v); }
	quaterniont<D> operator- () const { return quaterniont(-s, -v); }
	quaterniont<D> conj() const { return quaterniont(s, -v); }
	quaterniont<D>& operator+= (const quaterniont<D>& other) { s += other.s; v += other.v; return *this; }
	quaterniont<D>& operator-= (const quaterniont<D>& other) { s -= other.s; v -= other.v; return *this; }
	quaterniont<D>& operator*= (const quaterniont<D>& other) { *this = *this * other; return *this; }
	bool operator== (const quaterniont<D>& other) const { return s == other.s && v == other.v; }
	D square_length() const { return s * s + v * v; }
	D length() const { return D(sqrt(square_length())); }
	quaterniont<D> operator* (const quaterniont<D>& o) const { return quaterniont(s*o.s-v*o.v, o.v*s+v*o.s+v.cross(o.v)); }
	template<class D2> friend std::ostream& operator<< ( std::ostream& os, const quaterniont<D2>& q );
	template<class E> void assign(const vector2t<E>& other) { s = D(other.s); v.assign(other.v); }
	static quaterniont<D> vec(const D& x, const D& y, const D& z) { return quaterniont(0, vector3t<D>(x, y, z)); }
	static quaterniont<D> vec(const vector3t<D>& p) { return quaterniont(D(0), p); }
	static quaterniont<D> zero() { return quaterniont(D(0), vector3t<D>()); }
	// angle is divided by 2 for a rotation quaternion, so divide by 360, not 180.
	static quaterniont<D> rot(const D& angle, const D& x, const D& y, const D& z) { D rad = D(angle*M_PI/360.0); return quaterniont(cos(rad), vector3t<D>(x, y, z) * sin(rad)); }
	static quaterniont<D> rot(const D& angle, const vector3t<D>& axis) { D rad = D(angle*M_PI/360.0); return quaterniont(cos(rad), axis * sin(rad)); }
	static quaterniont<D> rot_rad(const D& angle_rad, const vector3t<D>& axis) { return quaterniont(cos(angle_rad), axis * sin(angle_rad)); }
	// reverse operation: angle * 2 * 180
	void angleaxis(D& angle, D& x, D& y, D& z) const { D a = acos(s); angle = D(a*360.0/M_PI); D sa = sin(a); x = v.x/sa; y = v.y/sa; z = v.z/sa; }
	void angleaxis(D& angle, vector3t<D>& axis) const { D a = acos(s); angle = D(a*360.0/M_PI); axis = v * (D(1.0)/sin(a)); }
	vector3t<D> rotate(const D& x, const D& y, const D& z) const { quaterniont<D> p2 = *this * vec(x, y, z) * this->conj(); return p2.v; }
	vector3t<D> rotate(const vector3t<D>& p) const { quaterniont<D> p2 = *this * vec(p) * this->conj(); return p2.v; }
	
	quaterniont<D> scale_rot_angle(const D& scal)
	{
		D ang = acos(s);
		if (ang <= D(0.00001))
			return quaterniont();
		D sa = sin(ang*scal)/sin(ang);
		return quaterniont(cos(ang*scal), v * sa);
	}
	
	matrixt<D, 3U> rotmat() const {
		D x2 = v.x*v.x, y2 = v.y*v.y, z2 = v.z*v.z;
		D xy = v.x*v.y, wz = s*v.z, xz = v.x*v.z, wy = s*v.y, yz = v.y*v.z, wx = s*v.x;
		return matrixt<D, 3U>(
			1-2*(y2+z2),	2*(xy-wz),	2*(xz+wy),
			2*(xy+wz),	1-2*(x2+z2),	2*(yz-wx),
			2*(xz-wy),	2*(yz+wx),	1-2*(x2+y2)
		);
	}

	matrixt<D, 4U> rotmat4() const {
		D x2 = v.x*v.x, y2 = v.y*v.y, z2 = v.z*v.z;
		D xy = v.x*v.y, wz = s*v.z, xz = v.x*v.z, wy = s*v.y, yz = v.y*v.z, wx = s*v.x;
		return matrixt<D, 4U>(
			1-2*(y2+z2),	2*(xy-wz),	2*(xz+wy),	0,
			2*(xy+wz),	1-2*(x2+z2),	2*(yz-wx),	0,
			2*(xz-wy),	2*(yz+wx),	1-2*(x2+y2),	0,
			0,	0,	0,	1
		);
	}

	// m must be a rotation matrix
	static quaterniont<D> from_rotmat(const matrixt<D, 4U>& m) {
		D tr = m.elem(0,0) + m.elem(1,1) + m.elem(2,2);
		quaterniont result;
		if (tr > D(0.0)) {
			D s = D(sqrt(tr + D(1.0)));
			result.s = s/2;
			s = D(1.0)/(D(2)*s);
			result.v.x = (m.elem(1,2) - m.elem(2,1)) * s;
			result.v.y = (m.elem(2,0) - m.elem(0,2)) * s;
			result.v.z = (m.elem(0,1) - m.elem(1,0)) * s;
		} else {
			unsigned next[3] = { 1, 2, 0 };
			D* ptr[3] = { &result.v.x, &result.v.y, &result.v.z };
			unsigned i = 0;
			if (m.elem(1,1) > m.elem(i,i)) i = 1;
			if (m.elem(2,2) > m.elem(i,i)) i = 2;
			unsigned j = next[i];
			unsigned k = next[j];
			D s = D(sqrt(m.elem(i,i) - (m.elem(j,j) + m.elem(k,k)) + D(1)));
			*(ptr[i]) = s/2;
			s = D(1.0)/(D(2)*s);
			result.s = (m.elem(j,k) - m.elem(k,j)) * s;
			*(ptr[j]) = (m.elem(i,j) - m.elem(j,i)) * s;
			*(ptr[k]) = (m.elem(i,k) - m.elem(k,i)) * s;
		}
		return result;
	}
};

template<class D2> inline quaterniont<D2> operator* (const D2& scalar, const quaterniont<D2>& q) { return q * scalar; }

template<class D>
std::ostream& operator<< ( std::ostream& os, const quaterniont<D>& q )
{
	os << "r=" << q.s << "; i=" << q.v.x << "; j=" << q.v.y << "; k=" << q.v.z;
	return os;
}

typedef quaterniont<double> quaternion;
typedef quaterniont<float> quaternionf;

#endif
