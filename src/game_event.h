/*
 * Danger from the Deep - Eventos de juego independientes de SDL
 * Para migración SDL2 → SDL3. Sustituye SDL_Event.
 */

#ifndef GAME_EVENT_H
#define GAME_EVENT_H

#include "vector2.h"
#include <cstdint>

/// Códigos de tecla (compatibles con SDL_Keycode para facilitar migración)
using key_code = int32_t;
/// Modificadores (compatibles con SDL_Keymod)
using key_mod = uint16_t;
/// Código de scancode (compatibles con SDL_Scancode)
using scan_code = uint32_t;

/// Scancodes usados en el juego (valores SDL2, SDL_SCANCODE_KP_POWER=107)
constexpr scan_code SCANCODE_KP_POWER = 107;

/// Estructura tipo SDL_Keysym (SDL3 la eliminó)
/// Los key_code usan valores SDLK_* de SDL; incluir SDL.h donde se comparan.
struct key_sym {
    key_code sym;
    scan_code scancode;
    key_mod mod;
};

/// Modificadores de tecla (valores compatibles con SDL2 KMOD_*)
/// Prefijo DFTD para evitar conflicto con SDL_keycode.h
constexpr key_mod DFTD_KMOD_NONE   = 0x0000;
constexpr key_mod DFTD_KMOD_LSHIFT = 0x0001;
constexpr key_mod DFTD_KMOD_RSHIFT = 0x0002;
constexpr key_mod DFTD_KMOD_LCTRL  = 0x0040;
constexpr key_mod DFTD_KMOD_RCTRL  = 0x0080;
constexpr key_mod DFTD_KMOD_LALT   = 0x0100;
constexpr key_mod DFTD_KMOD_RALT   = 0x0200;
constexpr key_mod DFTD_KMOD_MODE   = 0x4000;

/// Botones del ratón
constexpr uint8_t MOUSE_BUTTON_LEFT   = 1;
constexpr uint8_t MOUSE_BUTTON_MIDDLE = 2;
constexpr uint8_t MOUSE_BUTTON_RIGHT  = 3;
constexpr uint8_t MOUSE_BUTTON_WHEELUP   = 4;
constexpr uint8_t MOUSE_BUTTON_WHEELDOWN = 5;

/// Máscaras de botones (compatibles con SDL_BUTTON_*MASK)
constexpr uint32_t MOUSE_BUTTON_LMASK = 1;
constexpr uint32_t MOUSE_BUTTON_MMASK = 2;
constexpr uint32_t MOUSE_BUTTON_RMASK = 4;

/// Tipos de evento
enum class event_type : uint32_t {
    NONE = 0,
    QUIT,
    KEY_DOWN,
    KEY_UP,
    MOUSE_MOTION,
    MOUSE_BUTTON_DOWN,
    MOUSE_BUTTON_UP,
    MOUSE_WHEEL,
    WINDOW_EVENT
};

/// Sub-tipos de ventana
enum class window_event_type : uint8_t {
    NONE,
    FOCUS_LOST,
    FOCUS_GAINED,
    SIZE_CHANGED
};

/// Evento de juego (sustituto de SDL_Event)
struct game_event {
    event_type type = event_type::NONE;

    // KEY_DOWN / KEY_UP
    key_sym keysym;

    // MOUSE_MOTION
    int32_t motion_x = 0;
    int32_t motion_y = 0;
    int32_t motion_xrel = 0;
    int32_t motion_yrel = 0;
    uint32_t motion_state = 0;  // máscara de botones

    // MOUSE_BUTTON_DOWN / MOUSE_BUTTON_UP
    uint8_t button_button = 0;
    int32_t button_x = 0;
    int32_t button_y = 0;

    // MOUSE_WHEEL
    float wheel_x = 0;
    float wheel_y = 0;

    // WINDOW_EVENT
    uint32_t window_id = 0;
    window_event_type window_event = window_event_type::NONE;
    int32_t window_data1 = 0;
    int32_t window_data2 = 0;
};

#endif
