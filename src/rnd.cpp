/*
Danger from the Deep - Open source submarine simulation
Copyright (C) 2003-2006  Thorsten Jordan, Luis Barrancos and others.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
*/

#include "rnd.h"
#include <random>

namespace {
std::mt19937& global_rng() {
    static std::mt19937 rng(0u);
    return rng;
}
}

double rnd() {
    return std::generate_canonical<double, 32>(global_rng());
}

unsigned rnd(unsigned b) {
    if (b == 0) return 0;
    return static_cast<unsigned>(b * rnd());
}

void seed_global_rnd(unsigned seed) {
    global_rng().seed(seed);
}
