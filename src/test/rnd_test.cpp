/*
 * Test para rnd() y seed_global_rnd() (generador global, std::mt19937).
 * Cubre: rango [0,1), reproducibilidad con semilla, rnd(unsigned) en [0,b-1].
 */
#include "catch_amalgamated.hpp"
#include "../rnd.h"

TEST_CASE("rnd() - valores en [0, 1)", "[rnd]") {
    seed_global_rnd(42u);
    for (int i = 0; i < 100; ++i) {
        double v = rnd();
        REQUIRE(v >= 0.0);
        REQUIRE(v < 1.0);
    }
}

TEST_CASE("rnd() - reproducibilidad con semilla", "[rnd]") {
    seed_global_rnd(12345u);
    double a = rnd();
    double b = rnd();
    double c = rnd();

    seed_global_rnd(12345u);
    REQUIRE(rnd() == a);
    REQUIRE(rnd() == b);
    REQUIRE(rnd() == c);
}

TEST_CASE("rnd(unsigned) - rnd(0) devuelve 0", "[rnd]") {
    seed_global_rnd(999u);
    REQUIRE(rnd(0u) == 0u);
}

TEST_CASE("rnd(unsigned) - rnd(1) siempre 0", "[rnd]") {
    seed_global_rnd(111u);
    for (int i = 0; i < 20; ++i) {
        REQUIRE(rnd(1u) == 0u);
    }
}

TEST_CASE("rnd(unsigned) - rango [0, b-1] para varios b", "[rnd]") {
    seed_global_rnd(777u);
    for (unsigned b : {2u, 10u, 100u, 256u, 1000u}) {
        for (int i = 0; i < 50; ++i) {
            unsigned v = rnd(b);
            REQUIRE(v < b);
        }
    }
}
