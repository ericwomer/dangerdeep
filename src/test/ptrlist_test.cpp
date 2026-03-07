/*
 * Test para ptrlist.h: push_back, iteración, destructor.
 */
#include "catch_amalgamated.hpp"
#include "../ptrlist.h"
#include <cstdio>

struct A {
    int x;
    A(int y = 0) : x(y) {}
    void dodo() const { (void)x; }
};

TEST_CASE("ptrlist - push_back e iteración", "[ptrlist]") {
    ptrlist<A> foo;
    foo.push_back(new A(1));
    foo.push_back(new A(3));
    foo.push_back(new A(5));
    foo.push_back(new A(7));

    size_t count = 0;
    for (ptrlist<A>::const_iterator it = foo.begin(); it != foo.end(); ++it) {
        it->dodo();
        ++count;
    }
    REQUIRE(count == 4);
}
