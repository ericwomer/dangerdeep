/*
 * Test para logbook: gestión de entradas de bitácora.
 */
#include "../logbook.h"
#include <cassert>
#include <cstdio>

int main() {
    // Test 1: Constructor inicializa vacío
    logbook lb;
    assert(lb.size() == 0);
    assert(lb.begin() == lb.end());
    
    // Test 2: add_entry agrega una entrada
    lb.add_entry("entry1");
    assert(lb.size() == 1);
    assert(*lb.get_entry(0) == "entry1");
    
    // Test 3: Agregar múltiples entradas
    lb.add_entry("entry2");
    lb.add_entry("entry3");
    assert(lb.size() == 3);
    
    // Test 4: get_entry retorna correctamente por índice
    assert(*lb.get_entry(0) == "entry1");
    assert(*lb.get_entry(1) == "entry2");
    assert(*lb.get_entry(2) == "entry3");
    
    // Test 5: Entradas están en orden de agregado
    std::list<std::string>::const_iterator it = lb.begin();
    assert(*it == "entry1");
    ++it;
    assert(*it == "entry2");
    ++it;
    assert(*it == "entry3");
    ++it;
    assert(it == lb.end());
    
    // Test 6: begin() y end() son consistentes con size()
    logbook lb2;
    assert(lb2.begin() == lb2.end());
    
    lb2.add_entry("test");
    assert(lb2.begin() != lb2.end());
    
    int count = 0;
    for (auto it = lb2.begin(); it != lb2.end(); ++it) {
        count++;
    }
    assert(count == 1);
    assert(count == static_cast<int>(lb2.size()));
    
    // Test 7: Entradas con caracteres especiales
    logbook lb3;
    lb3.add_entry("Entry with spaces");
    lb3.add_entry("Entry\twith\ttabs");
    lb3.add_entry("Entry\nwith\nnewlines");
    lb3.add_entry("Entry with ümläüts and números 123");
    lb3.add_entry("");  // Entrada vacía
    
    assert(lb3.size() == 5);
    assert(*lb3.get_entry(0) == "Entry with spaces");
    assert(*lb3.get_entry(4) == "");
    
    // Test 8: Muchas entradas
    logbook lb4;
    for (int i = 0; i < 100; ++i) {
        lb4.add_entry("Entry " + std::to_string(i));
    }
    assert(lb4.size() == 100);
    assert(*lb4.get_entry(0) == "Entry 0");
    assert(*lb4.get_entry(50) == "Entry 50");
    assert(*lb4.get_entry(99) == "Entry 99");
    
    // Test 9: Recorrido completo con iteradores
    int entry_count = 0;
    for (auto it = lb4.begin(); it != lb4.end(); ++it) {
        entry_count++;
    }
    assert(entry_count == 100);
    
    // Test 10: Secuencia realista de bitácora de submarino
    logbook patrol_log;
    patrol_log.add_entry("0800: Departed from Kiel");
    patrol_log.add_entry("1200: Entered North Sea");
    patrol_log.add_entry("1800: Submerged to periscope depth");
    patrol_log.add_entry("0300: Sighted convoy, 12 ships");
    patrol_log.add_entry("0400: Fired torpedoes, 2 hits");
    patrol_log.add_entry("0415: Merchant vessel sunk, 5000 tons");
    patrol_log.add_entry("0500: Evading depth charge attack");
    patrol_log.add_entry("0700: Attack ended, surfacing");
    patrol_log.add_entry("1200: Returned to base");
    
    assert(patrol_log.size() == 9);
    assert(*patrol_log.get_entry(0) == "0800: Departed from Kiel");
    assert(*patrol_log.get_entry(8) == "1200: Returned to base");
    
    // Verificar que todas las entradas están presentes
    int patrol_count = 0;
    for (auto it = patrol_log.begin(); it != patrol_log.end(); ++it) {
        patrol_count++;
    }
    assert(patrol_count == 9);
    
    printf("logbook_test ok\n");
    return 0;
}
