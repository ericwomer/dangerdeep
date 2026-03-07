/*
 * Test para plane_t (plano 3D).
 */
#include "catch_amalgamated.hpp"
#include "../plane.h"
#include <cmath>

static bool near(double a, double b, double eps = 1e-9) {
    return std::fabs(a - b) <= eps;
}

TEST_CASE("plane - is_left y test_side", "[plane]") {
    vector3 n(0, 1, 0);
    planef pl(n, 0.0f);
    REQUIRE(pl.is_left(vector3f(0, 1, 0)));
    REQUIRE_FALSE(pl.is_left(vector3f(0, -1, 0)));
    REQUIRE(pl.test_side(vector3f(0, 0, 0)) == 0);
    REQUIRE(pl.test_side(vector3f(0, 1, 0)) == 1);
    REQUIRE(pl.test_side(vector3f(0, -1, 0)) == -1);
}

TEST_CASE("plane - distance", "[plane]") {
    vector3 n(0, 1, 0);
    planef pl(n, 0.0f);
    float d = pl.distance(vector3f(0, 5, 0));
    REQUIRE(near(d, 5.0));
}

TEST_CASE("plane - test_intersection", "[plane]") {
    vector3 n(0, 1, 0);
    planef pl(n, 0.0f);
    vector3f a(0, -1, 0), b(0, 1, 0), pt;
    REQUIRE(pl.test_intersection(a, b, pt));
    REQUIRE(near(pt.y, 0.0f));
}

TEST_CASE("plane - constructor desde dos puntos", "[plane]") {
    planef pl2(vector3f(1, 0, 0), vector3f(1, 0, 0));
    REQUIRE(near(pl2.distance(vector3f(1, 0, 0)), 0.0));
}
