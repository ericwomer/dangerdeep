/*
 * Test para sphere_t (esfera 3D).
 */
#include "../sphere.h"
#include <cassert>
#include <cmath>
#include <cstdio>

static bool near(double a, double b, double eps = 1e-5) {
    return std::fabs(a - b) <= eps;
}

int main() {
    // Test 1: Esfera básica centrada en origen
    spheref s(vector3f(0, 0, 0), 5.0f);
    assert(s.is_inside(vector3f(0, 0, 0)));  // Centro
    assert(s.is_inside(vector3f(3, 0, 0)));  // Dentro
    assert(!s.is_inside(vector3f(6, 0, 0))); // Fuera
    
    // Test 2: Punto cerca del borde (dentro pero cerca)
    assert(s.is_inside(vector3f(4.9f, 0, 0)));  // Casi en el borde X
    assert(s.is_inside(vector3f(0, 4.9f, 0)));  // Casi en el borde Y
    assert(s.is_inside(vector3f(0, 0, 4.9f)));  // Casi en el borde Z
    assert(s.is_inside(vector3f(2, 2, 2)));     // Dentro en diagonal (sqrt(12)≈3.46 < 5)
    
    // Test 3: Intersección de esferas - no intersectan
    spheref s2(vector3f(10, 0, 0), 3.0f);
    assert(!s.intersects(s2));
    
    // Test 4: Intersección de esferas - sí intersectan
    spheref s3(vector3f(4, 0, 0), 2.0f);
    assert(s.intersects(s3));
    
    // Test 5: Esferas que se superponen
    spheref s4(vector3f(7, 0, 0), 3.0f);  // Distancia 7, radios 5+3, se superponen
    assert(s.intersects(s4));
    
    // Test 6: Esfera contenida dentro de otra
    spheref s5(vector3f(1, 0, 0), 1.0f);
    assert(s.intersects(s5));
    
    // Test 7: compute_bound de dos esferas
    spheref bound = s.compute_bound(s3);
    assert(bound.radius >= 5.0f);
    assert(bound.is_inside(vector3f(0, 0, 0)));
    assert(bound.is_inside(vector3f(4, 0, 0)));
    
    // La esfera bound debe contener ambas esferas originales
    assert(bound.is_inside(s.center));
    assert(bound.is_inside(s3.center));
    
    // Test 8: compute_min_max genera AABB correcto
    vector3f minv(1e30f, 1e30f, 1e30f), maxv(-1e30f, -1e30f, -1e30f);
    s.compute_min_max(minv, maxv);
    assert(near(minv.x, -5.0f) && near(maxv.x, 5.0f));
    assert(near(minv.y, -5.0f) && near(maxv.y, 5.0f));
    assert(near(minv.z, -5.0f) && near(maxv.z, 5.0f));
    
    // Test 9: Esfera con centro desplazado
    spheref s6(vector3f(10, 20, 30), 7.0f);
    assert(s6.is_inside(vector3f(10, 20, 30)));
    assert(s6.is_inside(vector3f(16.9f, 20, 30)));  // Justo dentro
    assert(!s6.is_inside(vector3f(18, 20, 30)));
    
    vector3f min6(1e30f, 1e30f, 1e30f), max6(-1e30f, -1e30f, -1e30f);
    s6.compute_min_max(min6, max6);
    assert(near(min6.x, 3.0f) && near(max6.x, 17.0f));
    assert(near(min6.y, 13.0f) && near(max6.y, 27.0f));
    assert(near(min6.z, 23.0f) && near(max6.z, 37.0f));
    
    // Test 10: Radio muy pequeño
    spheref s_tiny(vector3f(0, 0, 0), 0.001f);
    assert(s_tiny.is_inside(vector3f(0, 0, 0)));
    assert(!s_tiny.is_inside(vector3f(0.01f, 0, 0)));
    
    // Test 11: Radio grande
    spheref s_huge(vector3f(0, 0, 0), 10000.0f);
    assert(s_huge.is_inside(vector3f(5000, 5000, 0)));
    assert(!s_huge.is_inside(vector3f(8000, 8000, 0)));
    
    // Test 12: Puntos en 3D (diagonal completa)
    spheref s7(vector3f(0, 0, 0), 10.0f);
    // Punto a distancia sqrt(3*5²) ≈ 8.66 < 10
    assert(s7.is_inside(vector3f(5, 5, 5)));
    // Punto a distancia sqrt(3*6²) ≈ 10.39 > 10
    assert(!s7.is_inside(vector3f(6, 6, 6)));
    
    // Test 13: compute_bound con esferas distantes
    spheref s8(vector3f(100, 0, 0), 5.0f);
    spheref s9(vector3f(-100, 0, 0), 5.0f);
    spheref bound2 = s8.compute_bound(s9);
    assert(bound2.is_inside(s8.center));
    assert(bound2.is_inside(s9.center));
    assert(bound2.radius >= 100.0f);  // Debe cubrir la distancia
    
    printf("sphere_test ok (13 tests)\n");
    return 0;
}
