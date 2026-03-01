/*
 * Test para singleton.h: instance(), destroy_instance(), create_instance().
 */
#include "../singleton.h"
#include <cassert>
#include <cstdio>

struct Counted {
    static int constructions;
    Counted() { ++constructions; }
};
int Counted::constructions = 0;

int main() {
    assert(Counted::constructions == 0);
    Counted &a = singleton<Counted>::instance();
    assert(Counted::constructions == 1);
    Counted &b = singleton<Counted>::instance();
    assert(&a == &b);
    assert(Counted::constructions == 1);

    singleton<Counted>::destroy_instance();
    assert(Counted::constructions == 1);
    (void)singleton<Counted>::instance();
    assert(Counted::constructions == 2);

    Counted *p = new Counted();
    singleton<Counted>::create_instance(p);
    assert(&singleton<Counted>::instance() == p);
    singleton<Counted>::destroy_instance();

    printf("singleton_test ok\n");
    return 0;
}
