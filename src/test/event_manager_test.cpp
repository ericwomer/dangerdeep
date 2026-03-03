/*
 * Test para event_manager: gestión de eventos del juego.
 * Usa eventos mock concretos para verificar el comportamiento.
 */
#include "catch_amalgamated.hpp"
#include "event_stub.h"
#include "../event_manager.h"
#include <memory>

TEST_CASE("event_manager - Constructor inicializa vacío", "[event_manager]") {
    event_manager em;
    REQUIRE(em.event_count() == 0);
    REQUIRE_FALSE(em.has_events());
    REQUIRE(em.get_events().empty());
}

TEST_CASE("event_manager - clear_events en manager vacío", "[event_manager]") {
    event_manager em;
    em.clear_events();
    REQUIRE(em.event_count() == 0);
}

TEST_CASE("event_manager - Múltiples instancias independientes", "[event_manager]") {
    event_manager em1;
    event_manager em2;
    event_manager em3;
    
    REQUIRE(em1.event_count() == 0);
    REQUIRE(em2.event_count() == 0);
    REQUIRE(em3.event_count() == 0);
    
    em1.clear_events();
    em2.clear_events();
    em3.clear_events();
}

TEST_CASE("event_manager - has_events es consistente con event_count", "[event_manager]") {
    event_manager em;
    REQUIRE(!em.has_events() == (em.event_count() == 0));
}

TEST_CASE("event_manager - get_events retorna referencia constante", "[event_manager]") {
    event_manager em;
    const std::list<std::unique_ptr<event>> &events = em.get_events();
    REQUIRE(events.empty());
    REQUIRE(events.size() == 0);
}

TEST_CASE("event_manager - add_event con unique_ptr", "[event_manager]") {
    event_manager em;
    em.add_event(std::make_unique<test_message_event>("Hello"));
    
    REQUIRE(em.event_count() == 1);
    REQUIRE(em.has_events());
    REQUIRE(em.get_events().size() == 1);
}

TEST_CASE("event_manager - add_event con raw pointer", "[event_manager]") {
    event_manager em;
    em.add_event(new test_signal_event());
    
    REQUIRE(em.event_count() == 1);
}

TEST_CASE("event_manager - add_event múltiples veces", "[event_manager]") {
    event_manager em;
    
    em.add_event(std::make_unique<test_message_event>("Hello"));
    em.add_event(new test_signal_event());
    em.add_event(std::make_unique<test_action_event>(1, 3.14));
    em.add_event(std::make_unique<test_action_event>(2, 2.71));
    em.add_event(std::make_unique<test_message_event>("World"));
    
    REQUIRE(em.event_count() == 5);
    REQUIRE(em.get_events().size() == 5);
}

TEST_CASE("event_manager - clear_events elimina todos", "[event_manager]") {
    event_manager em;
    
    em.add_event(std::make_unique<test_message_event>("Test"));
    em.add_event(std::make_unique<test_signal_event>());
    REQUIRE(em.event_count() == 2);
    
    em.clear_events();
    REQUIRE(em.event_count() == 0);
    REQUIRE_FALSE(em.has_events());
    REQUIRE(em.get_events().empty());
}

TEST_CASE("event_manager - add_event después de clear", "[event_manager]") {
    event_manager em;
    
    em.add_event(std::make_unique<test_signal_event>());
    em.clear_events();
    em.add_event(std::make_unique<test_signal_event>());
    
    REQUIRE(em.event_count() == 1);
}

TEST_CASE("event_manager - Verificar tipos de eventos en la lista", "[event_manager]") {
    event_manager em;
    em.add_event(std::make_unique<test_message_event>("Test1"));
    em.add_event(std::make_unique<test_action_event>(42, 9.99));
    em.add_event(std::make_unique<test_signal_event>());
    
    const auto &events = em.get_events();
    auto it = events.begin();
    
    REQUIRE(it != events.end());
    REQUIRE((*it)->get_type() == "message");
    ++it;
    
    REQUIRE(it != events.end());
    REQUIRE((*it)->get_type() == "action");
    ++it;
    
    REQUIRE(it != events.end());
    REQUIRE((*it)->get_type() == "signal");
    ++it;
    
    REQUIRE(it == events.end());
}

TEST_CASE("event_manager - add_event con nullptr no debe causar problemas", "[event_manager]") {
    event_manager em;
    em.add_event(std::make_unique<test_message_event>("Valid"));
    em.add_event(std::make_unique<test_action_event>(1, 1.0));
    em.add_event(std::make_unique<test_signal_event>());
    
    REQUIRE(em.event_count() == 3);
    
    em.add_event(std::unique_ptr<event>(nullptr));
    REQUIRE(em.event_count() == 3);  // No debe incrementar
    
    em.add_event(static_cast<event*>(nullptr));
    REQUIRE(em.event_count() == 3);  // No debe incrementar
}

TEST_CASE("event_manager - Secuencia realista de uso", "[event_manager][integration]") {
    event_manager em;
    
    SECTION("Frame 1: varios eventos") {
        em.add_event(std::make_unique<test_message_event>("Start"));
        em.add_event(std::make_unique<test_action_event>(1, 1.0));
        REQUIRE(em.event_count() == 2);
        
        // Limpiar después de procesar
        em.clear_events();
        REQUIRE(em.event_count() == 0);
    }
    
    SECTION("Frame 2: más eventos") {
        em.add_event(std::make_unique<test_signal_event>());
        REQUIRE(em.event_count() == 1);
    }
}
