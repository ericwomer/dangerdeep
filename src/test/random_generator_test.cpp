/*
 * Test mínimo para random_generator.
 */
#include "../random_generator.h"
#include <cassert>
#include <cstdio>

int main() {
    random_generator rng(12345);
    unsigned a = rng.rnd();
    unsigned b = rng.rnd();
    assert(a != b || a == 0);

    rng.set_seed(12345);
    unsigned a2 = rng.rnd();
    assert(a2 == a);

    float f = rng.rndf();
    assert(f >= 0.0f && f <= 1.0f);

    random_generator rng2(0);
    (void)rng2.rnd();
    printf("random_generator_test ok\n");
    return 0;
}
