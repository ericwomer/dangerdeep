/*
 * Test para thread.h/cpp: crear thread con new, start, join (threads must be allocated with new).
 */
#include "catch_amalgamated.hpp"
#include "../thread.h"

static int thread_ran = 0;
struct TestThread : thread {
    TestThread() : thread("test_th") {}
    void init() override { thread_ran = 1; request_abort(); }
};

TEST_CASE("thread - start y join", "[thread]") {
    thread_ran = 0;
    TestThread *t = new TestThread();
    t->start();
    t->join();  // join() does "delete this", do not use t after
    REQUIRE(thread_ran == 1);
}
