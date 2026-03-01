/* Test system.h/cpp: sys().millisec() */
#include "../system.h"
#include <cassert>
#include <cstdio>
int main() {
    unsigned t = sys().millisec();
    (void)t;
    printf("system_test ok\n");
    return 0;
}
