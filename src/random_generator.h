/*
Danger from the Deep - Open source submarine simulation
Copyright (C) 2003-2006  Thorsten Jordan, Luis Barrancos and others.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

//
//  Wrapper sobre la librería estándar de aleatorios C++11 (<random>).
//  Mantiene la interfaz rnd() / rndf() / set_seed() para no romper call sites.
//

#ifndef RANDOM_GENERATOR_H
#define RANDOM_GENERATOR_H

#include <random>

class random_generator {
  public:
    random_generator(unsigned seed = 0) : gen(seed) {}
    virtual ~random_generator() = default;

    virtual unsigned rnd() {
        return static_cast<unsigned>(gen());
    }

    virtual float rndf() {
        return static_cast<float>(gen() - gen.min()) / (static_cast<float>(gen.max() - gen.min()) + 1.0f);
    }

    virtual void set_seed(unsigned seed) { gen.seed(seed); }

  protected:
    std::mt19937 gen;
};

#endif
