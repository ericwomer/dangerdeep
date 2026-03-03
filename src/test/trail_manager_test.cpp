/*
 * Test para trail_manager: gestión de timing para grabación de posiciones.
 */
#include "catch_amalgamated.hpp"
#include "../trail_manager.h"
#include <cmath>

// Helper para comparaciones de punto flotante
inline bool near(double a, double b, double eps = 1e-9) {
    return std::fabs(a - b) <= eps;
}

TEST_CASE("trail_manager - Constructor por defecto", "[trail_manager]") {
    trail_manager tm;
    REQUIRE(near(tm.get_last_trail_time(), 0.0));
}

TEST_CASE("trail_manager - Constructor con tiempo inicial", "[trail_manager]") {
    trail_manager tm(10.0);
    // Debería inicializar con time - TRAIL_TIME
    REQUIRE(tm.get_last_trail_time() < 10.0);
}

TEST_CASE("trail_manager - should_record timing básico", "[trail_manager]") {
    trail_manager tm;
    
    SECTION("No debería grabar inmediatamente") {
        REQUIRE_FALSE(tm.should_record(0.5));
    }
    
    SECTION("Debería grabar después de TRAIL_TIME") {
        REQUIRE(tm.should_record(1.0));
        REQUIRE(tm.should_record(1.5));
        REQUIRE(tm.should_record(2.0));
    }
}

TEST_CASE("trail_manager - record_trail actualiza el tiempo", "[trail_manager]") {
    trail_manager tm;
    
    tm.record_trail(1.5);
    REQUIRE(near(tm.get_last_trail_time(), 1.5));
    REQUIRE_FALSE(tm.should_record(2.0));  // Ahora no debería grabar
    REQUIRE(tm.should_record(2.5));         // Pero sí después de 1.0 segundos
}

TEST_CASE("trail_manager - Múltiples grabaciones", "[trail_manager]") {
    trail_manager tm;
    
    tm.record_trail(2.5);
    REQUIRE(near(tm.get_last_trail_time(), 2.5));
    
    tm.record_trail(3.5);
    REQUIRE(near(tm.get_last_trail_time(), 3.5));
}

TEST_CASE("trail_manager - set_last_trail_time para cargar desde save", "[trail_manager]") {
    trail_manager tm;
    tm.set_last_trail_time(100.0);
    
    REQUIRE(near(tm.get_last_trail_time(), 100.0));
    REQUIRE_FALSE(tm.should_record(100.5));
    REQUIRE(tm.should_record(101.0));
}

TEST_CASE("trail_manager - get_trail_interval es constante", "[trail_manager]") {
    REQUIRE(near(trail_manager::get_trail_interval(), 1.0));
}

TEST_CASE("trail_manager - Secuencia realista de juego", "[trail_manager][integration]") {
    trail_manager tm(0.0);
    double game_time = 0.0;
    int record_count = 0;
    
    // Simular 5 segundos en pasos de 0.1s
    for (int i = 0; i < 50; ++i) {
        game_time += 0.1;
        if (tm.should_record(game_time)) {
            tm.record_trail(game_time);
            record_count++;
        }
    }
    
    // Deberíamos haber grabado aproximadamente 5 veces (cada 1 segundo)
    REQUIRE(record_count == 5);
}
