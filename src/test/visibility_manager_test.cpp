/*
 * Test para visibility_manager: cálculo de distancia máxima de visibilidad.
 */
#include "catch_amalgamated.hpp"
#include "../visibility_manager.h"
#include <cmath>

// Helper para comparaciones de punto flotante
inline bool near(double a, double b, double eps = 1e-6) {
    return std::fabs(a - b) <= eps;
}

TEST_CASE("visibility_manager - Constructor inicializa en 0", "[visibility_manager]") {
    visibility_manager vm;
    REQUIRE(near(vm.get_max_distance(), 0.0));
}

TEST_CASE("visibility_manager - compute con diferentes brillos", "[visibility_manager]") {
    visibility_manager vm;
    
    SECTION("Brillo 0 (noche oscura) = 5km") {
        vm.compute(0.0);
        REQUIRE(near(vm.get_max_distance(), 5000.0));
    }
    
    SECTION("Brillo 1.0 (día brillante) = 30km") {
        vm.compute(1.0);
        REQUIRE(near(vm.get_max_distance(), 30000.0));
    }
    
    SECTION("Brillo 0.5 (medio día) = 17.5km") {
        vm.compute(0.5);
        REQUIRE(near(vm.get_max_distance(), 17500.0));
    }
    
    SECTION("Brillo 0.25 = 11.25km") {
        vm.compute(0.25);
        REQUIRE(near(vm.get_max_distance(), 11250.0));
    }
    
    SECTION("Brillo 0.75 = 23.75km") {
        vm.compute(0.75);
        REQUIRE(near(vm.get_max_distance(), 23750.0));
    }
}

TEST_CASE("visibility_manager - set_max_distance para cargar desde save", "[visibility_manager]") {
    visibility_manager vm;
    vm.set_max_distance(15000.0);
    REQUIRE(near(vm.get_max_distance(), 15000.0));
}

TEST_CASE("visibility_manager - compute sobrescribe el valor anterior", "[visibility_manager]") {
    visibility_manager vm;
    vm.set_max_distance(15000.0);
    vm.compute(0.0);
    REQUIRE(near(vm.get_max_distance(), 5000.0));
}

TEST_CASE("visibility_manager - Valores edge case", "[visibility_manager]") {
    visibility_manager vm;
    
    SECTION("Brillo negativo") {
        vm.compute(-0.1);
        // Resultado: 5000 + (-0.1 * 25000) = 2500
        REQUIRE(vm.get_max_distance() < 5000.0);
    }
    
    SECTION("Brillo > 1.0") {
        vm.compute(2.0);
        // Resultado: 5000 + (2.0 * 25000) = 55000
        REQUIRE(vm.get_max_distance() > 30000.0);
    }
}

TEST_CASE("visibility_manager - Secuencia realista de ciclo día/noche", "[visibility_manager][integration]") {
    visibility_manager vm;
    double prev_dist = 0.0;
    
    // Simular amanecer a mediodía
    for (int i = 0; i <= 10; ++i) {
        double brightness = i / 10.0;  // 0.0 a 1.0
        vm.compute(brightness);
        double curr_dist = vm.get_max_distance();
        
        // La distancia debe incrementar con el brillo
        REQUIRE((curr_dist >= prev_dist || near(curr_dist, prev_dist)));
        prev_dist = curr_dist;
    }
    
    // Al final debe estar en máximo
    REQUIRE(near(vm.get_max_distance(), 30000.0));
}
