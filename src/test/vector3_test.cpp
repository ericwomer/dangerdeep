/*
 * Test para vector3 (vector 3D).
 */
#include "catch_amalgamated.hpp"
#include "../vector3.h"
#include <cmath>

inline bool near(double a, double b, double eps = 1e-9) {
    return std::fabs(a - b) <= eps;
}

TEST_CASE("vector3 - Constructor y componentes", "[vector3]") {
    vector3 a(0, 0, 0);
    vector3 d(1, 2, 2);
    
    REQUIRE(a.x == 0);
    REQUIRE(a.y == 0);
    REQUIRE(a.z == 0);
}

TEST_CASE("vector3 - Longitud y normalización", "[vector3]") {
    vector3 d(1, 2, 2);
    
    REQUIRE(near(d.length(), 3.0));
    REQUIRE(near(d.square_length(), 9.0));
    
    vector3 n = d.normal();
    REQUIRE(near(n.length(), 1.0));
    REQUIRE(near(n.x, 1.0/3.0));
    REQUIRE(near(n.y, 2.0/3.0));
    REQUIRE(near(n.z, 2.0/3.0));
}

TEST_CASE("vector3 - Operaciones aritméticas", "[vector3]") {
    vector3 b(1, 0, 0);
    vector3 c(0, 1, 0);
    
    vector3 sum = b + c;
    REQUIRE(sum.x == 1);
    REQUIRE(sum.y == 1);
    REQUIRE(sum.z == 0);
}

TEST_CASE("vector3 - Producto cruz", "[vector3]") {
    vector3 b(1, 0, 0);
    vector3 c(0, 1, 0);
    
    vector3 cross_bc = b.cross(c);
    REQUIRE(cross_bc.x == 0);
    REQUIRE(cross_bc.y == 0);
    REQUIRE(cross_bc.z == 1);
}

TEST_CASE("vector3 - Producto punto", "[vector3]") {
    vector3 b(1, 0, 0);
    vector3 c(0, 1, 0);
    
    REQUIRE(near(b * c, 0.0));
    REQUIRE(near(b * b, 1.0));
}

TEST_CASE("vector3 - Min, Max, Abs, Sign", "[vector3]") {
    REQUIRE(vector3(1, 2, 3).min(vector3(0, 3, 2)) == vector3(0, 2, 2));
    REQUIRE(vector3(1, 2, 3).max(vector3(0, 3, 2)) == vector3(1, 3, 3));
    REQUIRE(vector3(-1, 2, -3).abs() == vector3(1, 2, 3));
    REQUIRE(vector3(1.5, -0.3, 0).sign() == vector3(1, -1, 0));
}

TEST_CASE("vector3 - Distancia y proyección XY", "[vector3]") {
    REQUIRE(near(vector3(3, 0, 0).distance(vector3(0, 0, 0)), 3.0));
    
    vector2 xy = vector3(5, 7, 9).xy();
    REQUIRE(xy.x == 5);
    REQUIRE(xy.y == 7);
}
