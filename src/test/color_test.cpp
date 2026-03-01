/*
 * Test minimo para color y colorf.
 */
#include "../color.h"
#include <cassert>
#include <cmath>
#include <cstdio>

static bool near(float a, float b, float eps = 1e-5f) {
    return std::fabs(a - b) <= eps;
}

int main() {
    color black = color::black();
    assert(black.r == 0 && black.g == 0 && black.b == 0 && black.a == 255);
    color white = color::white();
    assert(white.r == 255 && white.g == 255 && white.b == 255);
    color red = color::red();
    assert(red.r == 255 && red.g == 0 && red.b == 0);

    color c1(100, 150, 200);
    assert(c1.r == 100 && c1.g == 150 && c1.b == 200 && c1.a == 255);
    float br = (100*0.299f + 150*0.587f + 200*0.114f) / 255.0f;
    assert(near(c1.brightness(), br));

    color grey = c1.grey_value();
    assert(grey.r == grey.g && grey.g == grey.b);
    assert(grey.a == c1.a);

    color blend(c1, white, 0.5f);
    assert(blend.r >= 100 && blend.r <= 255);
    color mul = c1 * color(255, 255, 255);
    assert(mul.r == 100 && mul.g == 150 && mul.b == 200);

    Uint8 rgb[3];
    c1.store_rgb(rgb);
    assert(rgb[0] == 100 && rgb[1] == 150 && rgb[2] == 200);

    colorf cf(0.5f, 0.5f, 0.5f);
    assert(near(cf.r, 0.5f) && near(cf.a, 1.0f));
    color from_cf(cf);
    assert(from_cf.r == 127 || from_cf.r == 128);
    colorf cf_from_c(c1);
    assert(near(cf_from_c.r, c1.r / 255.0f));

    printf("color_test ok\n");
    return 0;
}
