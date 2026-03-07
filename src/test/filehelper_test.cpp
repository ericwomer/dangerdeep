/*
 * Test para filehelper.h/cpp: get_current_directory, is_directory.
 */
#include "catch_amalgamated.hpp"
#include "../filehelper.h"
#include <string>

TEST_CASE("filehelper - get_current_directory e is_directory", "[filehelper]") {
    std::string cwd = get_current_directory();
    REQUIRE_FALSE(cwd.empty());
    REQUIRE(is_directory("."));
    REQUIRE(is_directory(".."));
}
