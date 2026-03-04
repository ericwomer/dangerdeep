/*
 * Test para cfg.h: struct cfg::key (constructor, operator=).
 * key_code usa valores SDLK_*; 0=UNKNOWN, 'a'=SDLK_a.
 */
#include "../cfg.h"
#include <cassert>
#include <cstdio>

int main() {
    cfg::key k1;
    assert(k1.keysym == 0);
    assert(!k1.ctrl && !k1.alt && !k1.shift);

    cfg::key k2("action", key_code('a'), true, false, true);
    assert(k2.action == "action");
    assert(k2.keysym == key_code('a'));
    assert(k2.ctrl && !k2.alt && k2.shift);

    k1 = k2;
    assert(k1.action == k2.action && k1.keysym == k2.keysym);

    printf("cfg_key_test ok\n");
    return 0;
}
