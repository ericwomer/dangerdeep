/*
 * Test para faulthandler.h: install_segfault_handler (no provocar crash).
 */
#include "../faulthandler.h"
#include <cassert>
#include <cstdio>

int main() {
    install_segfault_handler();
    printf("faulthandler_test ok\n");
    return 0;
}
