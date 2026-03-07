/*
 * Test para objcache.h: find, ref, unref, clear, ref_shared, reference.
 */
#include "catch_amalgamated.hpp"
#include "../objcache.h"
#include <memory>
#include <string>

struct DummyResource {
    std::string path;
    DummyResource(const std::string &p) : path(p) {}
};

struct CountedResource {
    std::string path;
    static int alive_count;

    CountedResource(const std::string &p) : path(p) { ++alive_count; }
    ~CountedResource() { --alive_count; }
};
int CountedResource::alive_count = 0;

TEST_CASE("objcache - basic ref/unref", "[objcache]") {
    objcachet<DummyResource> cache("/base/");
    REQUIRE(cache.find("") == nullptr);
    REQUIRE(cache.find("x") == nullptr);

    DummyResource *r = cache.ref("foo");
    REQUIRE(r != nullptr);
    REQUIRE(r->path == "/base/foo");
    REQUIRE(cache.find("foo") == r);
    DummyResource *r2 = cache.ref("foo");
    REQUIRE(r2 == r);

    cache.unref("foo");
    cache.unref("foo");
    REQUIRE(cache.find("foo") == nullptr);

    cache.ref("a");
    cache.ref("b");
    cache.clear();
    REQUIRE(cache.find("a") == nullptr);
    REQUIRE(cache.find("b") == nullptr);
}

TEST_CASE("objcache - ref_shared múltiples owners", "[objcache]") {
    objcachet<CountedResource> cache("/");
    REQUIRE(CountedResource::alive_count == 0);

    std::shared_ptr<CountedResource> sp1 = cache.ref_shared("shared");
    REQUIRE(sp1 != nullptr);
    REQUIRE(CountedResource::alive_count == 1);

    std::shared_ptr<CountedResource> sp2 = cache.ref_shared("shared");
    REQUIRE(sp2 != nullptr);
    REQUIRE(sp2.get() == sp1.get());
    REQUIRE(CountedResource::alive_count == 1);

    sp1.reset();
    REQUIRE(CountedResource::alive_count == 1);
    REQUIRE(sp2->path == "/shared");

    sp2.reset();
    REQUIRE(CountedResource::alive_count == 0);
}

TEST_CASE("objcache - reference sobrevive destrucción parcial", "[objcache]") {
    objcachet<CountedResource> cache("/");
    REQUIRE(CountedResource::alive_count == 0);

    objcachet<CountedResource>::reference ref1(cache, "multi");
    objcachet<CountedResource>::reference ref2(cache, "multi");
    objcachet<CountedResource>::reference ref3(cache, "multi");

    REQUIRE(ref1.get() == ref2.get());
    REQUIRE(ref2.get() == ref3.get());
    REQUIRE(CountedResource::alive_count == 1);

    (void)ref1->path;
    (void)ref2->path;

    ref1.reset();
    REQUIRE(CountedResource::alive_count == 1);
    REQUIRE(ref2->path == "/multi");
    REQUIRE(ref3->path == "/multi");

    ref2.reset();
    REQUIRE(CountedResource::alive_count == 1);
    REQUIRE(ref3->path == "/multi");

    ref3.reset();
    REQUIRE(CountedResource::alive_count == 0);
}

TEST_CASE("objcache - reference orden destrucción como world", "[objcache]") {
    objcachet<CountedResource> cache("/");
    REQUIRE(CountedResource::alive_count == 0);

    objcachet<CountedResource>::reference ref1(cache, "order");
    objcachet<CountedResource>::reference ref2(cache, "order");
    objcachet<CountedResource>::reference ref3(cache, "order");
    REQUIRE(CountedResource::alive_count == 1);

    ref3.reset();
    REQUIRE(CountedResource::alive_count == 1);
    REQUIRE(ref1->path == "/order");
    REQUIRE(ref2->path == "/order");

    ref2.reset();
    REQUIRE(CountedResource::alive_count == 1);
    REQUIRE(ref1->path == "/order");

    ref1.reset();
    REQUIRE(CountedResource::alive_count == 0);
}

TEST_CASE("objcache - reference load y move", "[objcache]") {
    objcachet<CountedResource> cache("/");
    objcachet<CountedResource>::reference ref1(cache, "a");
    REQUIRE(ref1);
    REQUIRE(ref1->path == "/a");

    ref1.load(cache, "b");
    REQUIRE(ref1);
    REQUIRE(ref1->path == "/b");

    objcachet<CountedResource>::reference ref2(std::move(ref1));
    REQUIRE_FALSE(ref1);
    REQUIRE(ref2);
    REQUIRE(ref2->path == "/b");
}
