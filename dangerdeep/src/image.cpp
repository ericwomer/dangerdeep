// OpenGL texture drawing based on SDL Surfaces
// (C)+(W) by Thorsten Jordan. See LICENSE

#include "image.h"
#ifdef WIN32
#include <SDL_image.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <GL/gl.h>
#else
#include <SDL/SDL_image.h>
#include <GL/gl.h>
#endif

image::image(const string& s, unsigned mapping_, bool clamp_, bool morealpha_,
	bool loaddynamically) :
	img(0), dynamic(loaddynamically), name(s), width(0), height(0),
	texturized(false), gltx(0), glty(0),
	mapping(mapping_), clamp(clamp_), morealpha(morealpha_)
{
	if (!dynamic) {
		img = IMG_Load(name.c_str());
		if (img == 0) { width = height = 0; return; }
		// assert img != 0 fixme
		width = img->w;
		height = img->h;
	}
}

image::~image()
{
	if (texturized) clear_textures();
	if (img) SDL_FreeSurface(img);
}

void image::texturize(void)
{
	if (!img) {
		img = IMG_Load(name.c_str());
		// assert img != 0 fixme
		width = img->w;
		height = img->h;
	}

	vector<unsigned> widths, heights;
	unsigned maxw = texture::get_max_width();
	unsigned maxh = texture::get_max_height();
	unsigned w = width, h = height;
	while (w > maxw) {
		widths.push_back(maxw);
		w -= maxw;
	}
	widths.push_back(w);
	while (h > maxh) {
		heights.push_back(maxh);
		h -= maxh;
	}
	heights.push_back(h);

	gltx = widths.size();
	glty = heights.size();
	textures.reserve(gltx*glty);
	unsigned ch = 0;
	for (unsigned y = 0; y < glty; ++y) {
		unsigned cw = 0;
		for (unsigned x = 0; x < gltx; ++x) {
			textures.push_back(new texture(img, cw, ch, widths[x], heights[y],
				mapping, clamp, morealpha));
			cw += widths[x];
		}
		ch += heights[y];
	}

	lastcolw = widths.back();
	lastrowh = heights.back();
	lastcolu = float(lastcolw)/float(textures[gltx-1]->get_width());
	lastrowv = float(lastrowh)/float(textures[gltx*(glty-1)]->get_height());
	
	texturized = true;
}

void image::draw(int x, int y)
{
	if (!texturized) texturize();
	unsigned texptr = 0;
	int yp = y;
	for (unsigned yy = 0; yy < glty; ++yy) {
		int xp = x;
		unsigned h = (yy == glty-1) ? lastrowh : textures[texptr]->get_height();
		float v = (yy == glty-1) ? lastrowv : 1.0f;
		for (unsigned xx = 0; xx < gltx; ++xx) {
			unsigned w = (xx = gltx-1) ? lastcolw : textures[texptr]->get_width();
			float u = (xx == gltx-1) ? lastcolu : 1.0f;
			glBindTexture(GL_TEXTURE_2D, textures[texptr]->get_opengl_name());
			glBegin(GL_QUADS);
			glTexCoord2f(0,0);
			glVertex2i(xp,yp);
			glTexCoord2f(0,v);
			glVertex2i(xp,yp+h);
			glTexCoord2f(u,v);
			glVertex2i(xp+w,yp+h);
			glTexCoord2f(u,0);
			glVertex2i(xp+w,yp);
			glEnd();
			++texptr;
			xp += w;
		}
		yp += h;
	}
}

void image::clear_textures(void)
{
	for (unsigned i = 0; i < textures.size(); ++i)
		delete textures[i];
	textures.clear();
	texturized = false;
	gltx = glty = 0;
	if (dynamic) {
		SDL_FreeSurface(img);
		img = 0;
	}
}
