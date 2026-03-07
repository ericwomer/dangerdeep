/*
 * Test para condvar: creación, signal, timed_wait (con mutex).
 */
#include "catch_amalgamated.hpp"
#include "../condvar.h"
#include "../mutex.h"

TEST_CASE("condvar - creación, signal, timed_wait", "[condvar]") {
    condvar cv;
    mutex m;
    m.lock();
    m.unlock();
    cv.signal();
    m.lock();
    bool got = cv.timed_wait(m, 1);
    m.unlock();
    REQUIRE_FALSE(got);
}
