//
//  A 3d vector (C)+(W) 2001 Thorsten Jordan
//

#ifndef VECTOR3_H
#define VECTOR3_H

#include <iostream>
#include "vector2.h"
#include <cmath>
using namespace std;

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
	vector3t<D> min(const vector3t<D>& other) { return vector3t(x < other.x ? x : other.x, y < other.y ? y : other.y, z < other.z ? z : other.z); };
	vector3t<D> max(const vector3t<D>& other) { return vector3t(x > other.x ? x : other.x, y > other.y ? y : other.y, z > other.z ? z : other.z); };
	bool operator== (const vector3t<D>& other) const { return x == other.x && y == other.y && z == other.z; };
	D square_length(void) const { return x * x + y * y + z * z; };
	D length(void) const { return D(sqrt(x * x + y * y + z * z)); };
	D square_distance(const vector3t<D>& other) const { vector3t<D> n = *this - other; return n.square_length(); };
	D distance(const vector3t<D>& other) const { vector3t<D> n = *this - other; return n.length(); };
	D operator* (const vector3t<D>& other) const { return x * other.x + y * other.y + z * other.z; };
	bool solve(const vector3t<D>& o1, const vector3t<D>& o2, const vector3t<D>& o3, D &s1, D &s2, D &s3) const;
	// multiplies 3x3 matrix (given in columns c0-c2) with *this.
	vector3t<D> matrixmul(const vector3t<D>& c0, const vector3t<D>& c1, const vector3t<D>& c2) const;
	vector2t<D> xy(void) const { return vector2t<D>(x, y); };
	vector2t<D> yz(void) const { return vector2t<D>(y, z); };
	template<class D2> friend ostream& operator<< ( ostream& os, const vector3t<D2>& v );
	template<class E> void assign(const vector2t<E>& other) { x = D(other.x); y = D(other.y); }
};

template<class D2> inline vector3t<D2> operator* (const D2& scalar, const vector3t<D2>& v) { return v * scalar; }
template<class D2> ostream& operator<< ( ostream& os, const vector3t<D2>& v );

typedef vector3t<double> vector3;
typedef vector3t<float> vector3f;

#endif
