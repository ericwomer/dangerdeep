/*
 * Test para bivector (matriz 2D / array 2D).
 */
#include "catch_amalgamated.hpp"
#include "../bivector.h"
#include "../vector2.h"

TEST_CASE("bivector - size y at", "[bivector]") {
    bivector<int> b(vector2i(3, 2), 0);
    REQUIRE(b.size().x == 3);
    REQUIRE(b.size().y == 2);
    REQUIRE(b.at(0, 0) == 0);
    b.at(1, 1) = 42;
    REQUIRE(b.at(1, 1) == 42);
    REQUIRE(b[vector2i(1, 1)] == 42);
}

TEST_CASE("bivector - get_min y get_max", "[bivector]") {
    bivector<int> b(vector2i(3, 2), 0);
    b.at(0, 0) = 1;
    b.at(1, 0) = 2;
    b.at(2, 0) = 3;
    b.at(0, 1) = 4;
    b.at(1, 1) = 5;
    b.at(2, 1) = 6;
    REQUIRE(b.get_min() == 1);
    REQUIRE(b.get_max() == 6);
}

TEST_CASE("bivector - resize", "[bivector]") {
    bivector<int> b2(vector2i(2, 2), 10);
    b2.resize(vector2i(2, 2), 10);
    REQUIRE(b2.size().x == 2);
    REQUIRE(b2.size().y == 2);
    b2.at(0, 0) = 7;
    b2.at(1, 0) = 8;
    b2.at(0, 1) = 9;
    b2.at(1, 1) = 10;
    REQUIRE(b2.get_min() == 7);
    REQUIRE(b2.get_max() == 10);
}

TEST_CASE("bivector - at fuera de rango lanza", "[bivector]") {
    bivector<int> b(vector2i(3, 2), 0);
    REQUIRE_THROWS_AS((void)b.at(10, 10), std::out_of_range);
}
