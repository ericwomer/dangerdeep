/*
 * Test para str() y str_wf() de global_data.h.
 * Cubre conversión a string antes de cambios en atoi/atof/stoi/stod en otros módulos.
 */
#include "catch_amalgamated.hpp"
#include "../global_data.h"

TEST_CASE("str - enteros", "[str_utils]") {
    REQUIRE(str(0) == "0");
    REQUIRE(str(42) == "42");
    REQUIRE(str(-1) == "-1");
}

TEST_CASE("str - flotantes", "[str_utils]") {
    REQUIRE(str(3.14) == "3.14");
    REQUIRE(str(0.5) == "0.5");
}

TEST_CASE("str - string", "[str_utils]") {
    REQUIRE(str(std::string("hi")) == "hi");
}

TEST_CASE("str_wf - con relleno cero", "[str_utils]") {
    REQUIRE(str_wf(5, 3) == "005");
    REQUIRE(str_wf(42, 2) == "42");
    REQUIRE(str_wf(123, 5) == "00123");
}

TEST_CASE("str_wf - con relleno personalizado", "[str_utils]") {
    REQUIRE(str_wf(9, 2, ' ') == " 9");
}
