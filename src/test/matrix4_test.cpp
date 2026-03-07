/*
 * Test para matrix4 (matriz 4x4).
 */
#include "catch_amalgamated.hpp"
#include "../matrix4.h"
#include <cmath>

static bool near(double a, double b, double eps = 1e-9) {
    return std::fabs(a - b) <= eps;
}

TEST_CASE("matrix4 - identidad", "[matrix4]") {
    matrix4f I = matrix4f::one();
    REQUIRE(near(I.elem(0, 0), 1.0f));
    REQUIRE(near(I.elem(1, 1), 1.0f));
    REQUIRE(near(I.elem(2, 2), 1.0f));
    REQUIRE(near(I.elem(3, 3), 1.0f));
    REQUIRE(near(I.elem(0, 1), 0.0f));
    REQUIRE(near(I.elem(1, 0), 0.0f));
}

TEST_CASE("matrix4 - diagonal y transpuesta", "[matrix4]") {
    matrix4f m = matrix4f::diagonal(2.0f, 2.0f, 2.0f, 2.0f);
    matrix4f half = m * 0.5f;
    REQUIRE(near(half.elem(0, 0), 1.0f));

    matrix4f mt = m.transpose();
    REQUIRE(mt.elem(0, 0) == m.elem(0, 0));
}

TEST_CASE("matrix4 - producto por vector", "[matrix4]") {
    matrix4f I = matrix4f::one();
    vector4f v(1, 0, 0, 0);
    vector4f Iv = I * v;
    REQUIRE(near(Iv.x, 1.0f));
    REQUIRE(near(Iv.y, 0.0f));
    REQUIRE(near(Iv.z, 0.0f));
    REQUIRE(near(Iv.w, 0.0f));
}

TEST_CASE("matrix4 - suma", "[matrix4]") {
    matrix4f m = matrix4f::diagonal(2.0f, 2.0f, 2.0f, 2.0f);
    matrix4f m2 = m + m;
    REQUIRE(near(m2.elem(0, 0), 4.0f));
}
