/*
 * Test para dmath.h: isfinite y compatibilidad entre plataformas.
 */
#include "catch_amalgamated.hpp"
#include "../dmath.h"
#include <cmath>
#include <limits>

TEST_CASE("dmath - isfinite valores finitos", "[dmath]") {
    REQUIRE(isfinite(0.0));
    REQUIRE(isfinite(1.0));
    REQUIRE(isfinite(-1.5));
    REQUIRE(isfinite(1e308));
}

TEST_CASE("dmath - isfinite infinito", "[dmath]") {
    REQUIRE_FALSE(isfinite(std::numeric_limits<double>::infinity()));
    REQUIRE_FALSE(isfinite(-std::numeric_limits<double>::infinity()));
}

TEST_CASE("dmath - isfinite NaN", "[dmath]") {
    REQUIRE_FALSE(isfinite(std::numeric_limits<double>::quiet_NaN()));
}

TEST_CASE("dmath - isfinite float", "[dmath]") {
    float f = 1.0f;
    REQUIRE(isfinite(f));
    REQUIRE_FALSE(isfinite(std::numeric_limits<float>::quiet_NaN()));
}
