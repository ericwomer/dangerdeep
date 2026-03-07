/*
 * Test para cfg.h: struct cfg::key (constructor, operator=).
 */
#include "catch_amalgamated.hpp"
#include "../cfg.h"

TEST_CASE("cfg::key - Constructor por defecto", "[cfg_key]") {
    cfg::key k1;
    REQUIRE(k1.keysym == 0);
    REQUIRE_FALSE(k1.ctrl);
    REQUIRE_FALSE(k1.alt);
    REQUIRE_FALSE(k1.shift);
}

TEST_CASE("cfg::key - Constructor con parámetros", "[cfg_key]") {
    cfg::key k2("action", key_code('a'), true, false, true);
    REQUIRE(k2.action == "action");
    REQUIRE(k2.keysym == key_code('a'));
    REQUIRE(k2.ctrl);
    REQUIRE_FALSE(k2.alt);
    REQUIRE(k2.shift);
}

TEST_CASE("cfg::key - operator=", "[cfg_key]") {
    cfg::key k1;
    cfg::key k2("action", key_code('a'), true, false, true);
    k1 = k2;
    REQUIRE(k1.action == k2.action);
    REQUIRE(k1.keysym == k2.keysym);
}
