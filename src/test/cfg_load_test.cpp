/*
 * Test para cfg::load() y set_str (conversión stoi/stod).
 * Verifica que las opciones int, float, bool se parsean correctamente desde XML.
 */
#include "catch_amalgamated.hpp"
#include "../cfg.h"
#include <fstream>
#include <string>
#include <unistd.h>

static std::string create_temp_cfg() {
    char tmp[] = "/tmp/dftd_cfg_load_test_XXXXXX";
    int fd = mkstemp(tmp);
    REQUIRE(fd >= 0);
    close(fd);
    std::ofstream of(tmp);
    of << "<?xml version=\"1.0\"?>\n"
       << "<dftd-cfg>\n"
       << "  <opt_bool value=\"true\"/>\n"
       << "  <opt_int value=\"-42\"/>\n"
       << "  <opt_float value=\"3.14\"/>\n"
       << "  <opt_str value=\"hello\"/>\n"
       << "  <opt_bool2 value=\"1\"/>\n"
       << "  <opt_bool3 value=\"yes\"/>\n"
       << "  <opt_bool4 value=\"false\"/>\n"
       << "</dftd-cfg>\n";
    of.close();
    return tmp;
}

TEST_CASE("cfg_load - parseo int float bool string", "[cfg_load]") {
    cfg::destroy_instance();
    std::string tmp = create_temp_cfg();
    cfg &c = cfg::instance();
    c.register_option("opt_bool", false);
    c.register_option("opt_int", 0);
    c.register_option("opt_float", 0.0f);
    c.register_option("opt_str", std::string(""));
    c.register_option("opt_bool2", false);
    c.register_option("opt_bool3", false);
    c.register_option("opt_bool4", true);

    c.load(tmp);
    unlink(tmp.c_str());

    REQUIRE(c.getb("opt_bool") == true);
    REQUIRE(c.geti("opt_int") == -42);
    REQUIRE(c.getf("opt_float") > 3.13f);
    REQUIRE(c.getf("opt_float") < 3.15f);
    REQUIRE(c.gets("opt_str") == "hello");
    REQUIRE(c.getb("opt_bool2") == true);   // "1" -> stoi -> true
    REQUIRE(c.getb("opt_bool3") == true);   // "yes" -> true directo
    REQUIRE(c.getb("opt_bool4") == false);

    cfg::destroy_instance();
}

TEST_CASE("cfg_load - save y roundtrip load", "[cfg_load]") {
    cfg::destroy_instance();
    char tmp[] = "/tmp/dftd_cfg_save_test_XXXXXX";
    int fd = mkstemp(tmp);
    REQUIRE(fd >= 0);
    close(fd);
    std::string path(tmp);

    cfg &c = cfg::instance();
    c.register_option("s_bool", true);
    c.register_option("s_int", 99);
    c.register_option("s_float", 2.71f);
    c.register_option("s_str", std::string("world"));
    c.set("s_bool", false);
    c.set("s_int", -7);
    c.set("s_float", 1.41f);
    c.set("s_str", std::string("saved"));

    c.save(path);

    cfg::destroy_instance();
    cfg &c2 = cfg::instance();
    c2.register_option("s_bool", true);
    c2.register_option("s_int", 0);
    c2.register_option("s_float", 0.0f);
    c2.register_option("s_str", std::string(""));
    c2.load(path);
    unlink(path.c_str());

    REQUIRE(c2.getb("s_bool") == false);
    REQUIRE(c2.geti("s_int") == -7);
    REQUIRE(c2.getf("s_float") > 1.40f);
    REQUIRE(c2.getf("s_float") < 1.42f);
    REQUIRE(c2.gets("s_str") == "saved");

    cfg::destroy_instance();
}
