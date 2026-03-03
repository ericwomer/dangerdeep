# Catch2 Testing Framework

Este proyecto usa **Catch2 v3.5.2** para tests unitarios.

## Archivos

- `catch_amalgamated.hpp` - Header único de Catch2
- `catch_amalgamated.cpp` - Implementación de Catch2

## Uso Básico

```cpp
#include "catch_amalgamated.hpp"

TEST_CASE("Descripción del test", "[tag]") {
    REQUIRE(1 + 1 == 2);
}
```

## Sintaxis Común

### Assertions
- `REQUIRE(expr)` - Falla y termina el test
- `CHECK(expr)` - Falla pero continúa el test
- `REQUIRE_FALSE(expr)` - Verifica que sea falso

### Comparaciones de Punto Flotante
```cpp
#include <catch2/matchers/catch_matchers_floating_point.hpp>
using Catch::Matchers::WithinAbs;

REQUIRE_THAT(value, WithinAbs(expected, epsilon));
```

### Secciones
```cpp
TEST_CASE("Test con setup común") {
    MyClass obj;  // Setup común
    
    SECTION("Primer test") {
        // obj se recrea aquí
    }
    
    SECTION("Segundo test") {
        // obj se recrea aquí también
    }
}
```

### Tests Parametrizados
```cpp
TEST_CASE("Test con múltiples valores") {
    auto value = GENERATE(1, 2, 3, 4, 5);
    REQUIRE(value > 0);
}
```

### BDD Style
```cpp
SCENARIO("Usuario registra cuenta", "[auth]") {
    GIVEN("Un usuario nuevo") {
        User user;
        
        WHEN("Se registra con datos válidos") {
            user.register("test@example.com", "password");
            
            THEN("La cuenta se crea correctamente") {
                REQUIRE(user.is_registered());
            }
        }
    }
}
```

## Ejecutar Tests

```bash
cd build
ctest --output-on-failure

# O ejecutar directamente
./src/test/test_name
```

## Documentación

- Oficial: https://github.com/catchorg/Catch2
- Tutorial: https://github.com/catchorg/Catch2/blob/devel/docs/tutorial.md
