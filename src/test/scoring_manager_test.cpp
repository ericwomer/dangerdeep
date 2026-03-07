/*
 * Test para scoring_manager: gestión de registros de barcos hundidos.
 */
#include "catch_amalgamated.hpp"
#include "../scoring_manager.h"
#include "../date.h"
#include "../xml.h"
#include <cstdio>
#include <unistd.h>

TEST_CASE("scoring_manager - Constructor inicia vacío", "[scoring_manager]") {
    scoring_manager sm;
    REQUIRE(sm.sunk_count() == 0);
    REQUIRE_FALSE(sm.has_records());
    REQUIRE(sm.total_tonnage() == 0);
    REQUIRE(sm.get_sunken_ships().empty());
}

TEST_CASE("scoring_manager - record_sunk_ship agrega un barco", "[scoring_manager]") {
    scoring_manager sm;
    date d1(1939, 9, 3, 10, 30);
    sm.record_sunk_ship(d1, "SS Athenia", "athenia.ddxml", 
                        "athenia", "default", 13465);
    
    REQUIRE(sm.sunk_count() == 1);
    REQUIRE(sm.has_records());
    REQUIRE(sm.total_tonnage() == 13465);
    
    const std::list<sink_record> &ships = sm.get_sunken_ships();
    REQUIRE(ships.size() == 1);
    
    const sink_record &sr1 = ships.front();
    REQUIRE(sr1.descr == "SS Athenia");
    REQUIRE(sr1.mdlname == "athenia.ddxml");
    REQUIRE(sr1.specfilename == "athenia");
    REQUIRE(sr1.layoutname == "default");
    REQUIRE(sr1.tons == 13465);
}

TEST_CASE("scoring_manager - Agregar múltiples barcos", "[scoring_manager]") {
    scoring_manager sm;
    date d1(1939, 9, 3, 10, 30);
    date d2(1939, 9, 17, 14, 15);
    date d3(1939, 10, 14, 1, 20);
    
    sm.record_sunk_ship(d1, "SS Athenia", "athenia.ddxml", "athenia", "default", 13465);
    sm.record_sunk_ship(d2, "HMS Courageous", "courageous.ddxml", "courageous", "default", 22500);
    sm.record_sunk_ship(d3, "HMS Royal Oak", "royal_oak.ddxml", "royal_oak", "default", 29150);
    
    REQUIRE(sm.sunk_count() == 3);
    REQUIRE(sm.total_tonnage() == 65115);
}

TEST_CASE("scoring_manager - Los barcos están en orden de agregado", "[scoring_manager]") {
    scoring_manager sm;
    date d1(1939, 9, 3, 10, 30);
    date d2(1939, 9, 17, 14, 15);
    date d3(1939, 10, 14, 1, 20);
    
    sm.record_sunk_ship(d1, "SS Athenia", "athenia.ddxml", "athenia", "default", 13465);
    sm.record_sunk_ship(d2, "HMS Courageous", "courageous.ddxml", "courageous", "default", 22500);
    sm.record_sunk_ship(d3, "HMS Royal Oak", "royal_oak.ddxml", "royal_oak", "default", 29150);
    
    const std::list<sink_record> &ships = sm.get_sunken_ships();
    auto it = ships.begin();
    REQUIRE(it->descr == "SS Athenia");
    ++it;
    REQUIRE(it->descr == "HMS Courageous");
    ++it;
    REQUIRE(it->descr == "HMS Royal Oak");
}

TEST_CASE("scoring_manager - clear limpia todo", "[scoring_manager]") {
    scoring_manager sm;
    date d1(1939, 9, 3, 10, 30);
    sm.record_sunk_ship(d1, "SS Athenia", "athenia.ddxml", "athenia", "default", 13465);
    
    sm.clear();
    REQUIRE(sm.sunk_count() == 0);
    REQUIRE_FALSE(sm.has_records());
    REQUIRE(sm.total_tonnage() == 0);
    REQUIRE(sm.get_sunken_ships().empty());
}

TEST_CASE("scoring_manager - Agregar después de clear", "[scoring_manager]") {
    scoring_manager sm;
    date d1(1939, 9, 3, 10, 30);
    date d2(1940, 5, 24, 6, 0);
    
    sm.record_sunk_ship(d1, "SS Athenia", "athenia.ddxml", "athenia", "default", 13465);
    sm.clear();
    sm.record_sunk_ship(d2, "HMS Hood", "hood.ddxml", "hood", "default", 42670);
    
    REQUIRE(sm.sunk_count() == 1);
    REQUIRE(sm.total_tonnage() == 42670);
}

TEST_CASE("scoring_manager - Tonelaje cero", "[scoring_manager]") {
    scoring_manager sm;
    date d1(1940, 5, 24, 6, 0);
    date d2(1940, 6, 1, 12, 0);
    
    sm.record_sunk_ship(d1, "HMS Hood", "hood.ddxml", "hood", "default", 42670);
    sm.record_sunk_ship(d2, "Small Boat", "boat.ddxml", "boat", "default", 0);
    
    REQUIRE(sm.sunk_count() == 2);
    REQUIRE(sm.total_tonnage() == 42670);
}

TEST_CASE("scoring_manager - Secuencia realista de patrulla", "[scoring_manager][integration]") {
    scoring_manager sm;
    
    // Patrulla 1: Sep 1939 - Oct 1939
    date d1(1939, 9, 3, 10, 30);
    date d2(1939, 9, 17, 14, 15);
    date d3(1939, 10, 14, 1, 20);
    
    sm.record_sunk_ship(d1, "SS Athenia", "athenia.ddxml", "athenia", "default", 13465);
    sm.record_sunk_ship(d2, "HMS Courageous", "courageous.ddxml", "courageous", "default", 22500);
    sm.record_sunk_ship(d3, "HMS Royal Oak", "royal_oak.ddxml", "royal_oak", "default", 29150);
    
    REQUIRE(sm.sunk_count() == 3);
    REQUIRE(sm.total_tonnage() == 65115);
}

TEST_CASE("scoring_manager - save/load roundtrip", "[scoring_manager]") {
    scoring_manager sm;
    date d1(1939, 9, 3, 10, 30);
    date d2(1939, 9, 17, 14, 15);
    sm.record_sunk_ship(d1, "SS Athenia", "athenia.ddxml", "athenia", "default", 13465);
    sm.record_sunk_ship(d2, "HMS Courageous", "courageous.ddxml", "courageous", "default", 22500);

    char tmp[] = "/tmp/dftd_scoring_save_XXXXXX";
    int fd = mkstemp(tmp);
    REQUIRE(fd >= 0);
    close(fd);

    xml_doc doc(tmp);
    xml_elem root = doc.add_child("root");
    sm.save(root);
    doc.save();

    scoring_manager sm2;
    xml_doc doc2(tmp);
    doc2.load();
    xml_elem root2 = doc2.first_child();
    sm2.load(root2);
    unlink(tmp);

    REQUIRE(sm2.sunk_count() == 2);
    REQUIRE(sm2.total_tonnage() == 35965);
    const std::list<sink_record> &ships = sm2.get_sunken_ships();
    auto it = ships.begin();
    REQUIRE(it->descr == "SS Athenia");
    REQUIRE(it->tons == 13465);
    ++it;
    REQUIRE(it->descr == "HMS Courageous");
    REQUIRE(it->tons == 22500);
}
