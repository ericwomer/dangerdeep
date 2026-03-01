/*
 * Test para thread.h/cpp: crear thread con new, start, join (threads must be allocated with new).
 */
#include "../thread.h"
#include <cassert>
#include <cstdio>

static int thread_ran = 0;
struct TestThread : thread {
    TestThread() : thread("test_th") {}
    void init() override { thread_ran = 1; request_abort(); }
};

int main() {
    TestThread* t = new TestThread();
    t->start();
    t->join();  // join() does "delete this", do not use t after
    assert(thread_ran == 1);
    printf("thread_test ok\n");
    return 0;
}
