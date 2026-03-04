/*
 * Danger from the Deep - Códigos de tecla independientes de SDL
 * Valores compatibles con SDL2 (SDL_Keycode) para migración SDL2 → SDL3.
 * Define DFTD_KEYS_ONLY antes de incluir para usar en lugar de SDL.h (solo teclas).
 * Si no se define, este header no hace nada (SDL.h define SDLK_*).
 */
#ifndef DFTD_KEYS_H
#define DFTD_KEYS_H

#include <cstdint>

#if defined(DFTD_KEYS_ONLY) && !defined(SDL_keycode_h_)

constexpr int32_t DFTD_SDLK_SCANCODE_MASK = (1 << 30);
#define DFTD_SDLK_SCANCODE_TO_KEYCODE(X) ((X) | DFTD_SDLK_SCANCODE_MASK)

constexpr int32_t SDLK_UNKNOWN = 0;
constexpr int32_t SDLK_RETURN = '\r';
constexpr int32_t SDLK_ESCAPE = '\x1B';
constexpr int32_t SDLK_BACKSPACE = '\b';
constexpr int32_t SDLK_TAB = '\t';
constexpr int32_t SDLK_SPACE = ' ';
constexpr int32_t SDLK_PLUS = '+';
constexpr int32_t SDLK_MINUS = '-';
constexpr int32_t SDLK_COMMA = ',';
constexpr int32_t SDLK_PERIOD = '.';
constexpr int32_t SDLK_SLASH = '/';
constexpr int32_t SDLK_LESS = '<';
constexpr int32_t SDLK_GREATER = '>';
constexpr int32_t SDLK_DELETE = '\x7F';

constexpr int32_t SDLK_0 = '0';
constexpr int32_t SDLK_1 = '1';
constexpr int32_t SDLK_2 = '2';
constexpr int32_t SDLK_3 = '3';
constexpr int32_t SDLK_4 = '4';
constexpr int32_t SDLK_5 = '5';
constexpr int32_t SDLK_6 = '6';
constexpr int32_t SDLK_7 = '7';
constexpr int32_t SDLK_8 = '8';
constexpr int32_t SDLK_9 = '9';

constexpr int32_t SDLK_a = 'a';
constexpr int32_t SDLK_b = 'b';
constexpr int32_t SDLK_c = 'c';
constexpr int32_t SDLK_d = 'd';
constexpr int32_t SDLK_e = 'e';
constexpr int32_t SDLK_f = 'f';
constexpr int32_t SDLK_g = 'g';
constexpr int32_t SDLK_h = 'h';
constexpr int32_t SDLK_i = 'i';
constexpr int32_t SDLK_j = 'j';
constexpr int32_t SDLK_k = 'k';
constexpr int32_t SDLK_l = 'l';
constexpr int32_t SDLK_m = 'm';
constexpr int32_t SDLK_n = 'n';
constexpr int32_t SDLK_o = 'o';
constexpr int32_t SDLK_p = 'p';
constexpr int32_t SDLK_q = 'q';
constexpr int32_t SDLK_r = 'r';
constexpr int32_t SDLK_s = 's';
constexpr int32_t SDLK_t = 't';
constexpr int32_t SDLK_u = 'u';
constexpr int32_t SDLK_v = 'v';
constexpr int32_t SDLK_w = 'w';
constexpr int32_t SDLK_x = 'x';
constexpr int32_t SDLK_y = 'y';
constexpr int32_t SDLK_z = 'z';

constexpr int32_t SDLK_F1  = DFTD_SDLK_SCANCODE_TO_KEYCODE(58);
constexpr int32_t SDLK_F2  = DFTD_SDLK_SCANCODE_TO_KEYCODE(59);
constexpr int32_t SDLK_F3  = DFTD_SDLK_SCANCODE_TO_KEYCODE(60);
constexpr int32_t SDLK_F4  = DFTD_SDLK_SCANCODE_TO_KEYCODE(61);
constexpr int32_t SDLK_F5  = DFTD_SDLK_SCANCODE_TO_KEYCODE(62);
constexpr int32_t SDLK_F6  = DFTD_SDLK_SCANCODE_TO_KEYCODE(63);
constexpr int32_t SDLK_F7  = DFTD_SDLK_SCANCODE_TO_KEYCODE(64);
constexpr int32_t SDLK_F8  = DFTD_SDLK_SCANCODE_TO_KEYCODE(65);
constexpr int32_t SDLK_F9  = DFTD_SDLK_SCANCODE_TO_KEYCODE(66);
constexpr int32_t SDLK_F10 = DFTD_SDLK_SCANCODE_TO_KEYCODE(67);
constexpr int32_t SDLK_F11 = DFTD_SDLK_SCANCODE_TO_KEYCODE(68);
constexpr int32_t SDLK_F12 = DFTD_SDLK_SCANCODE_TO_KEYCODE(69);

constexpr int32_t SDLK_PRINTSCREEN = DFTD_SDLK_SCANCODE_TO_KEYCODE(70);
constexpr int32_t SDLK_PAUSE       = DFTD_SDLK_SCANCODE_TO_KEYCODE(72);
constexpr int32_t SDLK_HOME        = DFTD_SDLK_SCANCODE_TO_KEYCODE(74);
constexpr int32_t SDLK_PAGEUP      = DFTD_SDLK_SCANCODE_TO_KEYCODE(75);
constexpr int32_t SDLK_END         = DFTD_SDLK_SCANCODE_TO_KEYCODE(77);
constexpr int32_t SDLK_PAGEDOWN    = DFTD_SDLK_SCANCODE_TO_KEYCODE(78);
constexpr int32_t SDLK_RIGHT       = DFTD_SDLK_SCANCODE_TO_KEYCODE(79);
constexpr int32_t SDLK_LEFT        = DFTD_SDLK_SCANCODE_TO_KEYCODE(80);
constexpr int32_t SDLK_DOWN        = DFTD_SDLK_SCANCODE_TO_KEYCODE(81);
constexpr int32_t SDLK_UP          = DFTD_SDLK_SCANCODE_TO_KEYCODE(82);

constexpr int32_t SDLK_KP_PLUS   = DFTD_SDLK_SCANCODE_TO_KEYCODE(87);
constexpr int32_t SDLK_KP_MINUS  = DFTD_SDLK_SCANCODE_TO_KEYCODE(86);
constexpr int32_t SDLK_KP_1      = DFTD_SDLK_SCANCODE_TO_KEYCODE(89);
constexpr int32_t SDLK_KP_2      = DFTD_SDLK_SCANCODE_TO_KEYCODE(90);
constexpr int32_t SDLK_KP_3      = DFTD_SDLK_SCANCODE_TO_KEYCODE(91);
constexpr int32_t SDLK_KP_4      = DFTD_SDLK_SCANCODE_TO_KEYCODE(92);
constexpr int32_t SDLK_KP_5      = DFTD_SDLK_SCANCODE_TO_KEYCODE(93);
constexpr int32_t SDLK_KP_6      = DFTD_SDLK_SCANCODE_TO_KEYCODE(94);
constexpr int32_t SDLK_KP_7      = DFTD_SDLK_SCANCODE_TO_KEYCODE(95);
constexpr int32_t SDLK_KP_8      = DFTD_SDLK_SCANCODE_TO_KEYCODE(96);
constexpr int32_t SDLK_KP_9      = DFTD_SDLK_SCANCODE_TO_KEYCODE(97);
constexpr int32_t SDLK_KP_0      = DFTD_SDLK_SCANCODE_TO_KEYCODE(98);

constexpr int32_t SDLK_LSHIFT = DFTD_SDLK_SCANCODE_TO_KEYCODE(56);
constexpr int32_t SDLK_RSHIFT = DFTD_SDLK_SCANCODE_TO_KEYCODE(57);
constexpr int32_t SDLK_LCTRL  = DFTD_SDLK_SCANCODE_TO_KEYCODE(224);
constexpr int32_t SDLK_RCTRL  = DFTD_SDLK_SCANCODE_TO_KEYCODE(228);
constexpr int32_t SDLK_TAB    = DFTD_SDLK_SCANCODE_TO_KEYCODE(43);

#undef DFTD_SDLK_SCANCODE_TO_KEYCODE

#endif /* DFTD_KEYS_ONLY && !SDL_keycode_h_ */
#endif
