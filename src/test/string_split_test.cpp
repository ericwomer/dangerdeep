/*
 * Test para string_split. Comprueba el comportamiento para poder
 * implementar con std::getline sin romper usos (regiones, países, etc.).
 */
#include "catch_amalgamated.hpp"
#include "../string_split.h"
#include <list>
#include <string>

TEST_CASE("string_split - múltiples elementos", "[string_split]") {
    std::list<std::string> r = string_split("a,b,c", ',');
    REQUIRE(r.size() == 3);
    REQUIRE(*r.begin() == "a");
    REQUIRE(*(++r.begin()) == "b");
    REQUIRE(*r.rbegin() == "c");
}

TEST_CASE("string_split - un solo elemento", "[string_split]") {
    std::list<std::string> r = string_split("a", ',');
    REQUIRE(r.size() == 1);
    REQUIRE(*r.begin() == "a");
}

TEST_CASE("string_split - trailing separator", "[string_split]") {
    std::list<std::string> r = string_split("a,", ',');
    REQUIRE(r.size() == 2);
    REQUIRE(*r.begin() == "a");
    REQUIRE(*r.rbegin() == "");
}

TEST_CASE("string_split - leading separator", "[string_split]") {
    std::list<std::string> r = string_split(",b", ',');
    REQUIRE(r.size() == 2);
    REQUIRE(*r.begin() == "");
    REQUIRE(*r.rbegin() == "b");
}

TEST_CASE("string_split - string vacía", "[string_split]") {
    std::list<std::string> r = string_split("", ',');
    REQUIRE(r.size() == 1);
    REQUIRE(*r.begin() == "");
}

TEST_CASE("string_split - separador distinto", "[string_split]") {
    std::list<std::string> r = string_split("one;two;three", ';');
    REQUIRE(r.size() == 3);
    REQUIRE(*r.begin() == "one");
    REQUIRE(*r.rbegin() == "three");
}
