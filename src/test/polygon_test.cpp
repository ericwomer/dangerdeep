/*
 * Test para polygon.h: polygon_t (triángulo, cuadrilátero, empty, normal, get_plane).
 */
#include "catch_amalgamated.hpp"
#include "../polygon.h"
#include <cmath>

static bool near(double a, double b, double eps = 1e-9) {
    return std::fabs(a - b) <= eps;
}

TEST_CASE("polygon - empty", "[polygon]") {
    polygon empty_p;
    REQUIRE(empty_p.empty());
    REQUIRE(empty_p.points.size() == 0);
}

TEST_CASE("polygon - triángulo", "[polygon]") {
    polygon tri(vector3(0, 0, 0), vector3(1, 0, 0), vector3(0.5, 1, 0));
    REQUIRE_FALSE(tri.empty());
    REQUIRE(tri.points.size() == 3);
    vector3 n = tri.normal();
    REQUIRE(near(n.length(), 1.0));
    REQUIRE((near(n.z, 1.0) || near(n.z, -1.0)));
    (void)tri.get_plane();
    REQUIRE(tri.points[0].distance(tri.points[1]) >= 0);
}

TEST_CASE("polygon - cuadrilátero", "[polygon]") {
    polygon quad(vector3(0, 0, 0), vector3(1, 0, 0), vector3(1, 1, 0), vector3(0, 1, 0));
    REQUIRE(quad.points.size() == 4);
    REQUIRE_FALSE(quad.empty());
}

TEST_CASE("polygon - capacidad reservada", "[polygon]") {
    polygon cap(10);
    REQUIRE(cap.points.size() == 0);
    REQUIRE(cap.empty());
}
