/*
 * Test para align16_allocator: tipo, operator==/!= y max_size.
 * Nota: allocate/deallocate asumen 4 bytes de encabezado (diseño 32-bit);
 * en 64-bit puede fallar, por eso no probamos vector con este allocator.
 */
#include "../align16_allocator.h"
#include <cassert>
#include <cstddef>
#include <cstdio>
#include <new>

int main() {
    align16_allocator<char> a1, a2;
    assert(a1 == a2);
    assert(!(a1 != a2));

    align16_allocator<int> ai;
    assert(ai.max_size() == size_t(-1) / sizeof(int));

    align16_allocator<double> ad;
    assert(ad.max_size() == size_t(-1) / sizeof(double));

    printf("align16_allocator_test ok\n");
    return 0;
}
