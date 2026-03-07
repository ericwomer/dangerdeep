/*
 * Test para global_constants.h: constantes físicas y geodésicas.
 */
#include "catch_amalgamated.hpp"
#include "../global_constants.h"
#include <cmath>

static bool near(double a, double b, double eps = 1e-6) {
    return std::fabs(a - b) <= eps;
}

TEST_CASE("global_constants - gravedad y geodésicas", "[global_constants]") {
    REQUIRE(near(GRAVITY, 9.806));
    REQUIRE(near(DEGREE_IN_METERS, 111194.9266388889));
    REQUIRE(near(MINUTE_IN_METERS, 1853.248777315));
    REQUIRE(SECOND_IN_METERS > 30);
    REQUIRE(SECOND_IN_METERS < 31);
    REQUIRE(near(EARTH_PERIMETER, 40030173.59));
}

TEST_CASE("global_constants - volúmenes y WGS84", "[global_constants]") {
    REQUIRE(BRT_VOLUME > 2.8);
    REQUIRE(BRT_VOLUME < 2.9);
    REQUIRE(near(WGS84_A, 6378137.0));
    REQUIRE(near(WGS84_B, 6356752.314));
    REQUIRE(WGS84_K > 0);
    REQUIRE(WGS84_K < 1);
    REQUIRE(near(EARTH_RADIUS, 6371000.785));
}

TEST_CASE("global_constants - radios y distancias", "[global_constants]") {
    REQUIRE(SUN_RADIUS > 695e6);
    REQUIRE(SUN_RADIUS < 697e6);
    REQUIRE(MOON_RADIUS > 1.7e6);
    REQUIRE(MOON_RADIUS < 1.8e6);
    REQUIRE(EARTH_SUN_DISTANCE > 149e9);
    REQUIRE(EARTH_SUN_DISTANCE < 150e9);
}

TEST_CASE("global_constants - ángulos y tiempos", "[global_constants]") {
    REQUIRE(near(EARTH_ROT_AXIS_ANGLE, 23.45));
    REQUIRE(EARTH_ROTATION_TIME > 86000);
    REQUIRE(EARTH_ROTATION_TIME < 87000);
    REQUIRE(EARTH_ORBIT_TIME > 31e6);
    REQUIRE(EARTH_ORBIT_TIME < 32e6);
    REQUIRE(near(MOON_POS_ADJUST, 300.0));
}
