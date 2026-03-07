/*
 * Test para align16_allocator: tipo, operator==/!= y max_size.
 */
#include "catch_amalgamated.hpp"
#include "../align16_allocator.h"
#include <cstddef>

TEST_CASE("align16_allocator - operator==", "[align16_allocator]") {
    align16_allocator<char> a1, a2;
    REQUIRE(a1 == a2);
    REQUIRE_FALSE(a1 != a2);
}

TEST_CASE("align16_allocator - max_size int", "[align16_allocator]") {
    align16_allocator<int> ai;
    REQUIRE(ai.max_size() == size_t(-1) / sizeof(int));
}

TEST_CASE("align16_allocator - max_size double", "[align16_allocator]") {
    align16_allocator<double> ad;
    REQUIRE(ad.max_size() == size_t(-1) / sizeof(double));
}
