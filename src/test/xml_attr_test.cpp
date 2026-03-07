/*
 * Test para xml_elem::attri() y attrf() (usan std::stoi/std::stod).
 * Crea XML temporal y verifica parseo de enteros y flotantes.
 */
#include "catch_amalgamated.hpp"
#include "../xml.h"
#include <cmath>
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
       << "<d value=\"0.5\"/><e x=\"1\" y=\"2\" z=\"3\"/>"
       << "<f x=\"1.5\" y=\"2.5\"/><g s=\"0.707\" x=\"0\" y=\"0\" z=\"0.707\"/>"
       << "<h angle=\"90\"/><i value=\"1\"/><j value=\"0\"/>"
       << "<items><item id=\"1\"/><item id=\"2\"/></items></root>\n";
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

TEST_CASE("xml_attr - attr, has_attr, has_child", "[xml_attr]") {
    std::string tmp = create_temp_xml();
    xml_doc doc(tmp);
    doc.load();
    xml_elem root = doc.first_child();
    xml_elem a = root.child("a");
    REQUIRE(a.attr("value") == "42");
    REQUIRE(a.has_attr("value"));
    REQUIRE_FALSE(a.has_attr("missing"));
    REQUIRE(root.has_child("a"));
    REQUIRE(root.has_child("items"));
    REQUIRE_FALSE(root.has_child("nonexistent"));
    unlink(tmp.c_str());
}

TEST_CASE("xml_attr - attrv2, attrq, attra, attrb", "[xml_attr]") {
    std::string tmp = create_temp_xml();
    xml_doc doc(tmp);
    doc.load();
    xml_elem root = doc.first_child();

    xml_elem f = root.child("f");
    vector2 v2 = f.attrv2();
    REQUIRE(v2.x == 1.5);
    REQUIRE(v2.y == 2.5);

    xml_elem g = root.child("g");
    quaternion q = g.attrq();
    REQUIRE(std::abs(q.s - 0.707) < 0.01);

    xml_elem h = root.child("h");
    angle ang = h.attra();
    REQUIRE(std::abs(ang.value() - 90.0) < 0.01);

    xml_elem i = root.child("i");
    REQUIRE(i.attrb("value") == true);
    xml_elem j = root.child("j");
    REQUIRE(j.attrb("value") == false);
    unlink(tmp.c_str());
}

TEST_CASE("xml_attr - iterate", "[xml_attr]") {
    std::string tmp = create_temp_xml();
    xml_doc doc(tmp);
    doc.load();
    xml_elem root = doc.first_child();
    xml_elem items = root.child("items");
    int count = 0;
    for (xml_elem::iterator it = items.iterate("item"); !it.end(); it.next()) {
        REQUIRE(it.elem().attri("id") == count + 1);
        ++count;
    }
    REQUIRE(count == 2);
    unlink(tmp.c_str());
}
