/*
 * Test para vector4 (vector 4D).
 */
#include "catch_amalgamated.hpp"
#include "../vector4.h"
#include <cmath>

static bool near(double a, double b, double eps = 1e-9) {
    return std::fabs(a - b) <= eps;
}

TEST_CASE("vector4 - componentes", "[vector4]") {
    vector4 a(0, 0, 0, 0), c(1, 2, 2, 1);
    REQUIRE(a.x == 0);
    REQUIRE(a.w == 0);
    REQUIRE(near(c.length(), std::sqrt(10.0)));
}

TEST_CASE("vector4 - normal", "[vector4]") {
    vector4 c(1, 2, 2, 1);
    vector4 n = c.normal();
    REQUIRE(near(n.length(), 1.0));
}

TEST_CASE("vector4 - suma y producto escalar", "[vector4]") {
    vector4 b(1, 0, 0, 0);
    vector4 sum = b + vector4(0, 1, 0, 0);
    REQUIRE(sum.x == 1);
    REQUIRE(sum.y == 1);
    REQUIRE(near(b * vector4(1, 0, 0, 0), 1.0));
}

TEST_CASE("vector4 - to_real homogéneo", "[vector4]") {
    vector4 homog(2, 4, 6, 2);
    vector3 real = homog.to_real();
    REQUIRE(near(real.x, 1.0));
    REQUIRE(near(real.y, 2.0));
    REQUIRE(near(real.z, 3.0));
}

TEST_CASE("vector4 - distance", "[vector4]") {
    REQUIRE(near(vector4(3, 0, 0, 0).distance(vector4(0, 0, 0, 0)), 3.0));
}
