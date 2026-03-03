/*
 * Stub mínimo de system para testing time_freezer
 */
#include "../system.h"
#include <chrono>

// Variable global para simular el tiempo del sistema
static unsigned simulated_time_ms = 0;

// Stub del singleton system
class system_stub : public system {
  public:
    system_stub() : system({}) {}
    
    unsigned millisec() override {
        // Retornar tiempo simulado incrementado
        return simulated_time_ms += 10;  // Simular 10ms por llamada
    }
};

// Implementación del singleton sys() que retorna un stub
system &sys() {
    static system_stub instance;
    return instance;
}

// Reset para tests
void reset_simulated_time() {
    simulated_time_ms = 0;
}
