/*
 * Stub mínimo para event_manager (solo para testing)
 */
#include "../event_manager.h"
#include <memory>

// Forward declaration
class event;

// Stubs de los métodos para que el test compile sin dependencias
void event_manager::add_event(std::unique_ptr<event>) {
    // Stub vacío
}

void event_manager::add_event(event *) {
    // Stub vacío
}

void event_manager::evaluate_events(user_interface &) {
    // Stub vacío
}

void event_manager::clear_events() {
    // events.clear(); // No podemos acceder a members privados
}
