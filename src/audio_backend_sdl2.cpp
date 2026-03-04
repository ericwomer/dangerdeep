/*
 * Danger from the Deep - Open source submarine simulation
 * Copyright (C) 2003-2006  Thorsten Jordan, Luis Barrancos and others.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Implementación del backend de audio usando SDL2_mixer.
 */

#include "audio_backend.h"
#include <SDL.h>
#include <SDL_mixer.h>

namespace {

struct sdl2_music_handle : audio_music_handle {
    Mix_Music* music;
    explicit sdl2_music_handle(Mix_Music* m) : music(m) {}
    ~sdl2_music_handle() override { Mix_FreeMusic(music); }
};

struct sdl2_chunk_handle : audio_chunk_handle {
    Mix_Chunk* chunk;
    explicit sdl2_chunk_handle(Mix_Chunk* c) : chunk(c) {}
    ~sdl2_chunk_handle() override { Mix_FreeChunk(chunk); }
};

class audio_backend_sdl2 : public audio_backend {
  public:
    bool init_audio_subsystem() override {
        return SDL_Init(SDL_INIT_AUDIO) == 0;
    }

    bool open_audio(unsigned rate, unsigned short format, unsigned channels, unsigned buffers) override {
        Uint16 sdl_format = (format != 0) ? static_cast<Uint16>(format) : MIX_DEFAULT_FORMAT;
        return Mix_OpenAudio(static_cast<int>(rate), sdl_format, static_cast<int>(channels), static_cast<int>(buffers)) == 0;
    }

    void close_audio() override {
        Mix_CloseAudio();
    }

    audio_music_handle* load_music(const std::string& path) override {
        Mix_Music* m = Mix_LoadMUS(path.c_str());
        if (!m)
            return nullptr;
        return new sdl2_music_handle(m);
    }

    void free_music(audio_music_handle* m) override {
        delete static_cast<sdl2_music_handle*>(m);
    }

    audio_chunk_handle* load_chunk(const std::string& path) override {
        Mix_Chunk* c = Mix_LoadWAV(path.c_str());
        if (!c)
            return nullptr;
        return new sdl2_chunk_handle(c);
    }

    void free_chunk(audio_chunk_handle* c) override {
        delete static_cast<sdl2_chunk_handle*>(c);
    }

    bool play_music(audio_music_handle* m, int loops, int fade_ms) override {
        if (!m)
            return false;
        Mix_Music* mix = static_cast<sdl2_music_handle*>(m)->music;
        int err;
        if (fade_ms > 0)
            err = Mix_FadeInMusic(mix, loops, fade_ms);
        else
            err = Mix_PlayMusic(mix, loops);
        return err == 0;
    }

    void halt_music() override {
        Mix_HaltMusic();
    }

    bool is_playing_music() const override {
        return Mix_PlayingMusic() != 0;
    }

    bool is_paused_music() const override {
        return Mix_PausedMusic() != 0;
    }

    void pause_music() override {
        Mix_PauseMusic();
    }

    void resume_music() override {
        Mix_ResumeMusic();
    }

    void rewind_music() override {
        Mix_RewindMusic();
    }

    bool set_music_position(double pos) override {
        return Mix_SetMusicPosition(pos) == 0;
    }

    void fade_out_music(int ms) override {
        Mix_FadeOutMusic(ms);
    }

    int play_channel(int channel, audio_chunk_handle* chunk, int loops) override {
        if (!chunk)
            return -1;
        Mix_Chunk* c = static_cast<sdl2_chunk_handle*>(chunk)->chunk;
        return Mix_PlayChannel(channel, c, loops);
    }

    bool is_channel_playing(int channel) const override {
        return Mix_Playing(channel) != 0;
    }

    void halt_channel(int channel) override {
        Mix_HaltChannel(channel);
    }

    bool set_channel_position(int channel, short angle, unsigned char dist) override {
        return Mix_SetPosition(channel, angle, dist) != 0;
    }

    bool allocate_channels(int n) override {
        return Mix_AllocateChannels(n) >= static_cast<int>(n);
    }

    bool reserve_channels(int n) override {
        return Mix_ReserveChannels(n) >= static_cast<int>(n);
    }

    void set_music_finished_callback(void (*callback)()) override {
        Mix_HookMusicFinished(callback);
    }

    void pause_channel(int channel) override {
        Mix_Pause(channel);
    }

    void resume_channel(int channel) override {
        Mix_Resume(channel);
    }

    const char* get_error() const override {
        return Mix_GetError();
    }
};

audio_backend_sdl2 g_backend_sdl2;

} // namespace

audio_backend* get_audio_backend() {
    return &g_backend_sdl2;
}
