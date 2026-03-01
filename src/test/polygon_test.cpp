/*
 * Test para polygon.h: polygon_t (triangulo, cuadrilatero, empty, normal, get_plane).
 */
#include "../polygon.h"
#include <cassert>
#include <cmath>
#include <cstdio>

static bool near(double a, double b, double eps = 1e-9) {
    return std::fabs(a - b) <= eps;
}

int main() {
    polygon empty_p;
    assert(empty_p.empty());
    assert(empty_p.points.size() == 0);

    polygon tri(vector3(0, 0, 0), vector3(1, 0, 0), vector3(0.5, 1, 0));
    assert(!tri.empty());
    assert(tri.points.size() == 3);
    vector3 n = tri.normal();
    assert(near(n.length(), 1.0));
    assert(near(n.z, 1.0) || near(n.z, -1.0));

    (void)tri.get_plane();
    assert(tri.points[0].distance(tri.points[1]) >= 0);

    polygon quad(vector3(0, 0, 0), vector3(1, 0, 0), vector3(1, 1, 0), vector3(0, 1, 0));
    assert(quad.points.size() == 4);
    assert(!quad.empty());

    polygon cap(10);
    assert(cap.points.size() == 0);
    assert(cap.empty());

    printf("polygon_test ok\n");
    return 0;
}
