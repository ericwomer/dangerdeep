/*
 * Test para time_freezer: gestión de pausas de tiempo del juego.
 * Nota: Este test es limitado porque time_freezer::freeze() y unfreeze()
 * dependen de system::millisec(), que tiene dependencias pesadas.
 */
#include "../time_freezer.h"
#include "../error.h"
#include <cassert>
#include <cstdio>

int main() {
    // Test 1: Constructor inicializa sin congelar
    time_freezer tf;
    assert(!tf.is_frozen());
    assert(tf.get_freezetime() == 0);
    assert(tf.get_freezetime_start() == 0);
    
    // Test 2: load carga el estado
    tf.load(5000, 0);  // 5 segundos congelados, no está actualmente congelado
    assert(tf.get_freezetime() == 5000);
    assert(!tf.is_frozen());
    assert(tf.get_freezetime_start() == 0);
    
    // Test 3: load con tiempo de inicio (simulando que está congelado)
    tf.load(3000, 12345);
    assert(tf.get_freezetime() == 3000);
    assert(tf.is_frozen());  // freezetime_start > 0
    assert(tf.get_freezetime_start() == 12345);
    
    // Test 4: get_state devuelve estado actual
    time_freezer tf2;
    unsigned ft, fts;
    tf2.get_state(ft, fts);
    assert(ft == 0 && fts == 0);
    
    tf2.load(10000, 54321);
    tf2.get_state(ft, fts);
    assert(ft == 10000);
    assert(fts == 54321);
    
    // Test 5: process_freezetime devuelve y resetea
    time_freezer tf3;
    tf3.load(7500, 0);
    assert(tf3.get_freezetime() == 7500);
    
    unsigned processed = tf3.process_freezetime();
    assert(processed == 7500);
    assert(tf3.get_freezetime() == 0);  // Debe estar reseteado
    
    // Test 6: Múltiples procesos
    time_freezer tf4;
    tf4.load(1000, 0);
    assert(tf4.process_freezetime() == 1000);
    assert(tf4.get_freezetime() == 0);
    
    // Procesar cuando no hay tiempo acumulado
    assert(tf4.process_freezetime() == 0);
    
    // Test 7: is_frozen basado en freezetime_start
    time_freezer tf5;
    assert(!tf5.is_frozen());
    
    tf5.load(0, 1);  // freezetime_start = 1
    assert(tf5.is_frozen());
    
    tf5.load(0, 0);  // freezetime_start = 0
    assert(!tf5.is_frozen());
    
    // Test 8: Sistema es no-copiable (compile-time check)
    // time_freezer tf_copy(tf); // No debe compilar
    // time_freezer tf_assign = tf; // No debe compilar
    
    // Nota: Para tests completos de freeze()/unfreeze(), se necesitaría:
    // - Mock de system::millisec() o
    // - Integración con sistema completo
    //
    // Los métodos freeze() y unfreeze() usan sys().millisec() para medir
    // tiempo real, lo que requiere todo el subsistema de system inicializado.
    //
    // Este test básico verifica:
    // - Estado inicial correcto
    // - load() carga estado correctamente
    // - get_state() retorna estado actual
    // - process_freezetime() procesa y resetea
    // - is_frozen() se basa correctamente en freezetime_start
    
    printf("time_freezer_test ok\n");
    return 0;
}
