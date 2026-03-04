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

// a generic object cache
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef OBJCACHE_H
#define OBJCACHE_H

#include <iostream>
#include <map>
#include <memory>
#include <string>

///\brief Handles and caches instances of globally used objects using std::shared_ptr.
///
/// This is a modernized version using C++11 smart pointers for automatic memory management.
/// The cache stores weak_ptr internally so objects are automatically cleaned up when
/// no external references exist. This is exception-safe and prevents memory leaks.
/// For backward compatibility, the API still returns raw pointers but uses shared_ptr internally.
template <class T>
class objcachet {
    std::map<std::string, std::weak_ptr<T>> cache;
    std::map<T *, std::shared_ptr<T>> active_refs; // Track active raw pointer references
    std::string basedir;

    objcachet() = delete;
    objcachet<T> &operator=(const objcachet<T> &) = delete;
    objcachet(const objcachet<T> &) = delete;

  public:
    objcachet(const std::string &basedir_) : basedir(basedir_) {}

    ~objcachet() {
        clear();
    }

    // call to deinit cache
    void clear() {
        active_refs.clear();
        cache.clear();
    }

    // Find object in cache, returns nullptr if not found or expired
    T *find(const std::string &objname) {
        if (objname.empty())
            return nullptr;

        auto it = cache.find(objname);
        if (it == cache.end())
            return nullptr;

        if (auto sp = it->second.lock()) {
            return sp.get();
        }
        return nullptr;
    }

    // Get shared_ptr for proper reference counting (each caller holds their own ref)
    std::shared_ptr<T> ref_shared(const std::string &objname) {
        if (objname.empty())
            return nullptr;

        auto it = cache.find(objname);
        if (it != cache.end()) {
            if (auto sp = it->second.lock()) {
                return sp;
            }
            cache.erase(it);
        }

        auto sp = std::make_shared<T>(basedir + objname);
        cache[objname] = sp;
        return sp;
    }

    // Get or create object (returns raw pointer; for shared objects use reference with ref_shared)
    T *ref(const std::string &objname) {
        auto sp = ref_shared(objname);
        active_refs[sp.get()] = sp;  // keep alive until unref (legacy single-ref use)
        return sp.get();
    }

    // Insert externally created object into cache
    bool ref(const std::string &objname, T *obj) {
        if (objname.empty() || !obj)
            return false;

        auto it = cache.find(objname);
        if (it != cache.end()) {
            // Check if still valid
            if (auto sp = it->second.lock()) {
                return false; // already exists
            }
        }

        // Wrap raw pointer in shared_ptr with custom deleter to prevent double delete
        auto sp = std::shared_ptr<T>(obj, [](T *) {});
        cache[objname] = sp;
        active_refs[obj] = sp;
        return true;
    }

    // Decrement reference count
    void unref(const std::string &objname) {
        if (objname.empty())
            return;

        auto it = cache.find(objname);
        if (it != cache.end()) {
            if (auto sp = it->second.lock()) {
                active_refs.erase(sp.get());
            }
            if (it->second.expired()) {
                cache.erase(it);
            }
        }
    }

    // Decrement reference count by pointer
    void unref(T *obj) {
        if (!obj)
            return;

        auto it = active_refs.find(obj);
        if (it != active_refs.end()) {
            active_refs.erase(it);
        }
    }

    void print() const {
        std::cout << "objcache: " << cache.size() << " entries.\n";
        for (const auto &entry : cache) {
            auto sp = entry.second.lock();
            std::cout << "key=\"" << entry.first << "\" valid=" << (sp ? "yes" : "no")
                      << " use_count=" << (sp ? sp.use_count() : 0)
                      << " addr=" << (sp ? sp.get() : nullptr) << "\n";
        }
    }

    // RAII reference handler - holds shared_ptr for correct refcount when multiple objects share
    class reference {
        std::shared_ptr<T> mysp;

      public:
        // Constructor that loads from cache
        reference(objcachet<T> &cache, const std::string &objname)
            : mysp(cache.ref_shared(objname)) {}

        // Default constructor (empty reference)
        reference() : mysp(nullptr) {}

        // Disable copy
        reference(const reference &) = delete;
        reference &operator=(const reference &) = delete;

        // Enable move
        reference(reference &&other) noexcept : mysp(std::move(other.mysp)) {}

        reference &operator=(reference &&other) noexcept {
            if (this != &other) {
                mysp = std::move(other.mysp);
            }
            return *this;
        }

        ~reference() = default;

        void reset() { mysp.reset(); }

        // Load a new object (releases old one if present)
        void load(objcachet<T> &cache, const std::string &objname) {
            mysp = cache.ref_shared(objname);
        }

        T *get() { return mysp.get(); }
        const T *get() const { return mysp.get(); }

        // Additional access methods
        T &operator*() { return *mysp; }
        const T &operator*() const { return *mysp; }
        T *operator->() { return mysp.get(); }
        const T *operator->() const { return mysp.get(); }

        explicit operator bool() const { return mysp != nullptr; }
    };

    // Convenient alias for RAII handles
    using ref_ptr = reference;
};

#endif
