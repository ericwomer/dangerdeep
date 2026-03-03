/*
 * Test para event_manager: gestión de eventos del juego.
 * Usa eventos mock concretos para verificar el comportamiento.
 */
#include "event_stub.h"  // Stub de event ANTES de event_manager.h
#include "../event_manager.h"
#include <cassert>
#include <cstdio>
#include <memory>

// Forward declaration
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
    
    // Test 7: add_event con unique_ptr agrega evento
    em.add_event(std::make_unique<test_message_event>("Hello"));
    assert(em.event_count() == 1);
    assert(em.has_events());
    assert(em.get_events().size() == 1);
    
    // Test 8: add_event con raw pointer agrega evento
    em.add_event(new test_signal_event());
    assert(em.event_count() == 2);
    
    // Test 9: add_event múltiples veces
    em.add_event(std::make_unique<test_action_event>(1, 3.14));
    em.add_event(std::make_unique<test_action_event>(2, 2.71));
    em.add_event(std::make_unique<test_message_event>("World"));
    
    assert(em.event_count() == 5);
    assert(em.get_events().size() == 5);
    
    // Test 10: clear_events elimina todos
    em.clear_events();
    assert(em.event_count() == 0);
    assert(!em.has_events());
    assert(em.get_events().empty());
    
    // Test 11: add_event después de clear
    em.add_event(std::make_unique<test_signal_event>());
    assert(em.event_count() == 1);
    
    // Test 12: Verificar tipos de eventos en la lista
    event_manager em4;
    em4.add_event(std::make_unique<test_message_event>("Test1"));
    em4.add_event(std::make_unique<test_action_event>(42, 9.99));
    em4.add_event(std::make_unique<test_signal_event>());
    
    const auto &events4 = em4.get_events();
    auto it = events4.begin();
    
    // Primer evento: message
    assert(it != events4.end());
    assert((*it)->get_type() == "message");
    ++it;
    
    // Segundo evento: action
    assert(it != events4.end());
    assert((*it)->get_type() == "action");
    ++it;
    
    // Tercer evento: signal
    assert(it != events4.end());
    assert((*it)->get_type() == "signal");
    ++it;
    
    assert(it == events4.end());
    
    // Test 13: add_event con nullptr no debe causar problemas
    em4.add_event(std::unique_ptr<event>(nullptr));
    assert(em4.event_count() == 3);  // No debe incrementar
    
    em4.add_event(static_cast<event*>(nullptr));
    assert(em4.event_count() == 3);  // No debe incrementar
    
    // Test 14: Secuencia realista de uso
    event_manager em5;
    
    // Frame 1: varios eventos
    em5.add_event(std::make_unique<test_message_event>("Start"));
    em5.add_event(std::make_unique<test_action_event>(1, 1.0));
    assert(em5.event_count() == 2);
    
    // Procesar eventos (stub vacío, pero no crashea)
    // user_interface *ui = nullptr;
    // em5.evaluate_events(*ui);  // Requeriría UI válido
    
    // Limpiar después de procesar
    em5.clear_events();
    assert(em5.event_count() == 0);
    
    // Frame 2: más eventos
    em5.add_event(std::make_unique<test_signal_event>());
    assert(em5.event_count() == 1);
    
    // Este test verifica:
    // - add_event() incrementa event_count() correctamente
    // - add_event() con unique_ptr y raw pointer funcionan
    // - clear_events() elimina todos los eventos y libera memoria
    // - get_events() retorna referencia válida a la lista
    // - Tipos de eventos se preservan correctamente
    // - nullptr no causa crashes ni cuenta como evento
    // - Secuencias realistas de uso funcionan correctamente
    
    printf("event_manager_test ok (14 tests)\n");
    return 0;
}
