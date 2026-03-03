/*
 * Test para color y colorf.
 */
#include "catch_amalgamated.hpp"
#include "../color.h"
#include <cmath>

inline bool near(float a, float b, float eps = 1e-5f) {
    return std::fabs(a - b) <= eps;
}

TEST_CASE("color - Constructores predefinidos", "[color]") {
    color black = color::black();
    REQUIRE(black.r == 0);
    REQUIRE(black.g == 0);
    REQUIRE(black.b == 0);
    REQUIRE(black.a == 255);
    
    color white = color::white();
    REQUIRE(white.r == 255);
    REQUIRE(white.g == 255);
    REQUIRE(white.b == 255);
    
    color red = color::red();
    REQUIRE(red.r == 255);
    REQUIRE(red.g == 0);
    REQUIRE(red.b == 0);
}

TEST_CASE("color - Constructor y brillo", "[color]") {
    color c1(100, 150, 200);
    REQUIRE(c1.r == 100);
    REQUIRE(c1.g == 150);
    REQUIRE(c1.b == 200);
    REQUIRE(c1.a == 255);
    
    float expected_br = (100*0.299f + 150*0.587f + 200*0.114f) / 255.0f;
    REQUIRE(near(c1.brightness(), expected_br));
}

TEST_CASE("color - Escala de grises", "[color]") {
    color c1(100, 150, 200);
    color grey = c1.grey_value();
    
    REQUIRE(grey.r == grey.g);
    REQUIRE(grey.g == grey.b);
    REQUIRE(grey.a == c1.a);
}

TEST_CASE("color - Operaciones de mezcla", "[color]") {
    color c1(100, 150, 200);
    color white = color::white();
    
    color blend(c1, white, 0.5f);
    REQUIRE(blend.r >= 100);
    REQUIRE(blend.r <= 255);
}

TEST_CASE("color - Multiplicación de colores", "[color]") {
    color c1(100, 150, 200);
    color mul = c1 * color(255, 255, 255);
    
    REQUIRE(mul.r == 100);
    REQUIRE(mul.g == 150);
    REQUIRE(mul.b == 200);
}

TEST_CASE("color - Almacenamiento RGB", "[color]") {
    color c1(100, 150, 200);
    Uint8 rgb[3];
    c1.store_rgb(rgb);
    
    REQUIRE(rgb[0] == 100);
    REQUIRE(rgb[1] == 150);
    REQUIRE(rgb[2] == 200);
}

TEST_CASE("colorf - Constructor y conversión", "[color]") {
    colorf cf(0.5f, 0.5f, 0.5f);
    REQUIRE(near(cf.r, 0.5f));
    REQUIRE(near(cf.a, 1.0f));
    
    color from_cf(cf);
    REQUIRE((from_cf.r == 127 || from_cf.r == 128));
}

TEST_CASE("colorf - Conversión desde color", "[color]") {
    color c1(100, 150, 200);
    colorf cf_from_c(c1);
    
    REQUIRE(near(cf_from_c.r, c1.r / 255.0f));
}
