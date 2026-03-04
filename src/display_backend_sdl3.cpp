/*
 * Danger from the Deep - Backend de display SDL3 (ventana, GL, eventos)
 */

#include "display_backend.h"
#include <SDL3/SDL.h>
#include <list>
#include <stdexcept>

static void sdl_throw(const char* msg) {
    const char* err = SDL_GetError();
    std::string full = msg;
    if (err && err[0]) {
        full += ": ";
        full += err;
    }
    throw std::runtime_error(full);
}

// --- Inicialización ---
void display_init_video() {
    if (!SDL_Init(SDL_INIT_VIDEO))  // SDL3: returns false on failure
        sdl_throw("video init failed");
}

void display_quit_video() {
    SDL_Quit();
}

void display_get_version(int* major, int* minor, int* patch) {
    int ver = SDL_GetVersion();
    *major = SDL_VERSIONNUM_MAJOR(ver);
    *minor = SDL_VERSIONNUM_MINOR(ver);
    *patch = SDL_VERSIONNUM_MICRO(ver);
}

void display_get_available_resolutions(int display_index, std::list<vector2i>& out) {
    out.clear();
    int num_displays = 0;
    SDL_DisplayID* displays = SDL_GetDisplays(&num_displays);
    if (!displays || display_index >= num_displays) {
        if (displays)
            SDL_free(displays);
        sdl_throw("Failed to query displays");
    }
    SDL_DisplayID display_id = displays[display_index];
    SDL_free(displays);

    int num_modes = 0;
    SDL_DisplayMode** modes = SDL_GetFullscreenDisplayModes(display_id, &num_modes);
    if (!modes) {
        sdl_throw("Failed to query display modes");
    }
    for (int i = 0; i < num_modes; ++i) {
        out.push_back(vector2i(modes[i]->w, modes[i]->h));
    }
    SDL_free(modes);
}

void display_create_window(const char* caption, int w, int h, bool fullscreen,
                          bool multisample, int multisample_level, int vsync,
                          void** out_window, void** out_glcontext) {
    if (!SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1))
        sdl_throw("setting double buffer failed");
    if (!SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, multisample ? 1 : 0))
        sdl_throw("setting multisampling failed");
    if (!SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, multisample_level))
        sdl_throw("setting multisample level failed");

    SDL_PropertiesID props = SDL_CreateProperties();
    SDL_SetStringProperty(props, SDL_PROP_WINDOW_CREATE_TITLE_STRING, caption);
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, w);
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, h);
    Uint64 flags = SDL_WINDOW_OPENGL | SDL_WINDOW_HIGH_PIXEL_DENSITY;
    if (fullscreen)
        flags |= SDL_WINDOW_FULLSCREEN;
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_FLAGS_NUMBER, flags);

    SDL_Window* win = SDL_CreateWindowWithProperties(props);
    SDL_DestroyProperties(props);
    if (!win)
        sdl_throw("Video mode set failed");

    SDL_GLContext ctx = SDL_GL_CreateContext(win);
    if (!ctx) {
        SDL_DestroyWindow(win);
        sdl_throw("Couldn't create OpenGL context");
    }

    if (!SDL_GL_MakeCurrent(win, ctx))
        sdl_throw("SDL_GL_MakeCurrent failed");

    if (!SDL_GL_SetSwapInterval(vsync))
        sdl_throw("setting VSync failed");

    *out_window = win;
    *out_glcontext = ctx;
}

void display_destroy_window(void* window, void* glcontext) {
    if (glcontext)
        SDL_GL_DestroyContext(static_cast<SDL_GLContext>(glcontext));
    if (window)
        SDL_DestroyWindow(static_cast<SDL_Window*>(window));
}

void display_set_window_size(void* window, int w, int h) {
    SDL_SetWindowSize(static_cast<SDL_Window*>(window), w, h);
}

void display_get_window_size(void* window, int* w, int* h) {
    /* SDL3: GetWindowSize devuelve tamaño lógico; OpenGL necesita píxeles reales
     * del framebuffer para glViewport (HiDPI: lógico ≠ píxeles).
     * Bombeamos eventos para que el WM tenga el tamaño final antes de consultar. */
    SDL_PumpEvents();
    if (!SDL_GetWindowSizeInPixels(static_cast<SDL_Window*>(window), w, h) || *w <= 0 || *h <= 0) {
        int lw, lh;
        SDL_GetWindowSize(static_cast<SDL_Window*>(window), &lw, &lh);
        float scale = SDL_GetWindowDisplayScale(static_cast<SDL_Window*>(window));
        *w = (lw > 0 && lh > 0 && scale > 0.f) ? static_cast<int>(lw * scale) : lw;
        *h = (lw > 0 && lh > 0 && scale > 0.f) ? static_cast<int>(lh * scale) : lh;
    }
}

uint32_t display_get_window_id(void* window) {
    return static_cast<uint32_t>(SDL_GetWindowID(static_cast<SDL_Window*>(window)));
}

void display_swap_buffers(void* window) {
    SDL_GL_SwapWindow(static_cast<SDL_Window*>(window));
}

void display_set_swap_interval(int interval) {
    SDL_GL_SetSwapInterval(interval);
}

const char* display_get_error() {
    return SDL_GetError();
}

uint32_t display_get_ticks() {
    return static_cast<uint32_t>(SDL_GetTicks());
}

void display_delay(uint32_t ms) {
    SDL_Delay(ms);
}

void display_setup_events_and_cursor() {
    /* Permitir PIXEL_SIZE_CHANGED para actualizar viewport cuando el WM asigna
     * el tamaño real (ej. tras creación inicial en HiDPI). */
    SDL_SetJoystickEventsEnabled(SDL_FALSE);
    SDL_ShowCursor();
}

namespace {

game_event sdl_to_game(const SDL_Event& e) {
    game_event ge;
    switch (e.type) {
    case SDL_EVENT_QUIT:
        ge.type = event_type::QUIT;
        break;
    case SDL_EVENT_KEY_DOWN:
        ge.type = event_type::KEY_DOWN;
        ge.keysym.sym = static_cast<key_code>(e.key.key);
        ge.keysym.scancode = e.key.scancode;
        ge.keysym.mod = static_cast<key_mod>(e.key.mod);
        break;
    case SDL_EVENT_KEY_UP:
        ge.type = event_type::KEY_UP;
        ge.keysym.sym = static_cast<key_code>(e.key.key);
        ge.keysym.scancode = e.key.scancode;
        ge.keysym.mod = static_cast<key_mod>(e.key.mod);
        break;
    case SDL_EVENT_MOUSE_MOTION:
        ge.type = event_type::MOUSE_MOTION;
        ge.motion_x = static_cast<int>(e.motion.x);
        ge.motion_y = static_cast<int>(e.motion.y);
        ge.motion_xrel = static_cast<int>(e.motion.xrel);
        ge.motion_yrel = static_cast<int>(e.motion.yrel);
        ge.motion_state = e.motion.state;
        break;
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
        ge.type = event_type::MOUSE_BUTTON_DOWN;
        ge.button_button = e.button.button;
        ge.button_x = static_cast<int>(e.button.x);
        ge.button_y = static_cast<int>(e.button.y);
        break;
    case SDL_EVENT_MOUSE_BUTTON_UP:
        ge.type = event_type::MOUSE_BUTTON_UP;
        ge.button_button = e.button.button;
        ge.button_x = static_cast<int>(e.button.x);
        ge.button_y = static_cast<int>(e.button.y);
        break;
    case SDL_EVENT_MOUSE_WHEEL:
        ge.type = event_type::MOUSE_WHEEL;
        ge.wheel_x = e.wheel.x;
        ge.wheel_y = e.wheel.y;
        break;
    case SDL_EVENT_WINDOW_FOCUS_LOST:
        ge.type = event_type::WINDOW_EVENT;
        ge.window_id = e.window.windowID;
        ge.window_event = window_event_type::FOCUS_LOST;
        break;
    case SDL_EVENT_WINDOW_FOCUS_GAINED:
        ge.type = event_type::WINDOW_EVENT;
        ge.window_id = e.window.windowID;
        ge.window_event = window_event_type::FOCUS_GAINED;
        break;
    case SDL_EVENT_WINDOW_RESIZED:
    case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
        ge.type = event_type::WINDOW_EVENT;
        ge.window_id = e.window.windowID;
        ge.window_data1 = static_cast<int>(e.window.data1);
        ge.window_data2 = static_cast<int>(e.window.data2);
        ge.window_event = window_event_type::SIZE_CHANGED;
        break;
    default:
        ge.type = event_type::NONE;
        break;
    }
    return ge;
}

} // namespace

std::list<game_event> poll_display_events() {
    std::list<game_event> events;
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        game_event ge = sdl_to_game(e);
        if (ge.type != event_type::NONE)
            events.push_back(ge);
    }
    return events;
}

const char* get_key_name(key_code k) {
    const char* s = SDL_GetKeyName(static_cast<SDL_Keycode>(k));
    return s ? s : "";
}

key_mod display_get_mod_state() {
    return static_cast<key_mod>(SDL_GetModState());
}

bool display_is_key_down(key_code k) {
    int numkeys = 0;
    const bool* state = SDL_GetKeyboardState(&numkeys);
    SDL_Scancode sc = SDL_GetScancodeFromKey(static_cast<SDL_Keycode>(k), nullptr);
    return state && static_cast<int>(sc) < numkeys && state[sc];
}

void display_save_bmp_rgb(const char* path, const uint8_t* rgb, int w, int h) {
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    SDL_PixelFormat format = SDL_GetPixelFormatForMasks(24, 0xff0000, 0x00ff00, 0x0000ff, 0);
#else
    SDL_PixelFormat format = SDL_GetPixelFormatForMasks(24, 0x0000ff, 0x00ff00, 0xff0000, 0);
#endif
    SDL_Surface* surf = SDL_CreateSurfaceFrom(w, h, format,
        const_cast<uint8_t*>(rgb), w * 3);
    if (surf) {
        SDL_SaveBMP(surf, path);
        SDL_DestroySurface(surf);
    }
}
