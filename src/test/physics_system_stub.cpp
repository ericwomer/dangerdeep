/*
 * Stub mínimo para physics_system (solo para testing)
 */
#include "../physics_system.h"

// Stubs de los métodos para que el test compile sin dependencias
void physics_system::check_collisions(const std::vector<ship *> &) {
    // Stub vacío
}

void physics_system::collision_response(sea_object &, sea_object &, const vector3 &) {
    // Stub vacío
}
