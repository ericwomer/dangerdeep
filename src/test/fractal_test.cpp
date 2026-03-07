/*
 * Test para fractal.h: fractal_noise constructor.
 */
#include "catch_amalgamated.hpp"
#include "../fractal.h"
#include <cmath>

TEST_CASE("fractal_noise - get_value_hybrid finito", "[fractal]") {
    fractal_noise fn(0.5, 2.0, 4, 0.0, 1.0);
    double v = fn.get_value_hybrid(vector3(0.1, 0.2, 0.3), 2);
    REQUIRE(std::isfinite(v));
}
