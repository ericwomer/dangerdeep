/*
 * Test para simplex_noise.h/cpp: noise() 2D/3D (valor en rango).
 */
#include "../simplex_noise.h"
#include <cassert>
#include <cmath>
#include <cstdio>

int main() {
    double v2 = simplex_noise::noise(vector2(0.1, 0.2));
    assert(std::isfinite(v2));
    double v3 = simplex_noise::noise(vector3(0.1, 0.2, 0.3));
    assert(std::isfinite(v3));
    std::vector<Uint8> map = simplex_noise::noise_map2D(vector2i(8, 8), 1, 1.0f, 0.01f);
    assert(map.size() == 64u);
    printf("simplex_noise_test ok\n");
    return 0;
}
