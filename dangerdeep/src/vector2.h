//
//  A 2d vector (C)+(W) 2001 Thorsten Jordan
//

#ifndef VECTOR2_H
#define VECTOR2_H

#include <cmath>
#include <iostream>
using namespace std;

#ifdef WIN32
#undef min
#undef max
#endif

template <class D2> class vector3t;

template <class D>
class vector2t
{
	public:
	D x, y;

	vector2t() : x(0), y(0) {};
	vector2t(const vector2t<D>& o) : x(o.x), y(o.y) {}
	vector2t& operator= (const vector2t<D>& o) { x = o.x; y = o.y; return *this; }
	vector2t(const D &x_, const D &y_) : x(x_), y(y_) {};
	vector2t<D> normal(void) const { D len = D(1.0)/length(); return vector2t(x * len, y * len); };
	void normalize(void) { D len = D(1.0)/length(); x *= len; y *= len; };
	vector2t<D> orthogonal(void) const { return vector2t(-y, x); };
	vector2t<D> operator* (const D &scalar) const { return vector2t(x * scalar, y * scalar); };
	vector2t<D> operator+ (const vector2t<D>& other) const { return vector2t(x + other.x, y + other.y); };
	vector2t<D> operator- (const vector2t<D>& other) const { return vector2t(x - other.x, y - other.y); };
	vector2t<D> operator- (void) const { return vector2t(-x, -y); };
	vector2t<D>& operator+= (const vector2t<D>& other) { x += other.x; y += other.y; return *this; };
	vector2t<D>& operator-= (const vector2t<D>& other) { x -= other.x; y -= other.y; return *this; };
	vector2t<D> min(const vector2t<D>& other) { return vector2t(x < other.x ? x : other.x, y < other.y ? y : other.y); };
	vector2t<D> max(const vector2t<D>& other) { return vector2t(x > other.x ? x : other.x, y > other.y ? y : other.y); };
	bool operator== (const vector2t<D>& other) const { return x == other.x && y == other.y; };
	D square_length(void) const { return x * x + y * y; };
	D length(void) const { return D(sqrt(x * x + y * y)); };
	D square_distance(const vector2t<D>& other) const { vector2t<D> n = *this - other; return n.square_length(); };
	D distance(const vector2t<D>& other) const { vector2t<D> n = *this - other; return n.length(); };
	D operator* (const vector2t<D>& other) const { return x * other.x + y * other.y; };
	bool solve(const vector2t<D>& o1, const vector2t<D>& o2, D& s1, D& s2) const;
	// multiplies 2x2 matrix (given in columns c0-c1) with *this.
	vector2t<D> matrixmul(const vector2t<D>& c0, const vector2t<D>& c1) const;
	vector3t<D> xy0(void) const { return vector3t<D>(x, y, 0); }
	template<class D2> friend ostream& operator<< ( ostream& os, const vector2t<D2>& v );
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

template<class D2> ostream& operator<< ( ostream& os, const vector2t<D2>& v )
{
	os << "x=" << v.x << "; y=" << v.y;
	return os;
}

typedef vector2t<double> vector2;
typedef vector2t<float> vector2f;
typedef vector2t<int> vector2i;

#endif
