/*
 * Test para matrix3 (matriz 3x3).
 */
#include "catch_amalgamated.hpp"
#include "../matrix3.h"
#include <cmath>

static bool near(double a, double b, double eps = 1e-9) {
    return std::fabs(a - b) <= eps;
}

TEST_CASE("matrix3 - identidad", "[matrix3]") {
    matrix3f I = matrix3f::one();
    REQUIRE(near(I.elem(0, 0), 1.0f));
    REQUIRE(near(I.elem(1, 1), 1.0f));
    REQUIRE(near(I.elem(2, 2), 1.0f));
    REQUIRE(near(I.elem(0, 1), 0.0f));
    REQUIRE(near(I.determinant(), 1.0f));
}

TEST_CASE("matrix3 - determinante y transpuesta", "[matrix3]") {
    matrix3f m(1, 0, 0, 0, 2, 0, 0, 0, 3);
    REQUIRE(near(m.determinant(), 6.0f));
    matrix3f mt = m.transpose();
    REQUIRE(mt.elem(0, 0) == m.elem(0, 0));
    REQUIRE(mt.elem(1, 0) == m.elem(0, 1));
}

TEST_CASE("matrix3 - producto por vector", "[matrix3]") {
    matrix3f I = matrix3f::one();
    vector3f v(1, 2, 3);
    vector3f mv = I * v;
    REQUIRE(mv.x == v.x);
    REQUIRE(mv.y == v.y);
    REQUIRE(mv.z == v.z);
}

TEST_CASE("matrix3 - vec_sqr", "[matrix3]") {
    matrix3f vv = matrix3f::vec_sqr(vector3f(1, 0, 0));
    REQUIRE(near(vv.elem(0, 0), 1.0f));
    REQUIRE(near(vv.elem(1, 0), 0.0f));
}
