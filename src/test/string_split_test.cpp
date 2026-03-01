/*
 * Test para string_split. Comprueba el comportamiento para poder
 * implementar con std::getline sin romper usos (regiones, países, etc.).
 */
#include "../string_split.h"
#include <cassert>
#include <cstdio>

int main() {
    std::list<std::string> r;

    r = string_split("a,b,c", ',');
    assert(r.size() == 3);
    assert(*r.begin() == "a");
    assert(*(++r.begin()) == "b");
    assert(*r.rbegin() == "c");

    r = string_split("a", ',');
    assert(r.size() == 1);
    assert(*r.begin() == "a");

    r = string_split("a,", ',');
    assert(r.size() == 2);
    assert(*r.begin() == "a");
    assert(*r.rbegin() == "");

    r = string_split(",b", ',');
    assert(r.size() == 2);
    assert(*r.begin() == "");
    assert(*r.rbegin() == "b");

    r = string_split("", ',');
    assert(r.size() == 1);
    assert(*r.begin() == "");

    r = string_split("one;two;three", ';');
    assert(r.size() == 3);
    assert(*r.begin() == "one");
    assert(*r.rbegin() == "three");

    printf("string_split_test ok\n");
    return 0;
}
