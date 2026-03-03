/*
 * Test para ping_manager: gestión de pings de sonar activo.
 */
#include "../ping_manager.h"
#include "../angle.h"
#include "../vector2.h"
#include <cassert>
#include <cstdio>
#include <cmath>

static bool near(double a, double b, double eps = 1e-6) {
    return std::fabs(a - b) <= eps;
}

int main() {
    // Test 1: Constructor inicia vacío
    ping_manager pm;
    assert(pm.ping_count() == 0);
    assert(!pm.has_pings());
    assert(pm.get_pings().empty());
    
    // Test 2: add_ping agrega un ping
    vector2 pos1(1000.0, 2000.0);
    angle dir1(0.0);
    pm.add_ping(pos1, dir1, 0.0, 5000.0, angle(30.0));
    
    assert(pm.ping_count() == 1);
    assert(pm.has_pings());
    
    const std::list<ping> &pings = pm.get_pings();
    assert(pings.size() == 1);
    
    const ping &p1 = pings.front();
    assert(p1.pos == pos1);
    assert(near(p1.dir.value(), dir1.value()));
    assert(near(p1.time, 0.0));
    assert(near(p1.range, 5000.0));
    assert(near(p1.ping_angle.value(), 30.0));
    
    // Test 3: Agregar múltiples pings
    vector2 pos2(2000.0, 3000.0);
    angle dir2(90.0);
    pm.add_ping(pos2, dir2, 0.5, 6000.0, angle(45.0));
    
    vector2 pos3(3000.0, 4000.0);
    angle dir3(180.0);
    pm.add_ping(pos3, dir3, 1.0, 7000.0, angle(60.0));
    
    assert(pm.ping_count() == 3);
    assert(pm.has_pings());
    
    // Test 4: update elimina pings viejos (> 1.0 segundo)
    pm.update(0.5);  // t=0.5: ping1 tiene 0.5s, ping2 tiene 0s
    assert(pm.ping_count() == 3);  // Todos siguen activos
    
    pm.update(1.0);  // t=1.0: ping1 tiene 1.0s, ping2 tiene 0.5s
    assert(pm.ping_count() == 3);  // Todos en el límite o menores
    
    pm.update(1.1);  // t=1.1: ping1 tiene 1.1s (debe eliminarse)
    assert(pm.ping_count() == 2);  // ping1 eliminado
    
    pm.update(1.6);  // t=1.6: ping2 tiene 1.1s (debe eliminarse)
    assert(pm.ping_count() == 1);  // ping2 eliminado
    
    pm.update(2.1);  // t=2.1: ping3 tiene 1.1s (debe eliminarse)
    assert(pm.ping_count() == 0);  // Todos eliminados
    assert(!pm.has_pings());
    
    // Test 5: clear elimina todos los pings
    pm.add_ping(pos1, dir1, 0.0, 5000.0, angle(30.0));
    pm.add_ping(pos2, dir2, 0.0, 6000.0, angle(45.0));
    assert(pm.ping_count() == 2);
    
    pm.clear();
    assert(pm.ping_count() == 0);
    assert(!pm.has_pings());
    
    // Test 6: Pings se agregan en orden
    pm.add_ping(pos1, angle(0.0), 1.0, 5000.0, angle(30.0));
    pm.add_ping(pos2, angle(90.0), 2.0, 6000.0, angle(45.0));
    pm.add_ping(pos3, angle(180.0), 3.0, 7000.0, angle(60.0));
    
    const std::list<ping> &pings2 = pm.get_pings();
    auto it = pings2.begin();
    assert(near(it->time, 1.0));
    ++it;
    assert(near(it->time, 2.0));
    ++it;
    assert(near(it->time, 3.0));
    
    // Test 7: Secuencia realista de uso de sonar activo
    ping_manager pm2;
    double game_time = 0.0;
    
    // Enviar ping inicial
    pm2.add_ping(vector2(0, 0), angle(0.0), game_time, 5000.0, angle(30.0));
    assert(pm2.ping_count() == 1);
    
    // Avanzar medio segundo
    game_time += 0.5;
    pm2.update(game_time);
    assert(pm2.ping_count() == 1);  // Ping todavía activo
    
    // Enviar otro ping
    pm2.add_ping(vector2(100, 100), angle(45.0), game_time, 6000.0, angle(45.0));
    assert(pm2.ping_count() == 2);
    
    // Avanzar otro medio segundo
    game_time += 0.5;
    pm2.update(game_time);
    assert(pm2.ping_count() == 2);  // Ambos activos (1.0s y 0.5s)
    
    // Avanzar 0.2 segundos más (total 1.2s desde primer ping)
    game_time += 0.2;
    pm2.update(game_time);
    assert(pm2.ping_count() == 1);  // Primer ping expiró
    
    // Test 8: Múltiples pings simultáneos (ráfaga)
    ping_manager pm3;
    pm3.add_ping(vector2(0, 0), angle(0.0), 5.0, 5000.0, angle(30.0));
    pm3.add_ping(vector2(0, 0), angle(45.0), 5.0, 5000.0, angle(30.0));
    pm3.add_ping(vector2(0, 0), angle(90.0), 5.0, 5000.0, angle(30.0));
    
    assert(pm3.ping_count() == 3);
    assert(pm3.has_pings());
    
    // Todos expiran juntos
    pm3.update(6.1);
    assert(pm3.ping_count() == 0);
    
    printf("ping_manager_test ok\n");
    return 0;
}
