//
//  A 4x4 matrix (C)+(W) 2001 Thorsten Jordan
//

#ifndef MATRIX4_H
#define MATRIX4_H

#ifdef WIN32
#undef min
#undef max
#endif

#ifndef DONT_USE_OPENGL
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include <gl.h>
#endif

#ifndef NO_IOSTREAM
#include <iostream>
#endif

#include "vector3.h"

#include <vector>
using namespace std;

#ifndef M_PI
#define M_PI 3.1415926536
#endif

template<class D>
class matrix4t
{
protected:
#define size 4
	//static const unsigned size;	// routines are generic, so use constant here
	vector<D> values;

        void columnpivot(vector<unsigned>& p, unsigned offset);

public:

	static D pi(void) { return D(M_PI); }
	static D pi_div_180(void) { return D(M_PI/180); }

	matrix4t() : values(size*size, D(0.0)) {}

	// construct in C++ order
        matrix4t(D* v) : values(size*size) {
		for (unsigned i = 0; i < size*size; ++i)
			values[i] = v[i];
	}

	matrix4t(const matrix4t<D>& other) : values(other.values) {}

	~matrix4t() {}

	// constuct in C++ order	
	matrix4t(const D& e0, const D& e1, const D& e2, const D& e3,
		const D& e4, const D& e5, const D& e6, const D& e7,
		const D& e8, const D& e9, const D& e10, const D& e11,
		const D& e12, const D& e13, const D& e14, const D& e15) {
		values.resize(16);
		values[ 0] =  e0; values[ 1] =  e1; values[ 2] =  e2; values[ 3] =  e3;
		values[ 4] =  e4; values[ 5] =  e5; values[ 6] =  e6; values[ 7] =  e7;
		values[ 8] =  e8; values[ 9] =  e9; values[10] = e10; values[11] = e11;
		values[12] = e12; values[13] = e13; values[14] = e14; values[15] = e15;
	}

	matrix4t<D>& operator= (const matrix4t<D>& other) { values = other.values; return *this; }

	static matrix4t<D> one(void) { matrix4t<D> r; for (unsigned i = 0; i < size; ++i) r.values[i+i*size] = D(1.0); return r; }

	matrix4t<D> operator- (const matrix4t<D>& other) const { matrix4t<D> r; for (unsigned i = 0; i < size*size; ++i) r.values[i] = values[i] - other.values[i]; return r; }

	matrix4t<D> operator+ (const matrix4t<D>& other) const { matrix4t<D> r; for (unsigned i = 0; i < size*size; ++i) r.values[i] = values[i] + other.values[i]; return r; }

	matrix4t<D> operator* (const matrix4t<D>& other) const {
		matrix4t<D> r;
		for (unsigned i = 0; i < size; ++i) {	// columns of "other"
			for (unsigned j = 0; j < size; ++j) {	// rows of "this"
				D tmp = D(0.0);
				for (unsigned k = 0; k < size; ++k)	// columns of "this"/rows of "other"
					tmp += values[k+j*size] * other.values[i+k*size];
				r.values[i+j*size] = tmp;
			}
		}
		return r;
	}

	matrix4t<D> operator- (void) const { matrix4t<D> r; for (unsigned i = 0; i < size*size; ++i) r.values[i] = -values[i]; return r; }

	// store in C++ order
	void to_array(D* v) const {
		for (unsigned i = 0; i < size*size; ++i)
			v[i] = values[i];
	}

	// store in OpenGL order
	void to_gl_array(D* v) const {
		for (unsigned i = 0; i < 4; ++i)
			for (unsigned j = 0; j < 4; ++j)
				v[i+j*4] = values[j+i*4];
	}

	// return pointer to array of elements
	const D* elemarray(void) const {
		return &values[0];
	}

	// no range tests for performance reasons
	D& elem(unsigned col, unsigned row) { return values[col + row * size]; }
	const D& elem(unsigned col, unsigned row) const { return values[col + row * size]; }
	D& elem_at(unsigned col, unsigned row) { return values.at(col + row * size); }
	const D& elem_at(unsigned col, unsigned row) const { return values.at(col + row * size); }

	// returns determinate of upper left 3x3 matrix
	D det3(void) const {
		// sarrus
		return	elem(0, 0) * elem(1, 1) * elem(2, 2) - elem(2, 0) * elem(1, 1) * elem(0, 2) +
			elem(1, 0) * elem(2, 1) * elem(0, 2) - elem(1, 0) * elem(0, 1) * elem(2, 2) +
			elem(2, 0) * elem(0, 1) * elem(1, 2) - elem(0, 0) * elem(2, 1) * elem(1, 2);
	}

#ifndef NO_IOSTREAM
	void print(void) const {
		for(unsigned y = 0; y < size; y++) {
			cout << "/ ";
			for(unsigned x = 0; x < size; x++) {
				cout << "\t" << values[y*size+x];
			}
			cout << "\t/\n";
		}
	}
#endif

	void swap_rows(unsigned r1, unsigned r2) {
		for (unsigned i = 0; i < size; ++i) {
			D tmp = values[i+r1*size];
			values[i+r1*size] = values[i+r2*size];
			values[i+r2*size] = tmp;
		}
	}

	void swap_columns(unsigned c1, unsigned c2) {
		for (unsigned i = 0; i < size; ++i) {
			D tmp = values[c1+i*size];
			values[c1+i*size] = values[c2+i*size];
			values[c2+i*size] = tmp;
		}
	}

	matrix4t<D> inverse(void) const;

	matrix4t<D> transpose(void) const {
		matrix4t<D> r;
		for (unsigned i = 0; i < size; ++i)
			for (unsigned j = 0; j < size; ++j)
				r.values[i+size*j] = values[j+size*i];
		return r;
	}

#ifndef DONT_USE_OPENGL
	void set_gl(GLenum pname) {		// GL_PROJECTION, GL_MODELVIEW, GL_TEXTURE
		GLdouble m[16];
		for (unsigned i = 0; i < 4; ++i)
			for (unsigned j = 0; j < 4; ++j)
				m[i+j*4] = GLdouble(values[j+i*4]);
		glMatrixMode(pname);
		glLoadMatrixd(m);
		glMatrixMode(GL_MODELVIEW);
	}
	
	void multiply_gl(void) const {
		GLdouble m[16];
		for (unsigned i = 0; i < 4; ++i)
			for (unsigned j = 0; j < 4; ++j)
				m[i+j*4] = GLdouble(values[j+i*4]);
		glMultMatrixd(m);
	}

	static matrix4t<D> get_gl(GLenum pname) {	// GL_PROJECTION_MATRIX, GL_MODELVIEW_MATRIX, GL_TEXTURE_MATRIX
		GLdouble m[16];
		glGetDoublev(pname, m);
		matrix4t<D> r;
		for (unsigned i = 0; i < 4; ++i)
			for (unsigned j = 0; j < 4; ++j)
				r.values[j+i*4] = D(m[i+j*4]);
		return r;
	}

	static matrix4t<D> get_glf(GLenum pname) {	// GL_PROJECTION_MATRIX, GL_MODELVIEW_MATRIX, GL_TEXTURE_MATRIX
		GLfloat m[16];
		glGetFloatv(pname, m);
		matrix4t<D> r;
		for (unsigned i = 0; i < 4; ++i)
			for (unsigned j = 0; j < 4; ++j)
				r.values[j+i*4] = D(m[i+j*4]);
		return r;
	}
#endif
	
	static matrix4t<D> rot_x(const D& degrees) {
		D a = degrees * pi_div_180();
		D c = D(cos(a)), s = D(sin(a)), o = D(1.0), n = D(0.0);
		return matrix4t<D>(o, n, n, n,  n, c,-s, n,  n, s, c, n,  n, n, n, o);
	}

	static matrix4t<D> rot_y(const D& degrees) {
		D a = degrees * pi_div_180();
		D c = D(cos(a)), s = D(sin(a)), o = D(1.0), n = D(0.0);
		return matrix4t<D>(c, n, s, n,  n, o, n, n, -s, n, c, n,  n, n, n, o);
	}

	static matrix4t<D> rot_z(const D& degrees) {
		D a = degrees * pi_div_180();
		D c = D(cos(a)), s = D(sin(a)), o = D(1.0), n = D(0.0);
		return matrix4t<D>(c,-s, n, n,  s, c, n, n,  n, n, o, n,  n, n, n, o);
	}
	
	static matrix4t<D> trans(const D& x, const D& y, const D& z) {
		D o = D(1.0), n = D(0.0);
		return matrix4t<D>(o, n, n, x,  n, o, n, y,  n, n, o, z,  n, n, n, o);
	}
	
	static matrix4t<D> trans(const vector3t<D>& v) {
		D o = D(1.0), n = D(0.0);
		return matrix4t<D>(o, n, n, v.x,  n, o, n, v.y,  n, n, o, v.z,  n, n, n, o);
	}
	
	static matrix4t<D> diagonal(const D& x, const D& y, const D& z, const D& w = D(1.0)) {
		D n = D(0.0);
		return matrix4t<D>(x, n, n, n,  n, y, n, n,  n, n, z, n,  n, n, n, w);
	}

	void clear_rot(void) {
		values[0] = values[5] = values[10] = D(1.0);
		values[1] = values[2] = values[4] = values[6] = values[8] = values[9] = D(0.0);
	}

	vector3t<D> operator* (const vector3t<D>& v) const {
		D r[4];
		for (unsigned j = 0; j < 4; ++j) {	// rows of "this"
			r[j] = values[j*4+0] * v.x + values[j*4+1] * v.y + values[j*4+2] * v.z + values[j*4+3];
		}
		// divide x,y,z by w
		return vector3t<D>(r[0]/r[3], r[1]/r[3], r[2]/r[3]);
	}

	// get n-th row/col, ignores last value
	vector3t<D> row(unsigned i) const {
		return vector3t<D>(values[i*4], values[i*4+1], values[i*4+2]);
	}
	vector3t<D> column(unsigned i) const {
		return vector3t<D>(values[i], values[i+4], values[i+2*4]);
	}
};



template<class D>
void matrix4t<D>::columnpivot(vector<unsigned>& p, unsigned offset)
{
	// find largest entry
        D max = values[offset * size + offset];
        unsigned maxi = 0;

	for (unsigned i = 1; i < size-offset; i++) {
                D tmp = values[(offset+i) * size + offset];
                if (fabs(tmp) > fabs(max)) {
                        max = tmp;
			maxi = i;
		}
	}

	// swap rows, change p
	if (maxi != 0) {
		swap_rows(offset, offset+maxi);
		unsigned tmp = p[offset];
		p[offset] = p[offset+maxi];
		p[offset+maxi] = tmp;
	}
}



template<class D>
matrix4t<D> matrix4t<D>::inverse(void) const
{
	matrix4t<D> r(*this);
	unsigned i, j, k;

	// prepare row swap
	vector<unsigned> p(size);
	for (i = 0; i < size; i++)
		p[i] = i;

	// LR - distribution
	for (i = 0; i < size-1; i++) { // columns of L
                r.columnpivot(p, i);
		for (j = i+1; j < size; j++) { // rows of L
			r.values[j*size+i] /= r.values[i*size+i];
			for (k = i+1; k < size; k++) { // columns of R
				r.values[j*size+k] -= r.values[j*size+i] * r.values[i*size+k];
			}
		}
	}

	// invert R without using extra memory
	for (j = size; j > 0; ) {
		--j;
		r.values[j*size+j] = D(1.0)/r.values[j*size+j];
                for (i = j; i > 0; ) {
                	--i;
			D s = r.values[i*size+j] * r.values[j*size+j];
			for (k = i+1; k < j; k++) {
				s += r.values[i*size+k] * r.values[k*size+j];
			}
			r.values[i*size+j] = -s/r.values[i*size+i];
		}
	}

	// invert L without using extra memory
	for (j = size; j > 0; ) {
		--j;
                for (i = j; i > 0; ) {
                	--i;
			D s = r.values[j*size+i];
			for (k = i+1; k < j; k++) {
				s += r.values[k*size+i] * r.values[j*size+k];
			}
			r.values[j*size+i] = -s;
		}
	}

	// compute R^-1 * L^-1 without using extra memory
	for (i = 0; i < size; i++) { // columns of L^-1
		for (j = 0; j < size; j++) { // rows of R^-1
			unsigned z = (i > j) ? i : j;
			D s = 0;
			for (k = z; k < size; k++) { // rows of L^-1
				if (i == k)
					s += r.values[j*size+k];
				else
					s += r.values[j*size+k] * r.values[k*size+i];
			}
			r.values[j*size+i] = s;
		}
	}

	// column swap
	vector<D> h(size);
	for (i = 0; i < size; i++) {
		for (j = 0; j < size; j++)
                        h[j] = r.values[i*size+j];
		for (j = 0; j < size; j++)
                        r.values[i*size+p[j]] = h[j];
	}

	return r;
}
#undef size

typedef matrix4t<double> matrix4;
typedef matrix4t<float> matrix4f;
typedef matrix4t<int> matrix4i;

#endif
