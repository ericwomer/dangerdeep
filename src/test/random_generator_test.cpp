/*
 * Test para random_generator.
 */
#include "catch_amalgamated.hpp"
#include "../random_generator.h"

TEST_CASE("random_generator - rnd produce valores distintos", "[random_generator]") {
    random_generator rng(12345);
    unsigned a = rng.rnd();
    unsigned b = rng.rnd();
    REQUIRE((a != b || a == 0));
}

TEST_CASE("random_generator - set_seed reproduce secuencia", "[random_generator]") {
    random_generator rng(12345);
    unsigned a = rng.rnd();
    rng.set_seed(12345);
    unsigned a2 = rng.rnd();
    REQUIRE(a2 == a);
}

TEST_CASE("random_generator - rndf en [0,1]", "[random_generator]") {
    random_generator rng(12345);
    float f = rng.rndf();
    REQUIRE(f >= 0.0f);
    REQUIRE(f <= 1.0f);
}

TEST_CASE("random_generator - constructor con seed 0", "[random_generator]") {
    random_generator rng2(0);
    (void)rng2.rnd();
}
