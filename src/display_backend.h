/*
 * Danger from the Deep - Abstracción de eventos de display
 * Para migración SDL2 → SDL3.
 */

#ifndef DISPLAY_BACKEND_H
#define DISPLAY_BACKEND_H

#include "game_event.h"
#include <list>

/// Convierte eventos SDL a game_event. En SDL2 usa SDL_PollEvent.
/// El sistema debe estar inicializado (SDL_Init, ventana creada).
std::list<game_event> poll_display_events();

/// Nombre legible de tecla
const char* get_key_name(key_code k);

#endif
