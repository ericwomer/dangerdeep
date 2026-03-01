/* Test countrycodes.h/cpp: countrycodes[], party_of_country con date. */
#include "../countrycodes.h"
#include "../date.h"
#include <cassert>
#include <cstdio>
int main() {
    assert(countrycodes[GERMANY] != nullptr);
    assert(party_of_country(GERMANY, date(1942, 1, 1)) == AXIS);
    assert(party_of_country(GREATBRITAIN, date(1940, 1, 1)) == ALLIES);
    printf("countrycodes_test ok\n");
    return 0;
}
