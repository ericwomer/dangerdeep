/*
 * Test para objcache.h: find, ref, unref, clear con tipo dummy.
 *
 * Tests que capturan el bug use-after-free corregido (sea_object/model):
 * - ref_shared_multiple_owners: múltiples shared_ptr, cada uno cuenta como ref
 * - reference_shared_survives_partial_destruction: N referencias compartidas,
 *   destruir una por una; las restantes deben seguir accediendo al objeto
 * - reference_destruction_order_like_world: orden como ~world (ships vector)
 *
 * Con el bug anterior (reference usaba raw ptr + unref), el primer unref
 * liberaba el modelo compartido y causaba use-after-free en unregister_layout.
 * Con ASan: heap-use-after-free en model::unregister_layout.
 */
#include "../objcache.h"
#include <cassert>
#include <cstdio>
#include <memory>
#include <string>

struct DummyResource {
    std::string path;
    DummyResource(const std::string &p) : path(p) {}
};

// Recurso que cuenta instancias vivas - detecta use-after-free y fugas
struct CountedResource {
    std::string path;
    static int alive_count;

    CountedResource(const std::string &p) : path(p) { ++alive_count; }
    ~CountedResource() { --alive_count; }
};
int CountedResource::alive_count = 0;

static void test_basic_ref_unref() {
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
}

// ref_shared: múltiples shared_ptr al mismo objeto; cada uno mantiene vivo
static void test_ref_shared_multiple_owners() {
    objcachet<CountedResource> cache("/");
    assert(CountedResource::alive_count == 0);

    std::shared_ptr<CountedResource> sp1 = cache.ref_shared("shared");
    assert(sp1 != nullptr);
    assert(CountedResource::alive_count == 1);

    std::shared_ptr<CountedResource> sp2 = cache.ref_shared("shared");
    assert(sp2 != nullptr);
    assert(sp2.get() == sp1.get());
    assert(CountedResource::alive_count == 1);

    sp1.reset();
    assert(CountedResource::alive_count == 1);  // sp2 sigue vivo
    assert(sp2->path == "/shared");

    sp2.reset();
    assert(CountedResource::alive_count == 0);
}

// reference: N referencias al mismo objeto; destruir una por una
// simula N sea_objects compartiendo un modelo - cada destructor debe
// poder acceder al objeto hasta que sea el último en destruirse
static void test_reference_shared_survives_partial_destruction() {
    objcachet<CountedResource> cache("/");
    assert(CountedResource::alive_count == 0);

    objcachet<CountedResource>::reference ref1(cache, "multi");
    objcachet<CountedResource>::reference ref2(cache, "multi");
    objcachet<CountedResource>::reference ref3(cache, "multi");

    assert(ref1.get() == ref2.get());
    assert(ref2.get() == ref3.get());
    assert(CountedResource::alive_count == 1);

    // Simular uso durante vida (como mymodel->unregister_layout)
    (void)ref1->path;
    (void)ref2->path;

    // Destruir ref1: objeto debe seguir vivo (ref2 y ref3 lo mantienen)
    ref1.reset();
    assert(CountedResource::alive_count == 1);
    assert(ref2->path == "/multi");
    assert(ref3->path == "/multi");

    // Destruir ref2: objeto sigue vivo
    ref2.reset();
    assert(CountedResource::alive_count == 1);
    assert(ref3->path == "/multi");

    // ref3 sale de scope -> última referencia -> destructor
    ref3.reset();
    assert(CountedResource::alive_count == 0);
}

// reference: destrucción en orden inverso (como vector de ships)
// ref3, ref2, ref1 - cada destructor usa el objeto
static void test_reference_destruction_order_like_world() {
    objcachet<CountedResource> cache("/");
    assert(CountedResource::alive_count == 0);

    objcachet<CountedResource>::reference ref1(cache, "order");
    objcachet<CountedResource>::reference ref2(cache, "order");
    objcachet<CountedResource>::reference ref3(cache, "order");
    assert(CountedResource::alive_count == 1);

    // Simular destructores de ship: cada uno llama mymodel->unregister_layout
    // al destruir ref3 (primero en vector), objeto sigue vivo
    ref3.reset();
    assert(CountedResource::alive_count == 1);
    assert(ref1->path == "/order");
    assert(ref2->path == "/order");

    ref2.reset();
    assert(CountedResource::alive_count == 1);
    assert(ref1->path == "/order");

    ref1.reset();
    assert(CountedResource::alive_count == 0);
}

// reference::load y move
static void test_reference_load_and_move() {
    objcachet<CountedResource> cache("/");
    objcachet<CountedResource>::reference ref1(cache, "a");
    assert(ref1 && ref1->path == "/a");

    ref1.load(cache, "b");
    assert(ref1 && ref1->path == "/b");

    objcachet<CountedResource>::reference ref2(std::move(ref1));
    assert(!ref1);
    assert(ref2 && ref2->path == "/b");
}

int main() {
    test_basic_ref_unref();
    test_ref_shared_multiple_owners();
    test_reference_shared_survives_partial_destruction();
    test_reference_destruction_order_like_world();
    test_reference_load_and_move();

    printf("objcache_test ok\n");
    return 0;
}
