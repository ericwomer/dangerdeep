// OpenGL texture drawing based on SDL Surfaces
// (C)+(W) by Thorsten Jordan. See LICENSE

#include "image.h"
#include "system.h"
#include "oglext/OglExt.h"
#include <SDL_image.h>
#include <sstream>


unsigned image::mem_used = 0;
unsigned image::mem_alloced = 0;
unsigned image::mem_freed = 0;


// cache
const image* image::cached_object = 0;
unsigned image::gltx = 0;
unsigned image::glty = 0;
vector<texture*> image::textures;


void image::clear_cache(void)
{
	for (vector<texture*>::iterator it = textures.begin(); it != textures.end(); ++it)
		delete *it;
	textures.clear();
	gltx = glty = 0;
	cached_object = 0;
}



void image::check_cache(const image* obj)
{
	if (cached_object != obj) {
		clear_cache();
		cached_object = obj;

		// generate cache values
		vector<unsigned> widths, heights;
		unsigned maxs = texture::get_max_size();

		// avoid wasting too much memory.
		if (maxs > 256) maxs = 256;

		unsigned w = obj->width, h = obj->height;
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
				textures.push_back(new texture(obj->img, cw, ch,
							       widths[x], heights[y],
							       GL_NEAREST, GL_CLAMP_TO_EDGE));
				cw += widths[x];
			}
			ch += heights[y];
		}
	}
}



image::image(const string& s) :
	img(0), name(s), width(0), height(0)
{
	img = IMG_Load(name.c_str());
	sys().myassert(img != 0, string("image: failed to load '") + s + string("'"));
	width = img->w;
	height = img->h;
	unsigned add_mem_used = width * height * img->format->BytesPerPixel;
	mem_used += add_mem_used;
	mem_alloced += add_mem_used;
	ostringstream oss; oss << "Allocated " << add_mem_used << " bytes of system memory for image '" << name << "', total image system mem use " << mem_used/1024 << " kb";
	ostringstream oss2; oss2 << "Image system mem usage " << mem_alloced << " vs " << mem_freed;
	sys().add_console(oss2.str());
}



image::~image()
{
	unsigned sub_mem_used = width * height * img->format->BytesPerPixel;
	mem_used -= sub_mem_used;
	mem_freed += sub_mem_used;
	ostringstream oss; oss << "Freed " << sub_mem_used << " bytes of system memory for image '" << name << "', total image system mem use " << mem_used/1024 << " kb";
	sys().add_console(oss.str());
	ostringstream oss2; oss2 << "Image system mem usage " << mem_alloced << " vs " << mem_freed;
	sys().add_console(oss2.str());

	if (cached_object == this)
		clear_cache();
	if (img)
		SDL_FreeSurface(img);
}



void image::draw(int x, int y) const
{
	check_cache(this);
	if (textures.size() > 0) {
		unsigned texptr = 0;
		int yp = y;
		for (unsigned yy = 0; yy < glty; ++yy) {
			int xp = x;
			unsigned h = textures[texptr]->get_height();
			for (unsigned xx = 0; xx < gltx; ++xx) {
				textures[texptr]->draw(xp, yp);
				xp += textures[texptr]->get_width();
				++texptr;
			}
			yp += h;
		}
	} else {	// use DrawPixels instead
		sys().myassert(false, "could should never reach this!");
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
