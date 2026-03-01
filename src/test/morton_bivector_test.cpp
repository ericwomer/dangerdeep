/*
 * Test para morton_bivector.h: resize, at, size, operator[].
 */
#include <cmath>
#include "../morton_bivector.h"
#include <cassert>
#include <cmath>
#include <cstdio>

int main() {
    morton_bivector<int> mb(2, 0);
    assert(mb.size() == 2);
    mb.at(0, 0) = 1;
    mb.at(1, 0) = 2;
    mb.at(0, 1) = 3;
    mb.at(1, 1) = 4;
    assert(mb.at(0, 0) == 1 && mb.at(1, 1) == 4);
    assert(mb[vector2i(1, 0)] == 2);

    morton_bivector<double> mb2(4, 1.0);
    assert(mb2.size() == 4);
    assert(std::fabs(mb2.at(0, 0) - 1.0) < 1e-9);

    printf("morton_bivector_test ok\n");
    return 0;
}
