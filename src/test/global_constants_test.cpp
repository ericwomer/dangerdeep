/*
 * Test para global_constants.h: constantes físicas y geodésicas.
 */
#include <cmath>
#include "../global_constants.h"
#include <cassert>
#include <cmath>
#include <cstdio>

static bool near(double a, double b, double eps = 1e-6) {
    return std::fabs(a - b) <= eps;
}

int main() {
    assert(near(GRAVITY, 9.806));
    assert(near(DEGREE_IN_METERS, 111194.9266388889));
    assert(near(MINUTE_IN_METERS, 1853.248777315));
    assert(SECOND_IN_METERS > 30 && SECOND_IN_METERS < 31);
    assert(near(EARTH_PERIMETER, 40030173.59));
    assert(BRT_VOLUME > 2.8 && BRT_VOLUME < 2.9);
    assert(near(WGS84_A, 6378137.0));
    assert(near(WGS84_B, 6356752.314));
    assert(WGS84_K > 0 && WGS84_K < 1);
    assert(near(EARTH_RADIUS, 6371000.785));
    assert(SUN_RADIUS > 695e6 && SUN_RADIUS < 697e6);
    assert(MOON_RADIUS > 1.7e6 && MOON_RADIUS < 1.8e6);
    assert(EARTH_SUN_DISTANCE > 149e9 && EARTH_SUN_DISTANCE < 150e9);
    assert(near(EARTH_ROT_AXIS_ANGLE, 23.45));
    assert(EARTH_ROTATION_TIME > 86000 && EARTH_ROTATION_TIME < 87000);
    assert(EARTH_ORBIT_TIME > 31e6 && EARTH_ORBIT_TIME < 32e6);
    assert(near(MOON_POS_ADJUST, 300.0));

    printf("global_constants_test ok\n");
    return 0;
}
