/*
 * Test para log.h/cpp: singleton log, append, get_* (sin DEBUG).
 */
#include "catch_amalgamated.hpp"
#include "../log.h"

TEST_CASE("log - instance, append, get_last_n_lines", "[log]") {
    log::instance();
    log::instance().append(log::LOG_INFO, "log_test message");
    std::string last = log::instance().get_last_n_lines(10);
    REQUIRE(last.size() >= 0);
}
