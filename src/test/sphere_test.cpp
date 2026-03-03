/*
 * Test para sphere_t (esfera 3D).
 */
#include "catch_amalgamated.hpp"
#include "../sphere.h"
#include <cmath>

inline bool near(double a, double b, double eps = 1e-5) {
    return std::fabs(a - b) <= eps;
}

TEST_CASE("spheref - Esfera básica centrada en origen", "[sphere]") {
    spheref s(vector3f(0, 0, 0), 5.0f);
    
    REQUIRE(s.is_inside(vector3f(0, 0, 0)));   // Centro
    REQUIRE(s.is_inside(vector3f(3, 0, 0)));   // Dentro
    REQUIRE_FALSE(s.is_inside(vector3f(6, 0, 0))); // Fuera
}

TEST_CASE("spheref - Punto cerca del borde", "[sphere]") {
    spheref s(vector3f(0, 0, 0), 5.0f);
    
    REQUIRE(s.is_inside(vector3f(4.9f, 0, 0)));  // Casi en el borde X
    REQUIRE(s.is_inside(vector3f(0, 4.9f, 0)));  // Casi en el borde Y
    REQUIRE(s.is_inside(vector3f(0, 0, 4.9f)));  // Casi en el borde Z
    REQUIRE(s.is_inside(vector3f(2, 2, 2)));     // Dentro en diagonal
}

TEST_CASE("spheref - Intersección de esferas", "[sphere]") {
    spheref s(vector3f(0, 0, 0), 5.0f);
    
    SECTION("No intersectan") {
        spheref s2(vector3f(10, 0, 0), 3.0f);
        REQUIRE_FALSE(s.intersects(s2));
    }
    
    SECTION("Sí intersectan") {
        spheref s3(vector3f(4, 0, 0), 2.0f);
        REQUIRE(s.intersects(s3));
    }
    
    SECTION("Esferas que se superponen") {
        spheref s4(vector3f(7, 0, 0), 3.0f);
        REQUIRE(s.intersects(s4));
    }
    
    SECTION("Esfera contenida dentro de otra") {
        spheref s5(vector3f(1, 0, 0), 1.0f);
        REQUIRE(s.intersects(s5));
    }
}

TEST_CASE("spheref - compute_bound de dos esferas", "[sphere]") {
    spheref s(vector3f(0, 0, 0), 5.0f);
    spheref s3(vector3f(4, 0, 0), 2.0f);
    
    spheref bound = s.compute_bound(s3);
    
    REQUIRE(bound.radius >= 5.0f);
    REQUIRE(bound.is_inside(vector3f(0, 0, 0)));
    REQUIRE(bound.is_inside(vector3f(4, 0, 0)));
    REQUIRE(bound.is_inside(s.center));
    REQUIRE(bound.is_inside(s3.center));
}

TEST_CASE("spheref - compute_min_max genera AABB correcto", "[sphere]") {
    spheref s(vector3f(0, 0, 0), 5.0f);
    
    vector3f minv(1e30f, 1e30f, 1e30f), maxv(-1e30f, -1e30f, -1e30f);
    s.compute_min_max(minv, maxv);
    
    REQUIRE(near(minv.x, -5.0f));
    REQUIRE(near(maxv.x, 5.0f));
    REQUIRE(near(minv.y, -5.0f));
    REQUIRE(near(maxv.y, 5.0f));
    REQUIRE(near(minv.z, -5.0f));
    REQUIRE(near(maxv.z, 5.0f));
}

TEST_CASE("spheref - Esfera con centro desplazado", "[sphere]") {
    spheref s(vector3f(10, 20, 30), 7.0f);
    
    REQUIRE(s.is_inside(vector3f(10, 20, 30)));
    REQUIRE(s.is_inside(vector3f(16.9f, 20, 30)));
    REQUIRE_FALSE(s.is_inside(vector3f(18, 20, 30)));
    
    vector3f minv(1e30f, 1e30f, 1e30f), maxv(-1e30f, -1e30f, -1e30f);
    s.compute_min_max(minv, maxv);
    
    REQUIRE(near(minv.x, 3.0f));
    REQUIRE(near(maxv.x, 17.0f));
    REQUIRE(near(minv.y, 13.0f));
    REQUIRE(near(maxv.y, 27.0f));
    REQUIRE(near(minv.z, 23.0f));
    REQUIRE(near(maxv.z, 37.0f));
}

TEST_CASE("spheref - Radio muy pequeño", "[sphere]") {
    spheref s(vector3f(0, 0, 0), 0.001f);
    
    REQUIRE(s.is_inside(vector3f(0, 0, 0)));
    REQUIRE_FALSE(s.is_inside(vector3f(0.01f, 0, 0)));
}

TEST_CASE("spheref - Radio grande", "[sphere]") {
    spheref s(vector3f(0, 0, 0), 10000.0f);
    
    REQUIRE(s.is_inside(vector3f(5000, 5000, 0)));
    REQUIRE_FALSE(s.is_inside(vector3f(8000, 8000, 0)));
}

TEST_CASE("spheref - Puntos en 3D (diagonal completa)", "[sphere]") {
    spheref s(vector3f(0, 0, 0), 10.0f);
    
    // Punto a distancia sqrt(3*5²) ≈ 8.66 < 10
    REQUIRE(s.is_inside(vector3f(5, 5, 5)));
    // Punto a distancia sqrt(3*6²) ≈ 10.39 > 10
    REQUIRE_FALSE(s.is_inside(vector3f(6, 6, 6)));
}

TEST_CASE("spheref - compute_bound con esferas distantes", "[sphere]") {
    spheref s1(vector3f(100, 0, 0), 5.0f);
    spheref s2(vector3f(-100, 0, 0), 5.0f);
    
    spheref bound = s1.compute_bound(s2);
    
    REQUIRE(bound.is_inside(s1.center));
    REQUIRE(bound.is_inside(s2.center));
    REQUIRE(bound.radius >= 100.0f);
}
