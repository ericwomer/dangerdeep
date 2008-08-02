/*
 * Danger from the Deep - Open source submarine simulation
 * Copyright (C) 2003-2006  Thorsten Jordan, Luis Barrancos and others.
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* Performs a Lapped Biothogonal Transformation (LBT) on a given image stream as
 * it was described in "Fast progressive image coding without wavelets" 
 * by Henrique Malvar (http://research.microsoft.com/~malvar/). Most of the code
 * was developed by Malvar for C and was just rewrited for C++. The original 
 * code can be found on his page.
 *
 * For more flexebility this coder does not compress anything. It just does the
 * LBT and quantization. So you are able to do anything you want with the output
 * stream (RLE, ZIP, etc.)
 */

#ifndef _TRANSFORM_CODER_H
#define	_TRANSFORM_CODER_H

#include <iostream>
#include <math.h>
#include <vector>

template <class T>
class transform_coder
{
protected:

    static const float ZERO = 0.0f;
    static const float HALF = 0.5f;
    static const float TWO  = 2.0f;
    static const double SQHALF = 0.70710678118655;
    static const double SQTWO = 1.4142135623731;
    static const double CC1p8 = 9.23879532511287e-01 * 0.70710678118655;
    static const double SS1p8 = 3.82683432365090e-01 * 0.70710678118655;
    static const double C00 = 0.9557930147983;
    static const double S00 = 0.2940403252323;
    static const double SQ1p8 = 7.07106781186548e-01 * 0.5;

    void dctII4(float* y);
    void dctIII4(float* y);
    void forward_hlbtvec(std::vector<T>& x, int xlen, std::vector<float>& y, std::vector<float>& z);
    void inverse_hlbtvec(std::vector<T>& x, int xlen, std::vector<float>& y, std::vector<float>& z);
    void forward_hlbt(std::vector<std::vector<T> >& in, std::vector<std::vector<T> >& out, int nrows, int ncols);
    void inverse_hlbt(std::vector<std::vector<T> >& in, std::vector<std::vector<T> >& out, int nrows, int ncols);

public:

    void compress(std::istream& is, std::ostream& os, int width, int height, float qstep);
    void decompress(std::istream& is, std::ostream& os, int width, int height);
};

template <class T>
void transform_coder<T>::compress(std::istream& is, std::ostream& os, int width, int height, float qstep)
{
    T qv;
    float invstep = 1.0 / qstep;
    std::vector<std::vector<T> > in, out;

    in.resize(height);
    out.resize(height);
    // read image
    for (int i = 0; i < height; i++) {
        in[i].resize(width);
        out[i].resize(width);
        for (int j = 0; j < width; j++) {
            is.read((char*) & in[i][j], sizeof (T));
        }
    }

    // perform the forward HLBT
    std::cout << "Performing HLBT on image" << std::endl;
    forward_hlbt(in, out, height, width);

    // quantize transform coefficients
    std::cout << "quantize transform coefficients" << std::endl;
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
void transform_coder<T>::inverse_hlbt(std::vector<std::vector<T> >& in, std::vector<std::vector<T> >& out, int nrows, int ncols)
{
    int k, j, nf;
    T v;
    std::vector<T> t;
    std::vector<float> y, z;

    /* Allocate memory for auxiliary vector */
    t.resize(nrows);
    nf = nrows;
    if (ncols > nf) nf = ncols;
    nf += 8;
    y.resize(nf);
    z.resize(nf);

    /* Transform columns */
    for (k = 0; k < ncols; k++) {
        for (j = 0; j < nrows; j++)
            t[j] = in[j][k];
        inverse_hlbtvec(t, nrows, y, z);
        for (j = 0; j < nrows; j++)
            in[j][k] = t[j];
    }

    /* Transform rows */
    for (k = 0; k < nrows; k++) {
        inverse_hlbtvec(in[k], ncols, y, z);
        for (j = 0; j < ncols; j++) {
            out[k][j] = in[k][j];
        }
    }
}

template <class T>
void transform_coder<T>::forward_hlbtvec(std::vector<T>& x, int xlen, std::vector<float>& y, std::vector<float>& z)
{
    float *p1, *p2, *p3, *p4;
    T *q1, *q2;
    int i;

    /* Copy x onto y and fold borders */
    q1 = &x[0];
    p1 = &y[0] + 2;
    p3 = p1 + xlen;
    while (p1 < p3) {
        *p1++ = *q1++;
    }
    p1 = p2 = &y[0] + 2;
    p3 = p4 = p1 + xlen;
    for (i = 2; i; i--) {
        *(--p2) = *(p1++);
        *(p4++) = *(--p3);
    }

    /* Stages of DCTs */
    p1 = &y[0];
    p2 = &y[0] + xlen + 4;
    while (p1 < p2) {
        dctII4(p1);
        p1[1] *= SQTWO;
        p1 += 4;
    }

    /* 1st stage of +1/-1 butterflies */
    z[0] = y[0];
    z[1] = y[2];
    p1 = &z[0] + 2;
    p2 = &z[0] + xlen - 2;
    p3 = &y[0] + 4;
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
    p1 = &y[0];
    p2 = &y[0] + xlen;
    p3 = &z[0];
    while (p1 < p2) {
        p1[0] = p3[0] + p3[2];
        p1[1] = p3[1] + p3[3];
        p1[2] = p3[0] - p3[2];
        p1[3] = p3[1] - p3[3];
        p1 += 4;
        p3 += 4;
    }

    /* 3rd stage : plane rotations */
    p1 = &z[0];
    p2 = &z[0] + xlen;
    p3 = &y[0];
    while (p1 < p2) {
        p1[0] = p3[0];
        p1[2] = p3[1];
        p1[1] = p3[2] * C00 - p3[3] * S00;
        p1[3] = p3[2] * S00 + p3[3] * C00;
        p1 += 4;
        p3 += 4;
    }

    /* 4th stage : combines DCs of adjacent blocks */
    q1 = &x[0];
    q2 = &x[0] + xlen;
    p3 = &z[0];
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
void transform_coder<T>::inverse_hlbtvec(std::vector<T>& x, int xlen, std::vector<float>& y, std::vector<float>& z)
{
    float *p1, *p2, *p3;
    T *q1, *q3;

    /* 4th stage : combines DCs of adjacent blocks */
    p1 = &z[0];
    p2 = &z[0] + xlen;
    q3 = &x[0];
    while (p1 < p2) {
        p1[0] = SQ1p8 * (q3[0] + (float) q3[1]);
        p1[4] = SQ1p8 * (q3[0] - (float) q3[1]);
        p1[1] = HALF * q3[2];
        p1[2] = HALF * q3[4];
        p1[3] = HALF * q3[6];
        p1[5] = HALF * q3[3];
        p1[6] = HALF * q3[5];
        p1[7] = HALF * q3[7];
        p1 += 8;
        q3 += 8;
    }

    /* 3rd stage : plane rotations */
    p1 = &y[0];
    p2 = &y[0] + xlen;
    p3 = &z[0];
    while (p1 < p2) {
        p1[0] = p3[0];
        p1[1] = p3[2];
        p1[2] = p3[1] * C00 + p3[3] * S00;
        p1[3] = p3[3] * C00 - p3[1] * S00;
        p1 += 4;
        p3 += 4;
    }

    /* 2nd stage of +1/-1 butterflies */
    p1 = &z[0];
    p2 = &z[0] + xlen;
    p3 = &y[0];
    while (p1 < p2) {
        p1[0] = p3[0] + p3[2];
        p1[1] = p3[1] + p3[3];
        p1[2] = p3[0] - p3[2];
        p1[3] = p3[1] - p3[3];
        p1 += 4;
        p3 += 4;
    }

    /* 1st stage of +1/-1 butterflies */
    y[0] = z[0] * TWO;
    y[2] = z[1] * TWO;
    y[1] = y[3] = ZERO;
    p1 = &y[0] + 4;
    p2 = &y[0] + xlen;
    p3 = &z[0] + 2;
    while (p1 < p2) {
        p1[0] = p3[0] + p3[2];
        p1[2] = p3[1] + p3[3];
        p1[1] = p3[0] - p3[2];
        p1[3] = p3[1] - p3[3];
        p1 += 4;
        p3 += 4;
    }
    p1[0] = p3[0] * TWO;
    p1[2] = p3[1] * TWO;
    p1[1] = p1[3] = ZERO;

    /* Stages of IDCTs */
    p1 = &y[0];
    p2 = &y[0] + xlen + 4;
    while (p1 < p2) {
        p1[1] *= SQHALF;
        dctIII4(p1);
        p1 += 4;
    }

    /* Copy y back to x */
    q1 = &x[0];
    p1 = &y[0] + 2;
    p3 = p1 + xlen;
    while (p1 < p3) {
        *q1++ = round(*p1++);
    }
}

template <class T>
void transform_coder<T>::dctIII4(float* y)
{
    std::vector<float> x(4);

    /* 1st butterfly stage */
    x[0] = (y[0] + y[2]) * HALF;
    x[1] = (y[0] - y[2]) * HALF;
    x[2] = SS1p8 * y[1] - CC1p8 * y[3];
    x[3] = SS1p8 * y[3] + CC1p8 * y[1];

    /* 2nd butterfly stage */
    y[0] = x[0] + x[3];
    y[1] = x[1] + x[2];
    y[2] = x[1] - x[2];
    y[3] = x[0] - x[3];
}

template <class T>
void transform_coder<T>::dctII4(float* y)
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
#endif	/* _TRANSFORM_CODER_H */

