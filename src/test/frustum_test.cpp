/*
 * Test para frustum.h/cpp: construcción desde polygon + vector3 + znear.
 */
#include "../frustum.h"
#include "../polygon.h"
#include <cassert>
#include <cmath>
#include <cstdio>

static bool near(double a, double b, double eps = 1e-9) {
    return std::fabs(a - b) <= eps;
}

int main() {
    // Test 1: Frustum básico cuadrado
    polygon view;
    view.points.push_back(vector3(0, 0, 0));
    view.points.push_back(vector3(1, 0, 0));
    view.points.push_back(vector3(1, 1, 0));
    view.points.push_back(vector3(0, 1, 0));
    
    vector3 viewp(0.5, 0.5, -10);
    frustum f(view, viewp, 1.0);
    
    assert(!f.planes.empty());
    assert(near(f.znear, 1.0));
    
    // Test 2: Frustum debe tener planos (número depende de implementación)
    assert(f.planes.size() > 0);
    
    // Test 3: Frustum con znear diferente
    frustum f2(view, viewp, 0.1);
    assert(near(f2.znear, 0.1));
    assert(!f2.planes.empty());
    
    // Test 4: Frustum con znear grande
    frustum f3(view, viewp, 100.0);
    assert(near(f3.znear, 100.0));
    
    // Test 5: Polígono triangular
    polygon tri_view;
    tri_view.points.push_back(vector3(0, 0, 0));
    tri_view.points.push_back(vector3(2, 0, 0));
    tri_view.points.push_back(vector3(1, 2, 0));
    
    vector3 tri_viewp(1, 0.67, -5);
    frustum f_tri(tri_view, tri_viewp, 1.0);
    
    assert(!f_tri.planes.empty());
    
    // Test 6: Posición de viewpoint diferente
    vector3 viewp2(0, 0, -20);  // Viewpoint más lejos
    frustum f4(view, viewp2, 1.0);
    assert(!f4.planes.empty());
    
    // Test 7: Polígono más grande
    polygon large_view;
    large_view.points.push_back(vector3(-10, -10, 0));
    large_view.points.push_back(vector3(10, -10, 0));
    large_view.points.push_back(vector3(10, 10, 0));
    large_view.points.push_back(vector3(-10, 10, 0));
    
    frustum f_large(large_view, viewp, 1.0);
    assert(!f_large.planes.empty());
    assert(near(f_large.znear, 1.0));
    
    // Test 8: Frustum con polígono pentagonal
    polygon penta_view;
    for (int i = 0; i < 5; ++i) {
        double angle = 2.0 * M_PI * i / 5.0;
        penta_view.points.push_back(vector3(std::cos(angle), std::sin(angle), 0));
    }
    
    frustum f_penta(penta_view, vector3(0, 0, -5), 1.0);
    assert(!f_penta.planes.empty());
    
    printf("frustum_test ok (8 tests)\n");
    return 0;
}
