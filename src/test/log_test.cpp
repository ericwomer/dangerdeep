/*
 * Test para log.h/cpp: singleton log, append, get_* (sin DEBUG).
 */
#include "../log.h"
#include <cassert>
#include <cstdio>

int main() {
    log::instance();
    log::instance().append(log::LOG_INFO, "log_test message");
    std::string last = log::instance().get_last_n_lines(10);
    (void)last;
    printf("log_test ok\n");
    return 0;
}
