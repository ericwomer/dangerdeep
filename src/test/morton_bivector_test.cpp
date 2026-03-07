/*
 * Test para morton_bivector.h: resize, at, size, operator[].
 */
#include "catch_amalgamated.hpp"
#include "../morton_bivector.h"
#include <cmath>

static bool near(double a, double b, double eps = 1e-9) {
    return std::fabs(a - b) < eps;
}

TEST_CASE("morton_bivector - at y operator[]", "[morton_bivector]") {
    morton_bivector<int> mb(2, 0);
    REQUIRE(mb.size() == 2);
    mb.at(0, 0) = 1;
    mb.at(1, 0) = 2;
    mb.at(0, 1) = 3;
    mb.at(1, 1) = 4;
    REQUIRE(mb.at(0, 0) == 1);
    REQUIRE(mb.at(1, 1) == 4);
    REQUIRE(mb[vector2i(1, 0)] == 2);
}

TEST_CASE("morton_bivector - double con valor inicial", "[morton_bivector]") {
    morton_bivector<double> mb2(4, 1.0);
    REQUIRE(mb2.size() == 4);
    REQUIRE(near(mb2.at(0, 0), 1.0));
}
