/*
 * Test para trail_manager: gestión de timing para grabación de posiciones.
 */
#include "../trail_manager.h"
#include <cassert>
#include <cmath>
#include <cstdio>

static bool near(double a, double b, double eps = 1e-9) {
    return std::fabs(a - b) <= eps;
}

int main() {
    // Test 1: Constructor por defecto
    trail_manager tm1;
    assert(near(tm1.get_last_trail_time(), 0.0));
    
    // Test 2: Constructor con tiempo inicial
    trail_manager tm2(10.0);
    // Debería inicializar con time - TRAIL_TIME
    assert(tm2.get_last_trail_time() < 10.0);
    
    // Test 3: should_record - no debería grabar inmediatamente
    assert(!tm1.should_record(0.5));
    
    // Test 4: should_record - debería grabar después de TRAIL_TIME
    assert(tm1.should_record(1.0));
    assert(tm1.should_record(1.5));
    assert(tm1.should_record(2.0));
    
    // Test 5: record_trail actualiza el tiempo
    tm1.record_trail(1.5);
    assert(near(tm1.get_last_trail_time(), 1.5));
    assert(!tm1.should_record(2.0));  // Ahora no debería grabar
    assert(tm1.should_record(2.5));   // Pero sí después de 1.0 segundos
    
    // Test 6: Múltiples grabaciones
    tm1.record_trail(2.5);
    assert(near(tm1.get_last_trail_time(), 2.5));
    tm1.record_trail(3.5);
    assert(near(tm1.get_last_trail_time(), 3.5));
    
    // Test 7: set_last_trail_time (para cargar desde save)
    trail_manager tm3;
    tm3.set_last_trail_time(100.0);
    assert(near(tm3.get_last_trail_time(), 100.0));
    assert(!tm3.should_record(100.5));
    assert(tm3.should_record(101.0));
    
    // Test 8: get_trail_interval es constante
    assert(near(trail_manager::get_trail_interval(), 1.0));
    
    // Test 9: Secuencia realista de juego
    trail_manager tm4(0.0);
    double game_time = 0.0;
    int record_count = 0;
    
    for (int i = 0; i < 50; ++i) {  // Simular 5 segundos en pasos de 0.1s
        game_time += 0.1;
        if (tm4.should_record(game_time)) {
            tm4.record_trail(game_time);
            record_count++;
        }
    }
    
    // Deberíamos haber grabado aproximadamente 5 veces (cada 1 segundo)
    assert(record_count == 5);
    
    printf("trail_manager_test ok\n");
    return 0;
}
