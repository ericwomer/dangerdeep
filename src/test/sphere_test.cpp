/*
 * Test mínimo para sphere_t (esfera 3D).
 */
#include "../sphere.h"
#include <cassert>
#include <cmath>
#include <cstdio>

static bool near(double a, double b, double eps = 1e-9) {
    return std::fabs(a - b) <= eps;
}

int main() {
    spheref s(vector3f(0, 0, 0), 5.0f);
    assert(s.is_inside(vector3f(0, 0, 0)));
    assert(s.is_inside(vector3f(3, 0, 0)));
    assert(!s.is_inside(vector3f(6, 0, 0)));

    spheref s2(vector3f(10, 0, 0), 3.0f);
    assert(!s.intersects(s2));
    spheref s3(vector3f(4, 0, 0), 2.0f);
    assert(s.intersects(s3));

    spheref bound = s.compute_bound(s3);
    assert(bound.radius >= 5.0f);
    assert(bound.is_inside(vector3f(0, 0, 0)));
    assert(bound.is_inside(vector3f(4, 0, 0)));

    vector3f minv(1e30f, 1e30f, 1e30f), maxv(-1e30f, -1e30f, -1e30f);
    s.compute_min_max(minv, maxv);
    assert(near(minv.x, -5.0f) && near(maxv.x, 5.0f));

    printf("sphere_test ok\n");
    return 0;
}
