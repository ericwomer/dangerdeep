/*
 * Test para fixed32 (punto fijo 32 bit).
 */
#include "catch_amalgamated.hpp"
#include "../fixed.h"

TEST_CASE("fixed32 - Operadores básicos", "[fixed]") {
    fixed32 zero;
    fixed32 one = fixed32::one();
    REQUIRE(zero < one);
    REQUIRE(one == fixed32(1.0f));
    REQUIRE(fixed32(2.0f) + fixed32(3.0f) == fixed32(5.0f));
    REQUIRE(fixed32(5.0f) - fixed32(2.0f) == fixed32(3.0f));
    REQUIRE(fixed32(2.0f) * fixed32(3.0f) == fixed32(6.0f));
    REQUIRE(fixed32(6.0f) / fixed32(2.0f) == fixed32(3.0f));
}

TEST_CASE("fixed32 - intpart y round", "[fixed]") {
    REQUIRE(fixed32(7.0f).intpart() == 7);
    REQUIRE(fixed32(7.0f).round() == 7);
    fixed32 two(2.0f);
    REQUIRE((two * 3).intpart() == 6);
    REQUIRE((fixed32(10.0f) / 2).intpart() == 5);

    fixed32 half(0.5f);
    REQUIRE((half.round() == 1 || half.round() == 0));
    REQUIRE(half.intpart() == 0);
}

TEST_CASE("fixed32 - Negativo y cero", "[fixed]") {
    fixed32 zero;
    fixed32 neg = -fixed32(5.0f);
    REQUIRE(neg + fixed32(5.0f) == zero);
}

TEST_CASE("fixed32 - Operadores de comparación", "[fixed]") {
    REQUIRE(fixed32(3.0f) <= fixed32(5.0f));
    REQUIRE(fixed32(3.0f) < fixed32(5.0f));
    REQUIRE(fixed32(5.0f) >= fixed32(3.0f));
    REQUIRE(fixed32(5.0f) > fixed32(3.0f));
    REQUIRE(fixed32(4.0f) != fixed32(5.0f));
}
