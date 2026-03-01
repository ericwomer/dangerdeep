/* Test date.h/cpp: length_of_year, length_of_month, date ctor, operator<. */
#include "../date.h"
#include <cassert>
#include <cstdio>
int main() {
    assert(date::length_of_year(1940) == 366);
    assert(date::length_of_year(1941) == 365);
    assert(date::length_of_month(1940, 2) == 29);
    date d(1942, 6, 15);
    assert(d.get_value(date::year) == 1942 && d.get_value(date::month) == 6);
    assert(date(1940,1,1) < date(1941,1,1));
    printf("date_test ok\n");
    return 0;
}
