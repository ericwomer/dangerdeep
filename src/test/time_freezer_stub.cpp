/*
 * Stub mínimo para time_freezer (solo para testing, sin system dependency)
 */
#include "../time_freezer.h"

// Constructor
time_freezer::time_freezer() : freezetime(0), freezetime_start(0) {
}

// Métodos que requieren system (no los implementamos, el test no los usa)
void time_freezer::freeze() {
    // No implementado - el test no debe llamar esto
}

void time_freezer::unfreeze() {
    // No implementado - el test no debe llamar esto
}
