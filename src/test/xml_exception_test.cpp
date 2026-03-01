/*
 * Test para xml.h: excepciones xml_error, xml_elem_error.
 */
#include "../xml.h"
#include <cassert>
#include <cstdio>
#include <string>

int main() {
    try {
        throw xml_error("test", "file.xml");
    } catch (const xml_error& e) {
        std::string w(e.what());
        assert(w.find("xml error") != std::string::npos);
        assert(w.find("file.xml") != std::string::npos);
    }
    try {
        throw xml_elem_error("child", "doc.xml");
    } catch (const xml_elem_error& e) {
        std::string w(e.what());
        assert(w.find("element") != std::string::npos || w.find("child") != std::string::npos);
    } catch (const xml_error&) {}
    printf("xml_exception_test ok\n");
    return 0;
}
