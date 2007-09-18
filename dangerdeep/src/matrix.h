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
//  A 4x4 matrix (C)+(W) 2001 Thorsten Jordan
//

#ifndef MATRIX_H
#define MATRIX_H

#ifdef WIN32
#undef min
#undef max
#endif

#ifdef WIN32
// 2006-11-30 doc1972 added check to prevent double definition. 
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif /* WIN32_LEAN_AND_MEAN */
#include <windows.h>
#endif

#ifdef USE_NATIVE_GL
#include <gl.h>
#else
#include "oglext/OglExt.h"
#endif

#include "vector4.h"
#include <iostream>

#ifndef M_PI
#define M_PI 3.1415926536
#endif

///\brief This class represents a NxN matrix.
///@note the following functions must not be called for other matrices than
///      4x4: to_gl_array, the constructor with 16 parameters,
///      any gl function, rot_x/y/z, clear_*, trans etc.
///      Any usage of these functions with matrix3 will lead to segfaults.
///      How can we avoid to call these functions already during compilation?
///      We would need to define them with empty body, and offer
///      specializations, but then we need to define them for every
///      data type of D, that could be generalized in another function though.
///      But this is much work...
template<class D, unsigned size>
class matrixt
{
protected:
	// use array, as it is more efficient here than vector, because
	// vector uses new().
	D values[size*size];

        void columnpivot(unsigned p[], unsigned offset);

public:

	static D pi() { return D(M_PI); }
	static D pi_div_180() { return D(M_PI/180); }

	matrixt() {
		for (unsigned i = 0; i < size*size; ++i)
			values[i] = D(0.0);
	}

	/// construct in C++ order
        matrixt(D* v) {
		for (unsigned i = 0; i < size*size; ++i)
			values[i] = v[i];
	}

	/// construct from stream
        matrixt(std::istream& is) {
		for (unsigned i = 0; i < size*size; ++i)
			is >> values[i];
	}


	// constuct in C++ order	
	matrixt(const D& e0, const D& e1, const D& e2, const D& e3,
		const D& e4, const D& e5, const D& e6, const D& e7,
		const D& e8) {
		elem(0, 0) = e0;
		elem(1, 0) = e1;
		elem(2, 0) = e2;
		elem(0, 1) = e3;
		elem(1, 1) = e4;
		elem(2, 1) = e5;
		elem(0, 2) = e6;
		elem(1, 2) = e7;
		elem(2, 2) = e8;
	}

	/// construct 4x4 matrix.
	///@note! will crash silently with matrices of dimension < 4
	matrixt(const D& e0, const D& e1, const D& e2, const D& e3,
		const D& e4, const D& e5, const D& e6, const D& e7,
		const D& e8, const D& e9, const D& e10, const D& e11,
		const D& e12, const D& e13, const D& e14, const D& e15) {
		elem(0, 0) = e0;
		elem(1, 0) = e1;
		elem(2, 0) = e2;
		elem(3, 0) = e3;
		elem(0, 1) = e4;
		elem(1, 1) = e5;
		elem(2, 1) = e6;
		elem(3, 1) = e7;
		elem(0, 2) = e8;
		elem(1, 2) = e9;
		elem(2, 2) = e10;
		elem(3, 2) = e11;
		elem(0, 3) = e12;
		elem(1, 3) = e13;
		elem(2, 3) = e14;
		elem(3, 3) = e15;
	}

	/// construct NxN matrix from one with different template type but same dimension
	template<class E> matrixt(const matrixt<E, size>& other) {
		for (unsigned i = 0; i < size; ++i) {
			for (unsigned j = 0; j < size; ++j) {
				elem(j, i) = D(other.elem(j, i));
			}
		}
	}

	template<unsigned subsize> matrixt<D, subsize> sub() const {
		matrixt<D, subsize> r;
		for (unsigned i = 0; i < subsize; ++i) {
			for (unsigned j = 0; j < subsize; ++j) {
				r.elem(j, i) = elem(j, i);
			}
		}
		return r;
	}

	static matrixt<D, size> one() { matrixt<D, size> r; for (unsigned i = 0; i < size; ++i) r.values[i+i*size] = D(1.0); return r; }

	matrixt<D, size> operator- (const matrixt<D, size>& other) const { matrixt<D, size> r; for (unsigned i = 0; i < size*size; ++i) r.values[i] = values[i] - other.values[i]; return r; }

	matrixt<D, size> operator+ (const matrixt<D, size>& other) const { matrixt<D, size> r; for (unsigned i = 0; i < size*size; ++i) r.values[i] = values[i] + other.values[i]; return r; }

	matrixt<D, size> operator* (const matrixt<D, size>& other) const {
		matrixt<D, size> r;
		// maybe plain code is faster than this three loops?
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

	/// print to stream
	void to_stream(std::ostream& os) const {
		os << values[0];
		for (unsigned i = 1; i < size*size; ++i)
			os << " " << values[i];
	}

	/// multiply by scalar
	matrixt<D, size> operator* (const D& s) const {
		matrixt<D, size> r;
		for (unsigned i = 0; i < size*size; ++i)
			r.values[i] = values[i] * s;
		return r;
	}

	matrixt<D, size> operator- () const { matrixt<D, size> r; for (unsigned i = 0; i < size*size; ++i) r.values[i] = -values[i]; return r; }

	// store in C++ order
	void to_array(D* v) const {
		for (unsigned i = 0; i < size*size; ++i)
			v[i] = values[i];
	}

	// store in OpenGL order, works only for size=4
	void to_gl_array(D* v) const {
		for (unsigned i = 0; i < size; ++i)
			for (unsigned j = 0; j < size; ++j)
				v[i+j*size] = values[j+i*size];
	}

	// return pointer to array of elements
	const D* elemarray() const {
		return &values[0];
	}

	// no range tests for performance reasons
	D& elem(unsigned col, unsigned row) { return values[col + row * size]; }
	const D& elem(unsigned col, unsigned row) const { return values[col + row * size]; }
	D& elem_at(unsigned col, unsigned row) { return values.at(col + row * size); }
	const D& elem_at(unsigned col, unsigned row) const { return values.at(col + row * size); }

	/// returns determinate of upper left 2x2 matrix
	D det2() const {
		return	elem(0, 0) * elem(1, 1) - elem(1, 0) * elem(0, 1);
	}

	/// returns determinate of upper left 3x3 matrix
	D det3() const {
		// sarrus (maybe direct indices are a bit faster...)
		return	elem(0, 0) * elem(1, 1) * elem(2, 2) - elem(2, 0) * elem(1, 1) * elem(0, 2) +
			elem(1, 0) * elem(2, 1) * elem(0, 2) - elem(1, 0) * elem(0, 1) * elem(2, 2) +
			elem(2, 0) * elem(0, 1) * elem(1, 2) - elem(0, 0) * elem(2, 1) * elem(1, 2);
	}

	void print() const {
		for(unsigned y = 0; y < size; y++) {
			std::cout << "/ ";
			for(unsigned x = 0; x < size; x++) {
				std::cout << "\t" << values[y*size+x];
			}
			std::cout << "\t/\n";
		}
	}

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

	matrixt<D, size> inverse() const;

	matrixt<D, size> transpose() const {
		matrixt<D, size> r;
		for (unsigned i = 0; i < size; ++i)
			for (unsigned j = 0; j < size; ++j)
				r.values[i+size*j] = values[j+size*i];
		return r;
	}

	// get n-th row/col, ignores last value
	vector3t<D> row3(unsigned i) const {
		return vector3t<D>(elem(0, i), elem(1, i), elem(2, i));
	}
	vector3t<D> column3(unsigned i) const {
		return vector3t<D>(elem(i, 0), elem(i, 1), elem(i, 2));
	}

	// get n-th row/col, with last value
	vector4t<D> row(unsigned i) const {
		return vector4t<D>(elem(0, i), elem(1, i), elem(2, i), elem(3, i));
	}
	vector4t<D> column(unsigned i) const {
		return vector4t<D>(elem(i, 0), elem(i, 1), elem(i, 2), elem(i, 3));
	}

	// -------------- the functions below work only with size=4 matrices!!! ---------
	void set_gl(GLenum pname);	// GL_PROJECTION, GL_MODELVIEW, GL_TEXTURE
	void set_glf(GLenum pname);	// GL_PROJECTION, GL_MODELVIEW, GL_TEXTURE
	void multiply_gl() const;
	void multiply_glf() const;
	static matrixt<D, size> get_gl(GLenum pname); // GL_PROJECTION_MATRIX, GL_MODELVIEW_MATRIX, GL_TEXTURE_MATRIX
	static matrixt<D, size> get_glf(GLenum pname); // GL_PROJECTION_MATRIX, GL_MODELVIEW_MATRIX, GL_TEXTURE_MATRIX
	
	static matrixt<D, size> rot_x(const D& degrees) {
		D a = degrees * pi_div_180();
		D c = D(cos(a)), s = D(sin(a)), o = D(1.0), n = D(0.0);
		return matrixt<D, size>(o, n, n, n,  n, c,-s, n,  n, s, c, n,  n, n, n, o);
	}

	static matrixt<D, size> rot_y(const D& degrees) {
		D a = degrees * pi_div_180();
		D c = D(cos(a)), s = D(sin(a)), o = D(1.0), n = D(0.0);
		return matrixt<D, size>(c, n, s, n,  n, o, n, n, -s, n, c, n,  n, n, n, o);
	}

	static matrixt<D, size> rot_z(const D& degrees) {
		D a = degrees * pi_div_180();
		D c = D(cos(a)), s = D(sin(a)), o = D(1.0), n = D(0.0);
		return matrixt<D, size>(c,-s, n, n,  s, c, n, n,  n, n, o, n,  n, n, n, o);
	}
	
	static matrixt<D, size> trans(const D& x, const D& y, const D& z) {
		D o = D(1.0), n = D(0.0);
		return matrixt<D, size>(o, n, n, x,  n, o, n, y,  n, n, o, z,  n, n, n, o);
	}
	
	static matrixt<D, size> trans(const vector3t<D>& v) {
		D o = D(1.0), n = D(0.0);
		return matrixt<D, size>(o, n, n, v.x,  n, o, n, v.y,  n, n, o, v.z,  n, n, n, o);
	}
	
	static matrixt<D, size> diagonal(const D& x, const D& y, const D& z, const D& w = D(1.0)) {
		D n = D(0.0);
		return matrixt<D, size>(x, n, n, n,  n, y, n, n,  n, n, z, n,  n, n, n, w);
	}

	void clear_rot() {
		values[0] = values[5] = values[10] = D(1.0);
		values[1] = values[2] = values[4] = values[6] = values[8] = values[9] = D(0.0);
	}

	void clear_trans() {
		values[3] = values[7] = values[11] = D(0.0);
	}

	/// multiply NxN matrix with N-vector
	vector4t<D> operator* (const vector4t<D>& v) const {
		D r[4];
		for (unsigned j = 0; j < 4; ++j) {	// rows of "this"
			r[j] = values[j*4+0] * v.x + values[j*4+1] * v.y + values[j*4+2] * v.z + values[j*4+3] * v.w;
		}
		return vector4t<D>(r[0], r[1], r[2], r[3]);
	}

	// ----------------- special code for dim=3 or 4

	/// multiply 3x3 matrix with 3-vector
	vector3t<D> mul3vec3 (const vector3t<D>& v) const {
		D r[3];
		for (unsigned j = 0; j < 3; ++j) {	// rows of "this"
			r[j] = values[j*3+0] * v.x + values[j*3+1] * v.y + values[j*3+2] * v.z;
		}
		return vector3t<D>(r[0], r[1], r[2]);
	}

	/// multiply 4x4 matrix with 3-vector, with w-renormalization
	vector3t<D> mul4vec3 (const vector3t<D>& v) const {
		D r[4];
		for (unsigned j = 0; j < 4; ++j) {	// rows of "this"
			r[j] = values[j*4+0] * v.x + values[j*4+1] * v.y + values[j*4+2] * v.z + values[j*4+3];
		}
		// divide x,y,z by w
		return vector3t<D>(r[0]/r[3], r[1]/r[3], r[2]/r[3]);
	}

	/// multiply 4x4 matrix with 3-vector, ignore projection part (faster)
	vector3t<D> mul4vec3xlat (const vector3t<D>& v) const {
		D r[3];
		for (unsigned j = 0; j < 3; ++j) {	// rows of "this"
			r[j] = values[j*4+0] * v.x + values[j*4+1] * v.y + values[j*4+2] * v.z + values[j*4+3];
		}
		return vector3t<D>(r[0], r[1], r[2]);
	}
};



template<class D, unsigned size>
void matrixt<D, size>::columnpivot(unsigned p[], unsigned offset)
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



template<class D, unsigned size>
matrixt<D, size> matrixt<D, size>::inverse() const
{
	matrixt<D, size> r(*this);
	unsigned i, j, k;

	// prepare row swap
	unsigned p[size];
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
	D h[size];
	for (i = 0; i < size; i++) {
		for (j = 0; j < size; j++)
                        h[j] = r.values[i*size+j];
		for (j = 0; j < size; j++)
                        r.values[i*size+p[j]] = h[j];
	}

	return r;
}



template<class D, unsigned size> void matrixt<D, size>::set_gl(GLenum pname) {
	GLdouble m[16];
	for (unsigned i = 0; i < 4; ++i)
		for (unsigned j = 0; j < 4; ++j)
			m[i+j*4] = GLdouble(values[j+i*4]);
	glMatrixMode(pname);
	glLoadMatrixd(m);
	glMatrixMode(GL_MODELVIEW);
}
template<class D, unsigned size> void matrixt<D, size>::set_glf(GLenum pname) {
	GLfloat m[16];
	for (unsigned i = 0; i < 4; ++i)
		for (unsigned j = 0; j < 4; ++j)
			m[i+j*4] = GLfloat(values[j+i*4]);
	glMatrixMode(pname);
	glLoadMatrixf(m);
	glMatrixMode(GL_MODELVIEW);
}
	
template<class D, unsigned size> void matrixt<D, size>::multiply_gl() const {
	GLdouble m[16];
	for (unsigned i = 0; i < 4; ++i)
		for (unsigned j = 0; j < 4; ++j)
			m[i+j*4] = GLdouble(values[j+i*4]);
	glMultMatrixd(m);
}
template<class D, unsigned size> void matrixt<D, size>::multiply_glf() const {
	GLfloat m[16];
	for (unsigned i = 0; i < 4; ++i)
		for (unsigned j = 0; j < 4; ++j)
			m[i+j*4] = GLfloat(values[j+i*4]);
	glMultMatrixf(m);
}

template<class D, unsigned size> matrixt<D, size> matrixt<D, size>::get_gl(GLenum pname) {
	GLdouble m[16];
	glGetDoublev(pname, m);
	matrixt<D, size> r;
	for (unsigned i = 0; i < 4; ++i)
		for (unsigned j = 0; j < 4; ++j)
			r.values[j+i*4] = D(m[i+j*4]);
	return r;
}
template<class D, unsigned size> matrixt<D, size> matrixt<D, size>::get_glf(GLenum pname) {
	GLfloat m[16];
	glGetFloatv(pname, m);
	matrixt<D, size> r;
	for (unsigned i = 0; i < 4; ++i)
		for (unsigned j = 0; j < 4; ++j)
			r.values[j+i*4] = D(m[i+j*4]);
	return r;
}

#endif
