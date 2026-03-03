/*
 * Test para vector2 (vector 2D).
 */
#include "catch_amalgamated.hpp"
#include "../vector2.h"
#include <cmath>

inline bool near(double a, double b, double eps = 1e-9) {
    return std::fabs(a - b) <= eps;
}

TEST_CASE("vector2 - Constructor y acceso a componentes", "[vector2]") {
    vector2 a(0, 0);
    vector2 b(3, 4);
    
    REQUIRE(a.x == 0);
    REQUIRE(a.y == 0);
    REQUIRE(b.x == 3);
    REQUIRE(b.y == 4);
}

TEST_CASE("vector2 - Longitud y normalización", "[vector2]") {
    vector2 b(3, 4);
    
    REQUIRE(near(b.length(), 5.0));
    REQUIRE(near(b.square_length(), 25.0));
    
    vector2 n = b.normal();
    REQUIRE(near(n.length(), 1.0));
    REQUIRE(near(n.x, 0.6));
    REQUIRE(near(n.y, 0.8));
}

TEST_CASE("vector2 - Operadores aritméticos", "[vector2]") {
    vector2 a(0, 0);
    vector2 b(3, 4);
    
    SECTION("Suma") {
        vector2 sum = a + b;
        REQUIRE(sum.x == 3);
        REQUIRE(sum.y == 4);
    }
    
    SECTION("Resta") {
        vector2 diff = vector2(5, 5) - vector2(2, 3);
        REQUIRE(diff.x == 3);
        REQUIRE(diff.y == 2);
    }
    
    SECTION("Escalado") {
        vector2 scaled = b * 2.0;
        REQUIRE(scaled.x == 6);
        REQUIRE(scaled.y == 8);
    }
}

TEST_CASE("vector2 - Producto punto", "[vector2]") {
    vector2 b(3, 4);
    REQUIRE(near(b * vector2(1, 0), 3.0));
    REQUIRE(near(b * vector2(0, 1), 4.0));
}

TEST_CASE("vector2 - Operaciones geométricas", "[vector2]") {
    SECTION("Orthogonal") {
        vector2 orth = vector2(1, 0).orthogonal();
        REQUIRE(near(orth.x, 0.0));
        REQUIRE(near(orth.y, 1.0));
    }
    
    SECTION("Distancia") {
        REQUIRE(near(vector2(1, 0).distance(vector2(4, 0)), 3.0));
    }
}

TEST_CASE("vector2 - Min y Max componentwise", "[vector2]") {
    vector2 minv = vector2(3, 1).min(vector2(2, 5));
    REQUIRE(minv.x == 2);
    REQUIRE(minv.y == 1);
    
    vector2 maxv = vector2(3, 1).max(vector2(2, 5));
    REQUIRE(maxv.x == 3);
    REQUIRE(maxv.y == 5);
}
