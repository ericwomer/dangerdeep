/*
 * Test para quaternion (rotaciones 3D).
 */
#include "catch_amalgamated.hpp"
#include "../quaternion.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

inline bool near(double a, double b, double eps = 1e-6) {
    return std::fabs(a - b) <= eps;
}

TEST_CASE("quaternion - Constructor por defecto", "[quaternion]") {
    quaternionf q;
    
    REQUIRE(near(q.s, 1.0f));
    REQUIRE(q.v.x == 0);
    REQUIRE(q.v.y == 0);
    REQUIRE(q.v.z == 0);
    REQUIRE(near(q.length(), 1.0f));
}

TEST_CASE("quaternion - Constructor desde vector", "[quaternion]") {
    quaternionf qv = quaternionf::vec(1, 0, 0);
    
    REQUIRE(qv.s == 0);
    REQUIRE(qv.v.x == 1);
    REQUIRE(qv.v.y == 0);
    REQUIRE(qv.v.z == 0);
}

TEST_CASE("quaternion - Rotación y conjugado", "[quaternion]") {
    quaternionf qr = quaternionf::rot(90.0f, 0, 1, 0);
    REQUIRE(near(qr.length(), 1.0f));
    
    quaternionf qc = qr.conj();
    REQUIRE(near(qc.s, qr.s));
    REQUIRE(near(qc.v.x, -qr.v.x));
}

TEST_CASE("quaternion - Quaternion cero", "[quaternion]") {
    quaternionf qz = quaternionf::zero();
    
    REQUIRE(qz.s == 0);
    REQUIRE(qz.v.x == 0);
    REQUIRE(qz.v.y == 0);
    REQUIRE(qz.v.z == 0);
}
