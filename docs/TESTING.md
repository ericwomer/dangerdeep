# Guía de Testing

## Índice

1. [Filosofía de Testing](#filosofía-de-testing)
2. [Estrategia Actual](#estrategia-actual)
3. [Framework: Catch2](#framework-catch2)
4. [Escribir Tests](#escribir-tests)
5. [Resolución de Dependencias](#resolución-de-dependencias)
6. [Ejecutar Tests](#ejecutar-tests)
7. [Mejores Prácticas](#mejores-prácticas)
8. [Troubleshooting](#troubleshooting)

---

## Filosofía de Testing

### Principios

1. **Tests son documentación ejecutable**: Un test claro explica cómo funciona el código
2. **Cobertura progresiva**: Mejor 101 tests buenos que 200 triviales
3. **Aislamiento**: Cada test debe ser independiente
4. **Velocidad**: Los tests deben ser rápidos (< 1 segundo total)
5. **Confiabilidad**: Tests determinísticos, no flaky

### Pirámide de Testing

```
       ╱╲
      ╱  ╲         E2E/Integration (pocos)
     ╱────╲        
    ╱      ╲       
   ╱────────╲      Integration (algunos)
  ╱          ╲     
 ╱────────────╲    Unit Tests (muchos)
╱──────────────╲   
```

**Enfoque actual**: Principalmente unit tests con algunos integration tests.

---

## Estrategia Actual

### Estadísticas

- **Total tests**: 101 (100% passing ✅)
- **Framework principal**: Catch2 v3.5.2 (50 tests)
- **Tests legacy**: assert-based (51 tests)
- **Tiempo ejecución**: ~0.11 segundos
- **Cobertura**: 50+ módulos

### Tipos de Tests

#### 1. Unit Tests de Subsistemas (17 tests Catch2)

**Objetivo**: Verificar subsistemas extraídos en aislamiento

**Ejemplos**:
- `event_manager_test.cpp` - 13 casos
- `physics_system_test.cpp` - 7 casos
- `time_freezer_test.cpp` - 10 casos
- `scoring_manager_test.cpp` - 8 casos
- etc.

**Características**:
- Usan stubs para eliminar dependencias
- Tests rápidos y determinísticos
- Cobertura de API completa

#### 2. Unit Tests Matemáticos/Estructuras (9 tests Catch2)

**Objetivo**: Verificar clases matemáticas y estructuras de datos

**Ejemplos**:
- `vector3_test.cpp` - Operaciones vectoriales
- `matrix_test.cpp` - Operaciones matriciales
- `quaternion_test.cpp` - Rotaciones
- `sphere_test.cpp` - Geometría 3D
- `bv_tree_leaf_test.cpp` - Spatial partitioning

**Características**:
- Sin dependencias externas
- Verifican edge cases
- Tolerancia numérica (floating point)

#### 3. Smoke Tests (33 tests Catch2)

**Objetivo**: Verificar compilación e instantiación básica

**Ejemplos**:
- `rnd_test.cpp`
- `player_info_test.cpp`
- `moon_test.cpp`
- `stars_test.cpp`
- etc.

**Características**:
- Tests mínimos pero útiles
- Detectan errores de linkeo
- Base para expandir después

#### 4. Tests Legacy (48 tests)

**Objetivo**: Tests existentes con `assert`

**Estado**: Se mantienen sin cambios, funcionando correctamente

---

## Framework: Catch2

### ¿Por qué Catch2?

✅ **Header-only**: Sin dependencias externas  
✅ **Sintaxis expresiva**: BDD-style natural  
✅ **Mensajes de error superiores**: Muestra valores esperados vs actuales  
✅ **SECTION**: Código de setup compartido  
✅ **Tags**: Filtrado de tests  
✅ **Generadores**: Tests parametrizados  
✅ **Compilación rápida**: Precompilado en `libcatch2_main.a`  

### Estructura Básica

```cpp
#include "catch_amalgamated.hpp"
#include "../my_class.h"

TEST_CASE("Descripción del test", "[tag]") {
    // Arrange
    my_class obj(42);
    
    // Act
    int result = obj.operation();
    
    // Assert
    REQUIRE(result == expected);
}
```

### Macros Principales

| Macro | Uso | Comportamiento |
|-------|-----|----------------|
| `TEST_CASE` | Definir un test | Función de test |
| `SECTION` | Sub-tests con setup compartido | Ejecuta setup para cada SECTION |
| `REQUIRE` | Assertion que detiene el test | Fatal |
| `REQUIRE_FALSE` | Assertion negativa | Fatal |
| `CHECK` | Assertion que continúa | Non-fatal |
| `REQUIRE_NOTHROW` | Verifica que no se lanza excepción | - |
| `REQUIRE_THROWS` | Verifica que se lanza excepción | - |

### Ejemplo Completo

```cpp
#include "catch_amalgamated.hpp"
#include "../vector3.h"

// Helper para comparaciones floating point
inline bool near(double a, double b, double eps = 1e-6) {
    return std::abs(a - b) < eps;
}

TEST_CASE("vector3 - Operaciones básicas", "[vector3][math]") {
    SECTION("Constructor y longitud") {
        vector3 v(3.0, 4.0, 0.0);
        REQUIRE(near(v.length(), 5.0));
    }
    
    SECTION("Normalización") {
        vector3 v(3.0, 4.0, 0.0);
        vector3 n = v.normal();
        REQUIRE(near(n.length(), 1.0));
    }
    
    SECTION("Producto punto") {
        vector3 a(1.0, 0.0, 0.0);
        vector3 b(0.0, 1.0, 0.0);
        REQUIRE(near(a * b, 0.0));  // Perpendiculares
    }
    
    SECTION("Producto cruz") {
        vector3 a(1.0, 0.0, 0.0);
        vector3 b(0.0, 1.0, 0.0);
        vector3 c = a.cross(b);
        REQUIRE(near(c.z, 1.0));  // Regla mano derecha
    }
}
```

---

## Escribir Tests

### 1. Tests de Subsistemas

**Template**:

```cpp
#include "catch_amalgamated.hpp"
#include "../my_subsystem.h"

TEST_CASE("my_subsystem - Constructor", "[my_subsystem]") {
    my_subsystem sys;
    REQUIRE(sys.is_valid());
}

TEST_CASE("my_subsystem - Operación básica", "[my_subsystem]") {
    my_subsystem sys;
    
    sys.configure(params);
    sys.execute();
    
    REQUIRE(sys.get_result() == expected);
}

TEST_CASE("my_subsystem - Edge cases", "[my_subsystem]") {
    my_subsystem sys;
    
    SECTION("Null input") {
        REQUIRE_NOTHROW(sys.process(nullptr));
    }
    
    SECTION("Empty input") {
        std::vector<int> empty;
        REQUIRE(sys.process(empty).empty());
    }
    
    SECTION("Large input") {
        std::vector<int> large(10000, 42);
        REQUIRE_NOTHROW(sys.process(large));
    }
}
```

### 2. Tests Matemáticos

**Template**:

```cpp
#include "catch_amalgamated.hpp"
#include "../math_class.h"

inline bool near(double a, double b, double eps = 1e-6) {
    return std::abs(a - b) < eps;
}

TEST_CASE("math_class - Operaciones", "[math]") {
    SECTION("Suma") {
        math_class a(1.0);
        math_class b(2.0);
        math_class c = a + b;
        REQUIRE(near(c.value(), 3.0));
    }
    
    SECTION("División por cero") {
        math_class a(1.0);
        REQUIRE_THROWS(a / 0.0);
    }
}
```

### 3. Smoke Tests

**Template**:

```cpp
#include "catch_amalgamated.hpp"
#include "../my_module.h"

TEST_CASE("my_module - Smoke test: compilación y estructura básica", 
          "[my_module][smoke]") {
    // Verifica que compila y se puede instanciar
    REQUIRE(true);
}

TEST_CASE("my_module - Constructor básico", "[my_module][smoke]") {
    my_module mod;
    REQUIRE_NOTHROW(mod.basic_operation());
}
```

### Agregar Test al Build

En `src/test/CMakeLists.txt`:

```cmake
# Test simple (solo .cpp del test)
add_catch2_test(my_test)

# Test con dependencias
add_catch2_test(my_test 
    ${SRC_PARENT}/my_class.cpp 
    ${SRC_PARENT}/dependency.cpp
)

# Test con librerías externas
add_catch2_test(my_test ${SRC_PARENT}/my_class.cpp)
target_link_libraries(my_test xml.cpp)
```

---

## Resolución de Dependencias

### Problema

Algunos módulos tienen dependencias pesadas (OpenGL, SDL, sistema completo).

### Estrategias

#### 1. Stubs Mínimos

Crear implementaciones vacías que satisfacen al linker.

**Ejemplo**: `physics_system_stub.cpp`
```cpp
#include "physics_system.h"

void physics_system::check_collisions(
    const std::vector<ship*>&,
    const std::vector<ship*>&) {
    // Stub vacío para testing
}

void physics_system::collision_response(
    sea_object*, sea_object*, const vector3&) {
    // Stub vacío para testing
}
```

**Uso en CMake**:
```cmake
add_executable(physics_system_test 
    physics_system_test.cpp
    physics_system_stub.cpp  # En lugar del real
)
```

#### 2. Mock Objects

Crear clases mock que implementan la interfaz necesaria.

**Ejemplo**: `event_stub.h`
```cpp
class event {
public:
    virtual ~event() = default;
    virtual void eval(game&) = 0;
};

class test_event : public event {
    std::string msg_;
public:
    test_event(const std::string& m) : msg_(m) {}
    void eval(game&) override { /* mock */ }
};
```

#### 3. Incluir Fuentes Directamente

Para dependencias simples, incluir el `.cpp` directamente.

```cmake
add_catch2_test(my_test 
    ${SRC_PARENT}/my_class.cpp
    ${SRC_PARENT}/simple_dep.cpp
)
```

#### 4. Tests Simplificados

Evitar llamar métodos que requieren dependencias pesadas.

```cpp
TEST_CASE("complex_system - Constructor solo", "[system]") {
    complex_system sys;  // OK
    // NO llamar a sys.init() que requiere OpenGL
    REQUIRE(true);
}
```

### Decisión: ¿Cuándo usar cada estrategia?

| Estrategia | Cuándo |
|-----------|--------|
| **Stub** | Dependencias muy pesadas (OpenGL, sistema completo) |
| **Mock** | Necesitas comportamiento controlado |
| **Include** | Dependencia simple y pequeña |
| **Simplificar** | Test de compilación/estructura básica |

---

## Ejecutar Tests

### Método 1: Script `check.sh`

```bash
# Compilar y ejecutar todos los tests
./check.sh --unit

# Compilar desde cero y ejecutar tests
./check.sh --clean --unit

# Con coverage
./check.sh --coverage
```

### Método 2: CTest directamente

```bash
cd build

# Todos los tests
ctest

# Con output detallado
ctest --output-on-failure

# Test específico
ctest -R vector3_test

# Tests con tag (Catch2)
./src/test/vector3_test "[math]"

# Solo tests smoke
ctest -R "smoke"
```

### Método 3: Ejecutable individual

```bash
cd build/src/test

# Ejecutar test
./vector3_test

# Ver todos los casos
./vector3_test --list-tests

# Ejecutar casos específicos
./vector3_test "vector3 - Normalización"

# Con tags
./vector3_test "[math]"
```

### Variables de Entorno

Algunos tests requieren configuración:

```bash
# parser_test necesita datos
export DFTD_DATA=/path/to/dangerdeep/data
ctest
```

---

## Mejores Prácticas

### DO ✅

1. **Un concepto por test**
   ```cpp
   TEST_CASE("vector3 - length") { /* solo longitud */ }
   TEST_CASE("vector3 - normalize") { /* solo normalización */ }
   ```

2. **Nombres descriptivos**
   ```cpp
   TEST_CASE("scoring_manager - records sunk ships in order")
   ```

3. **Usar SECTION para setup compartido**
   ```cpp
   TEST_CASE("manager - operations") {
       manager m;  // Setup compartido
       
       SECTION("operation A") { m.doA(); }
       SECTION("operation B") { m.doB(); }
   }
   ```

4. **Edge cases y valores límite**
   ```cpp
   SECTION("Empty input") { }
   SECTION("Null pointer") { }
   SECTION("Maximum value") { }
   ```

5. **Helpers para comparaciones**
   ```cpp
   inline bool near(double a, double b, double eps = 1e-6) {
       return std::abs(a - b) < eps;
   }
   ```

### DON'T ❌

1. **Tests dependientes**
   ```cpp
   // ❌ BAD: Test B depende de Test A
   TEST_CASE("A") { global_state = 1; }
   TEST_CASE("B") { REQUIRE(global_state == 1); }
   ```

2. **Tests lentos**
   ```cpp
   // ❌ BAD: Test que tarda segundos
   TEST_CASE("slow") {
       for (int i = 0; i < 1000000; ++i) { ... }
   }
   ```

3. **Tests no determinísticos**
   ```cpp
   // ❌ BAD: Usa números aleatorios sin seed
   TEST_CASE("random") {
       int r = rand();  // Diferente cada vez
       REQUIRE(r > 0);
   }
   ```

4. **Múltiples conceptos en un test**
   ```cpp
   // ❌ BAD: Test hace demasiado
   TEST_CASE("everything") {
       // 50 líneas testeando 10 cosas diferentes
   }
   ```

5. **Magic numbers**
   ```cpp
   // ❌ BAD
   REQUIRE(result == 42);
   
   // ✅ GOOD
   constexpr int EXPECTED_RESULT = 42;
   REQUIRE(result == EXPECTED_RESULT);
   ```

---

## Troubleshooting

### Problema: Test no compila

**Síntoma**: Errores de compilación al agregar test

**Soluciones**:
1. Verificar includes correctos
2. Agregar dependencias en `CMakeLists.txt`
3. Crear stub si dependencia muy pesada

```cmake
# En CMakeLists.txt
add_catch2_test(my_test ${SRC_PARENT}/dependency.cpp)
```

### Problema: Test no linkea

**Síntoma**: `undefined reference to ...`

**Soluciones**:
1. Agregar `.cpp` faltante a `add_catch2_test`
2. Agregar librería con `target_link_libraries`
3. Crear stub para función faltante

```cmake
add_catch2_test(my_test)
target_link_libraries(my_test xml.cpp)
```

### Problema: Test falla por dependencia externa

**Síntoma**: Test falla porque requiere OpenGL/SDL/archivos

**Soluciones**:
1. Usar stub que no requiere la dependencia
2. Simplificar test (no llamar esos métodos)
3. Marcar como integration test y skip

```cpp
TEST_CASE("system - basic", "[system][!integration]") {
    // El tag ! hace que CTest lo skip por defecto
}
```

### Problema: Floating point comparison fails

**Síntoma**: Test falla por precisión numérica

**Solución**: Usar helper con epsilon

```cpp
inline bool near(double a, double b, double eps = 1e-6) {
    return std::abs(a - b) < eps;
}

REQUIRE(near(result, 3.14159));
```

### Problema: Tests pasan localmente pero fallan en CI

**Causas comunes**:
1. Dependencia de archivos locales → Usar rutas relativas
2. Estado global entre tests → Asegurar aislamiento
3. Timing issues → Evitar sleep/timing en tests

---

## Roadmap de Testing

### Corto Plazo

- [ ] Migrar más tests legacy a Catch2
- [ ] Aumentar coverage de smoke tests
- [ ] Tests de integración básicos

### Medio Plazo

- [ ] Coverage de 80%+ en subsistemas core
- [ ] Performance benchmarks
- [ ] Mocking framework (si necesario)

### Largo Plazo

- [ ] Tests end-to-end automatizados
- [ ] Visual regression testing
- [ ] Fuzzing de parsers

---

## Referencias

- [Catch2 Documentation](https://github.com/catchorg/Catch2/tree/devel/docs)
- [ARCHITECTURE.md](ARCHITECTURE.md): Arquitectura del proyecto
- [SUBSYSTEMS.md](SUBSYSTEMS.md): Documentación de subsistemas
- Tests: `src/test/` (101 tests)
- Helper: `src/test/CATCH2_README.md`

---

**Última actualización**: 2026-03-06
**Mantenedor**: Equipo de desarrollo de Danger from the Deep
