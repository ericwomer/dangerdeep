//
//  A quaternion (C)+(W) 2001 Thorsten Jordan
//

#ifndef QUATERNION_H
#define QUATERNION_H

#include <iostream>
#include <cmath>
#include "vector3.h"
#include "matrix4.h"
using namespace std;

/*
  3d rotations with quaternions:
  qr is rotation quaternion: qr = quaternion::rot(angle, axis)
  qp is 3d point quaternion: qp = quaternion::vec(point)
  rotation is: qp' = qr*qp*qr^-1, with qr = unit quaternion, and hence qr^-1 = qr.conj()
  therefore: qp' = qr*qp*conj(qr)
*/

// attention: angles given as parameters or returned angles are in degrees!

template<class D>
class quaterniont
{
	public:		// we use a spatial representation
	D s;		// r
	vector3t<D> v;	// i, j, k

	quaterniont() : s(1), v() {}	// note: not fully zero, but neutral rotation instead
	quaterniont(const quaterniont<D>& o) : s(o.s), v(o.v) {}
	quaterniont& operator= (const quaterniont<D>& o) { s = o.s; v = o.v; return *this; }
	quaterniont(const D &r_, const D &i_, const D &j_, const D &k_) : s(r_), v(i_, j_, k_) {}
	quaterniont(const D &s_, const vector3t<D>& v_) : s(s_), v(v_) {}
	quaterniont<D> normal(void) const { D len = D(1.0)/length(); return quaterniont(s * len, v * len); }
	void normalize(void) { D len = D(1.0)/length(); s *= len; v *= len; }
	quaterniont<D> operator* (const D &scalar) const { return quaterniont(s * scalar, v * scalar); }
	quaterniont<D> operator+ (const quaterniont<D>& other) const { return quaterniont(s + other.s, v + other.v); }
	quaterniont<D> operator- (const quaterniont<D>& other) const { return quaterniont(s - other.s, v - other.v); }
	quaterniont<D> operator- (void) const { return quaterniont(-s, -v); }
	quaterniont<D> conj(void) const { return quaterniont(s, -v); }
	quaterniont<D>& operator+= (const quaterniont<D>& other) { s += other.s; v += other.v; return *this; }
	quaterniont<D>& operator-= (const quaterniont<D>& other) { s -= other.s; v -= other.v; return *this; }
	quaterniont<D>& operator*= (const quaterniont<D>& other) { *this = *this * other; return *this; }
	bool operator== (const quaterniont<D>& other) const { return s == other.s && v == other.v; }
	D square_length(void) const { return s * s + v * v; }
	D length(void) const { return D(sqrt(square_length())); }
	quaterniont<D> operator* (const quaterniont<D>& o) const { return quaterniont(s*o.s-v*o.v, o.v*s+v*o.s+v.cross(o.v)); }
	template<class D2> friend ostream& operator<< ( ostream& os, const quaterniont<D2>& q );
	template<class E> void assign(const vector2t<E>& other) { s = D(other.s); v.assign(other.v); }
	static quaterniont<D> vec(const D& x, const D& y, const D& z) { return quaterniont(0, vector3t<D>(x, y, z)); }
	static quaterniont<D> vec(const vector3t<D>& p) { return quaterniont(D(0), p); }
	static quaterniont<D> zero(void) { return quaterniont(D(0), vector3t<D>()); }
	static quaterniont<D> rot(const D& angle, const D& x, const D& y, const D& z) { D rad = D(angle*M_PI/180.0); return quaterniont(cos(rad/2), vector3t<D>(x, y, z) * sin(rad/2)); }
	static quaterniont<D> rot(const D& angle, const vector3t<D>& axis) { D rad = D(angle*M_PI/180.0); return quaterniont(cos(rad/2), axis * sin(rad/2)); }
	void angleaxis(D& angle, D& x, D& y, D& z) const { D a = acos(s); angle = D(2*a*180.0/M_PI); D sa = sin(a); x = v.x/sa; y = v.y/sa; z = v.z/sa; }
	void angleaxis(D& angle, vector3t<D>& axis) const { D a = acos(s); angle = D(2*a*180.0/M_PI); axis = v * (D(1.0)/sin(a)); }
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

	matrix4t<D> rotmat(void) const {
		D w2 = s*s, x2 = v.x*v.x, y2 = v.y*v.y, z2 = v.z*v.z;
		D xy = v.x*v.y, wz = s*v.z, xz = v.x*v.z, wy = s*v.y, yz = v.y*v.z, wx = s*v.x;
		//fixme: check if this matrix must get transposed!
		return matrix4t<D>(
			w2+x2-y2-z2,	2*(xy+wz),	2*(xz-wy),	0,
			2*(xy-wz),	w2-x2+y2-z2,	2*(yz+wx),	0,
			2*(xz+wy),	2*(yz-wx),	w2-x2-y2+z2,	0,
			0,	0,	0,	1
		);
	}
};

template<class D2> inline quaterniont<D2> operator* (const D2& scalar, const quaterniont<D2>& q) { return q * scalar; }

template<class D>
ostream& operator<< ( ostream& os, const quaterniont<D>& q )
{
	os << "r=" << q.s << "; i=" << q.v.x << "; j=" << q.v.y << "; k=" << q.v.z;
	return os;
}

typedef quaterniont<double> quaternion;
typedef quaterniont<float> quaternionf;

#endif
