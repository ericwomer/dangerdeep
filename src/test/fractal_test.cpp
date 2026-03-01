/*
 * Test para fractal.h: fractal_noise constructor.
 */
#include "../fractal.h"
#include <cassert>
#include <cstdio>

int main() {
    fractal_noise fn(0.5, 2.0, 4, 0.0, 1.0);
    double v = fn.get_value_hybrid(vector3(0.1, 0.2, 0.3), 2);
    assert(std::isfinite(v));
    printf("fractal_test ok\n");
    return 0;
}
