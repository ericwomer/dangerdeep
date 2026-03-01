/* Test cfg.h/cpp: cfg::instance() (config). */
#include "../cfg.h"
#include <cstdio>
int main() {
    (void)cfg::instance();
    printf("cfg_test ok\n");
    return 0;
}
