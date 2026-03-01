/*
 * Test para bspline.h: bsplinet (B-spline uniform).
 */
#include "../bspline.h"
#include "../vector2.h"
#include <cassert>
#include <cmath>
#include <cstdio>
#include <vector>

static bool near(double a, double b, double eps = 1e-9) {
    return std::fabs(a - b) <= eps;
}

int main() {
    std::vector<double> cp;
    cp.push_back(0.0);
    cp.push_back(1.0);
    cp.push_back(2.0);
    cp.push_back(3.0);
    bsplinet<double> spl(1, cp);
    assert(spl.control_points().size() == 4);
    double v0 = spl.value(0.0);
    double v1 = spl.value(1.0);
    assert(near(v0, 0.0));
    assert(near(v1, 3.0));
    double v05 = spl.value(0.5);
    assert(v05 >= 0.0 && v05 <= 3.0);

    std::vector<vector2> cp2;
    cp2.push_back(vector2(0, 0));
    cp2.push_back(vector2(1, 0));
    cp2.push_back(vector2(1, 1));
    cp2.push_back(vector2(0, 1));
    bsplinet<vector2> spl2(1, cp2);
    vector2 p0 = spl2.value(0.0);
    vector2 p1 = spl2.value(1.0);
    assert(near(p0.x, 0.0) && near(p0.y, 0.0));
    assert(near(p1.x, 0.0) && near(p1.y, 1.0));

    bool threw = false;
    try {
        std::vector<double> bad;
        bad.push_back(1.0);
        bsplinet<double> bad_spl(2, bad);
    } catch (const std::runtime_error&) {
        threw = true;
    }
    assert(threw);

    printf("bspline_test ok\n");
    return 0;
}
