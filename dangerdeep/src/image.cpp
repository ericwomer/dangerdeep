// OpenGL texture drawing based on SDL Surfaces
// (C)+(W) by Thorsten Jordan. See LICENSE

#include "image.h"
#include <SDL/SDL_image.h>
#include <GL/gl.h>

image::image(const string& s, unsigned mapping_, bool clamp_, bool morealpha_,
	bool loaddynamically) :
	img(0), dynamic(loaddynamically), name(s), width(0), height(0),
	texturized(false), gltx(0), glty(0),
	mapping(mapping_), clamp(clamp_), morealpha(morealpha_)
{
	if (!dynamic) {
		img = IMG_Load(name.c_str());
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
	
	texturized = true;
}

void image::draw(int x, int y)
{
	if (!texturized) texturize();
	unsigned texptr = 0;
	int yp = y;
	for (unsigned yy = 0; yy < glty; ++yy) {
		int xp = x;
		int ypadd = textures[texptr]->get_height();
		for (unsigned xx = 0; xx < gltx; ++xx) {
			unsigned w = textures[texptr]->get_width();
			unsigned h = textures[texptr]->get_height();
			glBindTexture(GL_TEXTURE_2D, textures[texptr]->get_opengl_name());
			glBegin(GL_QUADS);
			glTexCoord2i(0,0);
			glVertex2i(xp,yp);
			glTexCoord2i(0,1);
			glVertex2i(xp,yp+h);
			glTexCoord2i(1,1);
			glVertex2i(xp+w,yp+h);
			glTexCoord2i(1,0);
			glVertex2i(xp+w,yp);
			glEnd();
			++texptr;
			xp += w;
		}
		yp += ypadd;
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
