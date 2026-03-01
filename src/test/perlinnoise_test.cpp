/*
 * Test para perlinnoise.h/cpp: noise_func (si se puede construir) o clase perlinnoise.
 */
#include "../perlinnoise.h"
#include <cassert>
#include <cstdio>

int main() {
    perlinnoise::noise_func nf(4, 1, 0.0f, 0.0f);
    assert(nf.size == 4);
    assert(nf.frequency == 1);
    assert(nf.data.size() == 16u || nf.data.size() >= 4u);
    printf("perlinnoise_test ok\n");
    return 0;
}
