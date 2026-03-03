/*
 * Test para bzip.h: excepción bzip_failure y what().
 */
#include "../bzip.h"
#include <cassert>
#include <cstdio>
#include <string>

int main() {
    try {
        throw bzip_failure(BZ_PARAM_ERROR);
    } catch (const bzip_failure &e) {
        std::string w(e.what());
        assert(w.find("PARAM") != std::string::npos || w.find("BZ_") != std::string::npos);
    }
    try {
        throw bzip_failure(BZ_DATA_ERROR);
    } catch (const std::ios_base::failure &) {
    }
    printf("bzip_test ok\n");
    return 0;
}
