/*
 * Test para objcache.h: find, ref, unref, clear con tipo dummy.
 */
#include "../objcache.h"
#include <cassert>
#include <cstdio>
#include <string>

struct DummyResource {
    std::string path;
    DummyResource(const std::string &p) : path(p) {}
};

int main() {
    objcachet<DummyResource> cache("/base/");
    assert(cache.find("") == nullptr);
    assert(cache.find("x") == nullptr);

    DummyResource *r = cache.ref("foo");
    assert(r != nullptr);
    assert(r->path == "/base/foo");
    assert(cache.find("foo") == r);
    DummyResource *r2 = cache.ref("foo");
    assert(r2 == r);

    cache.unref("foo");
    cache.unref("foo");
    assert(cache.find("foo") == nullptr);

    cache.ref("a");
    cache.ref("b");
    cache.clear();
    assert(cache.find("a") == nullptr);
    assert(cache.find("b") == nullptr);

    printf("objcache_test ok\n");
    return 0;
}
