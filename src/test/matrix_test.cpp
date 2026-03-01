/*
 * Test para matrix.h: matrix_swap_rows, matrix_invert, matrixt.
 */
#include "../matrix.h"
#include <cassert>
#include <cmath>
#include <cstdio>

static bool near(double a, double b, double eps = 1e-9) {
    return std::fabs(a - b) <= eps;
}

int main() {
    double m2_id[4] = {1, 0, 0, 1};
    matrix_invert<double, 2>(m2_id);
    assert(near(m2_id[0], 1) && near(m2_id[3], 1) && near(m2_id[1], 0) && near(m2_id[2], 0));

    double m2_diag[4] = {2, 0, 0, 2};
    matrix_invert<double, 2>(m2_diag);
    assert(near(m2_diag[0], 0.5) && near(m2_diag[3], 0.5));

    double m2_swap[4] = {1, 2, 3, 4};
    matrix_swap_rows<double, 2>(m2_swap, 0, 1);
    assert(near(m2_swap[0], 3) && near(m2_swap[1], 4) && near(m2_swap[2], 1) && near(m2_swap[3], 2));

    matrixt<double, 2> i2 = matrixt<double, 2>::one();
    assert(near(i2.elem(0, 0), 1) && near(i2.elem(1, 1), 1));
    assert(near(i2.elem(0, 1), 0) && near(i2.elem(1, 0), 0));

    matrixt<double, 2> a;
    a.elem(0, 0) = 1;
    a.elem(1, 0) = 2;
    a.elem(0, 1) = 3;
    a.elem(1, 1) = 4;
    matrixt<double, 2> at = a.transpose();
    assert(near(at.elem(0, 0), 1) && near(at.elem(0, 1), 2));
    assert(near(at.elem(1, 0), 3) && near(at.elem(1, 1), 4));

    matrixt<double, 2> i2x2 = i2 * 3.0;
    assert(near(i2x2.elem(0, 0), 3) && near(i2x2.elem(1, 1), 3));

    matrixt<double, 2> sum = i2 + i2;
    assert(near(sum.elem(0, 0), 2) && near(sum.elem(1, 1), 2));

    printf("matrix_test ok\n");
    return 0;
}
