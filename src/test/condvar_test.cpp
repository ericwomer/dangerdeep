/*
 * Test para condvar: creación, signal, timed_wait (con mutex).
 */
#include "../condvar.h"
#include "../mutex.h"
#include <cassert>
#include <cstdio>

int main() {
    condvar cv;
    mutex m;
    m.lock();
    m.unlock();
    cv.signal();
    m.lock();
    bool got = cv.timed_wait(m, 1);
    m.unlock();
    assert(!got);
    printf("condvar_test ok\n");
    return 0;
}
