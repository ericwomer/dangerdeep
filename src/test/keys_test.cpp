/*
 * Test para keys: key_names tiene NR_OF_KEY_IDS entradas y cada .nr coincide.
 */
#include "../keys.h"
#include <cassert>
#include <cstdio>

int main() {
    for (unsigned i = 0; i < NR_OF_KEY_IDS; ++i) {
        assert(key_names[i].nr == i);
        assert(key_names[i].name != nullptr);
        assert(key_names[i].name[0] != '\0');
    }
    printf("keys_test ok\n");
    return 0;
}
