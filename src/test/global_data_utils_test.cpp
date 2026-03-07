/*
 * Test para utilidades de global_data.h: ulog2, nextgteqpow2, ispow2.
 */
#include "catch_amalgamated.hpp"
#include "../global_data.h"

TEST_CASE("global_data - ulog2", "[global_data_utils]") {
    REQUIRE(ulog2(1u) == 0);
    REQUIRE(ulog2(2u) == 1);
    REQUIRE(ulog2(3u) == 1);
    REQUIRE(ulog2(4u) == 2);
    REQUIRE(ulog2(5u) == 2);
    REQUIRE(ulog2(8u) == 3);
    REQUIRE(ulog2(255u) == 7);
    REQUIRE(ulog2(256u) == 8);
}

TEST_CASE("global_data - nextgteqpow2", "[global_data_utils]") {
    REQUIRE(nextgteqpow2(0u) == 1);
    REQUIRE(nextgteqpow2(1u) == 1);
    REQUIRE(nextgteqpow2(2u) == 2);
    REQUIRE(nextgteqpow2(3u) == 4);
    REQUIRE(nextgteqpow2(4u) == 4);
    REQUIRE(nextgteqpow2(5u) == 8);
    REQUIRE(nextgteqpow2(8u) == 8);
    REQUIRE(nextgteqpow2(9u) == 16);
}

TEST_CASE("global_data - ispow2", "[global_data_utils]") {
    REQUIRE(ispow2(1u));
    REQUIRE(ispow2(2u));
    REQUIRE_FALSE(ispow2(3u));
    REQUIRE(ispow2(4u));
    REQUIRE_FALSE(ispow2(5u));
    REQUIRE(ispow2(8u));
    REQUIRE_FALSE(ispow2(9u));
}
