# Danger from the Deep

Simulador de submarinos open source ambientado en la Segunda Guerra Mundial. Comandá un U-Boot, usá sonar, TDC, torpedos y atacá convoyes en un entorno 3D con física y sensores realistas.

**Versión:** 0.3.900  
**Licencia:** GPL v2 (ver cabeceras en el código fuente)

---

## Requisitos

- **Sistema:** Linux (X11), con soporte OpenGL 2.x o superior
- **Compilador:** GCC o Clang con C++11
- **CMake:** 2.8 o superior (recomendado 3.10+)

---

## Dependencias de compilación (Ubuntu / Debian)

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
  libx11-dev
```

---

## Compilación

```bash
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo
make -j$(nproc)
```

Opciones de CMake:

| Opción      | Valores     | Descripción              |
|------------|-------------|---------------------------|
| `USE_CLANG`| `ON` / `OFF`| Usar Clang en lugar de GCC |
| `USE_UNITY`| `ON` / `OFF`| Activar unity builds      |

El ejecutable se genera en `build/dangerdeep`. Los datos se buscan en `data/` respecto al directorio de trabajo o en la ruta de instalación.

---

## Ejecución

Desde la raíz del repositorio (para que encuentre `data/`):

```bash
./build/dangerdeep
```

O tras instalar en el prefijo por defecto (`build/dist`):

```bash
./build/dist/bin/dangerdeep
```

---

## Tests

El script `run_tests.sh` compila el proyecto y verifica que el build termine correctamente:

```bash
./run_tests.sh
```

Si en el futuro se añade un ejecutable de test OpenGL (p. ej. `dftdtester`), podés usar:

```bash
./run_tests.sh --opengl
```

---

## Estructura del proyecto

| Directorio   | Contenido                                      |
|-------------|-------------------------------------------------|
| `src/`      | Código fuente C++ (simulación, gráficos, UI)    |
| `data/`     | Datos del juego (modelos, texturas, sonidos, mapas) |
| `cmake/`    | Módulos y scripts de CMake                      |
| `packaging/`| Scripts y recetas para empaquetado (p. ej. Ubuntu) |

El proyecto usa SDL2 (vídeo, imagen, audio), OpenGL, FFTW, BZip2, TinyXML y X11.

---

## Licencia

Este programa es software libre; podés redistribuirlo y/o modificarlo bajo los términos de la **GNU General Public License** versión 2 o posterior. Ver las cabeceras de los archivos fuente y la licencia GPL para más detalles.
