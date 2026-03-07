/*
 * Test para perlinnoise.h/cpp: noise_func, perlinnoise, generate, value, etc.
 */
#include "catch_amalgamated.hpp"
#include "../perlinnoise.h"
#include "../rnd.h"
#include <cmath>
#include <vector>

static void seed_for_reproducibility() {
    seed_global_rnd(12345u);
}

TEST_CASE("perlinnoise::noise_func - constructor y estructura", "[perlinnoise]") {
    seed_for_reproducibility();
    perlinnoise::noise_func nf(4, 1, 0.0f, 0.0f);
    REQUIRE(nf.size == 4);
    REQUIRE(nf.frequency == 1);
    REQUIRE(nf.data.size() == 16u);
}

TEST_CASE("perlinnoise - constructor válido", "[perlinnoise]") {
    seed_for_reproducibility();
    perlinnoise pn(8, 2, 8);
    REQUIRE(pn.get_number_of_levels() >= 1);
}

TEST_CASE("perlinnoise - constructor inválido size no potencia de 2", "[perlinnoise]") {
    seed_for_reproducibility();
    REQUIRE_THROWS_AS(perlinnoise(7, 2, 8), std::invalid_argument);
    REQUIRE_THROWS_AS(perlinnoise(8, 3, 8), std::invalid_argument);
    REQUIRE_THROWS_AS(perlinnoise(8, 2, 9), std::invalid_argument);
}

TEST_CASE("perlinnoise - generate y generate_sqr", "[perlinnoise]") {
    seed_for_reproducibility();
    perlinnoise pn(8, 2, 8);
    std::vector<Uint8> gen = pn.generate();
    REQUIRE(gen.size() == 64u);
    for (size_t i = 0; i < gen.size(); ++i) {
        REQUIRE(gen[i] <= 255);
    }

    std::vector<Uint8> gen_sqr = pn.generate_sqr();
    REQUIRE(gen_sqr.size() == 64u);
}

TEST_CASE("perlinnoise - set_phase", "[perlinnoise]") {
    seed_for_reproducibility();
    perlinnoise pn(8, 2, 8);
    pn.set_phase(0, 0.5f, 0.3f);
    std::vector<Uint8> gen = pn.generate();
    REQUIRE(!gen.empty());
}

TEST_CASE("perlinnoise - value y valuef", "[perlinnoise]") {
    seed_for_reproducibility();
    perlinnoise pn(16, 2, 16);
    Uint8 v = pn.value(0, 0);
    REQUIRE(v <= 255);
    float vf = pn.valuef(0, 0);
    REQUIRE(std::isfinite(vf));  // valuef devuelve suma raw, no 0-1
}

TEST_CASE("perlinnoise - values y valuesf", "[perlinnoise]") {
    seed_for_reproducibility();
    perlinnoise pn(16, 2, 16);
    std::vector<Uint8> vals = pn.values(0, 0, 4, 4);
    REQUIRE(vals.size() == 16u);
    std::vector<float> valsf = pn.valuesf(0, 0, 4, 4);
    REQUIRE(valsf.size() == 16u);
}

TEST_CASE("perlinnoise - constructor large (levelsize, sizeminfreq, levels, dummy)", "[perlinnoise]") {
    seed_for_reproducibility();
    perlinnoise pn(8, 2, 2, true);
    REQUIRE(pn.get_number_of_levels() == 2);
    Uint8 v = pn.value(0, 0);
    REQUIRE(v <= 255);
}

TEST_CASE("perlinnoise - constructor large inválido", "[perlinnoise]") {
    seed_for_reproducibility();
    REQUIRE_THROWS_AS(perlinnoise(7, 2, 2, true), std::invalid_argument);
    REQUIRE_THROWS_AS(perlinnoise(8, 3, 2, true), std::invalid_argument);
    REQUIRE_THROWS_AS(perlinnoise(8, 2, 0, true), std::invalid_argument);
}
