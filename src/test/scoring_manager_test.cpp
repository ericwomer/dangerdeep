/*
 * Test para scoring_manager: gestión de registros de barcos hundidos.
 */
#include "../scoring_manager.h"
#include "../date.h"
#include <cassert>
#include <cstdio>

int main() {
    // Test 1: Constructor inicia vacío
    scoring_manager sm;
    assert(sm.sunk_count() == 0);
    assert(!sm.has_records());
    assert(sm.total_tonnage() == 0);
    assert(sm.get_sunken_ships().empty());
    
    // Test 2: record_sunk_ship agrega un barco
    date d1(1939, 9, 3, 10, 30);
    sm.record_sunk_ship(d1, "SS Athenia", "athenia.ddxml", 
                        "athenia", "default", 13465);
    
    assert(sm.sunk_count() == 1);
    assert(sm.has_records());
    assert(sm.total_tonnage() == 13465);
    
    const std::list<sink_record> &ships = sm.get_sunken_ships();
    assert(ships.size() == 1);
    
    const sink_record &sr1 = ships.front();
    assert(sr1.descr == "SS Athenia");
    assert(sr1.mdlname == "athenia.ddxml");
    assert(sr1.specfilename == "athenia");
    assert(sr1.layoutname == "default");
    assert(sr1.tons == 13465);
    
    // Test 3: Agregar múltiples barcos
    date d2(1939, 9, 17, 14, 15);
    sm.record_sunk_ship(d2, "HMS Courageous", "courageous.ddxml",
                        "courageous", "default", 22500);
    
    date d3(1939, 10, 14, 1, 20);
    sm.record_sunk_ship(d3, "HMS Royal Oak", "royal_oak.ddxml",
                        "royal_oak", "default", 29150);
    
    assert(sm.sunk_count() == 3);
    assert(sm.total_tonnage() == 13465 + 22500 + 29150);
    assert(sm.total_tonnage() == 65115);
    
    // Test 4: Los barcos están en orden de agregado
    const std::list<sink_record> &ships2 = sm.get_sunken_ships();
    auto it = ships2.begin();
    assert(it->descr == "SS Athenia");
    ++it;
    assert(it->descr == "HMS Courageous");
    ++it;
    assert(it->descr == "HMS Royal Oak");
    
    // Test 5: clear limpia todo
    sm.clear();
    assert(sm.sunk_count() == 0);
    assert(!sm.has_records());
    assert(sm.total_tonnage() == 0);
    assert(sm.get_sunken_ships().empty());
    
    // Test 6: Agregar después de clear
    date d4(1940, 5, 24, 6, 0);
    sm.record_sunk_ship(d4, "HMS Hood", "hood.ddxml",
                        "hood", "default", 42670);
    
    assert(sm.sunk_count() == 1);
    assert(sm.total_tonnage() == 42670);
    
    // Test 7: Tonelaje cero
    date d5(1940, 6, 1, 12, 0);
    sm.record_sunk_ship(d5, "Small Boat", "boat.ddxml",
                        "boat", "default", 0);
    
    assert(sm.sunk_count() == 2);
    assert(sm.total_tonnage() == 42670);  // Sin cambio
    
    // Test 8: Múltiples barcos del mismo tipo
    date d6(1940, 7, 1, 8, 0);
    date d7(1940, 7, 2, 10, 0);
    sm.record_sunk_ship(d6, "U-Boot Type VII", "type7.ddxml",
                        "type7", "default", 769);
    sm.record_sunk_ship(d7, "U-Boot Type VII", "type7.ddxml",
                        "type7", "default", 769);
    
    assert(sm.sunk_count() == 4);
    assert(sm.total_tonnage() == 42670 + 769 + 769);
    
    // Test 9: Secuencia realista de una patrulla
    scoring_manager sm2;
    
    // Día 1: hundir mercante
    date patrol_d1(1941, 3, 15, 2, 30);
    sm2.record_sunk_ship(patrol_d1, "Merchant Ship A", "merchant.ddxml",
                         "merchant", "variant1", 5000);
    
    // Día 3: hundir otro mercante
    date patrol_d3(1941, 3, 17, 23, 45);
    sm2.record_sunk_ship(patrol_d3, "Merchant Ship B", "merchant.ddxml",
                         "merchant", "variant2", 7500);
    
    // Día 5: hundir destructor enemigo
    date patrol_d5(1941, 3, 19, 14, 20);
    sm2.record_sunk_ship(patrol_d5, "HMS Destroyer", "destroyer.ddxml",
                         "destroyer", "default", 2000);
    
    // Verificar estadísticas de la patrulla
    assert(sm2.sunk_count() == 3);
    assert(sm2.total_tonnage() == 14500);
    assert(sm2.has_records());
    
    printf("scoring_manager_test ok\n");
    return 0;
}
