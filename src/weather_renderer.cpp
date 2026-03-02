/*
Danger from the Deep - Open source submarine simulation
Copyright (C) 2003-2006  Thorsten Jordan, Luis Barrancos and others.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

// weather renderer implementation
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "weather_renderer.h"
#include "matrix4.h"
#include "primitives.h"
#include "rnd.h"
#include "texture.h"
#include "vector2.h"
#include "vector3.h"
#include <glu.h>
#include <vector>

weather_renderer::weather_renderer() {
    init_rain();
    init_snow();
}

weather_renderer::~weather_renderer() {
}

void weather_renderer::init_rain() {
#ifdef RAIN
    const unsigned NR_OF_RAIN_FRAMES = 16;
    const unsigned NR_OF_RAIN_DROPS = 800;
    const unsigned RAIN_TEX_W = 256;
    const unsigned RAIN_TEX_H = 256;

    raintex.resize(NR_OF_RAIN_FRAMES);
    std::vector<Uint8> raintmptex(RAIN_TEX_W * RAIN_TEX_H * 2);

    for (unsigned j = 0; j < NR_OF_RAIN_FRAMES; ++j) {
        for (unsigned k = 0; k < RAIN_TEX_W * RAIN_TEX_H * 2; k += 2) {
            raintmptex[k + 0] = 128;
            raintmptex[k + 1] = 0;
        }
        for (unsigned i = 0; i < NR_OF_RAIN_DROPS; ++i) {
            vector2i pos(rnd(RAIN_TEX_W - 2) + 2, rnd(RAIN_TEX_H - 2));
            Uint8 c = rnd(64) + 128;
            raintmptex[(RAIN_TEX_W * pos.y + pos.x) * 2 + 0] = c;
            raintmptex[(RAIN_TEX_W * pos.y + pos.x) * 2 + 1] = 128;
            pos.x -= 1;
            pos.y += 1;
            raintmptex[(RAIN_TEX_W * pos.y + pos.x) * 2 + 0] = c;
            raintmptex[(RAIN_TEX_W * pos.y + pos.x) * 2 + 1] = 192;
            pos.x -= 1;
            pos.y += 1;
            raintmptex[(RAIN_TEX_W * pos.y + pos.x) * 2 + 0] = c;
            raintmptex[(RAIN_TEX_W * pos.y + pos.x) * 2 + 1] = 255;
        }
        raintex.reset(j, new texture(raintmptex, RAIN_TEX_W, RAIN_TEX_H, GL_LUMINANCE_ALPHA, texture::LINEAR_MIPMAP_LINEAR, texture::REPEAT));
    }
#endif
}

void weather_renderer::init_snow() {
#ifdef SNOW
    const unsigned NR_OF_SNOW_FRAMES = 23;
    const unsigned NR_OF_SNOW_FLAKES = 2000;
    const unsigned SNOW_TEX_W = 256;
    const unsigned SNOW_TEX_H = 256;

    snowtex.resize(NR_OF_SNOW_FRAMES);
    std::vector<Uint8> snowtmptex(SNOW_TEX_W * SNOW_TEX_H * 2, 255);
    std::vector<vector2i> snowflakepos(NR_OF_SNOW_FLAKES);
    std::vector<int> snowxrand(NR_OF_SNOW_FRAMES);

    // create random x coordinate sequence (perturbation)
    std::vector<unsigned> snowxtrans(SNOW_TEX_W);
    for (unsigned k = 0; k < SNOW_TEX_W; ++k) {
        snowxtrans[k] = k;
    }
    for (unsigned k = 0; k < SNOW_TEX_W * 20; ++k) {
        unsigned a = rnd(SNOW_TEX_W), b = rnd(SNOW_TEX_W);
        unsigned c = snowxtrans[a];
        snowxtrans[a] = snowxtrans[b];
        snowxtrans[b] = c;
    }

    for (unsigned j = 0; j < NR_OF_SNOW_FRAMES; ++j) {
        snowxrand[j] = rnd(3) - 1;
    }
    snowflakepos[0] = vector2i(snowxtrans[0], 0);
    for (unsigned i = 1; i < NR_OF_SNOW_FLAKES; ++i) {
        vector2i oldpos = snowflakepos[i - 1];
        for (unsigned j = 0; j < NR_OF_SNOW_FRAMES; ++j) {
            oldpos.x += snowxrand[(j + 3 * i) % NR_OF_SNOW_FRAMES];
            if (oldpos.x < 0)
                oldpos.x += SNOW_TEX_W;
            if (oldpos.x >= SNOW_TEX_W)
                oldpos.x -= SNOW_TEX_W;
            oldpos.y += 1;
            if (oldpos.y >= SNOW_TEX_H) {
                oldpos.x = snowxtrans[oldpos.x];
                oldpos.y = 0;
            }
        }
        snowflakepos[i] = oldpos;
    }
    for (unsigned i = 0; i < NR_OF_SNOW_FRAMES; ++i) {
        for (unsigned k = 0; k < SNOW_TEX_W * SNOW_TEX_H * 2; k += 2)
            snowtmptex[k + 1] = 0;
        for (unsigned j = 0; j < NR_OF_SNOW_FLAKES; ++j) {
            snowtmptex[(SNOW_TEX_H * snowflakepos[j].y + snowflakepos[j].x) * 2 + 1] = 255;
            vector2i &oldpos = snowflakepos[j];
            oldpos.x += snowxrand[(j + 3 * i) % NR_OF_SNOW_FRAMES];
            if (oldpos.x < 0)
                oldpos.x += SNOW_TEX_W;
            if (oldpos.x >= SNOW_TEX_W)
                oldpos.x -= SNOW_TEX_W;
            oldpos.y += 1;
            if (oldpos.y >= SNOW_TEX_H) {
                oldpos.x = snowxtrans[oldpos.x];
                oldpos.y = 0;
            }
        }
        snowtex.reset(i, new texture(snowtmptex, SNOW_TEX_W, SNOW_TEX_H, GL_LUMINANCE_ALPHA, texture::LINEAR_MIPMAP_LINEAR, texture::REPEAT));
    }
#endif
}

void weather_renderer::draw(double current_time) const {
#if defined(RAIN) || defined(SNOW)
    // draw layers of snow flakes or rain drops
    // get projection from frustum to view
    matrix4 c2w = (matrix4::get_gl(GL_PROJECTION_MATRIX) * matrix4::get_gl(GL_MODELVIEW_MATRIX)).inverse();

    // draw planes between z-near and z-far with ascending distance and 2d texture with flakes/strains
    texture *tex = 0;
#ifdef RAIN
    const unsigned NR_OF_RAIN_FRAMES = 16;
    unsigned sf = unsigned(current_time * NR_OF_RAIN_FRAMES) % NR_OF_RAIN_FRAMES;
    tex = raintex[sf];
#endif
#ifdef SNOW
    const unsigned NR_OF_SNOW_FRAMES = 23;
    unsigned sf = unsigned(current_time * NR_OF_SNOW_FRAMES) % NR_OF_SNOW_FRAMES;
    tex = snowtex[sf];
#endif

    // Draw weather effect planes at different depths
    double zd[3] = {0.3, 0.9, 0.7};
    for (unsigned i = 0; i < 1; ++i) {
        vector3 p0 = c2w * vector3(-1, 1, zd[i]);
        vector3 p1 = c2w * vector3(-1, -1, zd[i]);
        vector3 p2 = c2w * vector3(1, -1, zd[i]);
        vector3 p3 = c2w * vector3(1, 1, zd[i]);
        primitives::textured_quad(vector3f(p1),
                                  vector3f(p2),
                                  vector3f(p3),
                                  vector3f(p0),
                                  *tex,
                                  vector2f(0, 3),
                                  vector2f(3, 0));
    }
#endif
}

bool weather_renderer::is_enabled() const {
#if defined(RAIN) || defined(SNOW)
    return true;
#else
    return false;
#endif
}
