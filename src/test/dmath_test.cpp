/*
 * Test para dmath.h: isfinite y compatibilidad entre plataformas.
 */
#include <cmath>
#include <limits>
#include "../dmath.h"
#include <cassert>
#include <cstdio>

int main() {
    assert(isfinite(0.0));
    assert(isfinite(1.0));
    assert(isfinite(-1.5));
    assert(isfinite(1e308));

    assert(!isfinite(std::numeric_limits<double>::infinity()));
    assert(!isfinite(-std::numeric_limits<double>::infinity()));
    assert(!isfinite(std::numeric_limits<double>::quiet_NaN()));

    float f = 1.0f;
    assert(isfinite(f));
    assert(!isfinite(std::numeric_limits<float>::quiet_NaN()));

    printf("dmath_test ok\n");
    return 0;
}
