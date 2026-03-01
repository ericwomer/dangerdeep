/* Test tree_generator.h */
#include "../datadirs.h"
#include "../tree_generator.h"
#include <cstdio>
int main() {
    set_data_dir("/tmp");
    printf("tree_generator_test ok\n");
    return 0;
}
