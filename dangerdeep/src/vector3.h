//
//  A vector3 (C)+(W) 2001 Thorsten Jordan
//

#ifndef VECTOR3_H
#define VECTOR3_H

#include "vector2.h"
#include <cmath>

class vector3
{
	public:
	double x, y, z;

	vector3() : x(0), y(0), z(0) {};
	vector3(const double &x_, const double &y_, const double &z_) : x(x_), y(y_), z(z_) {};
	vector3 normal(void) const { double len = double(1.0/length()); return vector3(x * len, y * len, z * len); };
	void normalize(void) { double len = double(1.0/length()); x *= len; y *= len; z *= len; };
	vector3 orthogonal(const vector3 &other) const { return vector3(other.z * y - other.y * z, other.x * z - other.z * x, other.y * x - other.x * y); };
	vector3 operator* (const double &scalar) const { return vector3(x * scalar, y * scalar, z * scalar); };
	vector3 operator+ (const vector3 &other) const { return vector3(x + other.x, y + other.y, z + other.z); };
	vector3 operator- (const vector3 &other) const { return vector3(x - other.x, y - other.y, z - other.z); };
	vector3 operator- (void) const { return vector3(-x, -y, -z); };
	vector3& operator+= (const vector3& other) { x += other.x; y += other.y; z += other.z; return *this; };
	vector3& operator-= (const vector3& other) { x -= other.x; y -= other.y; z -= other.z; return *this; };
	vector3 min(const vector3& other) { return vector3(x < other.x ? x : other.x, y < other.y ? y : other.y, z < other.z ? z : other.z); };
	vector3 max(const vector3& other) { return vector3(x > other.x ? x : other.x, y > other.y ? y : other.y, z > other.z ? z : other.z); };
	bool operator== (const vector3& other) const { return x == other.x && y == other.y && z == other.z; };
	double square_length(void) const { return x * x + y * y + z * z; };
	double length(void) const { return double(sqrt(x * x + y * y + z * z)); };
	double square_distance(const vector3 &other) const { vector3 n = *this - other; return n.square_length(); };
	double distance(const vector3 &other) const { vector3 n = *this - other; return n.length(); };
	double operator* (const vector3 &other) const { return x * other.x + y * other.y + z * other.z; };
	bool solve(const vector3 &o1, const vector3 &o2, const vector3 &o3, double &s1, double &s2, double &s3) const;
	// multiplies 3x3 matrix (given in columns c0-c2) with *this.
	vector3 matrixmul(const vector3& c0, const vector3& c1, const vector3& c2) const;
	vector2 xy(void) const { return vector2(x, y); };
	vector2 yz(void) const { return vector2(y, z); };
};

inline vector3 operator* (const double& scalar, const vector3& v) { return v * scalar; }

#endif
