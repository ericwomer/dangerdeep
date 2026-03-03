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

// this class holds the game's configuration
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef SINGLETON_H
#define SINGLETON_H

#include <atomic>
#include <memory>

template <typename D>
class singleton {
  private:
    // Use atomic pointer for thread-safe lazy initialization
    static std::atomic<D *> &instance_ptr() {
        static std::atomic<D *> myinstanceptr{nullptr};
        return myinstanceptr;
    }

  public:
    // Thread-safe lazy initialization using atomic operations
    // This is an improvement over the old version which wasn't thread-safe
    static D &instance() {
        std::atomic<D *> &p = instance_ptr();
        D *inst = p.load(std::memory_order_acquire);

        if (!inst) {
            // Double-checked locking pattern for thread safety
            // Note: In C++11+, static local variables are guaranteed thread-safe
            // but we keep this pattern for compatibility with the existing API
            D *new_inst = new D();
            D *expected = nullptr;
            if (p.compare_exchange_strong(expected, new_inst, std::memory_order_release)) {
                inst = new_inst;
            } else {
                // Another thread created the instance
                delete new_inst;
                inst = expected;
            }
        }
        return *inst;
    }

    static void create_instance(D *ptr) {
        std::atomic<D *> &p = instance_ptr();
        D *old = p.exchange(ptr, std::memory_order_acq_rel);
        delete old;
    }

    static void destroy_instance() {
        std::atomic<D *> &p = instance_ptr();
        D *old = p.exchange(nullptr, std::memory_order_acq_rel);
        delete old;
    }

    static D *release_instance() {
        std::atomic<D *> &p = instance_ptr();
        return p.exchange(nullptr, std::memory_order_acq_rel);
    }

  protected:
    singleton() {}

  private:
    singleton(const singleton &) = delete;
    singleton &operator=(const singleton &) = delete;
};

#endif
