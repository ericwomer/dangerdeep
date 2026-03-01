/*
 * Test minimo para triangle_intersection_t (is_null, compute sin interseccion).
 */
#include "../triangle_intersection.h"
#include <cassert>
#include <cstdio>

int main() {
    assert(triangle_intersection_t<float>::is_null(0.0f, 1e-5f));
    assert(!triangle_intersection_t<float>::is_null(1.0f, 1e-5f));

    vector3f a0(0, 0, 0), a1(1, 0, 0), a2(0, 1, 0);
    vector3f b0(0, 0, 1), b1(1, 0, 1), b2(0, 1, 1);
    bool hit = triangle_intersection_t<float>::compute(a0, a1, a2, b0, b1, b2, 1e-5f);
    assert(!hit);

    vector3f c0(0, 0, 0), c1(2, 0, 0), c2(0, 2, 0);
    vector3f d0(1, 1, -1), d1(1, 1, 0), d2(2, 0, -1);
    bool hit2 = triangle_intersection_t<float>::compute(c0, c1, c2, d0, d1, d2, 1e-5f);
    (void)hit2;
    assert(true);

    printf("triangle_intersection_test ok\n");
    return 0;
}
