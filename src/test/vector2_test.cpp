/*
 * Test mínimo para vector2 (vector 2D).
 */
#include "../vector2.h"
#include <cassert>
#include <cmath>
#include <cstdio>

static bool near(double a, double b, double eps = 1e-9) {
    return std::fabs(a - b) <= eps;
}

int main() {
    vector2 a(0, 0), b(3, 4);
    assert(a.x == 0 && a.y == 0);
    assert(b.x == 3 && b.y == 4);

    assert(near(b.length(), 5.0));
    assert(near(b.square_length(), 25.0));

    vector2 n = b.normal();
    assert(near(n.length(), 1.0));
    assert(near(n.x, 0.6) && near(n.y, 0.8));

    vector2 sum = a + b;
    assert(sum.x == 3 && sum.y == 4);
    vector2 diff = vector2(5, 5) - vector2(2, 3);
    assert(diff.x == 3 && diff.y == 2);

    vector2 scaled = b * 2.0;
    assert(scaled.x == 6 && scaled.y == 8);
    assert(near(b * vector2(1, 0), 3.0));
    assert(near(b * vector2(0, 1), 4.0));

    assert(near(vector2(1, 0).orthogonal().x, 0.0) && near(vector2(1, 0).orthogonal().y, 1.0));
    assert(near(vector2(1, 0).distance(vector2(4, 0)), 3.0));

    vector2 minv = vector2(3, 1).min(vector2(2, 5));
    assert(minv.x == 2 && minv.y == 1);
    vector2 maxv = vector2(3, 1).max(vector2(2, 5));
    assert(maxv.x == 3 && maxv.y == 5);

    printf("vector2_test ok\n");
    return 0;
}
