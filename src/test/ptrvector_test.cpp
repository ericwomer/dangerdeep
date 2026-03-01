/*
 * Test mínimo para ptrvector (vector de punteros con ownership).
 */
#include "../ptrvector.h"
#include <cassert>
#include <memory>
#include <cstdio>

struct Item {
    int id;
    Item(int i) : id(i) {}
};

int main() {
    ptrvector<Item> v;
    assert(v.size() == 0);
    assert(v.empty());

    v.push_back(new Item(1));
    v.push_back(new Item(2));
    v.push_back(std::make_unique<Item>(3));
    assert(v.size() == 3);
    assert(!v.empty());
    assert((*v[0]).id == 1);
    assert((*v[1]).id == 2);
    assert((*v[2]).id == 3);

    v.resize(2);
    assert(v.size() == 2);
    assert((*v[0]).id == 1);
    assert((*v[1]).id == 2);

    v.clear();
    assert(v.size() == 0);
    assert(v.empty());

    printf("ptrvector_test ok\n");
    return 0;
}
