/*
 * Test for quaternion (3D rotations).
 */
#include "../quaternion.h"
#include <cassert>
#include <cmath>
#include <cstdio>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
static bool near(double a, double b, double eps = 1e-6) {
    return std::fabs(a - b) <= eps;
}

int main() {
    quaternionf q;
    assert(near(q.s, 1.0f) && q.v.x == 0 && q.v.y == 0 && q.v.z == 0);
    assert(near(q.length(), 1.0f));

    quaternionf qv = quaternionf::vec(1, 0, 0);
    assert(qv.s == 0 && qv.v.x == 1 && qv.v.y == 0 && qv.v.z == 0);

    quaternionf qr = quaternionf::rot(90.0f, 0, 1, 0);
    assert(near(qr.length(), 1.0f));
    quaternionf qc = qr.conj();
    assert(near(qc.s, qr.s) && near(qc.v.x, -qr.v.x));

    quaternionf qz = quaternionf::zero();
    assert(qz.s == 0 && qz.v.x == 0 && qz.v.y == 0 && qz.v.z == 0);

    printf("quaternion_test ok\n");
    return 0;
}
