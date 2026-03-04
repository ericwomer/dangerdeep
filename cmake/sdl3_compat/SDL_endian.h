#include <SDL3/SDL_endian.h>

/* SDL3 usa SDL_Swap16LE etc; SDL2 usaba SDL_SwapLE16. Nuestros defines corrigen
   el compat roto de SDL_oldnames.h (SDL_SwapLE16_renamed_SDL_Swap16LE no existe) */
#undef SDL_SwapLE16
#undef SDL_SwapLE32
#undef SDL_SwapLE64
#undef SDL_SwapBE16
#undef SDL_SwapBE32
#undef SDL_SwapBE64
#define SDL_SwapLE16 SDL_Swap16LE
#define SDL_SwapLE32 SDL_Swap32LE
#define SDL_SwapLE64 SDL_Swap64LE
#define SDL_SwapBE16 SDL_Swap16BE
#define SDL_SwapBE32 SDL_Swap32BE
#define SDL_SwapBE64 SDL_Swap64BE
