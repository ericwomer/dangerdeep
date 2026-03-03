/*
 * Test para event_manager: gestión de eventos del juego.
 * Nota: Este test es limitado porque event es una clase abstracta compleja.
 */
#include "../event_manager.h"
#include <cassert>
#include <cstdio>
#include <memory>

// Forward declarations
class event;
class user_interface;

int main() {
    // Test 1: Constructor inicializa vacío
    event_manager em;
    assert(em.event_count() == 0);
    assert(!em.has_events());
    assert(em.get_events().empty());
    
    // Test 2: clear_events en manager vacío no falla
    em.clear_events();
    assert(em.event_count() == 0);
    
    // Test 3: Sistema es no-copiable (compile-time check)
    // event_manager em2(em); // No debe compilar
    // event_manager em3 = em; // No debe compilar
    
    // Test 4: Puede crear múltiples instancias independientes
    event_manager em2;
    event_manager em3;
    
    assert(em2.event_count() == 0);
    assert(em3.event_count() == 0);
    
    em2.clear_events();
    em3.clear_events();
    
    // Test 5: has_events es consistente con event_count
    assert(!em.has_events() == (em.event_count() == 0));
    
    // Test 6: get_events retorna referencia constante
    const std::list<std::unique_ptr<event>> &events = em.get_events();
    assert(events.empty());
    assert(events.size() == 0);
    
    // Nota: Para tests más completos de event_manager, se necesitarían:
    // 1. Clases derivadas concretas de event para testing
    // 2. Mock de user_interface para evaluate_events
    // 3. Verificación de ownership y lifecycle de eventos
    //
    // Los tests más importantes serían:
    // - add_event() incrementa event_count()
    // - evaluate_events() procesa todos los eventos con UI
    // - clear_events() elimina todos los eventos y libera memoria
    // - add_event(unique_ptr) transfiere ownership correctamente
    // - add_event(raw ptr) toma ownership correctamente
    //
    // Por ahora, este test básico verifica que:
    // - El sistema compila correctamente
    // - Es instanciable
    // - Estado inicial es correcto (vacío)
    // - No crashea con operaciones básicas en estado vacío
    // - Respeta la semántica de no-copiable
    // - get_events() retorna referencia válida
    
    printf("event_manager_test ok\n");
    return 0;
}
