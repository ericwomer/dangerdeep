/*
 * Test para frustum.h/cpp: construcción desde polygon + vector3 + znear.
 */
#include "catch_amalgamated.hpp"
#include "../frustum.h"
#include "../polygon.h"
#include <cmath>

inline bool near(double a, double b, double eps = 1e-9) {
    return std::fabs(a - b) <= eps;
}

TEST_CASE("frustum - Frustum básico cuadrado", "[frustum]") {
    polygon view;
    view.points.push_back(vector3(0, 0, 0));
    view.points.push_back(vector3(1, 0, 0));
    view.points.push_back(vector3(1, 1, 0));
    view.points.push_back(vector3(0, 1, 0));
    
    vector3 viewp(0.5, 0.5, -10);
    frustum f(view, viewp, 1.0);
    
    REQUIRE_FALSE(f.planes.empty());
    REQUIRE(near(f.znear, 1.0));
    REQUIRE(f.planes.size() > 0);
}

TEST_CASE("frustum - Frustum con diferentes znear", "[frustum]") {
    polygon view;
    view.points.push_back(vector3(0, 0, 0));
    view.points.push_back(vector3(1, 0, 0));
    view.points.push_back(vector3(1, 1, 0));
    view.points.push_back(vector3(0, 1, 0));
    
    vector3 viewp(0.5, 0.5, -10);
    
    SECTION("znear pequeño") {
        frustum f(view, viewp, 0.1);
        REQUIRE(near(f.znear, 0.1));
        REQUIRE_FALSE(f.planes.empty());
    }
    
    SECTION("znear grande") {
        frustum f(view, viewp, 100.0);
        REQUIRE(near(f.znear, 100.0));
        REQUIRE_FALSE(f.planes.empty());
    }
}

TEST_CASE("frustum - Polígono triangular", "[frustum]") {
    polygon tri_view;
    tri_view.points.push_back(vector3(0, 0, 0));
    tri_view.points.push_back(vector3(2, 0, 0));
    tri_view.points.push_back(vector3(1, 2, 0));
    
    vector3 tri_viewp(1, 0.67, -5);
    frustum f_tri(tri_view, tri_viewp, 1.0);
    
    REQUIRE_FALSE(f_tri.planes.empty());
}

TEST_CASE("frustum - Posición de viewpoint diferente", "[frustum]") {
    polygon view;
    view.points.push_back(vector3(0, 0, 0));
    view.points.push_back(vector3(1, 0, 0));
    view.points.push_back(vector3(1, 1, 0));
    view.points.push_back(vector3(0, 1, 0));
    
    vector3 viewp(0, 0, -20);  // Viewpoint más lejos
    frustum f(view, viewp, 1.0);
    
    REQUIRE_FALSE(f.planes.empty());
}

TEST_CASE("frustum - Polígono más grande", "[frustum]") {
    polygon large_view;
    large_view.points.push_back(vector3(-10, -10, 0));
    large_view.points.push_back(vector3(10, -10, 0));
    large_view.points.push_back(vector3(10, 10, 0));
    large_view.points.push_back(vector3(-10, 10, 0));
    
    vector3 viewp(0.5, 0.5, -10);
    frustum f(large_view, viewp, 1.0);
    
    REQUIRE_FALSE(f.planes.empty());
    REQUIRE(near(f.znear, 1.0));
}

TEST_CASE("frustum - Frustum con polígono pentagonal", "[frustum]") {
    polygon penta_view;
    for (int i = 0; i < 5; ++i) {
        double angle = 2.0 * M_PI * i / 5.0;
        penta_view.points.push_back(vector3(std::cos(angle), std::sin(angle), 0));
    }
    
    frustum f(penta_view, vector3(0, 0, -5), 1.0);
    REQUIRE_FALSE(f.planes.empty());
}
