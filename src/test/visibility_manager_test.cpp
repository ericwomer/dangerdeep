/*
 * Test para visibility_manager: cálculo de distancia máxima de visibilidad.
 */
#include "../visibility_manager.h"
#include <cassert>
#include <cmath>
#include <cstdio>

static bool near(double a, double b, double eps = 1e-6) {
    return std::fabs(a - b) <= eps;
}

int main() {
    // Test 1: Constructor inicializa en 0
    visibility_manager vm;
    assert(near(vm.get_max_distance(), 0.0));
    
    // Test 2: compute con brillo 0 (noche oscura) = 5km
    vm.compute(0.0);
    assert(near(vm.get_max_distance(), 5000.0));
    
    // Test 3: compute con brillo 1.0 (día brillante) = 30km
    vm.compute(1.0);
    assert(near(vm.get_max_distance(), 30000.0));
    
    // Test 4: compute con brillo 0.5 (medio día) = 17.5km
    vm.compute(0.5);
    assert(near(vm.get_max_distance(), 17500.0));
    
    // Test 5: compute con brillo 0.25 = 11.25km
    vm.compute(0.25);
    assert(near(vm.get_max_distance(), 11250.0));
    
    // Test 6: compute con brillo 0.75 = 23.75km
    vm.compute(0.75);
    assert(near(vm.get_max_distance(), 23750.0));
    
    // Test 7: set_max_distance (para cargar desde save)
    visibility_manager vm2;
    vm2.set_max_distance(15000.0);
    assert(near(vm2.get_max_distance(), 15000.0));
    
    // Test 8: compute sobrescribe el valor anterior
    vm2.compute(0.0);
    assert(near(vm2.get_max_distance(), 5000.0));
    
    // Test 9: Valores edge case
    vm.compute(-0.1);  // Brillo negativo (no debería pasar, pero probamos)
    // Resultado: 5000 + (-0.1 * 25000) = 2500
    assert(vm.get_max_distance() < 5000.0);
    
    vm.compute(2.0);  // Brillo > 1.0 (no debería pasar, pero probamos)
    // Resultado: 5000 + (2.0 * 25000) = 55000
    assert(vm.get_max_distance() > 30000.0);
    
    // Test 10: Secuencia realista (amanecer a mediodía)
    visibility_manager vm3;
    double prev_dist = 0.0;
    for (int i = 0; i <= 10; ++i) {
        double brightness = i / 10.0;  // 0.0 a 1.0
        vm3.compute(brightness);
        double curr_dist = vm3.get_max_distance();
        
        // La distancia debe incrementar con el brillo
        assert(curr_dist >= prev_dist || near(curr_dist, prev_dist));
        prev_dist = curr_dist;
    }
    
    // Al final debe estar en máximo
    assert(near(vm3.get_max_distance(), 30000.0));
    
    printf("visibility_manager_test ok\n");
    return 0;
}
