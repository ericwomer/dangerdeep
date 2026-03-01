/*
 * Test para cfg.h: struct cfg::key (constructor, operator=).
 */
#include "../cfg.h"
#include <cassert>
#include <cstdio>

int main() {
    cfg::key k1;
    assert(k1.keysym == SDLK_UNKNOWN);
    assert(!k1.ctrl && !k1.alt && !k1.shift);

    cfg::key k2("action", SDLK_a, true, false, true);
    assert(k2.action == "action");
    assert(k2.keysym == SDLK_a);
    assert(k2.ctrl && !k2.alt && k2.shift);

    k1 = k2;
    assert(k1.action == k2.action && k1.keysym == k2.keysym);

    printf("cfg_key_test ok\n");
    return 0;
}
