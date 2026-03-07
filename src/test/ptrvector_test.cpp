/*
 * Test para ptrvector (vector de punteros con ownership).
 */
#include "catch_amalgamated.hpp"
#include "../ptrvector.h"
#include <memory>

struct Item {
    int id;
    Item(int i) : id(i) {}
};

TEST_CASE("ptrvector - vacío", "[ptrvector]") {
    ptrvector<Item> v;
    REQUIRE(v.size() == 0);
    REQUIRE(v.empty());
}

TEST_CASE("ptrvector - push_back raw y unique_ptr", "[ptrvector]") {
    ptrvector<Item> v;
    v.push_back(new Item(1));
    v.push_back(new Item(2));
    v.push_back(std::make_unique<Item>(3));
    REQUIRE(v.size() == 3);
    REQUIRE_FALSE(v.empty());
    REQUIRE((*v[0]).id == 1);
    REQUIRE((*v[1]).id == 2);
    REQUIRE((*v[2]).id == 3);
}

TEST_CASE("ptrvector - resize", "[ptrvector]") {
    ptrvector<Item> v;
    v.push_back(new Item(1));
    v.push_back(new Item(2));
    v.push_back(std::make_unique<Item>(3));
    v.resize(2);
    REQUIRE(v.size() == 2);
    REQUIRE((*v[0]).id == 1);
    REQUIRE((*v[1]).id == 2);
}

TEST_CASE("ptrvector - clear", "[ptrvector]") {
    ptrvector<Item> v;
    v.push_back(new Item(1));
    v.clear();
    REQUIRE(v.size() == 0);
    REQUIRE(v.empty());
}
