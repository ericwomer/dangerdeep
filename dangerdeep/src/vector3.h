//
//  A 3d vector (C)+(W) 2001 Thorsten Jordan
//

#ifndef VECTOR3_H
#define VECTOR3_H

#ifdef WIN32
#undef min
#undef max
#endif

#include "vector2.h"

template<class D>
class vector3t
{
	public:
	D x, y, z;

	vector3t() : x(0), y(0), z(0) {};
	vector3t(const D &x_, const D &y_, const D &z_) : x(x_), y(y_), z(z_) {};
	vector3t<D> normal(void) const { D len = D(1.0)/length(); return vector3t(x * len, y * len, z * len); };
	void normalize(void) { D len = D(1.0)/length(); x *= len; y *= len; z *= len; };
	vector3t<D> orthogonal(const vector3t<D>& other) const { return vector3t(other.z * y - other.y * z, other.x * z - other.z * x, other.y * x - other.x * y); };
	vector3t<D> operator* (const D &scalar) const { return vector3t(x * scalar, y * scalar, z * scalar); };
	vector3t<D> operator+ (const vector3t<D>& other) const { return vector3t(x + other.x, y + other.y, z + other.z); };
	vector3t<D> operator- (const vector3t<D>& other) const { return vector3t(x - other.x, y - other.y, z - other.z); };
	vector3t<D> operator- (void) const { return vector3t(-x, -y, -z); };
	vector3t<D>& operator+= (const vector3t<D>& other) { x += other.x; y += other.y; z += other.z; return *this; };
	vector3t<D>& operator-= (const vector3t<D>& other) { x -= other.x; y -= other.y; z -= other.z; return *this; };
	vector3t<D> min(const vector3t<D>& other) const { return vector3t(x < other.x ? x : other.x, y < other.y ? y : other.y, z < other.z ? z : other.z); };
	vector3t<D> max(const vector3t<D>& other) const { return vector3t(x > other.x ? x : other.x, y > other.y ? y : other.y, z > other.z ? z : other.z); };
	bool operator== (const vector3t<D>& other) const { return x == other.x && y == other.y && z == other.z; };
	D square_length(void) const { return x * x + y * y + z * z; };
	D length(void) const { return D(sqrt(square_length())); };
	D square_distance(const vector3t<D>& other) const { vector3t<D> n = *this - other; return n.square_length(); };
	D distance(const vector3t<D>& other) const { vector3t<D> n = *this - other; return n.length(); };
	D operator* (const vector3t<D>& other) const { return x * other.x + y * other.y + z * other.z; };
	vector3t<D> cross(const vector3t<D>& other) const {
		return vector3t(y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x ); }
	bool solve(const vector3t<D>& o1, const vector3t<D>& o2, const vector3t<D>& o3, D &s1, D &s2, D &s3) const;
	// multiplies 3x3 matrix (given in columns c0-c2) with *this.
	vector3t<D> matrixmul(const vector3t<D>& c0, const vector3t<D>& c1, const vector3t<D>& c2) const;
	vector3t<D> coeff_mul(const vector3t<D>& other) const { return vector3t(x * other.x, y * other.y, z * other.z); }
	vector2t<D> xy(void) const { return vector2t<D>(x, y); };
	vector2t<D> yz(void) const { return vector2t<D>(y, z); };
#ifndef NO_IOSTREAM
	template<class D2> friend ostream& operator<< ( ostream& os, const vector3t<D2>& v );
#endif
	template<class E> void assign(const vector3t<E>& other) { x = D(other.x); y = D(other.y); z = D(other.z); }
};

template<class D2> inline vector3t<D2> operator* (const D2& scalar, const vector3t<D2>& v) { return v * scalar; }

template<class D>
bool vector3t<D>::solve(const vector3t<D> &o1, const vector3t<D> &o2, const vector3t<D> &o3, D &s1, D &s2, D &s3) const
{
	D det = // sarrus
		o1.x*o2.y*o3.z - o1.x*o3.y*o2.z +
		o2.x*o3.y*o1.z - o2.x*o1.y*o3.z +
		o3.x*o1.y*o2.z - o3.x*o2.y*o1.z;
	if (!det)
		return false;

	s1 = (	  (o2.y*o3.z - o2.z*o3.y)*x
		+ (o2.z*o3.x - o2.x*o3.z)*y
		+ (o2.x*o3.y - o2.y*o3.x)*z ) / det;

	s2 = (	  (o1.z*o3.y - o1.y*o3.z)*x
		+ (o1.x*o3.z - o1.z*o3.x)*y
		+ (o1.y*o3.x - o1.x*o3.y)*z ) / det;

	s3 = (	  (o1.y*o2.z - o1.z*o2.y)*x
		+ (o1.z*o2.x - o1.x*o2.z)*y
		+ (o1.x*o2.y - o1.y*o2.x)*z ) / det;
	return true;
}

template<class D>
vector3t<D> vector3t<D>::matrixmul(const vector3t<D>& c0, const vector3t<D>& c1, const vector3t<D>& c2) const
{
	return vector3t<D>(	c0.x * x + c1.x * y + c2.x * z,
				c0.y * x + c1.y * y + c2.y * z,
				c0.z * x + c1.z * y + c2.z * z);
}

#ifndef NO_IOSTREAM
template<class D>
ostream& operator<< ( ostream& os, const vector3t<D>& v )
{
	os << "x=" << v.x << "; y=" << v.y << "; z=" << v.z;
	return os;
}
#endif

typedef vector3t<double> vector3;
typedef vector3t<float> vector3f;
typedef vector3t<int> vector3i;

#endif
