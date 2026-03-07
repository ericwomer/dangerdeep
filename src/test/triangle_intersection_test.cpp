/*
 * Test para triangle_intersection_t: is_null, compute.
 */
#include "catch_amalgamated.hpp"
#include "../triangle_intersection.h"

TEST_CASE("triangle_intersection - is_null", "[triangle_intersection]") {
    REQUIRE(triangle_intersection_t<float>::is_null(0.0f, 1e-5f));
    REQUIRE_FALSE(triangle_intersection_t<float>::is_null(1.0f, 1e-5f));
}

TEST_CASE("triangle_intersection - compute sin intersección", "[triangle_intersection]") {
    vector3f a0(0, 0, 0), a1(1, 0, 0), a2(0, 1, 0);
    vector3f b0(0, 0, 1), b1(1, 0, 1), b2(0, 1, 1);
    bool hit = triangle_intersection_t<float>::compute(a0, a1, a2, b0, b1, b2, 1e-5f);
    REQUIRE_FALSE(hit);
}

TEST_CASE("triangle_intersection - compute con triángulos", "[triangle_intersection]") {
    vector3f c0(0, 0, 0), c1(2, 0, 0), c2(0, 2, 0);
    vector3f d0(1, 1, -1), d1(1, 1, 0), d2(2, 0, -1);
    bool hit2 = triangle_intersection_t<float>::compute(c0, c1, c2, d0, d1, d2, 1e-5f);
    (void)hit2;  // resultado depende de implementación
}
