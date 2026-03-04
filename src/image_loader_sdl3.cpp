/*
 * Danger from the Deep - Open source submarine simulation
 * Copyright (C) 2003-2006  Thorsten Jordan, Luis Barrancos and others.
 *
 * Implementación del backend de carga de imágenes usando SDL3_image.
 */

#include "image_loader.h"
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <cstring>

namespace {

class image_loader_sdl3 : public image_loader_backend {
  public:
    std::unique_ptr<image_data> load(const std::string& path) override {
        SDL_Surface* surf = IMG_Load(path.c_str());
        if (!surf)
            return nullptr;

        std::unique_ptr<image_data> result = surface_to_image_data(surf);
        SDL_DestroySurface(surf);
        return result;
    }

    const char* get_error() const override {
        return SDL_GetError();
    }

  private:
    static std::unique_ptr<image_data> surface_to_image_data(SDL_Surface* surf) {
        auto data = std::make_unique<image_data>();
        data->width = static_cast<unsigned>(surf->w);
        data->height = static_cast<unsigned>(surf->h);

        bool has_alpha = SDL_ISPIXELFORMAT_ALPHA(surf->format);

        /* Igual que SDL2: ABGR8888 en little-endian produce R,G,B,A en memoria
         * (correcto para OpenGL GL_RGBA). RGB24 da R,G,B directo. */
        SDL_PixelFormat dst_format;
        if (has_alpha) {
            dst_format = SDL_PIXELFORMAT_ABGR8888;
            data->bytes_per_pixel = 4;
            data->gl_format = 0x1908;  // GL_RGBA
        } else {
            dst_format = SDL_PIXELFORMAT_RGB24;
            data->bytes_per_pixel = 3;
            data->gl_format = 0x1907;  // GL_RGB
        }
        data->pitch = data->width * data->bytes_per_pixel;
        data->pixels.resize(data->width * data->height * data->bytes_per_pixel);

        SDL_Surface* converted = SDL_ConvertSurface(surf, dst_format);
        if (!converted)
            return nullptr;

        const unsigned char* src = static_cast<const unsigned char*>(converted->pixels);
        const unsigned src_pitch = converted->pitch;
        unsigned char* dst = data->pixels.data();
        const unsigned row_bytes = data->width * data->bytes_per_pixel;

        for (unsigned y = 0; y < data->height; ++y) {
            memcpy(dst, src, row_bytes);
            src += src_pitch;
            dst += data->pitch;
        }
        SDL_DestroySurface(converted);

        return data;
    }
};

image_loader_sdl3 g_loader_impl;

} // namespace

image_loader_backend* get_image_loader() {
    return &g_loader_impl;
}
