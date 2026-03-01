/*
 * Test minimo para vector4 (vector 4D).
 */
#include "../vector4.h"
#include <cassert>
#include <cmath>
#include <cstdio>

static bool near(double a, double b, double eps = 1e-9) {
    return std::fabs(a - b) <= eps;
}

int main() {
    vector4 a(0, 0, 0, 0), b(1, 0, 0, 0), c(1, 2, 2, 1);
    assert(a.x == 0 && a.w == 0);
    assert(near(c.length(), std::sqrt(10.0)));
    vector4 n = c.normal();
    assert(near(n.length(), 1.0));

    vector4 sum = b + vector4(0, 1, 0, 0);
    assert(sum.x == 1 && sum.y == 1);
    assert(near(b * vector4(1, 0, 0, 0), 1.0));
    vector4 homog(2, 4, 6, 2);
    vector3 real = homog.to_real();
    assert(near(real.x, 1.0) && near(real.y, 2.0) && near(real.z, 3.0));
    assert(near(vector4(3, 0, 0, 0).distance(vector4(0, 0, 0, 0)), 3.0));

    printf("vector4_test ok\n");
    return 0;
}
