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

## 3. Display y eventos (`display_backend`) ✅ HECHO

**Interfaz** (`src/display_backend.h`):
- `display_init_video()`, `display_quit_video()`
- `display_get_version()`, `display_get_available_resolutions()`
- `display_create_window()`, `display_destroy_window()`
- `display_set_window_size()`, `display_get_window_size()`, `display_get_window_id()`
- `display_swap_buffers()`, `display_set_swap_interval()`
- `display_get_ticks()`, `display_delay()`
- `display_setup_events_and_cursor()`
- `poll_display_events()` → lista de `game_event`
- `display_save_bmp_rgb()` para screenshots

**Implementación**: `display_backend_sdl2.cpp` (SDL2)

**Cliente**: `system.cpp` usa la API de display_backend; ya no incluye `<SDL.h>`. Los handles de ventana y GL son `void*` opacos.

## 3b. Códigos de tecla (`dftd_keys.h`)

**Propósito**: Definir constantes SDLK_* sin depender de SDL, para que el código de juego no incluya SDL.h cuando solo necesita códigos de tecla.

**Uso**: `#define DFTD_KEYS_ONLY` antes de `#include "dftd_keys.h"` en archivos que solo necesitan teclas. Los valores son compatibles con SDL2.

**Constantes de botones**: `MOUSE_BUTTON_LEFT`, `MOUSE_BUTTON_LMASK`, etc. en `game_event.h` (ya migrados en map_display, sub_torpedo_display, widget).

## 4. Red (`network_backend`)

Interfaz para TCP/UDP. `network.cpp` ya está aislado; basta con un backend que implemente `connect`, `send`, `recv`, etc.

## Orden de implementación sugerido

1. **Audio** – Mayor beneficio, API de mixer muy distinta en SDL3.
2. **Imagen** – `sdl_image` → `image_loader` + datos raw.
3. **Display** – Más invasivo; hacer tras audio e imagen.
4. **Red** – Menor impacto; último.

## Estado actual de migración SDL3 (marzo 2025)

### ✅ Hecho
- **CMake**: Opción `USE_SDL3`, selección condicional de backends (display, audio, image)
- **display_backend_sdl3.cpp**: Ventana, GL, eventos, screenshots BMP. API SDL3: `SDL_GetDisplays(int*)`, `SDL_GetFullscreenDisplayModes(id, int*)`, `SDL_GetScancodeFromKey(key, mod*)`
- **image_loader_sdl3.cpp**: Carga de imágenes vía SDL3_image
- **audio_backend_sdl3.cpp**: Música y SFX vía SDL3_mixer (MIX_LoadAudio(mixer, path, predecode), MIX_StopTrack(track, fade), MIX_SetTrack3DPosition(track, &pos), callback (void*, MIX_Track*))
- **texture.cpp**: Rama SDL3 en `sdl_init()`: SDL_ConvertSurface a RGBA/XRGB, luego `image_data_init()`
- **cmake/sdl3_compat/**: Wrappers para SDL.h (SDL_FALSE/TRUE, SDLK_x→SDLK_X, SwapLE16→Swap16LE)

### ✅ Compilación con USE_SDL3
La compilación con SDL3 **completa correctamente**.

### ⏳ Pendiente / verificar en runtime
- **Pruebas funcionales**: Ejecutar el juego con SDL3 y verificar audio, imagen y display.
- **Red (network)**: Módulo aún no migrado (SDL2_net → SDL3_net cuando exista).

### Cómo compilar
```bash
# SDL2 (por defecto)
cmake -B build && cmake --build build

# SDL3 (requiere libsdl3, libsdl3-mixer, libsdl3-image instalados)
cmake -B build -DUSE_SDL3=ON && cmake --build build
```

## Referencias

- [SDL 3.0 Migration Guide](https://wiki.libsdl.org/README-migration)
- [SDL3_mixer Migration](https://wiki.libsdl.org/SDL3_mixer/README-migration)
- [SDL3_image](https://wiki.libsdl.org/SDL3_image)
