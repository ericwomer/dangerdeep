/*
 * Test mínimo para plane_t (plano 3D).
 */
#include "../plane.h"
#include <cassert>
#include <cmath>
#include <cstdio>

static bool near(double a, double b, double eps = 1e-9) {
    return std::fabs(a - b) <= eps;
}

int main() {
    vector3 n(0, 1, 0);
    planef pl(n, 0.0f);
    assert(pl.is_left(vector3f(0, 1, 0)));
    assert(!pl.is_left(vector3f(0, -1, 0)));
    assert(pl.test_side(vector3f(0, 0, 0)) == 0);
    assert(pl.test_side(vector3f(0, 1, 0)) == 1);
    assert(pl.test_side(vector3f(0, -1, 0)) == -1);

    float d = pl.distance(vector3f(0, 5, 0));
    assert(near(d, 5.0));

    vector3f a(0, -1, 0), b(0, 1, 0), pt;
    assert(pl.test_intersection(a, b, pt));
    assert(near(pt.y, 0.0f));

    planef pl2(vector3f(1, 0, 0), vector3f(1, 0, 0));
    assert(near(pl2.distance(vector3f(1, 0, 0)), 0.0));

    printf("plane_test ok\n");
    return 0;
}
