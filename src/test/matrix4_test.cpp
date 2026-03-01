/*
 * Test para matrix4 (4x4 matrix).
 */
#include "../matrix4.h"
#include <cassert>
#include <cmath>
#include <cstdio>

static bool near(double a, double b, double eps = 1e-9) {
    return std::fabs(a - b) <= eps;
}

int main() {
    matrix4f I = matrix4f::one();
    assert(near(I.elem(0, 0), 1.0f) && near(I.elem(1, 1), 1.0f));
    assert(near(I.elem(2, 2), 1.0f) && near(I.elem(3, 3), 1.0f));
    assert(near(I.elem(0, 1), 0.0f) && near(I.elem(1, 0), 0.0f));

    matrix4f m = matrix4f::diagonal(2.0f, 2.0f, 2.0f, 2.0f);
    matrix4f half = m * 0.5f;
    assert(near(half.elem(0, 0), 1.0f));

    matrix4f mt = m.transpose();
    assert(mt.elem(0, 0) == m.elem(0, 0));

    vector4f v(1, 0, 0, 0);
    vector4f Iv = I * v;
    assert(near(Iv.x, 1.0f) && near(Iv.y, 0.0f) && near(Iv.z, 0.0f) && near(Iv.w, 0.0f));

    matrix4f m2 = m + m;
    assert(near(m2.elem(0, 0), 4.0f));

    printf("matrix4_test ok\n");
    return 0;
}
