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

TEST_CASE("xml_attr - child inexistente lanza xml_elem_error", "[xml_attr]") {
    std::string tmp = create_temp_xml();
    xml_doc doc(tmp);
    doc.load();
    xml_elem root = doc.first_child();
    REQUIRE_THROWS_AS(root.child("nonexistent"), xml_elem_error);
    unlink(tmp.c_str());
}

TEST_CASE("xml_attr - add_child_text y child_text", "[xml_attr]") {
    char tmp[] = "/tmp/dftd_xml_text_XXXXXX";
    int fd = mkstemp(tmp);
    REQUIRE(fd >= 0);
    close(fd);
    {
        xml_doc doc(tmp);
        xml_elem root = doc.add_child("root");
        xml_elem n = root.add_child("node");
        n.add_child_text("hello world");
        doc.save();
    }
    {
        xml_doc doc(tmp);
        doc.load();
        xml_elem root = doc.first_child();
        xml_elem n = root.child("node");
        REQUIRE(n.child_text() == "hello world");
    }
    unlink(tmp);
}

TEST_CASE("xml_attr - attri attrf sin atributo retornan 0", "[xml_attr]") {
    std::string tmp = create_temp_xml();
    xml_doc doc(tmp);
    doc.load();
    xml_elem root = doc.first_child();
    xml_elem a = root.child("a");
    REQUIRE(a.attri("missing") == 0);
    REQUIRE(a.attrf("missing") == 0.0);
    REQUIRE(a.attru("missing") == 0u);
    unlink(tmp.c_str());
}

TEST_CASE("xml_attr - attr sin atributo retorna vacío", "[xml_attr]") {
    std::string tmp = create_temp_xml();
    xml_doc doc(tmp);
    doc.load();
    xml_elem root = doc.first_child();
    xml_elem a = root.child("a");
    REQUIRE(a.attr("missing") == "");
    unlink(tmp.c_str());
}

TEST_CASE("xml_attr - set_attr y roundtrip", "[xml_attr]") {
    char tmp[] = "/tmp/dftd_xml_setattr_XXXXXX";
    int fd = mkstemp(tmp);
    REQUIRE(fd >= 0);
    close(fd);
    {
        xml_doc doc(tmp);
        xml_elem root = doc.add_child("root");
        xml_elem n = root.add_child("n");
        n.set_attr(123, "i");
        n.set_attr(456u, "u");
        n.set_attr(-3.14, "f");
        n.set_attr(std::string("hello"), "s");
        n.set_attr(vector3(1, 2, 3));
        xml_elem n2 = root.add_child("n2");
        n2.set_attr(vector2(4.5, 6.5));
        n2.set_attr(angle(90));
        n2.set_attr(true, "flag");
        doc.save();
    }
    {
        xml_doc doc(tmp);
        doc.load();
        xml_elem root = doc.first_child();
        xml_elem n = root.child("n");
        REQUIRE(n.attri("i") == 123);
        REQUIRE(n.attru("u") == 456u);
        REQUIRE(std::abs(n.attrf("f") - (-3.14)) < 0.01);
        REQUIRE(n.attr("s") == "hello");
        vector3 v3 = n.attrv3();
        REQUIRE(v3.x == 1.0);
        REQUIRE(v3.y == 2.0);
        REQUIRE(v3.z == 3.0);
        xml_elem n2 = root.child("n2");
        vector2 v2 = n2.attrv2();
        REQUIRE(v2.x == 4.5);
        REQUIRE(v2.y == 6.5);
        REQUIRE(n2.attrb("flag") == true);
    }
    unlink(tmp);
}

TEST_CASE("xml_attr - child_text sin hijo lanza", "[xml_attr]") {
    std::string tmp = create_temp_xml();
    xml_doc doc(tmp);
    doc.load();
    xml_elem root = doc.first_child();
    xml_elem a = root.child("a");
    REQUIRE_THROWS_AS(a.child_text(), xml_error);
    unlink(tmp.c_str());
}

TEST_CASE("xml_attr - iterate sin childname", "[xml_attr]") {
    std::string tmp = create_temp_xml();
    xml_doc doc(tmp);
    doc.load();
    xml_elem root = doc.first_child();
    int count = 0;
    for (xml_elem::iterator it = root.iterate(); !it.end(); it.next()) {
        REQUIRE(it.elem().get_name().size() > 0);
        ++count;
    }
    REQUIRE(count >= 2);
    unlink(tmp.c_str());
}

TEST_CASE("xml_attr - xml_doc child inexistente lanza", "[xml_attr]") {
    std::string tmp = create_temp_xml();
    xml_doc doc(tmp);
    doc.load();
    REQUIRE_THROWS_AS(doc.child("nonexistent"), xml_elem_error);
    unlink(tmp.c_str());
}

TEST_CASE("xml_attr - get_filename", "[xml_attr]") {
    std::string tmp = create_temp_xml();
    xml_doc doc(tmp);
    REQUIRE(doc.get_filename() == tmp);
    doc.load();
    REQUIRE(doc.get_filename() == tmp);
    unlink(tmp.c_str());
}
