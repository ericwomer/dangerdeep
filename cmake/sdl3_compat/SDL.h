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
/* SDL3 SDL_oldnames.h usa SDLK_x_renamed_SDLK_X que falla en switch; #undef y #define correcto */
#undef SDLK_a
#define SDLK_a SDLK_A
#undef SDLK_b
#define SDLK_b SDLK_B
#undef SDLK_c
#define SDLK_c SDLK_C
#undef SDLK_d
#define SDLK_d SDLK_D
#undef SDLK_e
#define SDLK_e SDLK_E
#undef SDLK_f
#define SDLK_f SDLK_F
#undef SDLK_g
#define SDLK_g SDLK_G
#undef SDLK_h
#define SDLK_h SDLK_H
#undef SDLK_i
#define SDLK_i SDLK_I
#undef SDLK_j
#define SDLK_j SDLK_J
#undef SDLK_k
#define SDLK_k SDLK_K
#undef SDLK_l
#define SDLK_l SDLK_L
#undef SDLK_m
#define SDLK_m SDLK_M
#undef SDLK_n
#define SDLK_n SDLK_N
#undef SDLK_o
#define SDLK_o SDLK_O
#undef SDLK_p
#define SDLK_p SDLK_P
#undef SDLK_q
#define SDLK_q SDLK_Q
#undef SDLK_r
#define SDLK_r SDLK_R
#undef SDLK_s
#define SDLK_s SDLK_S
#undef SDLK_t
#define SDLK_t SDLK_T
#undef SDLK_u
#define SDLK_u SDLK_U
#undef SDLK_v
#define SDLK_v SDLK_V
#undef SDLK_w
#define SDLK_w SDLK_W
#undef SDLK_x
#define SDLK_x SDLK_X
#undef SDLK_y
#define SDLK_y SDLK_Y
#undef SDLK_z
#define SDLK_z SDLK_Z
