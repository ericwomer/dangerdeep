# Uso de la librería estándar de C++

Documento sobre sustituciones de código propio por la estándar (C++11/17/20).

---

## Hecho

### Aleatorios

- **random_generator** (`src/random_generator.h`): La clase mantiene la interfaz `rnd()`, `rndf()`, `set_seed()` pero internamente usa **`std::mt19937`** (`<random>`).
- **rnd() global**: Extraído a **`rnd.h` / `rnd.cpp`** con **`std::mt19937`**. `global_data.h` incluye `rnd.h`. `seed_global_rnd(unsigned)` se llama en los puntos de entrada (subsim, portal, ai, etc.). Test: **rnd_global_test**.
- **string_split**: Extraído a **`string_split.h` / `string_split.cpp`**, implementado con **`std::istringstream`** y **`std::getline`** (compatibilidad con cadena vacía y trailing separator). Test: **string_split_test**.
- **ulog2, nextgteqpow2, ispow2**: Tests en **global_data_utils_test**; en C++20 se pueden sustituir por `std::bit_width`, `std::bit_ceil`, `std::has_single_bit`.

---

## Candidatos a sustituir por la std

### C++17 (ya en uso)

| Código actual | Sustitución estándar | Esfuerzo / notas |
|---------------|----------------------|-------------------|
| **atoi / atof** (xml.cpp, cfg.cpp, parser) | **std::stoi**, **std::stod**, **std::stoul** | Bajo. Mejor manejo de errores (excepciones). Revisar que el comportamiento en entradas inválidas sea el deseado. |
| **str(T)** (global_data.h) | Ya usa **std::ostringstream**; para números se podría usar **std::to_string** en overloads concretos | Opcional. El template actual es genérico y correcto. |
| Rutas y archivos (datadirs, filehelper) | **std::filesystem** (C++17) para exists, is_directory, path concatenation | Medio. Reduciría dependencias de utilidades propias. |
| Tipo “optional” (si existiera uno propio) | **std::optional** (C++17) | Bajo si el uso es local. |

### C++20 (si se sube el estándar)

| Código actual | Sustitución estándar | Esfuerzo / notas |
|---------------|----------------------|-------------------|
| **ulog2**, **nextgteqpow2**, **ispow2** (global_data.h) | **std::bit_width**, **std::bit_ceil**, **std::has_single_bit** | Bajo. Comportamiento equivalente. |
| **string_split** (global_data.cpp, devuelve list\<string\>) | Lógica con **std::views::split** (C++20) o mantener actual con **std::stringstream** + **getline** | Bajo/medio. Si se queda en C++17, el código actual es aceptable. |

### Concurrency (C++11)

| Código actual | Sustitución estándar | Esfuerzo / notas |
|---------------|----------------------|-------------------|
| **mutex** (src/mutex.h/cpp) | **std::mutex** + **std::lock_guard** / **std::unique_lock** | Medio. Cambiar todos los usos de lock/unlock por RAII. |
| **condvar** (src/condvar.h/cpp) | **std::condition_variable** | Medio. Va ligado al cambio de mutex. |
| **thread** (src/thread.h/cpp) | **std::thread** | Alto. La clase actual tiene API propia (auto_ptr al worker, etc.); hay que migrar usos. |

### Fecha/hora

| Código actual | Sustitución estándar | Esfuerzo / notas |
|---------------|----------------------|-------------------|
| **date** (src/date.h/cpp) | **std::chrono** (C++11) para tiempo lineal; **std::chrono::year_month_day** (C++20) para fechas civiles | Alto. La clase `date` está muy integrada (XML, juego). Solo tiene sentido si se hace una migración gradual. |

### Otros

| Código actual | Sustitución estándar | Esfuerzo / notas |
|---------------|----------------------|-------------------|
| **rand()** restante (ocean_wave_generator.h, perlinnoise.cpp, stars.cpp, particle.cpp, ai.cpp, make_mesh.cpp) | Usar **rnd()** global o un **std::mt19937** local según el contexto | Bajo a medio. Unifica generación y calidad de aleatorios. |
| **ptrlist** / **ptrvector** | **std::vector\<std::unique_ptr\<T\>\>** o similar | Alto. Cambio de interfaz en muchos sitios; los contenedores actuales ya delegan en unique_ptr. |

---

## Resumen

- **Hecho:** Aleatorios (clase `random_generator` y función global `rnd()`) usan **`<random>`** y **std::mt19937**.
- **Fácil:** atoi/atof → std::stoi/std::stod; en C++20, ulog2/nextgteqpow2/ispow2 → std::bit_*.
- **Medio:** mutex/condvar → std::mutex/std::condition_variable; datadirs → std::filesystem; sustituir rand() restante por rnd() o std::mt19937.
- **Alto:** thread → std::thread; date → std::chrono; ptrlist/ptrvector → contenedores estándar.

Cuando se toque código en alguna de estas áreas, conviene valorar si compensa usar la estándar (menos código propio, mejor portabilidad y posibles optimizaciones del compilador).
