<p align="center">
  <strong>Danger from the Deep</strong>
</p>
<p align="center">
  <em>Simulador de submarinos open source — Segunda Guerra Mundial</em>
</p>
<p align="center">
  <sub>v0.3.900 · GPL v2 · Linux</sub>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/C++-20-00599C?logo=cplusplus&logoColor=white" alt="C++20" />
  <img src="https://img.shields.io/badge/CMake-3.20+-064F8C?logo=cmake&logoColor=white" alt="CMake" />
  <img src="https://img.shields.io/badge/SDL2-2.0-8B0000?logo=sdl" alt="SDL2" />
  <img src="https://img.shields.io/badge/OpenGL-2.x-5586A4?logo=opengl" alt="OpenGL" />
  <img src="https://img.shields.io/badge/Linux-Wayland-FCC624?logo=linux&logoColor=black" alt="Linux" />
  <img src="https://img.shields.io/badge/License-GPL%20v2-blue" alt="GPL v2" />
</p>

<p align="center">
  <img src="https://github.com/cavazquez/dangerdeep/actions/workflows/ci.yml/badge.svg" alt="CI" />
  <img src="https://github.com/cavazquez/dangerdeep/actions/workflows/format.yml/badge.svg" alt="Format" />
  <img src="https://github.com/cavazquez/dangerdeep/actions/workflows/lint.yml/badge.svg" alt="Lint" />
  <img src="https://github.com/cavazquez/dangerdeep/actions/workflows/coverage.yml/badge.svg" alt="Coverage" />
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
- [CI/CD](#cicd)
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
| Sistema     | Linux (Wayland), OpenGL 2.x o superior |
| Compilador  | GCC o Clang, estándar C++20       |
| CMake       | 3.20 o superior                   |

Dependencias de sistema (las versiones son las que proporcione tu distro; no se fijan en el proyecto):

| Dependencia   | Uso en el proyecto | Versión mínima recomendada |
| ------------- | ------------------ | --------------------------- |
| **SDL2**      | Ventana, eventos   | 2.0.5+                      |
| **SDL2_image**| Carga de imágenes  | 2.0.x                       |
| **SDL2_mixer**| Audio / música     | 2.0.x                       |
| **FFTW3**     | FFT (sonar, etc.)  | 3.3.x                       |
| **TinyXML**   | XML (datos, config)| 2.6.x                       |
| **bzip2**     | Descompresión      | 1.0.x                       |
| **OpenGL**    | Render 3D          | 2.x (Mesa o drivers)        |

Puedes **actualizar** las dependencias con lo que ofrezca tu sistema (p. ej. en Debian/Ubuntu: `sudo apt update && sudo apt upgrade`). Al reconfigurar y recompilar, el proyecto usará las versiones instaladas. Si CMake muestra versiones al configurar (SDL2_MIXER_VERSION, etc.), son las que está usando.

### Posibles mejoras de dependencias

| Cambio | ¿Se puede? | Esfuerzo y notas |
|--------|------------|-------------------|
| **SDL2 → SDL3** | Sí | Medio–alto. API distinta (ventana, eventos, etc.) en muchas fuentes. SDL3 mejora Wayland y es la línea futura. |
| **FFTW3 → “FFTW4”** | No | No existe FFTW4; la rama estable actual es FFTW 3.3.x. El proyecto ya usa FFTW3. |
| **bzip2 → otro compresor** | Sí | Medio. Uso acotado: `bzip.h`/`bzip.cpp` y datos de tiles (`.bz2`). Se puede sustituir por **zstd** o **zlib** con una capa tipo stream similar; habría que reconvertir/recomprimir datos o soportar ambos formatos. |
| **OpenGL → Vulkan** | Sí, en teoría | Muy alto. Todo el render (shaders, texturas, modelos, agua, cielo, oglext, etc.) está en OpenGL. Implica reescribir el pipeline de render. |
| **X11 → Wayland** | Completado | El juego usa **SDL2** para ventana e input; se usa Wayland. |

El código usa C++20 (p. ej. `std::unique_ptr`, `std::make_unique`, `std::optional`, concepts, ranges cuando aplica).

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
  libglu1-mesa-dev
```

Opcionales para formato, lint, tests y cobertura:

```bash
sudo apt install clang-format cppcheck valgrind lcov
```

Opcional para **acelerar recompilaciones** (CMake lo usa automáticamente si está instalado):

```bash
sudo apt install ccache
```

---

## Compilación

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo
make -j$(nproc)
```

Si tienes **ccache** instalado, CMake lo usará como launcher del compilador; las recompilaciones serán mucho más rápidas al reutilizar objetos ya compilados (en la salida de `cmake` verás "ccache: activado").

El ejecutable se genera en `build/src/dangerdeep` (en algunas configuraciones puede estar en `build/dangerdeep`).

### Opciones de CMake

| Opción                     | Valores    | Descripción                                      |
| -------------------------- | ---------- | ------------------------------------------------- |
| `USE_CLANG`                | `ON`/`OFF` | Usar Clang en lugar de GCC                        |
| `USE_UNITY`                | `ON`/`OFF` | Activar unity builds                              |
| `BUILD_ASAN`               | `ON`/`OFF` | AddressSanitizer + LeakSanitizer (detección fugas) |
| `BUILD_VALGRIND_FRIENDLY`  | `ON`/`OFF` | Binario compatible con Valgrind (sin `-march=native`) |
| `BUILD_UNIT_TESTS`         | `ON`/`OFF` | Compilar tests unitarios (ptrlist_test, mutex_test, parser_test) |
| `BUILD_COVERAGE`           | `ON`/`OFF` | Cobertura de código (gcov; para reporte usar `./check.sh --coverage`) |

---

## Ejecución

Por defecto el juego usa el directorio `data/` del repositorio (CMake define la ruta absoluta al configurar). Desde la raíz del repo:

```bash
./build/src/dangerdeep
```

Para indicar otra ruta de datos en tiempo de ejecución:

```bash
./build/src/dangerdeep --datadir /ruta/a/data/
```

Para una instalación en el sistema, configurar con: `cmake .. -Ddatadir=/usr/local/share/dangerdeep/` (o la ruta donde se instalarán los datos).

---

## CI/CD

El proyecto usa **GitHub Actions** para integración y despliegue continuo. Todos los workflows se ejecutan automáticamente en cada push y pull request.

### Workflows Configurados

### Workflows Configurados

<p align="center">
  <img src="https://github.com/cavazquez/dangerdeep/actions/workflows/ci.yml/badge.svg" alt="CI" />
  <img src="https://github.com/cavazquez/dangerdeep/actions/workflows/format.yml/badge.svg" alt="Format" />
  <img src="https://github.com/cavazquez/dangerdeep/actions/workflows/coverage.yml/badge.svg" alt="Coverage" />
</p>

| Workflow | Qué verifica | Cuándo |
|----------|--------------|--------|
| **CI** | Build + Tests (98 tests) | Push / PR |
| **Format** | clang-format | Push / PR |
| **Coverage** | Cobertura de código | Push / PR (master, main) |
| **Lint** | cppcheck completo | Solo manual (Actions → Run workflow) |

> El lint completo (cppcheck) tarda varios minutos; se ejecuta manualmente desde la pestaña Actions de GitHub.

### Características

- ✅ **Build automático**: Compila en Ubuntu latest con todas las dependencias
- ✅ **98 tests**: Ejecuta toda la suite de tests unitarios (100% passing)
- ✅ **Cache**: ccache para compilación, apt para dependencias (builds más rápidos)
- ✅ **Format check**: Verifica que el código sigue el estilo del proyecto
- ✅ **Static analysis**: cppcheck detecta problemas potenciales
- ✅ **Coverage**: Genera reportes de cobertura de código
- ✅ **Artifacts**: Guarda logs y reportes para debugging

### Verificación Local

Antes de hacer push, ejecutá localmente las mismas verificaciones que CI:

```bash
# Verificar todo
./check.sh --format   # Formato
./check.sh --lint     # Lint
./check.sh --unit     # Tests

# O todo en uno
./check.sh --format && ./check.sh --lint && ./check.sh --unit
```

---

## Tests y calidad de código

El script `check.sh` centraliza operaciones de QA (formato, lint, compilación, tests). Sin argumentos muestra el menú de ayuda.

| Comando | Descripción |
| ------- | ----------- |
| `./check.sh` | Mostrar ayuda con todas las opciones |
| `./check.sh --build` | Compilar el proyecto |
| `./check.sh --force` | Forzar recompilación completa (make clean + make) |
| `./check.sh --reconfigure` | Regenerar makefiles de CMake + compilar |
| `./check.sh --clean` | Borrar `build/` y recompilar desde cero |
| `./check.sh --unit` | Compilar y ejecutar tests unitarios |
| `./check.sh --coverage` | Generar reporte de cobertura (líneas + branches) en `build/coverage/html/` |
| `./check.sh --format` | Verificar formato de código (clang-format) |
| `./check.sh --format-apply` | Aplicar formato automáticamente |
| `./check.sh --lint` | Análisis estático rápido (cppcheck) |
| `./check.sh --lint-full` | Análisis estático completo (warning, style, performance) |
| `./check.sh --asan` | Compilar con AddressSanitizer/LeakSanitizer y ejecutar tests |
| `./check.sh --valgrind` | Ejecutar bajo Valgrind (detección de memory leaks) |
| `./check.sh --opengl` | Test de capacidades OpenGL (dftdtester) |

Los tests unitarios usan **CTest**; `parser_test` necesita el directorio `data/` (variable de entorno `DFTD_DATA` o argumento con la ruta a `data`).  
Para el reporte de cobertura se usa **lcov** con *branch coverage*; instalación: `sudo apt install lcov`.

> Con **Valgrind**, el juego se lanza con `--datadir` apuntando a `data/`. Si los assets son punteros Git LFS, el script avisa y sugiere `git lfs pull`.  
> El script usa `valgrind-suppressions.supp` para ignorar fugas conocidas de librerías del sistema (NVIDIA, SDL2, PulseAudio, D-Bus); el test solo falla si hay fugas en código del proyecto.

---

## Estructura del proyecto

| Directorio     | Contenido |
| -------------- | --------- |
| `src/`         | Código C++ (simulación, gráficos, UI) |
| `data/`        | Datos del juego (modelos, texturas, sonidos, mapas; parte vía Git LFS) |
| `cmake/`       | Módulos de CMake |
| `docs/`        | Documentación adicional: [REFACTORING.md](docs/REFACTORING.md) (refactor), [STD_ALTERNATIVES.md](docs/STD_ALTERNATIVES.md) (uso de la std) |
| `packaging/`   | Scripts para empaquetado (p. ej. Ubuntu) |

### Stack técnico

| Categoría | Herramienta | Uso |
|-----------|--------------|-----|
| **Lenguaje** | C++20 | Core del proyecto |
| **Build** | CMake 3.20+ | Sistema de compilación |
| **Gráficos** | OpenGL 2.x | Render 3D |
| **Multimedia** | SDL2 | Ventanas, input, audio, imágenes |
| **Matemáticas** | FFTW3 | FFT (sonar, simulación) |
| **Datos** | TinyXML, BZip2 | Serialización, compresión |
| **Testing** | Catch2, CTest | 98 tests unitarios |
| **QA** | clang-format, cppcheck | Formato, análisis estático |
| **CI/CD** | GitHub Actions | Build, tests, coverage automáticos |

---

## Licencia

Este programa es software libre; podés redistribuirlo y/o modificarlo bajo los términos de la **GNU General Public License** v2 o posterior. Ver las cabeceras en el código y el texto de la GPL.
