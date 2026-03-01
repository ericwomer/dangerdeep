/*
 * Test para excepciones error, file_read_error (error.h).
 */
#include "../error.h"
#include <cassert>
#include <cstdio>
#include <stdexcept>
#include <string>

int main() {
    try {
        throw error("test message");
    } catch (const std::exception &e) {
        std::string w(e.what());
        assert(w.find("DftD error:") != std::string::npos);
        assert(w.find("test message") != std::string::npos);
    }

    try {
        throw file_read_error("missing.dat");
    } catch (const file_read_error &e) {
        std::string w(e.what());
        assert(w.find("failed to load:") != std::string::npos);
        assert(w.find("missing.dat") != std::string::npos);
    } catch (...) {
        assert(0 && "expected file_read_error");
    }

    try {
        throw error("x");
    } catch (const error &e) {
        assert(std::string(e.what()).find("x") != std::string::npos);
    }

    printf("error_test ok\n");
    return 0;
}
