# Guía para Desarrolladores

Bienvenido al proyecto **Danger from the Deep**! Esta guía te ayudará a comenzar a contribuir rápidamente.

## 🚀 Quick Start (5 minutos)

```bash
# 1. Clonar repo
git clone https://github.com/dangerdeep/dangerdeep.git
cd dangerdeep

# 2. Instalar dependencias (Ubuntu/Debian)
sudo apt install build-essential cmake libsdl2-dev libsdl2-image-dev \
    libsdl2-mixer-dev libsdl2-net-dev libopenal-dev libgl1-mesa-dev \
    libtinyxml-dev clang-format cppcheck

# 3. Compilar
./check.sh --build

# 4. Ejecutar tests
./check.sh --unit

# 5. Jugar!
./build/src/dangerdeep --datadir data/
```

## 📚 Documentación Esencial

| Documento | Qué cubre |
|-----------|-----------|
| [ARCHITECTURE.md](ARCHITECTURE.md) | Visión general de arquitectura, subsistemas |
| [SUBSYSTEMS.md](SUBSYSTEMS.md) | API detallada de cada subsistema |
| [TESTING.md](TESTING.md) | Cómo escribir y ejecutar tests |
| [TERRAIN_TILES.md](TERRAIN_TILES.md) | Generación de tiles de batimetría (ETOPO1, map_precompute) |
| [CONTRIBUTING.md](../CONTRIBUTING.md) | Mejores prácticas, estilo de código |
| [REFACTORING.md](REFACTORING.md) | Historia de refactorings realizados |

## 🏗️ Estructura del Proyecto

```
dangerdeep/
├── src/                    # Código fuente
│   ├── game.cpp/h          # Coordinador central
│   ├── *_manager.cpp/h     # Subsistemas (9 extraídos)
│   ├── test/               # Tests (101 tests, 100% passing)
│   └── oglext/             # Extensiones OpenGL
├── data/                   # Assets (modelos, texturas, sonidos)
├── docs/                   # Documentación (este directorio)
├── build/                  # Compilación (generado)
└── check.sh                # Script de QA
```

## 🛠️ Flujo de Trabajo

### 1. Crear Branch

```bash
git checkout -b feature/mi-feature
```

### 2. Hacer Cambios

Edita código siguiendo el estilo del proyecto (ver [CONTRIBUTING.md](../CONTRIBUTING.md))

### 3. Verificar Calidad

```bash
# Verificar formato
./check.sh --format

# Aplicar formato
./check.sh --format-apply

# Ejecutar lint
./check.sh --lint

# Compilar y testear
./check.sh --unit
```

### 4. Commit

```bash
git add .
git commit -m "Descripción clara del cambio"
```

### 5. Push y Pull Request

```bash
git push origin feature/mi-feature
```

Luego crear PR en GitHub.

## 🧪 Testing

### Ejecutar Tests

```bash
# Todos los tests
./check.sh --unit

# Test específico
cd build && ctest -R vector3_test

# Con coverage
./check.sh --coverage
# Abre: build/coverage/html/index.html
```

### Escribir Nuevo Test

```cpp
// src/test/my_feature_test.cpp
#include "catch_amalgamated.hpp"
#include "../my_feature.h"

TEST_CASE("my_feature - basic operation", "[my_feature]") {
    my_feature f;
    f.configure(params);
    
    REQUIRE(f.execute() == expected);
}
```

Agregar a `src/test/CMakeLists.txt`:
```cmake
add_catch2_test(my_feature_test ${SRC_PARENT}/my_feature.cpp)
```

Ver [TESTING.md](TESTING.md) para guía completa.

## 🏛️ Arquitectura

### Subsistemas Extraídos (9)

El proyecto migró desde una clase `game` monolítica hacia subsistemas independientes:

1. **event_manager** - Cola de eventos
2. **physics_system** - Física y colisiones
3. **time_freezer** - Control de pausas
4. **trail_manager** - Estelas de barcos
5. **visibility_manager** - Cálculo de visibilidad
6. **scoring_manager** - Puntuación
7. **ping_manager** - Pings de sonar
8. **logbook** - Bitácora del capitán
9. **network_manager** - Multijugador

Ver [ARCHITECTURE.md](ARCHITECTURE.md) y [SUBSYSTEMS.md](SUBSYSTEMS.md) para detalles.

## 💡 Tareas para Principiantes

### Nivel Fácil

- [ ] Mejorar smoke tests (agregar más casos)
- [ ] Documentar funciones sin docs
- [ ] Corregir warnings de compilación
- [ ] Mejorar mensajes de error

### Nivel Intermedio

- [ ] Agregar tests unitarios a módulos sin coverage
- [ ] Refactorizar código duplicado
- [ ] Migrar código a C++20 features
- [ ] Mejorar UI del juego

### Nivel Avanzado

- [ ] Extraer más subsistemas de `game`
- [ ] Optimizar rendering (profiling)
- [ ] Implementar nuevas features
- [ ] Refactoring arquitectural

## 🐛 Debugging

### Compilar con Debug Symbols

```bash
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)
```

### Usar GDB

```bash
gdb ./build/src/dangerdeep
(gdb) run --datadir data/
(gdb) break game.cpp:123
(gdb) continue
```

### Valgrind (Memory Leaks)

```bash
./check.sh --valgrind
# Ver: build/valgrind.log
```

### AddressSanitizer

```bash
./check.sh --asan
```

### SIGSEGV en coastmap

Si el juego crashea con SIGSEGV tras cargar el mapa de costas (mensajes de carga: music → water → coastmap):

1. **Compilar con símbolos de depuración** para obtener un stack trace útil:
   ```bash
   cmake .. -DCMAKE_BUILD_TYPE=Debug
   make -j$(nproc)
   gdb ./build/src/dangerdeep
   (gdb) run --datadir /ruta/absoluta/a/data/
   (gdb) bt
   ```

2. **Usar AddressSanitizer** para detectar accesos OOB y fugas:
   ```bash
   cmake .. -DBUILD_ASAN=ON
   make -j$(nproc)
   ./build/src/dangerdeep --datadir ...
   ```

3. **Verificar `--datadir`**: usa una ruta absoluta o asegúrate de ejecutar desde el directorio del proyecto cuando uses rutas relativas.

## 📦 Dependencias

### Build Essentials

- **C++20 compiler** (GCC 10+, Clang 12+)
- **CMake 3.20+**
- **Make** o **Ninja**

### Librerías

- **SDL2**: Ventanas, input
- **OpenGL 2.1+**: Rendering
- **TinyXML**: Serialización
- **OpenAL**: Audio (opcional)

### Herramientas de Desarrollo

- **clang-format**: Formato de código
- **cppcheck**: Análisis estático
- **valgrind**: Memory leaks
- **gdb**: Debugging

## 🎯 Convenciones de Código

### Naming

```cpp
class my_class;              // clases: snake_case
void my_function();          // funciones: snake_case
int my_variable;             // variables: snake_case
constexpr int MY_CONSTANT = 42;  // constantes: UPPER_CASE
```

### Formatting

Usamos **clang-format** (aplicar con `./check.sh --format-apply`)

### Comentarios

```cpp
// ✅ Explica el "por qué"
// Usamos binary search porque la lista está ordenada
int result = binary_search(data, key);

// ❌ Explica el "qué" (obvio)
// Incrementa i
i++;
```

Ver [CONTRIBUTING.md](../CONTRIBUTING.md) para detalles completos.

## 🤝 Comunidad

- **GitHub Issues**: Reportar bugs, proponer features
- **Pull Requests**: Contribuir código
- **Discussions**: Preguntas y discusiones

## 📖 Recursos Adicionales

### C++ Moderno

- [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/)
- [cppreference.com](https://en.cppreference.com/)

### Patrones de Diseño

- [Refactoring Guru](https://refactoring.guru/design-patterns)
- Game Programming Patterns (libro)

### Testing

- [Catch2 Docs](https://github.com/catchorg/Catch2/tree/devel/docs)
- [TDD Best Practices](https://testdriven.io/)

## ❓ FAQ

**P: ¿Cómo ejecuto el juego después de compilar?**  
R: `./build/src/dangerdeep --datadir data/`

**P: ¿Los tests fallan en mi máquina?**  
R: Verifica que `DFTD_DATA` apunte a `data/`: `export DFTD_DATA=$(pwd)/data`

**P: ¿Cómo agrego una nueva feature?**  
R: Lee [ARCHITECTURE.md](ARCHITECTURE.md), crea branch, implementa, agrega tests, PR.

**P: ¿Puedo contribuir sin ser experto en C++?**  
R: ¡Sí! Mejoras de documentación, tests, y corrección de bugs son bienvenidas.

**P: ¿Cuánto tiempo toma compilar?**  
R: ~30-60 segundos en hardware moderno (con ccache más rápido).

---

**¡Bienvenido al proyecto y feliz coding!** 🚀

Para preguntas: abre un issue en GitHub o revisa la documentación en `docs/`.

---

**Última actualización**: 2026-03-03
