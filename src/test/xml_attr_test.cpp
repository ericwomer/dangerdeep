/*
 * Test para xml_elem::attri() y attrf() (usan std::stoi/std::stod).
 * Crea XML temporal y verifica parseo de enteros y flotantes.
 */
#include "catch_amalgamated.hpp"
#include "../xml.h"
#include <fstream>
#include <string>
#include <unistd.h>

static std::string create_temp_xml() {
    char tmp[] = "/tmp/dftd_xml_attr_test_XXXXXX";
    int fd = mkstemp(tmp);
    REQUIRE(fd >= 0);
    close(fd);
    std::ofstream of(tmp);
    of << "<?xml version=\"1.0\"?>\n"
       << "<root><a value=\"42\"/><b value=\"-17\"/><c value=\"3.14159\"/>"
       << "<d value=\"0.5\"/><e x=\"1\" y=\"2\" z=\"3\"/></root>\n";
    of.close();
    return tmp;
}

TEST_CASE("xml_attr - attri attru", "[xml_attr]") {
    std::string tmp = create_temp_xml();
    xml_doc doc(tmp);
    doc.load();
    xml_elem root = doc.first_child();
    REQUIRE(root.get_name() == "root");

    xml_elem a = root.child("a");
    REQUIRE(a.attri("value") == 42);
    REQUIRE(a.attru("value") == 42u);

    xml_elem b = root.child("b");
    REQUIRE(b.attri("value") == -17);
    unlink(tmp.c_str());
}

TEST_CASE("xml_attr - attrf", "[xml_attr]") {
    std::string tmp = create_temp_xml();
    xml_doc doc(tmp);
    doc.load();
    xml_elem root = doc.first_child();
    xml_elem c = root.child("c");
    double cf = c.attrf("value");
    REQUIRE(cf > 3.14);
    REQUIRE(cf < 3.15);

    xml_elem d = root.child("d");
    REQUIRE(d.attrf("value") == 0.5);
    unlink(tmp.c_str());
}

TEST_CASE("xml_attr - attrv3", "[xml_attr]") {
    std::string tmp = create_temp_xml();
    xml_doc doc(tmp);
    doc.load();
    xml_elem root = doc.first_child();
    xml_elem e = root.child("e");
    REQUIRE(e.attri("x") == 1);
    REQUIRE(e.attri("y") == 2);
    REQUIRE(e.attri("z") == 3);
    vector3 v = e.attrv3();
    REQUIRE(v.x == 1.0);
    REQUIRE(v.y == 2.0);
    REQUIRE(v.z == 3.0);
    unlink(tmp.c_str());
}
