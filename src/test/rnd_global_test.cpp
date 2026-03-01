/*
 * Test para rnd() y seed_global_rnd() (generador global, ya usa std::mt19937).
 * Fijamos semilla y comprobamos que la secuencia es determinista y en [0,1).
 */
#include "../rnd.h"
#include <cassert>
#include <cstdio>

int main() {
    seed_global_rnd(12345u);
    double a = rnd();
    double b = rnd();
    assert(a >= 0.0 && a < 1.0);
    assert(b >= 0.0 && b < 1.0);

    seed_global_rnd(12345u);
    double a2 = rnd();
    assert(a == a2);

    assert(rnd(0u) == 0u);
    unsigned u = rnd(10u);
    assert(u < 10u);

    printf("rnd_global_test ok\n");
    return 0;
}
