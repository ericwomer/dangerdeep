/*
 * Tests para utilidades de global_data.h: ulog2, nextgteqpow2, ispow2.
 * Son inline en el header; el test verifica el comportamiento para poder
 * sustituir por std::bit_* en C++20 sin romper nada.
 */
#include "../global_data.h"
#include <cassert>
#include <cstdio>

int main() {
    // ulog2: floor(log2(x)), ulog2(0) es undefined (bucle)
    assert(ulog2(1u) == 0);
    assert(ulog2(2u) == 1);
    assert(ulog2(3u) == 1);
    assert(ulog2(4u) == 2);
    assert(ulog2(5u) == 2);
    assert(ulog2(8u) == 3);
    assert(ulog2(255u) == 7);
    assert(ulog2(256u) == 8);

    // nextgteqpow2: menor potencia de 2 >= x
    assert(nextgteqpow2(0u) == 1);
    assert(nextgteqpow2(1u) == 1);
    assert(nextgteqpow2(2u) == 2);
    assert(nextgteqpow2(3u) == 4);
    assert(nextgteqpow2(4u) == 4);
    assert(nextgteqpow2(5u) == 8);
    assert(nextgteqpow2(8u) == 8);
    assert(nextgteqpow2(9u) == 16);

    // ispow2: true si x es potencia de 2 (no probamos 0: impl actual 0&(0-1)==0)
    assert(ispow2(1u));
    assert(ispow2(2u));
    assert(!ispow2(3u));
    assert(ispow2(4u));
    assert(!ispow2(5u));
    assert(ispow2(8u));
    assert(!ispow2(9u));

    printf("global_data_utils_test ok\n");
    return 0;
}
