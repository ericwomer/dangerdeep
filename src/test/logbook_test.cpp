/* Test logbook.h/cpp */
#include "../logbook.h"
#include <cassert>
#include <cstdio>
int main() {
    logbook lb;
    lb.add_entry("entry1");
    assert(lb.size() == 1);
    assert(*lb.get_entry(0) == "entry1");
    printf("logbook_test ok\n");
    return 0;
}
