/*
 * Test para bspline.h: bsplinet (B-spline uniform).
 */
#include "catch_amalgamated.hpp"
#include "../bspline.h"
#include "../vector2.h"
#include <cmath>
#include <vector>

static bool near(double a, double b, double eps = 1e-9) {
    return std::fabs(a - b) <= eps;
}

TEST_CASE("bspline - control_points y value double", "[bspline]") {
    std::vector<double> cp = {0.0, 1.0, 2.0, 3.0};
    bsplinet<double> spl(1, cp);
    REQUIRE(spl.control_points().size() == 4);
    double v0 = spl.value(0.0);
    double v1 = spl.value(1.0);
    REQUIRE(near(v0, 0.0));
    REQUIRE(near(v1, 3.0));
    double v05 = spl.value(0.5);
    REQUIRE(v05 >= 0.0);
    REQUIRE(v05 <= 3.0);
}

TEST_CASE("bspline - value vector2", "[bspline]") {
    std::vector<vector2> cp2 = {vector2(0, 0), vector2(1, 0), vector2(1, 1), vector2(0, 1)};
    bsplinet<vector2> spl2(1, cp2);
    vector2 p0 = spl2.value(0.0);
    vector2 p1 = spl2.value(1.0);
    REQUIRE(near(p0.x, 0.0));
    REQUIRE(near(p0.y, 0.0));
    REQUIRE(near(p1.x, 0.0));
    REQUIRE(near(p1.y, 1.0));
}

TEST_CASE("bspline - pocos puntos lanza runtime_error", "[bspline]") {
    std::vector<double> bad = {1.0};
    REQUIRE_THROWS_AS(bsplinet<double>(2, bad), std::runtime_error);
}
