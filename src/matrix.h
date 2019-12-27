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

#include "vector4.h"
#include <iostream>

#ifndef M_PI
#define M_PI 3.1415926536
#endif


// helper functions
template<class D, unsigned size>
void matrix_swap_rows(D* values, unsigned r1, unsigned r2) {
	for (unsigned i = 0; i < size; ++i) {
		D tmp = values[i+r1*size];
		values[i+r1*size] = values[i+r2*size];
		values[i+r2*size] = tmp;
	}
}

template<class D, unsigned size>
void matrix_columnpivot(D* values, unsigned p[], unsigned offset)
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
		matrix_swap_rows<D, size>(values, offset, offset+maxi);
		unsigned tmp = p[offset];
		p[offset] = p[offset+maxi];
		p[offset+maxi] = tmp;
	}
}

template<class D, unsigned size>
void matrix_invert(D* values)
{
	unsigned i, j, k;

	// prepare row swap
	unsigned p[size];
	for (i = 0; i < size; i++)
		p[i] = i;

	// LR - distribution
	for (i = 0; i < size-1; i++) { // columns of L
                matrix_columnpivot<D, size>(values, p, i);
		for (j = i+1; j < size; j++) { // rows of L
			values[j*size+i] /= values[i*size+i];
			for (k = i+1; k < size; k++) { // columns of R
				values[j*size+k] -= values[j*size+i] * values[i*size+k];
			}
		}
	}

	// invert R without using extra memory
	for (j = size; j > 0; ) {
		--j;
		values[j*size+j] = D(1.0)/values[j*size+j];
                for (i = j; i > 0; ) {
                	--i;
			D s = values[i*size+j] * values[j*size+j];
			for (k = i+1; k < j; k++) {
				s += values[i*size+k] * values[k*size+j];
			}
			values[i*size+j] = -s/values[i*size+i];
		}
	}

	// invert L without using extra memory
	for (j = size; j > 0; ) {
		--j;
                for (i = j; i > 0; ) {
                	--i;
			D s = values[j*size+i];
			for (k = i+1; k < j; k++) {
				s += values[k*size+i] * values[j*size+k];
			}
			values[j*size+i] = -s;
		}
	}

	// compute R^-1 * L^-1 without using extra memory
	for (i = 0; i < size; i++) { // columns of L^-1
		for (j = 0; j < size; j++) { // rows of R^-1
			unsigned z = (i > j) ? i : j;
			D s = 0;
			for (k = z; k < size; k++) { // rows of L^-1
				if (i == k)
					s += values[j*size+k];
				else
					s += values[j*size+k] * values[k*size+i];
			}
			values[j*size+i] = s;
		}
	}

	// column swap
	D h[size];
	for (i = 0; i < size; i++) {
		for (j = 0; j < size; j++)
                        h[j] = values[i*size+j];
		for (j = 0; j < size; j++)
                        values[i*size+p[j]] = h[j];
	}
}



///\brief This class represents a generic NxN matrix.
template<class D, unsigned size>
class matrixt
{
protected:
	// use array, as it is more efficient here than vector, because
	// vector uses new().
	D values[size*size];

        void columnpivot(unsigned p[], unsigned offset);

public:

	matrixt() {
		for (unsigned i = 0; i < size*size; ++i)
			values[i] = D(0.0);
	}

	/// construct from stream
        matrixt(std::istream& is) {
		for (unsigned i = 0; i < size*size; ++i)
			is >> values[i];
	}


	/// construct NxN matrix from one with different template type but same dimension
	template<class E> matrixt(const matrixt<E, size>& other) {
		for (unsigned i = 0; i < size; ++i) {
			for (unsigned j = 0; j < size; ++j) {
				elem(j, i) = D(other.elem(j, i));
			}
		}
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

	// no range tests for performance reasons
	D& elem(unsigned col, unsigned row) { return values[col + row * size]; }
	const D& elem(unsigned col, unsigned row) const { return values[col + row * size]; }
	D& elem_at(unsigned col, unsigned row) { return values.at(col + row * size); }
	const D& elem_at(unsigned col, unsigned row) const { return values.at(col + row * size); }

	void print() const {
		for(unsigned y = 0; y < size; y++) {
			std::cout << "/ ";
			for(unsigned x = 0; x < size; x++) {
				std::cout << "\t" << values[y*size+x];
			}
			std::cout << "\t/\n";
		}
	}

	matrixt<D, size> inverse() const {
		matrixt<D, size> r(*this);
		matrix_invert<D, size>(values);
		return r;
	}

	matrixt<D, size> transpose() const {
		matrixt<D, size> r;
		for (unsigned i = 0; i < size; ++i)
			for (unsigned j = 0; j < size; ++j)
				r.values[i+size*j] = values[j+size*i];
		return r;
	}

	/// multiply NxN matrix with N-vector
	vector4t<D> operator* (const vector4t<D>& v) const {
		D r[4];
		for (unsigned j = 0; j < 4; ++j) {	// rows of "this"
			r[j] = values[j*4+0] * v.x + values[j*4+1] * v.y + values[j*4+2] * v.z + values[j*4+3] * v.w;
		}
		return vector4t<D>(r[0], r[1], r[2], r[3]);
	}
};

#endif
