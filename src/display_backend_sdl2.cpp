/*
 * Danger from the Deep - Backend de display SDL2 (ventana, GL, eventos)
 */

#include "display_backend.h"
#include <SDL.h>
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
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        sdl_throw("video init failed");
}

void display_quit_video() {
    SDL_Quit();
}

void display_get_version(int* major, int* minor, int* patch) {
    SDL_version v;
    SDL_GetVersion(&v);
    *major = v.major;
    *minor = v.minor;
    *patch = v.patch;
}

void display_get_available_resolutions(int display_index, std::list<vector2i>& out) {
    out.clear();
    int n = SDL_GetNumDisplayModes(display_index);
    if (n <= 0)
        sdl_throw("Failed to query number of display modes");
    SDL_DisplayMode dm;
    for (int i = 0; i < n; ++i) {
        if (SDL_GetDisplayMode(display_index, i, &dm) == 0)
            out.push_back(vector2i(dm.w, dm.h));
    }
}

void display_create_window(const char* caption, int w, int h, bool fullscreen,
                          bool multisample, int multisample_level, int vsync,
                          void** out_window, void** out_glcontext) {
    Uint32 flags = SDL_WINDOW_OPENGL;
    if (fullscreen)
        flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;

    if (SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1) < 0)
        sdl_throw("setting double buffer failed");
    if (SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, multisample ? 1 : 0) < 0)
        sdl_throw("setting multisampling failed");
    if (SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, multisample_level) < 0)
        sdl_throw("setting multisample level failed");

    SDL_Window* win = SDL_CreateWindow(caption,
                                      SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                      w, h, flags);
    if (!win)
        sdl_throw("Video mode set failed");

    SDL_GLContext ctx = SDL_GL_CreateContext(win);
    if (!ctx) {
        SDL_DestroyWindow(win);
        sdl_throw("Couldn't create OpenGL context");
    }

    if (SDL_GL_SetSwapInterval(vsync) < 0)
        sdl_throw("setting VSync failed");

    *out_window = win;
    *out_glcontext = ctx;
}

void display_destroy_window(void* window, void* glcontext) {
    if (glcontext)
        SDL_GL_DeleteContext(static_cast<SDL_GLContext>(glcontext));
    if (window)
        SDL_DestroyWindow(static_cast<SDL_Window*>(window));
}

void display_set_window_size(void* window, int w, int h) {
    SDL_SetWindowSize(static_cast<SDL_Window*>(window), w, h);
}

void display_get_window_size(void* window, int* w, int* h) {
    SDL_GetWindowSize(static_cast<SDL_Window*>(window), w, h);
}

uint32_t display_get_window_id(void* window) {
    return SDL_GetWindowID(static_cast<SDL_Window*>(window));
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
    return SDL_GetTicks();
}

void display_delay(uint32_t ms) {
    SDL_Delay(ms);
}

void display_setup_events_and_cursor() {
    SDL_EventState(SDL_WINDOWEVENT_SIZE_CHANGED, SDL_IGNORE);
    SDL_EventState(SDL_SYSWMEVENT, SDL_IGNORE);
    SDL_JoystickEventState(SDL_IGNORE);
    SDL_ShowCursor(SDL_ENABLE);
}

namespace {

game_event sdl_to_game(const SDL_Event& e) {
    game_event ge;
    switch (e.type) {
    case SDL_QUIT:
        ge.type = event_type::QUIT;
        break;
    case SDL_KEYDOWN:
        ge.type = event_type::KEY_DOWN;
        ge.keysym.sym = static_cast<key_code>(e.key.keysym.sym);
        ge.keysym.scancode = e.key.keysym.scancode;
        ge.keysym.mod = static_cast<key_mod>(e.key.keysym.mod);
        break;
    case SDL_KEYUP:
        ge.type = event_type::KEY_UP;
        ge.keysym.sym = static_cast<key_code>(e.key.keysym.sym);
        ge.keysym.scancode = e.key.keysym.scancode;
        ge.keysym.mod = static_cast<key_mod>(e.key.keysym.mod);
        break;
    case SDL_MOUSEMOTION:
        ge.type = event_type::MOUSE_MOTION;
        ge.motion_x = e.motion.x;
        ge.motion_y = e.motion.y;
        ge.motion_xrel = e.motion.xrel;
        ge.motion_yrel = e.motion.yrel;
        ge.motion_state = e.motion.state;
        break;
    case SDL_MOUSEBUTTONDOWN:
        ge.type = event_type::MOUSE_BUTTON_DOWN;
        ge.button_button = e.button.button;
        ge.button_x = e.button.x;
        ge.button_y = e.button.y;
        break;
    case SDL_MOUSEBUTTONUP:
        ge.type = event_type::MOUSE_BUTTON_UP;
        ge.button_button = e.button.button;
        ge.button_x = e.button.x;
        ge.button_y = e.button.y;
        break;
    case SDL_MOUSEWHEEL:
        ge.type = event_type::MOUSE_WHEEL;
        ge.wheel_x = static_cast<float>(e.wheel.x);
        ge.wheel_y = static_cast<float>(e.wheel.y);
        break;
    case SDL_WINDOWEVENT:
        ge.type = event_type::WINDOW_EVENT;
        ge.window_id = e.window.windowID;
        ge.window_data1 = e.window.data1;
        ge.window_data2 = e.window.data2;
        if (e.window.event == SDL_WINDOWEVENT_FOCUS_LOST)
            ge.window_event = window_event_type::FOCUS_LOST;
        else if (e.window.event == SDL_WINDOWEVENT_FOCUS_GAINED)
            ge.window_event = window_event_type::FOCUS_GAINED;
        else if (e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
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

void display_save_bmp_rgb(const char* path, const uint8_t* rgb, int w, int h) {
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    Uint32 rmask = 0xff000000, gmask = 0x00ff0000, bmask = 0x0000ff00;
#else
    Uint32 rmask = 0x000000ff, gmask = 0x0000ff00, bmask = 0x00ff0000;
#endif
    SDL_Surface* surf = SDL_CreateRGBSurfaceFrom(
        const_cast<uint8_t*>(rgb), w, h, 24, w * 3, rmask, gmask, bmask, 0);
    if (surf) {
        SDL_SaveBMP(surf, path);
        SDL_FreeSurface(surf);
    }
}
