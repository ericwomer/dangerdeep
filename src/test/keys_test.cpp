/*
 * Test para keys: key_names tiene NR_OF_KEY_IDS entradas y cada .nr coincide.
 */
#include "catch_amalgamated.hpp"
#include "../keys.h"

TEST_CASE("keys - key_names estructura", "[keys]") {
    for (unsigned i = 0; i < NR_OF_KEY_IDS; ++i) {
        REQUIRE(key_names[i].nr == i);
        REQUIRE(key_names[i].name != nullptr);
        REQUIRE(key_names[i].name[0] != '\0');
    }
}
