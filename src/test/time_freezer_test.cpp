/*
 * Test para time_freezer: gestión de pausas de tiempo del juego.
 * Nota: Este test es limitado porque time_freezer::freeze() y unfreeze()
 * dependen de system::millisec(), que tiene dependencias pesadas.
 */
#include "catch_amalgamated.hpp"
#include "../time_freezer.h"
#include "../error.h"

TEST_CASE("time_freezer - Constructor inicializa sin congelar", "[time_freezer]") {
    time_freezer tf;
    REQUIRE_FALSE(tf.is_frozen());
    REQUIRE(tf.get_freezetime() == 0);
    REQUIRE(tf.get_freezetime_start() == 0);
}

TEST_CASE("time_freezer - load carga el estado", "[time_freezer]") {
    time_freezer tf;
    
    SECTION("Sin congelar") {
        tf.load(5000, 0);
        REQUIRE(tf.get_freezetime() == 5000);
        REQUIRE_FALSE(tf.is_frozen());
        REQUIRE(tf.get_freezetime_start() == 0);
    }
    
    SECTION("Con tiempo de inicio (congelado)") {
        tf.load(3000, 12345);
        REQUIRE(tf.get_freezetime() == 3000);
        REQUIRE(tf.is_frozen());
        REQUIRE(tf.get_freezetime_start() == 12345);
    }
}

TEST_CASE("time_freezer - get_state devuelve estado actual", "[time_freezer]") {
    time_freezer tf;
    unsigned ft, fts;
    
    SECTION("Estado inicial") {
        tf.get_state(ft, fts);
        REQUIRE(ft == 0);
        REQUIRE(fts == 0);
    }
    
    SECTION("Después de load") {
        tf.load(10000, 54321);
        tf.get_state(ft, fts);
        REQUIRE(ft == 10000);
        REQUIRE(fts == 54321);
    }
}

TEST_CASE("time_freezer - process_freezetime devuelve y resetea", "[time_freezer]") {
    time_freezer tf;
    tf.load(7500, 0);
    REQUIRE(tf.get_freezetime() == 7500);
    
    unsigned processed = tf.process_freezetime();
    REQUIRE(processed == 7500);
    REQUIRE(tf.get_freezetime() == 0);
}

TEST_CASE("time_freezer - Múltiples procesos", "[time_freezer]") {
    time_freezer tf;
    tf.load(1000, 0);
    
    REQUIRE(tf.process_freezetime() == 1000);
    REQUIRE(tf.get_freezetime() == 0);
    
    // Procesar cuando no hay tiempo acumulado
    REQUIRE(tf.process_freezetime() == 0);
}

TEST_CASE("time_freezer - is_frozen basado en freezetime_start", "[time_freezer]") {
    time_freezer tf;
    REQUIRE_FALSE(tf.is_frozen());
    
    tf.load(0, 1);
    REQUIRE(tf.is_frozen());
    
    tf.load(0, 0);
    REQUIRE_FALSE(tf.is_frozen());
}

TEST_CASE("time_freezer - Edge cases con valores grandes", "[time_freezer]") {
    time_freezer tf;
    unsigned ft, fts;
    
    tf.load(4294967295u, 4294967295u);
    tf.get_state(ft, fts);
    
    REQUIRE(ft == 4294967295u);
    REQUIRE(fts == 4294967295u);
    REQUIRE(tf.is_frozen());
    
    unsigned large_time = tf.process_freezetime();
    REQUIRE(large_time == 4294967295u);
    REQUIRE(tf.get_freezetime() == 0);
}

TEST_CASE("time_freezer - Secuencia load-process repetida", "[time_freezer][integration]") {
    time_freezer tf;
    
    for (int i = 1; i <= 5; ++i) {
        tf.load(i * 1000, 0);
        REQUIRE(tf.get_freezetime() == static_cast<unsigned>(i * 1000));
        
        unsigned proc = tf.process_freezetime();
        REQUIRE(proc == static_cast<unsigned>(i * 1000));
        REQUIRE(tf.get_freezetime() == 0);
    }
}

TEST_CASE("time_freezer - load preserva freezetime_start", "[time_freezer]") {
    time_freezer tf;
    
    tf.load(1000, 5000);
    REQUIRE(tf.is_frozen());
    REQUIRE(tf.get_freezetime() == 1000);
    REQUIRE(tf.get_freezetime_start() == 5000);
    
    // Cargar nuevo estado sin congelar
    tf.load(2000, 0);
    REQUIRE_FALSE(tf.is_frozen());
    REQUIRE(tf.get_freezetime() == 2000);
}

TEST_CASE("time_freezer - get_state no modifica el estado", "[time_freezer]") {
    time_freezer tf;
    tf.load(9999, 8888);
    
    unsigned ft_before, fts_before;
    tf.get_state(ft_before, fts_before);
    
    unsigned ft_after, fts_after;
    tf.get_state(ft_after, fts_after);
    
    REQUIRE(ft_before == ft_after);
    REQUIRE(fts_before == fts_after);
    REQUIRE(ft_after == 9999);
    REQUIRE(fts_after == 8888);
}
