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

// OpenGL texture drawing based on SDL Surfaces
// (C)+(W) by Thorsten Jordan. See LICENSE

#include "image.h"
#include "texture.h"
#include "system.h"
#include "oglext/OglExt.h"
#include <SDL_image.h>
#include <sstream>
#include <vector>
using std::string;



image::image(const string& s) :
	name(s), width(0), height(0), gltx(0), glty(0)
{
	sdl_image img(name);
	width = img->w;
	height = img->h;

	// create vector of textures
	if (texture::size_non_power_two()) {
		gltx = glty = 1;
		textures.resize(1);
		textures.reset(0, new texture(img, 0, 0, width, height,
					      texture::NEAREST, texture::CLAMP));
	} else {
		std::vector<unsigned> widths, heights;
		unsigned maxs = texture::get_max_size();

		// avoid wasting too much memory.
		if (maxs > 256) maxs = 256;

		unsigned w = width, h = height;
		while (w > maxs) {
			widths.push_back(maxs);
			w -= maxs;
		}
		widths.push_back(w);
		while (h > maxs) {
			heights.push_back(maxs);
			h -= maxs;
		}
		heights.push_back(h);

		gltx = widths.size();
		glty = heights.size();
		textures.resize(gltx*glty);
		unsigned ch = 0;
		for (unsigned y = 0; y < glty; ++y) {
			unsigned cw = 0;
			for (unsigned x = 0; x < gltx; ++x) {
				textures.reset(y*gltx+x, new texture(img, cw, ch,
								     widths[x], heights[y],
								     texture::NEAREST, texture::CLAMP));
				cw += widths[x];
			}
			ch += heights[y];
		}
	}
}



void image::draw(int x, int y, const colorf& col) const
{
	unsigned texptr = 0;
	int yp = y;
	for (unsigned yy = 0; yy < glty; ++yy) {
		int xp = x;
		unsigned h = textures[texptr]->get_height();
		for (unsigned xx = 0; xx < gltx; ++xx) {
			textures[texptr]->draw(xp, yp, col);
			xp += textures[texptr]->get_width();
			++texptr;
		}
		yp += h;
	}
}
