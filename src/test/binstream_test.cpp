/*
 * Test para binstream.h: roundtrip write/read con stringstream.
 */
#include "catch_amalgamated.hpp"
#include "../binstream.h"
#include <cmath>
#include <sstream>

static bool near_f(float a, float b, float eps = 1e-5f) {
    return std::fabs(a - b) < eps;
}
static bool near_d(double a, double b, double eps = 1e-12) {
    return std::fabs(a - b) < eps;
}

TEST_CASE("binstream - tipos primitivos roundtrip", "[binstream]") {
    std::stringstream buf;

    write_i32(buf, -12345);
    write_u32(buf, 40000u);
    write_float(buf, 3.14f);
    write_double(buf, -2.5);
    write_bool(buf, true);
    write_bool(buf, false);
    write_string(buf, "hello");
    write_string(buf, "");

    buf.seekg(0);
    REQUIRE(read_i32(buf) == -12345);
    REQUIRE(read_u32(buf) == 40000u);
    REQUIRE(near_f(read_float(buf), 3.14f));
    REQUIRE(near_d(read_double(buf), -2.5));
    REQUIRE(read_bool(buf) == true);
    REQUIRE(read_bool(buf) == false);
    REQUIRE(read_string(buf) == "hello");
    REQUIRE(read_string(buf).empty());
}

TEST_CASE("binstream - vector2 roundtrip", "[binstream]") {
    std::stringstream buf2;
    vector2 v2(1.0, 2.0);
    write_vector2(buf2, v2);
    buf2.seekg(0);
    vector2 v2r = read_vector2(buf2);
    REQUIRE(near_d(v2r.x, 1.0));
    REQUIRE(near_d(v2r.y, 2.0));
}
