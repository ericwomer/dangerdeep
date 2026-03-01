/*
 * Test mínimo para fixed32 (punto fijo 32 bit).
 * fixed32(Sint32 n) almacena n en crudo (valor = n/65536); fixed32(float) usa 65536 por unidad.
 */
#include "../fixed.h"
#include <cassert>
#include <cstdio>

int main() {
    fixed32 zero;
    fixed32 one = fixed32::one();
    assert(zero < one);
    assert(one == fixed32(1.0f));
    assert(fixed32(2.0f) + fixed32(3.0f) == fixed32(5.0f));
    assert(fixed32(5.0f) - fixed32(2.0f) == fixed32(3.0f));
    assert(fixed32(2.0f) * fixed32(3.0f) == fixed32(6.0f));
    assert(fixed32(6.0f) / fixed32(2.0f) == fixed32(3.0f));

    assert(fixed32(7.0f).intpart() == 7);
    assert(fixed32(7.0f).round() == 7);
    fixed32 two(2.0f);
    assert((two * 3).intpart() == 6);
    assert((fixed32(10.0f) / 2).intpart() == 5);

    fixed32 half(0.5f);
    assert(half.round() == 1 || half.round() == 0);
    assert(half.intpart() == 0);
    fixed32 neg = -fixed32(5.0f);
    assert(neg + fixed32(5.0f) == zero);

    assert(fixed32(3.0f) <= fixed32(5.0f));
    assert(fixed32(3.0f) < fixed32(5.0f));
    assert(fixed32(5.0f) >= fixed32(3.0f));
    assert(fixed32(5.0f) > fixed32(3.0f));
    assert(fixed32(4.0f) != fixed32(5.0f));

    printf("fixed_test ok\n");
    return 0;
}
