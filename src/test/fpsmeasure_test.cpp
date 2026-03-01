/* Test fpsmeasure.h/cpp */
#include "../fpsmeasure.h"
#include <cassert>
#include <cstdio>
int main() {
    fpsmeasure fm(1.0f);
    float f = fm.account_frame();
    (void)f;
    printf("fpsmeasure_test ok\n");
    return 0;
}
