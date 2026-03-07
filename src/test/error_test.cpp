/*
 * Test para excepciones error, file_read_error, sdl_error (error.h, error.cpp).
 */
#include "catch_amalgamated.hpp"
#include "../error.h"
#include <string>

TEST_CASE("error - mensaje básico", "[error]") {
    try {
        throw error("test message");
    } catch (const std::exception &e) {
        std::string w(e.what());
        REQUIRE(w.find("DftD error:") != std::string::npos);
        REQUIRE(w.find("test message") != std::string::npos);
    }
}

TEST_CASE("error - catch por error", "[error]") {
    try {
        throw error("x");
    } catch (const error &e) {
        REQUIRE(std::string(e.what()).find("x") != std::string::npos);
    }
}

TEST_CASE("file_read_error - mensaje", "[error]") {
    try {
        throw file_read_error("missing.dat");
    } catch (const file_read_error &e) {
        std::string w(e.what());
        REQUIRE(w.find("failed to load:") != std::string::npos);
        REQUIRE(w.find("missing.dat") != std::string::npos);
    }
}

TEST_CASE("sdl_error - constructor (error.cpp)", "[error]") {
    try {
        throw sdl_error("init failed");
    } catch (const sdl_error &e) {
        std::string w(e.what());
        REQUIRE(w.find("SDL error:") != std::string::npos);
        REQUIRE(w.find("init failed") != std::string::npos);
        REQUIRE(w.find("(test stub)") != std::string::npos);
    }
}
