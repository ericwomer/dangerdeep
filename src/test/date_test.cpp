/*
 * Test para date: representación de fechas (tiempo lineal desde 1.1.1939).
 */
#include "catch_amalgamated.hpp"
#include "../date.h"
#include "../xml.h"
#include <cstdio>
#include <unistd.h>

TEST_CASE("date - length_of_year", "[date]") {
    REQUIRE(date::length_of_year(1939) == 365);
    REQUIRE(date::length_of_year(1940) == 366);  // bisiesto
    REQUIRE(date::length_of_year(1900) == 365); // divisible por 100, no por 400
    REQUIRE(date::length_of_year(2000) == 366); // divisible por 400
}

TEST_CASE("date - length_of_month", "[date]") {
    REQUIRE(date::length_of_month(1939, date::January) == 31);
    REQUIRE(date::length_of_month(1939, date::February) == 28);
    REQUIRE(date::length_of_month(1940, date::February) == 29);
    REQUIRE(date::length_of_month(1939, date::April) == 30);
    REQUIRE(date::length_of_month(1939, date::December) == 31);
    REQUIRE(date::length_of_month(1939, date::March) == 31);
    REQUIRE(date::length_of_month(1939, date::May) == 31);
    REQUIRE(date::length_of_month(1939, date::June) == 30);
}

TEST_CASE("date - Constructor año/mes/día", "[date]") {
    date d(1939, 9, 3, 10, 30, 0);
    REQUIRE(d.get_value(date::year) == 1939);
    REQUIRE(d.get_value(date::month) == 9);
    REQUIRE(d.get_value(date::day) == 3);
    REQUIRE(d.get_value(date::hour) == 10);
    REQUIRE(d.get_value(date::minute) == 30);
    REQUIRE(d.get_value(date::second) == 0);
}

TEST_CASE("date - Constructor tiempo lineal", "[date]") {
    date d0(0);
    REQUIRE(d0.get_value(date::year) == 1939);
    REQUIRE(d0.get_value(date::month) == 1);
    REQUIRE(d0.get_value(date::day) == 1);
    REQUIRE(d0.get_value(date::hour) == 0);
    REQUIRE(d0.get_value(date::minute) == 0);
    REQUIRE(d0.get_value(date::second) == 0);
}

TEST_CASE("date - to_str", "[date]") {
    date d(1939, 9, 3, 10, 30);
    std::string s = d.to_str();
    REQUIRE(s.find("1939") != std::string::npos);
    REQUIRE(s.find("9") != std::string::npos);
    REQUIRE(s.find("3") != std::string::npos);
}

TEST_CASE("date - Operadores de comparación", "[date]") {
    date d1(1939, 9, 1);
    date d2(1939, 9, 3);
    date d3(1939, 9, 3);

    REQUIRE(d1 < d2);
    REQUIRE_FALSE(d2 < d1);
    REQUIRE(d1 <= d2);
    REQUIRE(d2 <= d3);
    REQUIRE(d2 == d3);
    REQUIRE_FALSE(d1 == d2);
    REQUIRE(d2 >= d1);
    REQUIRE(d2 > d1);
}

TEST_CASE("date - Constructor desde string", "[date]") {
    date d("1939/9/3");
    REQUIRE(d.get_value(date::year) == 1939);
    REQUIRE(d.get_value(date::month) == 9);
    REQUIRE(d.get_value(date::day) == 3);
}

TEST_CASE("date - Constructor desde string inválido lanza", "[date]") {
    REQUIRE_THROWS_AS(date("invalid"), error);
    REQUIRE_THROWS_AS(date("1939-9-3"), error);  // formato incorrecto
}

static void write_date_xml(const char *path, const char *day, const char *time) {
    xml_doc doc(path);
    xml_elem root = doc.add_child("root");
    xml_elem d = root.add_child("date");
    d.set_attr(std::string(day), "day");
    d.set_attr(std::string(time), "time");
    doc.save();
}

TEST_CASE("date - load validación year", "[date]") {
    char tmp[] = "/tmp/dftd_date_inv_XXXXXX";
    int fd = mkstemp(tmp);
    REQUIRE(fd >= 0);
    close(fd);
    write_date_xml(tmp, "1938/9/3", "10:30:0");
    date d(0);
    xml_doc doc(tmp);
    doc.load();
    REQUIRE_THROWS_AS(d.load(doc.first_child()), error);
    write_date_xml(tmp, "1946/1/1", "0:0:0");
    doc.load();
    REQUIRE_THROWS_AS(d.load(doc.first_child()), error);
    unlink(tmp);
}

TEST_CASE("date - load validación month/day/time", "[date]") {
    char tmp[] = "/tmp/dftd_date_inv_XXXXXX";
    int fd = mkstemp(tmp);
    REQUIRE(fd >= 0);
    close(fd);
    date d(0);
    xml_doc doc(tmp);
    write_date_xml(tmp, "1939/0/1", "0:0:0");
    doc.load();
    REQUIRE_THROWS_AS(d.load(doc.first_child()), error);
    write_date_xml(tmp, "1939/13/1", "0:0:0");
    doc.load();
    REQUIRE_THROWS_AS(d.load(doc.first_child()), error);
    write_date_xml(tmp, "1939/1/0", "0:0:0");
    doc.load();
    REQUIRE_THROWS_AS(d.load(doc.first_child()), error);
    write_date_xml(tmp, "1939/1/32", "0:0:0");
    doc.load();
    REQUIRE_THROWS_AS(d.load(doc.first_child()), error);
    write_date_xml(tmp, "1939/1/1", "24:0:0");
    doc.load();
    REQUIRE_THROWS_AS(d.load(doc.first_child()), error);
    write_date_xml(tmp, "1939/1/1", "0:60:0");
    doc.load();
    REQUIRE_THROWS_AS(d.load(doc.first_child()), error);
    write_date_xml(tmp, "1939/1/1", "0:0:60");
    doc.load();
    REQUIRE_THROWS_AS(d.load(doc.first_child()), error);
    unlink(tmp);
}

TEST_CASE("date - load sin hijo date lanza", "[date]") {
    char tmp[] = "/tmp/dftd_date_nochild_XXXXXX";
    int fd = mkstemp(tmp);
    REQUIRE(fd >= 0);
    close(fd);
    {
        xml_doc doc(tmp);
        xml_elem root = doc.add_child("root");
        doc.save();
    }
    date d(0);
    xml_doc doc(tmp);
    doc.load();
    REQUIRE_THROWS_AS(d.load(doc.first_child()), xml_elem_error);
    unlink(tmp);
}

TEST_CASE("date - Constructor lineal cruza año", "[date]") {
    date d(365 * 86400);  // 365 días después de 1.1.1939 -> 1.1.1940
    REQUIRE(d.get_value(date::year) == 1940);
    REQUIRE(d.get_value(date::month) == 1);
    REQUIRE(d.get_value(date::day) == 1);
}

TEST_CASE("date - save/load roundtrip", "[date]") {
    date d_orig(1939, 9, 3, 10, 30, 45);

    char tmp[] = "/tmp/dftd_date_save_test_XXXXXX";
    int fd = mkstemp(tmp);
    REQUIRE(fd >= 0);
    close(fd);

    {
        xml_doc doc(tmp);
        xml_elem root = doc.add_child("root");
        d_orig.save(root);
        doc.save();
    }

    date d_loaded(0);
    {
        xml_doc doc(tmp);
        doc.load();
        xml_elem root = doc.first_child();
        d_loaded.load(root);
    }
    unlink(tmp);

    REQUIRE(d_loaded.get_value(date::year) == 1939);
    REQUIRE(d_loaded.get_value(date::month) == 9);
    REQUIRE(d_loaded.get_value(date::day) == 3);
    REQUIRE(d_loaded.get_value(date::hour) == 10);
    REQUIRE(d_loaded.get_value(date::minute) == 30);
    REQUIRE(d_loaded.get_value(date::second) == 45);
}
