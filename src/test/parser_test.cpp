/*
 * Test para parser.h/cpp: lectura de CSV (get_cell, get_cell_number, next_column, next_line).
 * Requiere DFTD_DATA apuntando al directorio data/ del proyecto.
 */
#include "catch_amalgamated.hpp"
#include "../parser.h"
#include <cstdlib>
#include <string>

TEST_CASE("parser - lectura de common.csv", "[parser]") {
    const char *data_env = std::getenv("DFTD_DATA");
    REQUIRE(data_env != nullptr);
    std::string csv_path = std::string(data_env) + "/texts/common.csv";
    parser p(csv_path);
    std::string first_cell = p.get_cell();
    REQUIRE(first_cell == "CODE");
    unsigned n = 0;
    bool ok = p.get_cell_number(n);
    REQUIRE_FALSE(ok);  // "CODE" no es número
}
