/*
 * Test para bzip.h: excepción bzip_failure y what().
 */
#include "catch_amalgamated.hpp"
#include "../bzip.h"
#include <string>

TEST_CASE("bzip_failure - BZ_PARAM_ERROR", "[bzip]") {
    try {
        throw bzip_failure(BZ_PARAM_ERROR);
    } catch (const bzip_failure &e) {
        std::string w(e.what());
        REQUIRE((w.find("PARAM") != std::string::npos || w.find("BZ_") != std::string::npos));
    }
}

TEST_CASE("bzip_failure - BZ_DATA_ERROR hereda ios_base::failure", "[bzip]") {
    REQUIRE_NOTHROW([] {
        try {
            throw bzip_failure(BZ_DATA_ERROR);
        } catch (const std::ios_base::failure &) {
        }
    }());
}
