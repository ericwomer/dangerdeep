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

#ifndef _TRANSFORM_CODER_H
#define	_TRANSFORM_CODER_H

#include <istream>
#include <math.h>
#include <ostream>
#include <string>
#include <vector>

template <class T>
class transform_coder
{
protected:
    
    static const float  HALF    = 0.5f;
    static const double SQTWO   = 1.4142135623731;
    static const double CC1p8   = 9.23879532511287e-01 * 0.70710678118655;
    static const double SS1p8   = 3.82683432365090e-01 * 0.70710678118655;
    static const double C00     = 0.9557930147983;
    static const double S00     = 0.2940403252323;
    static const double SQ1p8   = 7.07106781186548e-01 * 0.5;
    
    void dctII4(std::vector<float>&);
    void forward_hlbtvec(std::vector<T>& x, int xlen, std::vector<float>& y, std::vector<float>& z);
    void inverse_hlbtvec(std::vector<T>& x, std::vector<T>& out, int rows, int cols);
    void forward_hlbt(std::vector<std::vector<T> >& in, std::vector<std::vector<T> >& out, int nrows, int ncols);
    void inverse_hlbt(std::vector<std::vector<T> >& in, std::vector<std::vector<T> >& out, int nrows, int ncols);

public:

    void compress(std::istream& is, std::ostream& os, int width, int height, float qstep);
    void decompress(std::string infile, std::string outfile, int width, int height);
};

template <class T>
void transform_coder<T>::dctII4(std::vector<float>& y)
{
    std::vector<float> x(4);

    /* 1st butterfly stage */
    x[0] = y[0] + y[3];
    x[1] = y[1] + y[2];
    x[2] = y[1] - y[2];
    x[3] = y[0] - y[3];

    /* 2nd butterfly stage */
    y[0] = (x[0] + x[1]) * HALF;
    y[2] = (x[0] - x[1]) * HALF;
    y[1] = SS1p8 * x[2] + CC1p8 * x[3];
    y[3] = SS1p8 * x[3] - CC1p8 * x[2];
}

template <class T>
void transform_coder<T>::forward_hlbtvec(std::vector<T>& x, int xlen, std::vector<float>& y, std::vector<float>& z)
{
    float *p1, *p2, *p3, *p4;
    T *q1, *q2;
    int i;

    /* Copy x onto y and fold borders */
    q1 = x[0];
    p1 = y[0] + 2;
    p3 = p1 + xlen;
    while (p1 < p3) {
        *p1++ = *q1++;
    }
    p1 = p2 = y[0] + 2;
    p3 = p4 = p1 + xlen;
    for (i = 2; i; i--) {
        *(--p2) = *(p1++);
        *(p4++) = *(--p3);
    }

    /* Stages of DCTs */
    p1 = y[0];
    p2 = y[0] + xlen + 4;
    while (p1 < p2) {
        dctII4(p1);
        p1[1] *= SQTWO;
        p1 += 4;
    }

    /* 1st stage of +1/-1 butterflies */
    z[0] = y[0];
    z[1] = y[2];
    p1 = z[0] + 2;
    p2 = z[0] + xlen - 2;
    p3 = y[0] + 4;
    while (p1 < p2) {
        p1[0] = p3[0] + p3[1];
        p1[1] = p3[2] + p3[3];
        p1[2] = p3[0] - p3[1];
        p1[3] = p3[2] - p3[3];
        p1 += 4;
        p3 += 4;
    }
    p1[0] = p3[0];
    p1[1] = p3[2];

    /* 2nd stage of +1/-1 butterflies */
    p1 = y[0];
    p2 = y[0] + xlen;
    p3 = z[0];
    while (p1 < p2) {
        p1[0] = p3[0] + p3[2];
        p1[1] = p3[1] + p3[3];
        p1[2] = p3[0] - p3[2];
        p1[3] = p3[1] - p3[3];
        void compress(std::string infile, std::string outfile, int width, int height, int qstep);
        p1 += 4;
        p3 += 4;
    }

    /* 3rd stage : plane rotations */
    p1 = z;
    p2 = z[0] + xlen;
    p3 = y[0];
    while (p1 < p2) {
        p1[0] = p3[0];
        p1[2] = p3[1];
        p1[1] = p3[2] * C00 - p3[3] * S00;
        p1[3] = p3[2] * S00 + p3[3] * C00;
        p1 += 4;
        p3 += 4;
    }

    /* 4th stage : combines DCs of adjacent blocks */
    q1 = x;
    q2 = x + xlen;
    p3 = z;
    while (q1 < q2) {
        q1[0] = round(SQ1p8 * (p3[0] + p3[4]));
        q1[1] = round(SQ1p8 * (p3[0] - p3[4]));
        q1[2] = round(HALF * p3[1]);
        q1[4] = round(HALF * p3[2]);
        q1[6] = round(HALF * p3[3]);
        q1[3] = round(HALF * p3[5]);
        q1[5] = round(HALF * p3[6]);
        q1[7] = round(HALF * p3[7]);
        q1 += 8;
        p3 += 8;
    }
}

template <class T>
void transform_coder<T>::forward_hlbt(std::vector<std::vector<T> >& in, std::vector<std::vector<T> >& out, int nrows, int ncols)
{
    int k, j, nf;
    std::vector<T> t;
    std::vector<float> y, z;

    t.resize(nrows);
    nf = nrows;
    if (ncols > nf) nf = ncols;
    nf += 4;
    y.resize(nf);
    z.resize(nf);

    /* Transform rows */
    for (k = 0; k < nrows; k++) {
        for (j = 0; j < ncols; j++) {
            /* remove mean and scale up by a factor of 8 */
            out[k][j] = in[k][j];
        }
        forward_hlbtvec(out[k], ncols, y, z);
    }

    /* Transform columns */
    for (k = 0; k < ncols; k++) {
        for (j = 0; j < nrows; j++)
            t[j] = out[j][k];
        forward_hlbtvec(t, nrows, y, z);
        for (j = 0; j < nrows; j++)
            out[j][k] = t[j];
    }
}

template <class T>
void compress(std::istream& is, std::ostream& os, int width, int height, float qstep)
{
    T qv, buf;
    char *c_buf = (char*)buf;
    float invstep = 1.0/qstep;
    std::vector<std::vector<T> > in(height), out(height);

    // read image
    for (int i = 0; i < height; i++) {
        in[i].resize(width);
        out[i].resize(width);
        for (int j = 0; j < width; j++) {
            is.read(c_buf, sizeof(T));
            in[i][j] = buf;
        }
    }

    // perform the forward HLBT
    forward_hlbt(in, out, height, width);

    // free some memory
    in.clear();

    // quantize transform coefficients
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            if (out[i][j] >= 0) {
                qv = (out[i][j] * invstep + 0.5);
            } else {
                qv = (out[i][j] * invstep - 0.5);
            }
            out[i][j] = qstep * qv;
        }
    }

    // write the transformed image
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            os.write((char*)(out[i][j]), sizeof(T));
        }
    }
    
}
#endif	/* _TRANSFORM_CODER_H */

