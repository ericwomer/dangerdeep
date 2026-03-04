/*
 * Danger from the Deep - Open source submarine simulation
 * Copyright (C) 2003-2006  Thorsten Jordan, Luis Barrancos and others.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Abstracción de carga de imágenes para migración SDL2_image → SDL3_image.
 * El código del juego no incluye tipos SDL; solo esta interfaz.
 */

#ifndef IMAGE_LOADER_H
#define IMAGE_LOADER_H

#include <memory>
#include <string>
#include <vector>

/// Datos de imagen en formato normalizado (RGB, RGBA o LUMINANCE)
struct image_data {
    unsigned width;
    unsigned height;
    unsigned pitch;          /// bytes por fila (puede ser > width*bpp)
    unsigned bytes_per_pixel; /// 1=LUMINANCE, 2=LUMINANCE_ALPHA, 3=RGB, 4=RGBA
    int gl_format;            /// GL_RGB, GL_RGBA, GL_LUMINANCE, GL_LUMINANCE_ALPHA
    std::vector<uint8_t> pixels;

    /// Compatibilidad con código que usa surf->w, surf->h (como SDL_Surface)
    unsigned w() const { return width; }
    unsigned h() const { return height; }
};

/// Interfaz de backend de carga de imágenes (SDL2_image o SDL3_image)
class image_loader_backend {
  public:
    virtual ~image_loader_backend() = default;

    /// Cargar imagen desde archivo. Devuelve nullptr en error.
    virtual std::unique_ptr<image_data> load(const std::string& path) = 0;

    /// Último mensaje de error
    virtual const char* get_error() const = 0;
};

/// Obtener backend actual (SDL2). Más adelante: factory según USE_SDL3
image_loader_backend* get_image_loader();

#endif
