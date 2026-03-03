/*
 * Test para logbook: gestión de entradas de bitácora.
 */
#include "catch_amalgamated.hpp"
#include "../logbook.h"

TEST_CASE("logbook - Constructor inicializa vacío", "[logbook]") {
    logbook lb;
    REQUIRE(lb.size() == 0);
    REQUIRE(lb.begin() == lb.end());
}

TEST_CASE("logbook - add_entry agrega una entrada", "[logbook]") {
    logbook lb;
    lb.add_entry("entry1");
    
    REQUIRE(lb.size() == 1);
    REQUIRE(*lb.get_entry(0) == "entry1");
}

TEST_CASE("logbook - Agregar múltiples entradas", "[logbook]") {
    logbook lb;
    lb.add_entry("entry1");
    lb.add_entry("entry2");
    lb.add_entry("entry3");
    
    REQUIRE(lb.size() == 3);
}

TEST_CASE("logbook - get_entry retorna correctamente por índice", "[logbook]") {
    logbook lb;
    lb.add_entry("entry1");
    lb.add_entry("entry2");
    lb.add_entry("entry3");
    
    REQUIRE(*lb.get_entry(0) == "entry1");
    REQUIRE(*lb.get_entry(1) == "entry2");
    REQUIRE(*lb.get_entry(2) == "entry3");
}

TEST_CASE("logbook - Entradas están en orden de agregado", "[logbook]") {
    logbook lb;
    lb.add_entry("entry1");
    lb.add_entry("entry2");
    lb.add_entry("entry3");
    
    std::list<std::string>::const_iterator it = lb.begin();
    REQUIRE(*it == "entry1");
    ++it;
    REQUIRE(*it == "entry2");
    ++it;
    REQUIRE(*it == "entry3");
    ++it;
    REQUIRE(it == lb.end());
}

TEST_CASE("logbook - begin() y end() son consistentes con size()", "[logbook]") {
    logbook lb;
    REQUIRE(lb.begin() == lb.end());
    
    lb.add_entry("test");
    REQUIRE(lb.begin() != lb.end());
}

TEST_CASE("logbook - Entradas con caracteres especiales", "[logbook]") {
    logbook lb;
    lb.add_entry("Entry with spaces and punctuation!");
    lb.add_entry("Entry\twith\ttabs");
    lb.add_entry("Entry\nwith\nnewlines");
    
    REQUIRE(lb.size() == 3);
    REQUIRE(*lb.get_entry(0) == "Entry with spaces and punctuation!");
}

TEST_CASE("logbook - Muchas entradas", "[logbook]") {
    logbook lb;
    
    for (int i = 0; i < 100; ++i) {
        lb.add_entry("Entry " + std::to_string(i));
    }
    
    REQUIRE(lb.size() == 100);
    REQUIRE(*lb.get_entry(0) == "Entry 0");
    REQUIRE(*lb.get_entry(99) == "Entry 99");
}

TEST_CASE("logbook - Secuencia realista de patrulla", "[logbook][integration]") {
    logbook lb;
    
    lb.add_entry("1939-09-03 10:00 - Departed from Wilhelmshaven");
    lb.add_entry("1939-09-03 15:30 - Sighted convoy");
    lb.add_entry("1939-09-03 16:45 - Attacked SS Athenia - SUNK");
    lb.add_entry("1939-09-05 08:00 - Submerged due to enemy aircraft");
    lb.add_entry("1939-09-10 12:00 - Returned to base");
    
    REQUIRE(lb.size() == 5);
    REQUIRE(*lb.get_entry(0) == "1939-09-03 10:00 - Departed from Wilhelmshaven");
    REQUIRE(*lb.get_entry(4) == "1939-09-10 12:00 - Returned to base");
}
