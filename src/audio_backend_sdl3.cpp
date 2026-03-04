/*
 * Danger from the Deep - Open source submarine simulation
 * Copyright (C) 2003-2006  Thorsten Jordan, Luis Barrancos and others.
 *
 * Implementación del backend de audio usando SDL3_mixer.
 */

#include "audio_backend.h"
#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <cmath>
#include <vector>

namespace {

struct sdl3_music_handle : audio_music_handle {
    MIX_Audio* audio;
    explicit sdl3_music_handle(MIX_Audio* a) : audio(a) {}
    ~sdl3_music_handle() override { MIX_DestroyAudio(audio); }
};

struct sdl3_chunk_handle : audio_chunk_handle {
    MIX_Audio* audio;
    explicit sdl3_chunk_handle(MIX_Audio* a) : audio(a) {}
    ~sdl3_chunk_handle() override { MIX_DestroyAudio(audio); }
};

class audio_backend_sdl3 : public audio_backend {
    SDL_AudioSpec mix_spec{};
    MIX_Mixer* mixer = nullptr;
    MIX_Track* music_track = nullptr;
    std::vector<MIX_Track*> channel_tracks;
    void (*music_finished_cb)() = nullptr;

    static void track_stopped_callback(void* userdata, MIX_Track* /*track*/) {
        auto* self = static_cast<audio_backend_sdl3*>(userdata);
        if (self->music_finished_cb)
            self->music_finished_cb();
    }

  public:
    ~audio_backend_sdl3() override {
        close_audio();
    }

    bool init_audio_subsystem() override {
        return SDL_Init(SDL_INIT_AUDIO);
    }

    bool open_audio(unsigned rate, unsigned short format, unsigned channels, unsigned buffers) override {
        if (mixer)
            return true;
        mix_spec.format = (format != 0) ? static_cast<SDL_AudioFormat>(format) : SDL_AUDIO_S16;
        mix_spec.channels = static_cast<int>(channels);
        mix_spec.freq = static_cast<int>(rate);
        mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &mix_spec);
        if (!mixer)
            return false;
        music_track = MIX_CreateTrack(mixer);
        return music_track != nullptr;
    }

    void close_audio() override {
        if (mixer) {
            for (MIX_Track* t : channel_tracks) {
                if (t)
                    MIX_SetTrackStoppedCallback(t, nullptr, nullptr);
            }
            channel_tracks.clear();
            if (music_track) {
                MIX_SetTrackStoppedCallback(music_track, nullptr, nullptr);
                music_track = nullptr;
            }
            MIX_DestroyMixer(mixer);
            mixer = nullptr;
        }
    }

    audio_music_handle* load_music(const std::string& path) override {
        if (!mixer)
            return nullptr;
        MIX_Audio* a = MIX_LoadAudio(mixer, path.c_str(), false);
        if (!a)
            return nullptr;
        return new sdl3_music_handle(a);
    }

    void free_music(audio_music_handle* m) override {
        delete static_cast<sdl3_music_handle*>(m);
    }

    audio_chunk_handle* load_chunk(const std::string& path) override {
        if (!mixer)
            return nullptr;
        MIX_Audio* a = MIX_LoadAudio(mixer, path.c_str(), false);
        if (!a)
            return nullptr;
        return new sdl3_chunk_handle(a);
    }

    void free_chunk(audio_chunk_handle* c) override {
        delete static_cast<sdl3_chunk_handle*>(c);
    }

    bool play_music(audio_music_handle* m, int loops, int fade_ms) override {
        if (!m || !mixer || !music_track)
            return false;
        MIX_Audio* a = static_cast<sdl3_music_handle*>(m)->audio;
        MIX_SetTrackAudio(music_track, a);
        MIX_SetTrackStoppedCallback(music_track, track_stopped_callback, this);
        SDL_PropertiesID opts = SDL_CreateProperties();
        if (fade_ms > 0)
            SDL_SetNumberProperty(opts, MIX_PROP_PLAY_FADE_IN_MILLISECONDS_NUMBER, fade_ms);
        SDL_SetNumberProperty(opts, MIX_PROP_PLAY_LOOPS_NUMBER, loops);
        bool ok = MIX_PlayTrack(music_track, opts);
        SDL_DestroyProperties(opts);
        return ok;
    }

    void halt_music() override {
        if (music_track)
            MIX_StopTrack(music_track, 0);
    }

    bool is_playing_music() const override {
        return music_track && MIX_TrackPlaying(music_track);
    }

    bool is_paused_music() const override {
        return music_track && MIX_TrackPaused(music_track);
    }

    void pause_music() override {
        if (music_track)
            MIX_PauseTrack(music_track);
    }

    void resume_music() override {
        if (music_track)
            MIX_ResumeTrack(music_track);
    }

    void rewind_music() override {
        if (music_track)
            MIX_SetTrackPlaybackPosition(music_track, 0);
    }

    bool set_music_position(double pos) override {
        if (!music_track)
            return false;
        return MIX_SetTrackPlaybackPosition(music_track, pos);
    }

    void fade_out_music(int ms) override {
        if (music_track) {
            Sint64 fade_frames = MIX_TrackMSToFrames(music_track, ms);
            MIX_StopTrack(music_track, fade_frames);
        }
    }

    int play_channel(int channel, audio_chunk_handle* chunk, int loops) override {
        if (!chunk || !mixer)
            return -1;
        if (channel < 0) {
            for (size_t i = 0; i < channel_tracks.size(); ++i) {
                if (!MIX_TrackPlaying(channel_tracks[i])) {
                    channel = static_cast<int>(i);
                    break;
                }
            }
            if (channel < 0) {
                channel = static_cast<int>(channel_tracks.size());
                MIX_Track* t = MIX_CreateTrack(mixer);
                if (!t)
                    return -1;
                channel_tracks.push_back(t);
            }
        }
        while (static_cast<int>(channel_tracks.size()) <= channel) {
            MIX_Track* t = MIX_CreateTrack(mixer);
            if (!t)
                return -1;
            channel_tracks.push_back(t);
        }
        MIX_Track* tr = channel_tracks[channel];
        MIX_SetTrackAudio(tr, static_cast<sdl3_chunk_handle*>(chunk)->audio);
        SDL_PropertiesID opts = 0;
        if (loops != 0) {
            opts = SDL_CreateProperties();
            SDL_SetNumberProperty(opts, MIX_PROP_PLAY_LOOPS_NUMBER, loops);
        }
        MIX_PlayTrack(tr, opts);
        if (opts)
            SDL_DestroyProperties(opts);
        return channel;
    }

    bool is_channel_playing(int channel) const override {
        if (channel < 0 || static_cast<size_t>(channel) >= channel_tracks.size())
            return false;
        return MIX_TrackPlaying(channel_tracks[channel]);
    }

    void halt_channel(int channel) override {
        if (channel >= 0 && static_cast<size_t>(channel) < channel_tracks.size())
            MIX_StopTrack(channel_tracks[channel], 0);
    }

    bool set_channel_position(int channel, short angle, unsigned char dist) override {
        if (channel < 0 || static_cast<size_t>(channel) >= channel_tracks.size())
            return false;
        float rad = angle * (3.14159265f / 180.0f);
        float d = (255 - dist) / 255.0f * 100.0f;
        MIX_Point3D pos = { std::cos(rad) * d, 0.0f, std::sin(rad) * d };
        return MIX_SetTrack3DPosition(channel_tracks[channel], &pos);
    }

    bool allocate_channels(int n) override {
        if (!mixer)
            return false;
        while (channel_tracks.size() < static_cast<size_t>(n)) {
            MIX_Track* t = MIX_CreateTrack(mixer);
            if (!t)
                return false;
            channel_tracks.push_back(t);
        }
        return true;
    }

    bool reserve_channels(int n) override {
        return allocate_channels(n);
    }

    void set_music_finished_callback(void (*callback)()) override {
        music_finished_cb = callback;
    }

    void pause_channel(int channel) override {
        if (channel < 0) {
            MIX_PauseAllTracks(mixer);
        } else if (channel >= 0 && static_cast<size_t>(channel) < channel_tracks.size()) {
            MIX_PauseTrack(channel_tracks[channel]);
        }
    }

    void resume_channel(int channel) override {
        if (channel < 0) {
            MIX_ResumeAllTracks(mixer);
        } else if (channel >= 0 && static_cast<size_t>(channel) < channel_tracks.size()) {
            MIX_ResumeTrack(channel_tracks[channel]);
        }
    }

    const char* get_error() const override {
        return SDL_GetError();
    }
};

audio_backend_sdl3 g_backend_sdl3;

} // namespace

audio_backend* get_audio_backend() {
    return &g_backend_sdl3;
}
