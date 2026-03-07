/*
 * Test para mutex.h: lock recursivo, unlock.
 */
#include "catch_amalgamated.hpp"
#include "../mutex.h"

TEST_CASE("mutex - lock recursivo y unlock", "[mutex]") {
    mutex m;
    m.lock();
    m.lock();  // recursivo
    m.unlock();
    m.unlock();
    REQUIRE(true);
}
