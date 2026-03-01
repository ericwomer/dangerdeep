/*
 * Test para filehelper.h/cpp: get_current_directory, is_directory, is_file.
 */
#include "../filehelper.h"
#include <cassert>
#include <cstdio>
#include <string>

int main() {
    std::string cwd = get_current_directory();
    assert(!cwd.empty());
    assert(is_directory("."));
    assert(is_directory(".."));
    printf("filehelper_test ok\n");
    return 0;
}
