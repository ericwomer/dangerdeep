//
//  A 2d vector3 (C)+(W) 2001 Thorsten Jordan
//

#ifndef VECTOR2_H
#define VECTOR2_H

#include <cmath>

class vector2
{
	public:
	double x, y;

	vector2() : x(0), y(0) {};
	vector2(const double &x_, const double &y_) : x(x_), y(y_) {};
	vector2 normal(void) const { double len = double(1.0/length()); return vector2(x * len, y * len); };
	void normalize(void) { double len = 1.0/length(); x *= len; y *= len; };
	vector2 orthogonal(void) const { return vector2(-y, x); };
	vector2 operator* (const double &scalar) const { return vector2(x * scalar, y * scalar); };
	vector2 operator+ (const vector2 &other) const { return vector2(x + other.x, y + other.y); };
	vector2 operator- (const vector2 &other) const { return vector2(x - other.x, y - other.y); };
	vector2 operator- (void) const { return vector2(-x, -y); };
	vector2& operator+= (const vector2& other) { x += other.x; y += other.y; return *this; };
	vector2& operator-= (const vector2& other) { x -= other.x; y -= other.y; return *this; };
	vector2 min(const vector2& other) { return vector2(x < other.x ? x : other.x, y < other.y ? y : other.y); };
	vector2 max(const vector2& other) { return vector2(x > other.x ? x : other.x, y > other.y ? y : other.y); };
	bool operator== (const vector2& other) const { return x == other.x && y == other.y; };
	double square_length(void) const { return x * x + y * y; };
	double length(void) const { return double(sqrt(x * x + y * y)); };
	double square_distance(const vector2 &other) const { vector2 n = *this - other; return n.square_length(); };
	double distance(const vector2 &other) const { vector2 n = *this - other; return n.length(); };
	double operator* (const vector2 &other) const { return x * other.x + y * other.y; };
	bool solve(const vector2 &o1, const vector2 &o2, double &s1, double &s2) const;
};

#endif
