/*
 * Test para angle (ángulo náutico, 0..360, clockwise).
 */
#include "../angle.h"
#include "../vector2.h"
#include "catch_amalgamated.hpp"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

inline bool near(double a, double b, double eps = 1e-9) {
    return std::fabs(a - b) <= eps;
}

TEST_CASE("angle - Constructor por defecto", "[angle]") {
    angle a;
    REQUIRE(near(a.value(), 0.0));
    REQUIRE(near(a.value_pm180(), 0.0));
}

TEST_CASE("angle - Constructor con valor", "[angle]") {
    angle a90(90.0);
    REQUIRE(near(a90.value(), 90.0));
    REQUIRE(near(a90.value_pm180(), 90.0));
    REQUIRE(near(a90.rad(), M_PI / 2.0));
}

TEST_CASE("angle - Normalización 360 grados", "[angle]") {
    angle a360(360.0);
    REQUIRE(near(a360.value(), 0.0));
}

TEST_CASE("angle - Ángulos negativos", "[angle]") {
    angle a_neg(-90.0);
    REQUIRE(near(a_neg.value(), 270.0));
    REQUIRE(near(a_neg.value_pm180(), -90.0));
}

TEST_CASE("angle - Operadores aritméticos", "[angle]") {
    SECTION("Suma") {
        angle sum = angle(90.0) + angle(10.0);
        REQUIRE(near(sum.value(), 100.0));
    }

    SECTION("Resta") {
        angle diff = angle(30.0) - angle(10.0);
        REQUIRE(near(diff.value(), 20.0));
    }
}

TEST_CASE("angle - Construcción desde radianes", "[angle]") {
    angle from_r = angle::from_rad(M_PI);
    REQUIRE(near(from_r.value(), 180.0));
}

TEST_CASE("angle - Diferencia angular", "[angle]") {
    REQUIRE(near(angle(0.0).diff(angle(90.0)), 90.0));
    REQUIRE(near(angle(0.0).diff(angle(270.0)), 90.0));
}

TEST_CASE("angle - Vector dirección", "[angle]") {
    SECTION("0 grados") {
        vector2 d = angle(0.0).direction();
        REQUIRE(near(d.x, 0.0));
        REQUIRE(near(d.y, 1.0));
    }

    SECTION("90 grados") {
        vector2 d90 = angle(90.0).direction();
        REQUIRE(near(d90.x, 1.0));
        REQUIRE(near(d90.y, 0.0));
    }
}
