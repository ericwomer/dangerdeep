/*
 * Stub mínimo para event_manager (solo para testing)
 */
#include "../event_manager.h"
#include "event_stub.h"
#include <memory>

// Stubs de los métodos que realmente hacen algo (para que el test pueda verificarlos)
void event_manager::add_event(std::unique_ptr<event> e) {
    if (e) {
        events.push_back(std::move(e));
    }
}

void event_manager::add_event(event *e) {
    if (e) {
        events.push_back(std::unique_ptr<event>(e));
    }
}

void event_manager::evaluate_events(user_interface &) {
    // Stub vacío - en el código real procesaría cada evento
}

void event_manager::clear_events() {
    events.clear();
}
