/*
 * Wrapper para compilación con USE_SDL3.
 * Redirige #include <SDL.h> a SDL3 e incluye compatibilidad SDL2.
 */
#include <SDL3/SDL.h>

/* Corregir SDL_oldnames.h que define macros rotas */
#undef SDL_SwapLE16
#undef SDL_SwapLE32
#undef SDL_SwapLE64
#define SDL_SwapLE16 SDL_Swap16LE
#define SDL_SwapLE32 SDL_Swap32LE
#define SDL_SwapLE64 SDL_Swap64LE
#undef SDL_FALSE
#undef SDL_TRUE
#define SDL_FALSE false
#define SDL_TRUE true
/* SDL3 renombró SDLK_x a SDLK_X para letras; mantener compatibilidad */
#define SDLK_a SDLK_A
#define SDLK_b SDLK_B
#define SDLK_c SDLK_C
#define SDLK_d SDLK_D
#define SDLK_e SDLK_E
#define SDLK_f SDLK_F
#define SDLK_g SDLK_G
#define SDLK_h SDLK_H
#define SDLK_i SDLK_I
#define SDLK_j SDLK_J
#define SDLK_k SDLK_K
#define SDLK_l SDLK_L
#define SDLK_m SDLK_M
#define SDLK_n SDLK_N
#define SDLK_o SDLK_O
#define SDLK_p SDLK_P
#define SDLK_q SDLK_Q
#define SDLK_r SDLK_R
#define SDLK_s SDLK_S
#define SDLK_t SDLK_T
#define SDLK_u SDLK_U
#define SDLK_v SDLK_V
#define SDLK_w SDLK_W
#define SDLK_x SDLK_X
#define SDLK_y SDLK_Y
#define SDLK_z SDLK_Z
