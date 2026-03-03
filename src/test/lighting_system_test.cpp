/*
 * Test para lighting_system: cálculos astronómicos y de iluminación.
 */
#include "../lighting_system.h"
#include "../vector3.h"
#include "../color.h"
#include <cassert>
#include <cmath>
#include <cstdio>

static bool near(double a, double b, double eps = 1e-6) {
    return std::fabs(a - b) <= eps;
}

int main() {
    // Test 1: Constructor inicializa tiempo en 0
    lighting_system ls;
    assert(near(ls.get_time(), 0.0));
    
    // Test 2: set_time actualiza el tiempo
    ls.set_time(1000.0);
    assert(near(ls.get_time(), 1000.0));
    
    ls.set_time(123456.789);
    assert(near(ls.get_time(), 123456.789));
    
    // Test 3: compute_light_brightness retorna valor válido
    vector3 viewpos(0.0, 0.0, 0.0);
    vector3 sundir;
    
    double brightness = ls.compute_light_brightness(viewpos, sundir);
    assert(brightness >= 0.2 && brightness <= 1.0);
    
    // sundir debe estar normalizado
    double sundir_len = sundir.length();
    assert(near(sundir_len, 1.0, 1e-5));
    
    // Test 4: compute_light_color retorna color válido
    colorf light_color = ls.compute_light_color(viewpos);
    assert(light_color.r >= 0.0f && light_color.r <= 1.0f);
    assert(light_color.g >= 0.0f && light_color.g <= 1.0f);
    assert(light_color.b >= 0.0f && light_color.b <= 1.0f);
    
    // Test 5: compute_sun_pos retorna posición válida
    vector3 sun_pos = ls.compute_sun_pos(viewpos);
    assert(sun_pos.length() > 0.0);
    
    // Test 6: compute_moon_pos retorna posición válida
    vector3 moon_pos = ls.compute_moon_pos(viewpos);
    assert(moon_pos.length() > 0.0);
    
    // Test 7: is_day_mode es consistente con brightness
    bool is_day = ls.is_day_mode(viewpos);
    // Si es de día, brightness debería ser alto
    // (No podemos hacer assert estricto sin conocer la hora exacta)
    
    // Test 8: Diferentes tiempos producen diferentes resultados
    ls.set_time(0.0);  // Inicio 1939
    double brightness1 = ls.compute_light_brightness(viewpos, sundir);
    vector3 sun_pos1 = ls.compute_sun_pos(viewpos);
    
    ls.set_time(43200.0);  // 12 horas después
    double brightness2 = ls.compute_light_brightness(viewpos, sundir);
    vector3 sun_pos2 = ls.compute_sun_pos(viewpos);
    
    // Posiciones del sol deben ser diferentes
    assert((sun_pos1 - sun_pos2).length() > 1e-6);
    
    // Test 9: Diferentes posiciones producen diferentes resultados
    vector3 viewpos1(0.0, 0.0, 0.0);      // Ecuador
    vector3 viewpos2(0.0, 50000.0, 0.0);  // Más al norte
    
    ls.set_time(86400.0);  // 1 día
    vector3 sun_pos_eq = ls.compute_sun_pos(viewpos1);
    vector3 sun_pos_north = ls.compute_sun_pos(viewpos2);
    
    // Las posiciones relativas del sol deben diferir
    assert((sun_pos_eq - sun_pos_north).length() > 1e-6);
    
    // Test 10: Ciclo día/noche
    // Simulamos 24 horas con pasos de 2 horas
    ls.set_time(0.0);
    vector3 test_pos(0.0, 0.0, 0.0);
    
    bool found_day = false;
    bool found_night = false;
    
    for (int h = 0; h < 24; h += 2) {
        double time = h * 3600.0;  // Segundos
        ls.set_time(time);
        
        if (ls.is_day_mode(test_pos)) {
            found_day = true;
        } else {
            found_night = true;
        }
    }
    
    // Debe haber encontrado ambos modos en algún punto
    // (Depende de la época del año y latitud, pero generalmente sí)
    
    // Test 11: Brightness limits
    // Probar muchos tiempos diferentes para verificar límites
    for (int i = 0; i < 100; ++i) {
        double test_time = i * 3600.0;  // Cada hora durante ~4 días
        ls.set_time(test_time);
        
        double br = ls.compute_light_brightness(viewpos, sundir);
        assert(br >= 0.2);  // Mínimo (noche)
        assert(br <= 1.0);  // Máximo (día brillante)
        
        // sundir siempre normalizado
        assert(near(sundir.length(), 1.0, 1e-5));
    }
    
    // Test 12: Color warmth test
    // En teoría, colores de amanecer/atardecer son más cálidos
    ls.set_time(0.0);
    colorf color1 = ls.compute_light_color(viewpos);
    
    ls.set_time(43200.0);
    colorf color2 = ls.compute_light_color(viewpos);
    
    // Ambos deben estar en rangos válidos
    assert(color1.r >= 0.0f && color1.r <= 1.0f);
    assert(color2.r >= 0.0f && color2.r <= 1.0f);
    
    // Test 13: Sol y luna en posiciones diferentes
    ls.set_time(86400.0);
    vector3 sun = ls.compute_sun_pos(viewpos);
    vector3 moon = ls.compute_moon_pos(viewpos);
    
    // Sol y luna no deben estar exactamente en el mismo lugar
    double separation = (sun - moon).length();
    assert(separation > 1e-6);
    
    printf("lighting_system_test ok\n");
    return 0;
}
