/*
 * Danger from the Deep - Conversión de eventos SDL2 a game_event
 */

#include "display_backend.h"
#include <SDL.h>
#include <list>

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
