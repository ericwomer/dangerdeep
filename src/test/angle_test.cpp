/*
 * Test mínimo para angle (ángulo náutico, 0..360, clockwise).
 */
#include "../angle.h"
#include "../vector2.h"
#include <cassert>
#include <cmath>
#include <cstdio>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static bool near(double a, double b, double eps = 1e-9) {
    return std::fabs(a - b) <= eps;
}

int main() {
    angle a0;
    assert(near(a0.value(), 0.0));
    assert(near(a0.value_pm180(), 0.0));

    angle a90(90.0);
    assert(near(a90.value(), 90.0));
    assert(near(a90.value_pm180(), 90.0));
    assert(near(a90.rad(), M_PI / 2.0));

    angle a360(360.0);
    assert(near(a360.value(), 0.0));

    angle a_neg(-90.0);
    assert(near(a_neg.value(), 270.0));
    assert(near(a_neg.value_pm180(), -90.0));

    angle sum = a90 + angle(10.0);
    assert(near(sum.value(), 100.0));

    angle diff = angle(30.0) - angle(10.0);
    assert(near(diff.value(), 20.0));

    angle from_r = angle::from_rad(M_PI);
    assert(near(from_r.value(), 180.0));

    assert(near(angle(0.0).diff(angle(90.0)), 90.0));
    assert(near(angle(0.0).diff(angle(270.0)), 90.0));

    vector2 d = angle(0.0).direction();
    assert(near(d.x, 0.0) && near(d.y, 1.0));
    vector2 d90 = angle(90.0).direction();
    assert(near(d90.x, 1.0) && near(d90.y, 0.0));

    printf("angle_test ok\n");
    return 0;
}
