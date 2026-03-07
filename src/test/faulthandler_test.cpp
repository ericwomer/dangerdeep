/*
 * Test para faulthandler.h: install_segfault_handler (no provocar crash).
 */
#include "catch_amalgamated.hpp"
#include "../faulthandler.h"

TEST_CASE("faulthandler - install_segfault_handler", "[faulthandler]") {
    install_segfault_handler();
    REQUIRE(true);
}
