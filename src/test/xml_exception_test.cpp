/*
 * Test para xml.h: excepciones xml_error, xml_elem_error.
 */
#include "catch_amalgamated.hpp"
#include "../xml.h"
#include <string>

TEST_CASE("xml_error - what contiene mensaje y archivo", "[xml]") {
    try {
        throw xml_error("test", "file.xml");
    } catch (const xml_error &e) {
        std::string w(e.what());
        REQUIRE(w.find("xml error") != std::string::npos);
        REQUIRE(w.find("file.xml") != std::string::npos);
    }
}

TEST_CASE("xml_elem_error - what contiene elemento", "[xml]") {
    try {
        throw xml_elem_error("child", "doc.xml");
    } catch (const xml_elem_error &e) {
        std::string w(e.what());
        REQUIRE((w.find("element") != std::string::npos || w.find("child") != std::string::npos));
    } catch (const xml_error &) {
        // hereda de xml_error, puede capturarse así
    }
}
