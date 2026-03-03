/*
 * Test para ping_manager: gestión de pings de sonar activo.
 */
#include "catch_amalgamated.hpp"
#include "../ping_manager.h"
#include "../angle.h"
#include "../vector2.h"
#include <cmath>

inline bool near(double a, double b, double eps = 1e-6) {
    return std::fabs(a - b) <= eps;
}

TEST_CASE("ping_manager - Constructor inicia vacío", "[ping_manager]") {
    ping_manager pm;
    REQUIRE(pm.ping_count() == 0);
    REQUIRE_FALSE(pm.has_pings());
    REQUIRE(pm.get_pings().empty());
}

TEST_CASE("ping_manager - add_ping agrega un ping", "[ping_manager]") {
    ping_manager pm;
    vector2 pos1(1000.0, 2000.0);
    angle dir1(0.0);
    pm.add_ping(pos1, dir1, 0.0, 5000.0, angle(30.0));
    
    REQUIRE(pm.ping_count() == 1);
    REQUIRE(pm.has_pings());
    
    const std::list<ping> &pings = pm.get_pings();
    REQUIRE(pings.size() == 1);
    
    const ping &p1 = pings.front();
    REQUIRE(p1.pos == pos1);
    REQUIRE(near(p1.dir.value(), dir1.value()));
    REQUIRE(near(p1.time, 0.0));
    REQUIRE(near(p1.range, 5000.0));
    REQUIRE(near(p1.ping_angle.value(), 30.0));
}

TEST_CASE("ping_manager - Agregar múltiples pings", "[ping_manager]") {
    ping_manager pm;
    vector2 pos1(1000.0, 2000.0);
    vector2 pos2(2000.0, 3000.0);
    vector2 pos3(3000.0, 4000.0);
    
    pm.add_ping(pos1, angle(0.0), 0.0, 5000.0, angle(30.0));
    pm.add_ping(pos2, angle(90.0), 0.5, 6000.0, angle(45.0));
    pm.add_ping(pos3, angle(180.0), 1.0, 7000.0, angle(60.0));
    
    REQUIRE(pm.ping_count() == 3);
    REQUIRE(pm.has_pings());
}

TEST_CASE("ping_manager - update elimina pings viejos", "[ping_manager]") {
    ping_manager pm;
    vector2 pos1(1000.0, 2000.0);
    vector2 pos2(2000.0, 3000.0);
    vector2 pos3(3000.0, 4000.0);
    
    pm.add_ping(pos1, angle(0.0), 0.0, 5000.0, angle(30.0));
    pm.add_ping(pos2, angle(90.0), 0.5, 6000.0, angle(45.0));
    pm.add_ping(pos3, angle(180.0), 1.0, 7000.0, angle(60.0));
    
    pm.update(0.5);
    REQUIRE(pm.ping_count() == 3);
    
    pm.update(1.0);
    REQUIRE(pm.ping_count() == 3);
    
    pm.update(1.1);
    REQUIRE(pm.ping_count() == 2);
    
    pm.update(1.6);
    REQUIRE(pm.ping_count() == 1);
    
    pm.update(2.1);
    REQUIRE(pm.ping_count() == 0);
    REQUIRE_FALSE(pm.has_pings());
}

TEST_CASE("ping_manager - clear elimina todos los pings", "[ping_manager]") {
    ping_manager pm;
    vector2 pos1(1000.0, 2000.0);
    vector2 pos2(2000.0, 3000.0);
    
    pm.add_ping(pos1, angle(0.0), 0.0, 5000.0, angle(30.0));
    pm.add_ping(pos2, angle(90.0), 0.0, 6000.0, angle(45.0));
    REQUIRE(pm.ping_count() == 2);
    
    pm.clear();
    REQUIRE(pm.ping_count() == 0);
    REQUIRE_FALSE(pm.has_pings());
}

TEST_CASE("ping_manager - Pings se agregan en orden", "[ping_manager]") {
    ping_manager pm;
    vector2 pos1(1000.0, 2000.0);
    vector2 pos2(2000.0, 3000.0);
    vector2 pos3(3000.0, 4000.0);
    
    pm.add_ping(pos1, angle(0.0), 1.0, 5000.0, angle(30.0));
    pm.add_ping(pos2, angle(90.0), 2.0, 6000.0, angle(45.0));
    pm.add_ping(pos3, angle(180.0), 3.0, 7000.0, angle(60.0));
    
    const std::list<ping> &pings = pm.get_pings();
    auto it = pings.begin();
    REQUIRE(near(it->time, 1.0));
    ++it;
    REQUIRE(near(it->time, 2.0));
    ++it;
    REQUIRE(near(it->time, 3.0));
}

TEST_CASE("ping_manager - Secuencia realista de sonar activo", "[ping_manager][integration]") {
    ping_manager pm;
    vector2 pos(5000.0, 10000.0);
    double game_time = 0.0;
    
    // Emisión cada 5 segundos
    for (int i = 0; i < 5; ++i) {
        pm.add_ping(pos, angle(45.0), game_time, 8000.0, angle(30.0));
        game_time += 5.0;
    }
    
    REQUIRE(pm.ping_count() == 5);
    
    // Avanzar tiempo: deberían ir expirando
    pm.update(game_time);
    REQUIRE(pm.ping_count() < 5);
}
