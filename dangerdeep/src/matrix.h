//
//  A 4x4 matrix (C)+(W) 2001 Thorsten Jordan
//

// todo: get rid of int's (unsigneds for row/column indices, avoid warnings about type mismatch)

#ifndef MATRIX4_H
#define MATRIX4_H

#ifdef WIN32
#undef min
#undef max
#endif

#include <vector>
#include <cmath>
#include <iostream>
using namespace std;

template<class D>
class matrix4t
{
protected:
	static const unsigned size = 4;
	vector<D> values;

        void columnpivot(vector<int>& p, int offset);

public:

	matrix4t() : values(size*size, D(0.0)) {}

        matrix4t(D* v) : values(size*size) { // construct in opengl order from D array
		for (unsigned j = 0; j < size; ++j)
			for (unsigned i = 0; i < size; ++i)
				values[i+j*size] = v[j+i*size];
	}

	matrix4t(const matrix4t<D>& other) : values(other.values) {}

	~matrix4t() {}

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

	void print(void) const {
		for(unsigned y = 0; y < size; y++) {
			cout << "| ";
			for(unsigned x = 0; x < size; x++) {
				cout << "\t" << values[y*size+x];
			}
			cout << "\t|\n";
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

	matrix4t<D> inverse(void) const;

	matrix4t<D> transpose(void) const {
		matrix4t<D> r;
		for (unsigned i = 0; i < size; ++i)
			for (unsigned j = 0; j < size; ++j)
				r.values[i+size*j] = values[j+size*i];
		return r;
	}

//	void set_gl(GLint type);
//	static matrix get_gl(GLint type);
};



template<class D>
void matrix4t<D>::columnpivot(vector<int>& p, int offset)
{
	// find largest entry
        D max = values[offset * size + offset];
        int maxi = 0;

	for (int i = 1; i < size-offset; i++) {
                double tmp = values[(offset+i) * size + offset];
                if (fabs(tmp) > fabs(max)) {
                        max = tmp;
			maxi = i;
		}
	}

	// swap rows, change p
	if (maxi != 0) {
		swap_rows(offset, offset+maxi);
		int tmp = p[offset];
		p[offset] = p[offset+maxi];
		p[offset+maxi] = tmp;
	}
}



template<class D>
matrix4t<D> matrix4t<D>::inverse(void) const
{
	matrix4t<D> r(*this);
        int i, j, k;

        // prepare row swap
	vector<int> p(size);
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
	for (j = size-1; j >= 0; j--) {
		r.values[j*size+j] = D(1.0)/r.values[j*size+j];
                for (i = j-1; i >= 0; i--) {
			D s = r.values[i*size+j] * r.values[j*size+j];
			for (k = i+1; k < j; k++) {
				s += r.values[i*size+k] * r.values[k*size+j];
			}
			r.values[i*size+j] = -s/r.values[i*size+i];
		}
	}

	// invert L without using extra memory
	for (j = size-1; j >= 0; j--) {
                for (i = j-1; i >= 0; i--) {
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
			int z = (i > j) ? i : j;
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



typedef matrix4t<double> matrix4;
typedef matrix4t<float> matrix4f;
typedef matrix4t<int> matrix4i;

#endif
