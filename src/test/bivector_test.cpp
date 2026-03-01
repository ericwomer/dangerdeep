/*
 * Test mínimo para bivector (matriz 2D / array 2D).
 */
#include "../bivector.h"
#include "../vector2.h"
#include <cassert>
#include <cstdio>

int main() {
    bivector<int> b(vector2i(3, 2), 0);
    assert(b.size().x == 3 && b.size().y == 2);
    assert(b.at(0, 0) == 0);
    b.at(1, 1) = 42;
    assert(b.at(1, 1) == 42);
    assert(b[vector2i(1, 1)] == 42);

    b.at(0, 0) = 1;
    b.at(1, 0) = 2;
    b.at(2, 0) = 3;
    b.at(0, 1) = 4;
    b.at(1, 1) = 5;
    b.at(2, 1) = 6;
    assert(b.get_min() == 1);
    assert(b.get_max() == 6);

    bivector<int> b2(vector2i(2, 2), 10);
    b2.resize(vector2i(2, 2), 10);
    assert(b2.size().x == 2 && b2.size().y == 2);
    b2.at(0, 0) = 7;
    b2.at(1, 0) = 8;
    b2.at(0, 1) = 9;
    b2.at(1, 1) = 10;
    assert(b2.get_min() == 7 && b2.get_max() == 10);

    bool threw = false;
    try {
        (void)b.at(10, 10);
    } catch (const std::out_of_range &) {
        threw = true;
    }
    assert(threw);

    printf("bivector_test ok\n");
    return 0;
}
