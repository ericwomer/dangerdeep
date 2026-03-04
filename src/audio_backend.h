/*
 * Danger from the Deep - Open source submarine simulation
 * Copyright (C) 2003-2006  Thorsten Jordan, Luis Barrancos and others.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Abstracción de audio para permitir migración SDL2_mixer → SDL3_mixer.
 * El código del juego no incluye tipos SDL; solo esta interfaz.
 */

#ifndef AUDIO_BACKEND_H
#define AUDIO_BACKEND_H

#include <string>

/// Handle opaco para música cargada (el backend conoce el tipo real)
struct audio_music_handle {
    virtual ~audio_music_handle() = default;
  protected:
    audio_music_handle() = default;
};

/// Handle opaco para chunk de SFX
struct audio_chunk_handle {
    virtual ~audio_chunk_handle() = default;
  protected:
    audio_chunk_handle() = default;
};

/// Interfaz de backend de audio (SDL2_mixer o SDL3_mixer)
class audio_backend {
  public:
    virtual ~audio_backend() = default;

    /// Inicializar subsistema de audio (SDL_Init(SDL_INIT_AUDIO))
    virtual bool init_audio_subsystem() = 0;

    /// Abrir dispositivo de audio
    virtual bool open_audio(unsigned rate, unsigned short format, unsigned channels, unsigned buffers) = 0;

    /// Cerrar audio
    virtual void close_audio() = 0;

    /// Cargar música desde archivo. Devuelve nullptr en error.
    virtual audio_music_handle* load_music(const std::string& path) = 0;

    /// Liberar música
    virtual void free_music(audio_music_handle* m) = 0;

    /// Cargar chunk (WAV) desde archivo
    virtual audio_chunk_handle* load_chunk(const std::string& path) = 0;

    /// Liberar chunk
    virtual void free_chunk(audio_chunk_handle* c) = 0;

    /// Reproducir música (loops: -1 = infinito, fade_ms: 0 = sin fade)
    virtual bool play_music(audio_music_handle* m, int loops, int fade_ms) = 0;

    /// Detener música
    virtual void halt_music() = 0;

    /// ¿Está sonando música?
    virtual bool is_playing_music() const = 0;

    /// ¿Música en pausa?
    virtual bool is_paused_music() const = 0;

    /// Pausar música
    virtual void pause_music() = 0;

    /// Reanudar música
    virtual void resume_music() = 0;

    /// Rebobinar al inicio
    virtual void rewind_music() = 0;

    /// Ir a posición (segundos)
    virtual bool set_music_position(double pos) = 0;

    /// Fade out en ms
    virtual void fade_out_music(int ms) = 0;

    /// Reproducir chunk en canal (-1 = libre). Devuelve canal o -1 en error
    virtual int play_channel(int channel, audio_chunk_handle* chunk, int loops) = 0;

    /// ¿Canal activo?
    virtual bool is_channel_playing(int channel) const = 0;

    /// Detener canal
    virtual void halt_channel(int channel) = 0;

    /// Posicionamiento 3D (angle: 0-360, dist: 0-255)
    virtual bool set_channel_position(int channel, short angle, unsigned char dist) = 0;

    /// Asignar canales (mínimo)
    virtual bool allocate_channels(int n) = 0;

    /// Reservar canales al inicio
    virtual bool reserve_channels(int n) = 0;

    /// Callback cuando termina la música
    virtual void set_music_finished_callback(void (*callback)()) = 0;

    /// Pausar todos los canales (channel -1 = todos)
    virtual void pause_channel(int channel) = 0;

    /// Reanudar canales
    virtual void resume_channel(int channel) = 0;

    /// Último mensaje de error
    virtual const char* get_error() const = 0;
};

/// Obtener backend actual (SDL2). Más adelante: factory según USE_SDL3
audio_backend* get_audio_backend();

#endif
