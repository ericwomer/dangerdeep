// OpenGL texture drawing based on SDL Surfaces
// (C)+(W) by Thorsten Jordan. See LICENSE

#include "image.h"
#include "system.h"
#include "oglext/OglExt.h"
#include <SDL_image.h>



image::image(const string& s, bool maketex, int mapping, int clamp) :
	img(0), maketexture(maketex), name(s), width(0), height(0),
	gltx(0), glty(0)
{
	img = IMG_Load(name.c_str());
	sys().myassert(img != 0, string("image: failed to load '") + s + string("'"));
	width = img->w;
	height = img->h;
	
	if (maketexture) {
		vector<unsigned> widths, heights;
		unsigned maxs = texture::get_max_size();
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
		textures.reserve(gltx*glty);
		unsigned ch = 0;
		for (unsigned y = 0; y < glty; ++y) {
			unsigned cw = 0;
			for (unsigned x = 0; x < gltx; ++x) {
				textures.push_back(new texture(img, cw, ch, widths[x], heights[y],
					mapping, clamp));
				cw += widths[x];
			}
			ch += heights[y];
		}

		lastcolw = widths.back();
		lastrowh = heights.back();
		lastcolu = float(lastcolw)/float(textures[gltx-1]->get_width());
		lastrowv = float(lastrowh)/float(textures[gltx*(glty-1)]->get_height());
		
		// data is safe in texture memory, so free image.
		SDL_FreeSurface(img);
		img = 0;
	}
}



image::~image()
{
	for (unsigned i = 0; i < textures.size(); ++i)
		delete textures[i];

	if (img)
		SDL_FreeSurface(img);
}



void image::draw(int x, int y) const
{
	if (textures.size() > 0) {
		unsigned texptr = 0;
		int yp = y;
		for (unsigned yy = 0; yy < glty; ++yy) {
			int xp = x;
			unsigned h = (yy == glty-1) ? lastrowh : textures[texptr]->get_height();
			float v = (yy == glty-1) ? lastrowv : 1.0f;
			for (unsigned xx = 0; xx < gltx; ++xx) {
				unsigned w = (xx == gltx-1) ? lastcolw : textures[texptr]->get_width();
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
	} else {	// use DrawPixels instead
		sys().myassert(img->format->palette == 0, string("image: can't use paletted images for direct pixel draw (fixme), image '")+name+string("'"));
		unsigned bpp = img->format->BytesPerPixel;
		sys().myassert(bpp == 3 || bpp == 4, string("image: bpp must be 3 or 4 (RGB or RGBA), image '")+name+string("'"));

		glBindTexture(GL_TEXTURE_2D, 0);
		glColor4f(1,1,1,1);
		SDL_LockSurface(img);
		GLenum pixelformat = (img->format->Amask != 0) ? GL_RGBA : GL_RGB;
		unsigned pixelpitch = img->pitch / bpp;
		// images with 3 bpp and an odd width could give problems (997*3=2991,pitch is 2992, doesn't work with OpenGL's pixel pitch)
		if (pixelpitch * bpp == img->pitch) {
			glRasterPos2i(x, y);
			glPixelStorei(GL_UNPACK_ROW_LENGTH, img->pitch / bpp);
			glDrawPixels(width, height, pixelformat, GL_UNSIGNED_BYTE, img->pixels);
		} else {	// we have to draw each line individually
			for (int l = 0; l < int(height); ++l) {
				glRasterPos2i(x, y+l);
				glDrawPixels(width, 1, pixelformat, GL_UNSIGNED_BYTE, (unsigned char*)(img->pixels) + img->pitch*l);
			}
		}
		SDL_UnlockSurface(img);
		glRasterPos2i(0, 0);	// fixme: the RedBook suggests using RasterPos 0.375 for 3d drawing, check if this must be set up here, too
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	}
}
