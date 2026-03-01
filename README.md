<p align="center">
  <strong>Danger from the Deep</strong>
</p>
<p align="center">
  <em>Simulador de submarinos open source — Segunda Guerra Mundial</em>
</p>
<p align="center">
  <sub>v0.3.900 · GPL v2 · Linux</sub>
</p>

---

Comandá un U-Boot en un entorno 3D realista: sonar, TDC, torpedos y ataques a convoyes con física y sensores fieles a la época.

---

## Contenido

- [Datos del juego (Git LFS)](#datos-del-juego-git-lfs)
- [Requisitos](#requisitos)
- [Instalación](#instalación)
- [Compilación](#compilación)
- [Ejecución](#ejecución)
- [Tests y calidad de código](#tests-y-calidad-de-código)
- [Estructura del proyecto](#estructura-del-proyecto)
- [Licencia](#licencia)

---

## Datos del juego (Git LFS)

Los assets (fuentes, texturas, etc.) están en **Git LFS**. Si al arrancar aparece `failed to load ... font_arial.png` o el script avisa de punteros LFS, descargá los archivos reales:

```bash
sudo apt install git-lfs
git lfs install
git lfs pull
```

> Sin este paso, muchos archivos en `data/` son solo punteros y el juego no iniciará.

---

## Requisitos

| Requisito   | Detalle                          |
| ----------- | --------------------------------- |
| Sistema     | Linux (X11), OpenGL 2.x o superior |
| Compilador  | GCC o Clang, estándar C++11       |
| CMake       | 3.10 o superior                   |

El código usa C++11 (p. ej. `std::unique_ptr`); no se usa `std::auto_ptr`.

---

## Instalación

### Dependencias (Ubuntu / Debian)

```bash
sudo apt-get update
sudo apt-get install -y \
  libbz2-dev \
  libfftw3-dev \
  libsdl2-dev \
  libsdl2-image-dev \
  libsdl2-mixer-dev \
  libtinyxml-dev \
  libgl1-mesa-dev \
  libglu1-mesa-dev \
  libx11-dev
```

Opcionales para formato, lint, tests y cobertura:

```bash
sudo apt install clang-format cppcheck valgrind lcov
```

---

## Compilación

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo
make -j$(nproc)
```

El ejecutable se genera en `build/src/dangerdeep` (en algunas configuraciones puede estar en `build/dangerdeep`).

### Opciones de CMake

| Opción                     | Valores    | Descripción                                      |
| -------------------------- | ---------- | ------------------------------------------------- |
| `USE_CLANG`                | `ON`/`OFF` | Usar Clang en lugar de GCC                        |
| `USE_UNITY`                | `ON`/`OFF` | Activar unity builds                              |
| `BUILD_ASAN`               | `ON`/`OFF` | AddressSanitizer + LeakSanitizer (detección fugas) |
| `BUILD_VALGRIND_FRIENDLY`  | `ON`/`OFF` | Binario compatible con Valgrind (sin `-march=native`) |
| `BUILD_UNIT_TESTS`         | `ON`/`OFF` | Compilar tests unitarios (ptrlist_test, mutex_test, parser_test) |
| `BUILD_COVERAGE`           | `ON`/`OFF` | Cobertura de código (gcov; para reporte usar `./run_tests.sh --coverage`) |

---

## Ejecución

Desde la raíz del repositorio (para que se use el directorio `data/`):

```bash
./build/src/dangerdeep
```

Para indicar otra ruta de datos:

```bash
./build/src/dangerdeep --datadir /ruta/a/data/
```

---

## Tests y calidad de código

El script `run_tests.sh` aplica por defecto **formato** (clang-format) y **lint** (cppcheck). La compilación solo se ejecuta si se pasa `--build` o `-b`. Requiere **Bash** (si se invoca con `sh`, se reejecuta con `bash`).

| Comando | Descripción |
| ------- | ----------- |
| `./run_tests.sh` | Formato + lint (por defecto) |
| `./run_tests.sh --build` | Solo compilar (sin formato ni lint) |
| `./run_tests.sh --format` | Verificar formato (falla si hay diferencias) |
| `./run_tests.sh --lint` | Lint rápido |
| `./run_tests.sh --lint-full` | Lint completo (warning, style, performance) |
| `./run_tests.sh --asan -b` | Compilar con ASan+LSan y ejecutar tests unitarios (detecta fugas/errores de memoria) |
| `./run_tests.sh --valgrind -b` | Compilar compatible con Valgrind y ejecutar bajo Valgrind |
| `./run_tests.sh --valgrind` | Ejecutar bajo Valgrind (compilar antes con `--valgrind -b` si aparece «Instrucción ilegal») |
| `./run_tests.sh --unit` | Compilar y ejecutar **tests unitarios** (ptrlist_test, mutex_test, parser_test) |
| `./run_tests.sh --coverage` | Compilar con cobertura, ejecutar tests y generar **reporte de cobertura** (líneas + branches) en `build/coverage/html/` |
| `./run_tests.sh --opengl` | Test de capacidades OpenGL (dftdtester) |
| `./run_tests.sh --help` | Ver todas las opciones |

Los tests unitarios usan **CTest**; `parser_test` necesita el directorio `data/` (variable de entorno `DFTD_DATA` o argumento con la ruta a `data`).  
Para el reporte de cobertura se usa **lcov** con *branch coverage*; instalación: `sudo apt install lcov`.

> Con **Valgrind**, el juego se lanza con `--datadir` apuntando a `data/`. Si los assets son punteros Git LFS, el script avisa y sugiere `git lfs pull`.  
> El script usa `valgrind-suppressions.supp` para ignorar fugas conocidas de librerías del sistema (NVIDIA, X11, SDL2, PulseAudio, D-Bus); el test solo falla si hay fugas en código del proyecto.

---

## Estructura del proyecto

| Directorio     | Contenido |
| -------------- | --------- |
| `src/`         | Código C++ (simulación, gráficos, UI) |
| `data/`        | Datos del juego (modelos, texturas, sonidos, mapas; parte vía Git LFS) |
| `cmake/`       | Módulos de CMake |
| `packaging/`   | Scripts para empaquetado (p. ej. Ubuntu) |

Stack: SDL2 (vídeo, imagen, audio), OpenGL, FFTW, BZip2, TinyXML, X11.  
Los headers se incluyen por nombre (`#include "foo.h"`) desde `src/`; CMake añade las rutas de dependencias externas. No se usa precompiled header.

---

## Licencia

Este programa es software libre; podés redistribuirlo y/o modificarlo bajo los términos de la **GNU General Public License** v2 o posterior. Ver las cabeceras en el código y el texto de la GPL.
