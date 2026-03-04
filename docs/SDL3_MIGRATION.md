# Plan de migración SDL2 → SDL3

Migración mediante **abstracciones**: interfaces propias que ocultan SDL. Así cambiamos la implementación sin tocar todos los puntos de uso.

## Estrategia

1. **Definir interfaces** sin tipos SDL (solo tipos del juego: `vector2i`, `vector3`, etc.).
2. **Implementar backend SDL2** que cumple la interfaz (refactor sin cambiar comportamiento).
3. **Implementar backend SDL3** cuando el proyecto use SDL3.
4. **Seleccionar backend en tiempo de compilación** (CMAKE) o ejecución.

## Módulos a abstraer

| Módulo | Archivos afectados | API SDL actual | Complejidad |
|--------|-------------------|---------------|-------------|
| **Audio** | music.cpp/h | SDL2_mixer (Mix_*) | Alta |
| **Imagen** | texture.cpp, image.cpp, font.cpp, etc. | SDL2_image (IMG_Load, SDL_Surface) | Media |
| **Display/Eventos** | system.cpp, widget.cpp, subsim.cpp | SDL2 (window, events, init) | Alta |
| **Red** | network.cpp/h | SDL2_net | Baja |

## 1. Audio (`audio_backend`)

**Interfaz** (`src/audio_backend.h`):
```cpp
struct audio_backend {
    virtual ~audio_backend() = default;
    virtual bool init(unsigned rate, unsigned channels) = 0;
    virtual void shutdown() = 0;
    virtual bool open_audio(unsigned rate, unsigned channels, unsigned buffers) = 0;
    virtual void close_audio() = 0;
    virtual void* load_music(const char* path) = 0;   // opaque handle
    virtual void* load_chunk(const char* path) = 0;
    virtual void free_music(void* m) = 0;
    virtual void free_chunk(void* c) = 0;
    virtual bool play_music(void* m, int loops, int fade_ms) = 0;
    virtual void halt_music() = 0;
    virtual int play_channel(int ch, void* chunk, int loops) = 0;
    virtual void set_channel_position(int ch, short angle, unsigned char dist) = 0;
    virtual bool allocate_channels(int n) = 0;
    virtual bool reserve_channels(int n) = 0;
    virtual void set_music_finished_callback(void (*cb)()) = 0;
    virtual bool is_playing_music() = 0;
    virtual bool is_paused_music() = 0;
    virtual void pause_music() = 0;
    virtual void resume_music() = 0;
    virtual void rewind_music() = 0;
    virtual bool set_music_position(double pos) = 0;
    virtual void fade_out_music(int ms) = 0;
    virtual bool is_channel_playing(int ch) = 0;
    virtual void halt_channel(int ch) = 0;
    virtual void pause_all(int ch) = 0;
    virtual void resume_all(int ch) = 0;
};
```

**Implementaciones**: `audio_backend_sdl2.cpp`, `audio_backend_sdl3.cpp`

**Cliente**: `music` usa el backend inyectado (singleton o parámetro de construcción).

## 2. Carga de imágenes (`image_loader`) ✅ HECHO

**Interfaz** (`src/image_loader.h`):
- `image_data`: width, height, pitch, bytes_per_pixel, gl_format, pixels
- `image_loader_backend`: load(path), get_error()

**Implementación**: `image_loader_sdl2.cpp` (SDL2_image → image_data normalizado)
- JPG sin alpha → RGB (3 bpp)
- PNG u otras con alpha → RGBA (4 bpp)

**Cliente**: `sdl_image` usa `get_image_loader()->load()`, guarda `image_data`. `texture` usa `image_data_init()` en lugar de SDL_Surface. Eliminado `#include <SDL_image.h>` de: texture.h, image.cpp, font.cpp, coastmap.cpp, height_generator_map.cpp, global_data.cpp, system.cpp.

## 3. Display y eventos (`display_backend`)

**Interfaz** (simplificada):
- `init()`, `quit()`
- `create_window(w, h, fullscreen)` → handle opaco
- `poll_events()` → lista de eventos en formato propio (key, button, motion, etc.)
- `swap_buffers(window)`
- `get_window_size()` → `vector2i`

**Problema**: Muchos sitios usan `SDL_Event` directamente. Opciones:
- (A) Definir `game_event` con los campos necesarios; el backend traduce.
- (B) Mantener `SDL_Event` en la interfaz pero solo en el .cpp del backend; el resto del código no incluye SDL.

## 4. Red (`network_backend`)

Interfaz para TCP/UDP. `network.cpp` ya está aislado; basta con un backend que implemente `connect`, `send`, `recv`, etc.

## Orden de implementación sugerido

1. **Audio** – Mayor beneficio, API de mixer muy distinta en SDL3.
2. **Imagen** – `sdl_image` → `image_loader` + datos raw.
3. **Display** – Más invasivo; hacer tras audio e imagen.
4. **Red** – Menor impacto; último.

## Build: selección de backend

En CMake:
```cmake
option(USE_SDL3 "Build with SDL3" OFF)
if(USE_SDL3)
    find_package(SDL3 REQUIRED)
    find_package(SDL3_mixer REQUIRED)
    add_definitions(-DUSE_SDL3=1)
    set(AUDIO_BACKEND_SRC audio_backend_sdl3.cpp)
else()
    find_package(SDL2 REQUIRED)
    find_package(SDL2_mixer REQUIRED)
    set(AUDIO_BACKEND_SRC audio_backend_sdl2.cpp)
endif()
```

## Referencias

- [SDL 3.0 Migration Guide](https://wiki.libsdl.org/README-migration)
- [SDL3_mixer Migration](https://wiki.libsdl.org/SDL3_mixer/README-migration)
- [SDL3_image](https://wiki.libsdl.org/SDL3_image)
