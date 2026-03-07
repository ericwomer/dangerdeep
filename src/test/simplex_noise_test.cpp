/*
 * Test para simplex_noise.h/cpp: noise() 2D/3D (valor en rango).
 */
#include "catch_amalgamated.hpp"
#include "../simplex_noise.h"
#include <cmath>

TEST_CASE("simplex_noise - noise 2D finito", "[simplex_noise]") {
    double v2 = simplex_noise::noise(vector2(0.1, 0.2));
    REQUIRE(std::isfinite(v2));
}

TEST_CASE("simplex_noise - noise 3D finito", "[simplex_noise]") {
    double v3 = simplex_noise::noise(vector3(0.1, 0.2, 0.3));
    REQUIRE(std::isfinite(v3));
}

TEST_CASE("simplex_noise - noise_map2D tamaño", "[simplex_noise]") {
    std::vector<Uint8> map = simplex_noise::noise_map2D(vector2i(8, 8), 1, 1.0f, 0.01f);
    REQUIRE(map.size() == 64u);
}
