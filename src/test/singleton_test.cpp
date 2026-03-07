/*
 * Test para singleton.h: instance(), destroy_instance(), create_instance().
 */
#include "catch_amalgamated.hpp"
#include "../singleton.h"

struct Counted {
    static int constructions;
    Counted() { ++constructions; }
    ~Counted() { --constructions; }
};
int Counted::constructions = 0;

TEST_CASE("singleton - instance, destroy, create_instance", "[singleton]") {
    singleton<Counted>::destroy_instance();
    Counted::constructions = 0;

    SECTION("instance crea y retorna misma referencia") {
        Counted &a = singleton<Counted>::instance();
        REQUIRE(Counted::constructions == 1);
        Counted &b = singleton<Counted>::instance();
        REQUIRE(&a == &b);
        REQUIRE(Counted::constructions == 1);
    }

    SECTION("destroy_instance y recreación") {
        (void)singleton<Counted>::instance();
        REQUIRE(Counted::constructions == 1);
        singleton<Counted>::destroy_instance();
        REQUIRE(Counted::constructions == 0);
        (void)singleton<Counted>::instance();
        REQUIRE(Counted::constructions == 1);
    }

    SECTION("create_instance con puntero") {
        singleton<Counted>::destroy_instance();
        Counted *p = new Counted();
        singleton<Counted>::create_instance(p);
        REQUIRE(&singleton<Counted>::instance() == p);
    }

    singleton<Counted>::destroy_instance();
}
