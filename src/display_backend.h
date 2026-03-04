/*
 * Danger from the Deep - Abstracción de display (ventana, GL, eventos)
 * Para migración SDL2 → SDL3. system.cpp usa esta API en lugar de SDL directo.
 */

#ifndef DISPLAY_BACKEND_H
#define DISPLAY_BACKEND_H

#include "game_event.h"
#include "vector2.h"
#include <cstdint>
#include <list>

// --- Inicialización ---
/// Inicializa subsistema de video. Lanzar excepción en error.
void display_init_video();

/// Cierra subsistema de video
void display_quit_video();

/// Obtiene versión de SDL (para log en inicio)
void display_get_version(int* major, int* minor, int* patch);

// --- Resoluciones disponibles ---
/// Rellena out con resoluciones del display (índice 0 = principal)
void display_get_available_resolutions(int display_index, std::list<vector2i>& out);

// --- Ventana y contexto GL ---
/// Crea ventana y contexto GL. window y glcontext son handles opacos (SDL_Window*, SDL_GLContext).
/// multisample: usar multisampling. multisample_level: 0, 2, 4, etc.
/// vsync: 1=on, 0=off, -1=adaptive.
void display_create_window(const char* caption, int w, int h, bool fullscreen,
                          bool multisample, int multisample_level, int vsync,
                          void** out_window, void** out_glcontext);

void display_destroy_window(void* window, void* glcontext);

void display_set_window_size(void* window, int w, int h);
void display_get_window_size(void* window, int* w, int* h);
uint32_t display_get_window_id(void* window);

void display_swap_buffers(void* window);
void display_set_swap_interval(int interval);
const char* display_get_error();

// --- Tiempo y delay ---
uint32_t display_get_ticks();
void display_delay(uint32_t ms);

// --- Post-init (tras crear ventana) ---
void display_setup_events_and_cursor();

// --- Eventos ---
std::list<game_event> poll_display_events();
const char* get_key_name(key_code k);

// --- Screenshot ---
/// Guarda imagen RGB (3 bytes/pixel, row-major) como BMP
void display_save_bmp_rgb(const char* path, const uint8_t* rgb, int w, int h);

#endif
