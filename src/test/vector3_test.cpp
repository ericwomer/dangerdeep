/*
 * Test mínimo para vector3 (vector 3D).
 */
#include "../vector3.h"
#include <cassert>
#include <cmath>
#include <cstdio>

static bool near(double a, double b, double eps = 1e-9) {
    return std::fabs(a - b) <= eps;
}

int main() {
    vector3 a(0, 0, 0), b(1, 0, 0), c(0, 1, 0), d(1, 2, 2);
    assert(a.x == 0 && a.y == 0 && a.z == 0);
    assert(near(d.length(), 3.0));
    assert(near(d.square_length(), 9.0));

    vector3 n = d.normal();
    assert(near(n.length(), 1.0));
    assert(near(n.x, 1.0/3.0) && near(n.y, 2.0/3.0) && near(n.z, 2.0/3.0));

    vector3 sum = b + c;
    assert(sum.x == 1 && sum.y == 1 && sum.z == 0);
    vector3 cross_bc = b.cross(c);
    assert(cross_bc.x == 0 && cross_bc.y == 0 && cross_bc.z == 1);
    assert(near(b * c, 0.0));
    assert(near(b * b, 1.0));

    assert(vector3(1, 2, 3).min(vector3(0, 3, 2)) == vector3(0, 2, 2));
    assert(vector3(1, 2, 3).max(vector3(0, 3, 2)) == vector3(1, 3, 3));
    assert(vector3(-1, 2, -3).abs() == vector3(1, 2, 3));
    assert(vector3(1.5, -0.3, 0).sign() == vector3(1, -1, 0));

    assert(near(vector3(3, 0, 0).distance(vector3(0, 0, 0)), 3.0));
    vector2 xy = vector3(5, 7, 9).xy();
    assert(xy.x == 5 && xy.y == 7);

    printf("vector3_test ok\n");
    return 0;
}
