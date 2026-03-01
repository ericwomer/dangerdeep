/*
 * Test for matrix3 (3x3 matrix).
 */
#include "../matrix3.h"
#include <cassert>
#include <cmath>
#include <cstdio>

static bool near(double a, double b, double eps = 1e-9) {
    return std::fabs(a - b) <= eps;
}

int main() {
    matrix3f I = matrix3f::one();
    assert(near(I.elem(0, 0), 1.0f) && near(I.elem(1, 1), 1.0f) && near(I.elem(2, 2), 1.0f));
    assert(near(I.elem(0, 1), 0.0f));
    assert(near(I.determinant(), 1.0f));

    matrix3f m(1, 0, 0, 0, 2, 0, 0, 0, 3);
    assert(near(m.determinant(), 6.0f));
    matrix3f mt = m.transpose();
    assert(mt.elem(0, 0) == m.elem(0, 0) && mt.elem(1, 0) == m.elem(0, 1));

    vector3f v(1, 2, 3);
    vector3f mv = I * v;
    assert(mv.x == v.x && mv.y == v.y && mv.z == v.z);

    matrix3f vv = matrix3f::vec_sqr(vector3f(1, 0, 0));
    assert(near(vv.elem(0, 0), 1.0f) && near(vv.elem(1, 0), 0.0f));

    printf("matrix3_test ok\n");
    return 0;
}
