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

// SDL/OpenGL based textures
// (C)+(W) by Thorsten Jordan. See LICENSE

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "oglext/OglExt.h"
#include <glu.h>

#include "system.h"

#include <SDL.h>
#include <SDL_image.h>
#include "vector3.h"
#include "texture.h"
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
using namespace std;

#undef  MEMMEASURE

#undef COMPRESSED_TEXTURES

#ifdef MEMMEASURE
unsigned texture::mem_used = 0;
unsigned texture::mem_alloced = 0;
unsigned texture::mem_freed = 0;
#endif

int texture::size_non_power_2 = -1;

bool texture::size_non_power_two()
{
	if (size_non_power_2 < 0) {
		size_non_power_2 = sys().extension_supported("GL_ARB_texture_non_power_of_two") ? 1 : 0;
		if (size_non_power_2)
			sys().add_console("Textures with non-power-two sizes are supported and used.");
	}
	return (size_non_power_2 > 0);
}

// ------------------------------- GL mode tables -------------------
static GLuint mapmodes[texture::NR_OF_MAPPING_MODES] = {
	GL_NEAREST,
	GL_LINEAR,
	GL_NEAREST_MIPMAP_NEAREST,
	GL_NEAREST_MIPMAP_LINEAR,
	GL_LINEAR_MIPMAP_NEAREST,
	GL_LINEAR_MIPMAP_LINEAR
};

static bool do_mipmapping[texture::NR_OF_MAPPING_MODES] = {
	false,
	false,
	true,
	true,
	true,
	true
};

static GLuint magfilter[texture::NR_OF_MAPPING_MODES] = {
	GL_NEAREST,
	GL_LINEAR,
	GL_NEAREST,
	GL_NEAREST,
	GL_LINEAR,
	GL_LINEAR
};

static GLuint clampmodes[texture::NR_OF_CLAMPING_MODES] = {
	GL_REPEAT,
	GL_CLAMP,
	GL_CLAMP_TO_EDGE
};
// --------------------------------------------------



sdl_image::sdl_image(const std::string& filename)
	: img(0)
{
	// get extension
	string::size_type st = filename.rfind(".");
	string extension = filename.substr(st);

	if (extension != ".jpg|png") {
		// standard texture, just one file
		img = IMG_Load(filename.c_str());
		if (!img)
			throw file_read_error(filename);
	} else {
		// special texture, using jpg for RGB and png/grey for A.
		string fnrgb = filename.substr(0, st) + ".jpg";
		string fna = filename.substr(0, st) + ".png";
		// "recursive" use of constructor. looks wild, but is valid.
		sdl_image teximagergb(fnrgb);
		sdl_image teximagea(fna);

		// combine surfaces to one
		if (teximagergb->w != teximagea->w || teximagergb->h != teximagea->h)
			throw texture::texerror(filename, "jpg/png load: widths/heights don't match");

		if (teximagergb->format->BytesPerPixel != 3
		    || (teximagergb->format->Amask != 0))
			throw texture::texerror(fnrgb, ".jpg: no 3 byte/pixel RGB image!");

		if (teximagea->format->BytesPerPixel != 1
		    || teximagea->format->palette == 0
		    || teximagea->format->palette->ncolors != 256
		    || ((teximagea->flags & SDL_SRCCOLORKEY) != 0))
			throw texture::texerror(fna, ".png: no 8bit greyscale non-alpha-channel image!");

		Uint32 rmask, gmask, bmask, amask;

		/* SDL interprets each pixel as a 32-bit number, so our masks must depend
		   on the endianness (byte order) of the machine */
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		rmask = 0xff000000;
		gmask = 0x00ff0000;
		bmask = 0x0000ff00;
		amask = 0x000000ff;
#else
		rmask = 0x000000ff;
		gmask = 0x0000ff00;
		bmask = 0x00ff0000;
		amask = 0xff000000;
#endif

		SDL_Surface* result = SDL_CreateRGBSurface(SDL_SWSURFACE,
							   teximagergb->w,
							   teximagergb->h,
							   32, rmask, gmask, bmask, amask);
		if (!result)
			throw file_read_error(filename);

		try {
			// copy pixel data
			teximagergb.lock();
			teximagea.lock();
			SDL_LockSurface(result);

			// fixme: when reading pixels out of sdl surfaces,
			// we need to take care of the pixel format...
			unsigned char* ptr = ((unsigned char*)(result->pixels));
			unsigned char* offsetrgb = ((unsigned char*)(teximagergb->pixels));
			unsigned char* offseta = ((unsigned char*)(teximagea->pixels));
			// 2006-12-01 doc1972 images with negative width and height doesn´t exist, so we cast to unsigned
			for (unsigned y = 0; y < (unsigned int)teximagergb->h; ++y) {
				for (unsigned x = 0; x < (unsigned int)teximagergb->w; ++x) {
					ptr[4*x  ] = offsetrgb[3*x  ];
					ptr[4*x+1] = offsetrgb[3*x+1];
					ptr[4*x+2] = offsetrgb[3*x+2];
					ptr[4*x+3] = offseta[x];
				}
				offsetrgb += teximagergb->pitch;
				offseta += teximagea->pitch;
				ptr += result->pitch;
			}

			teximagergb.unlock();
			teximagea.unlock();
			SDL_UnlockSurface(result);
		}
		catch (...) {
			if (result)
				SDL_FreeSurface(result);
			throw;
		}

		img = result;
	}
}



sdl_image::~sdl_image()
{
	SDL_FreeSurface(img);
}



void sdl_image::lock()
{
	SDL_LockSurface(img);
}



void sdl_image::unlock()
{
	SDL_UnlockSurface(img);
}



// --------------------------------------------------

void texture::sdl_init(SDL_Surface* teximage, unsigned sx, unsigned sy, unsigned sw, unsigned sh,
		       bool makenormalmap, float detailh, bool rgb2grey)
{
	// compute texture width and height
	unsigned tw = sw, th = sh;
	if (!size_non_power_two()) {
		tw = 1;
		th = 1;
		while (tw < sw) tw *= 2;
		while (th < sh) th *= 2;
	}
	width = sw;
	height = sh;
	gl_width = tw;
	gl_height = th;

//	not longer necessary because of automatic texture resize in update()
//	(use throw here if enabled again!)
//	sys().myassert(tw <= get_max_size(), "texture: texture width too big");
//	sys().myassert(th <= get_max_size(), "texture: texture height too big");

	SDL_LockSurface(teximage);

	const SDL_PixelFormat& fmt = *(teximage->format);
	unsigned bpp = fmt.BytesPerPixel;

/*
	cout << "texture: " << texfilename
	     << " palette: " << teximage->format->palette
	     << " bpp " << unsigned(teximage->format->BitsPerPixel)
	     << " bytepp " << unsigned(teximage->format->BytesPerPixel)
	     << " Rmask " << teximage->format->Rmask
	     << " Gmask " << teximage->format->Gmask
	     << " Bmask " << teximage->format->Bmask
	     << " Amask " << teximage->format->Amask
	     << "\n";
*/

	vector<Uint8> data;
	if (fmt.palette != 0) {
		//old color table code, does not work
		//glEnable(GL_COLOR_TABLE);
		if (bpp != 1)
			throw texerror(get_name(), "only 8bit palette files supported");
		int ncol = fmt.palette->ncolors;
		if (ncol > 256)
			throw texerror(get_name(), "max. 256 colors in palette supported");
		bool usealpha = (teximage->flags & SDL_SRCCOLORKEY);

		// check for greyscale images (GL_LUMINANCE), fixme: add also LUMINANCE_ALPHA!
		bool lumi = false;
		if (ncol == 256 && !usealpha) {
			unsigned i = 0;
			for ( ; i < 256; ++i) {
				if (unsigned(fmt.palette->colors[i].r) != i) break;
				if (unsigned(fmt.palette->colors[i].g) != i) break;
				if (unsigned(fmt.palette->colors[i].b) != i) break;
			}
			if (i == 256) lumi = true;
		}

		if (lumi) {
			// grey value images
			format = GL_LUMINANCE;
			bpp = 1;

			data.resize(tw*th*bpp);
			unsigned char* ptr = &data[0];
			unsigned char* offset = ((unsigned char*)(teximage->pixels)) + sy*teximage->pitch + sx;
			for (unsigned y = 0; y < sh; y++) {
				memcpy(ptr, offset, sw/* * bpp */);
				offset += teximage->pitch;
				ptr += tw*bpp;
			}
		} else {
			// color images
			format = usealpha ? GL_RGBA : GL_RGB;
			bpp = usealpha ? 4 : 3;

			//old color table code, does not work		
			//glColorTable(GL_TEXTURE_2D, internalformat, 256, GL_RGBA, GL_UNSIGNED_BYTE, &(palette[0]));
			//internalformat = GL_COLOR_INDEX8_EXT;
			//externalformat = GL_COLOR_INDEX;
			data.resize(tw*th*bpp);
			unsigned char* ptr = &data[0];
			unsigned char* offset = ((unsigned char*)(teximage->pixels)) + sy*teximage->pitch + sx;
			for (unsigned y = 0; y < sh; y++) {
				unsigned char* ptr2 = ptr;
				for (unsigned x = 0; x < sw; ++x) {
					Uint8 pixindex = *(offset+x);
					const SDL_Color& pixcolor = fmt.palette->colors[pixindex];
					*ptr2++ = pixcolor.r;
					*ptr2++ = pixcolor.g;
					*ptr2++ = pixcolor.b;
					if (usealpha)
						*ptr2++ = (pixindex == (fmt.colorkey & 0xff)) ? 0x00 : 0xff;
				}
				//old color table code, does not work
				//memcpy(ptr, offset, sw);
				offset += teximage->pitch;
				ptr += tw*bpp;
			}
		}
	} else {
		bool usealpha = fmt.Amask != 0;
		if (rgb2grey) {
			if (usealpha) {
				format = GL_LUMINANCE_ALPHA;
				bpp = 2;
			} else {
				format = GL_LUMINANCE;
				bpp = 1;
			}
		} else {
			if (usealpha) {
				format = GL_RGBA;
				bpp = 4;
			} else {
				format = GL_RGB;
				bpp = 3;
			}
		}
		data.resize(tw*th*bpp);
		unsigned char* ptr = &data[0];
		unsigned char* offset = ((unsigned char*)(teximage->pixels))
			+ sy*teximage->pitch + sx*bpp;
		if (rgb2grey) {
			for (unsigned y = 0; y < sh; y++) {
				for (unsigned x = 0; x < sw; ++x)
					ptr[x*bpp] = offset[x*(bpp+2)+1]; // take green value, it doesn't matter
				if (bpp == 2)
					// with alpha
					for (unsigned x = 0; x < sw; ++x)
						ptr[x*2+1] = offset[x*4+3];
				offset += teximage->pitch;
				ptr += tw*bpp;
			}
		} else {
			for (unsigned y = 0; y < sh; y++) {
#if 0
				// new code, that uses the RGB-masks of SDL to load/transform
				// colors.
				if (bpp == 3) {
					Uint8* linedst = (Uint8*)ptr;
					Uint8* linesrc = (Uint8*)offset;
					for (unsigned x = 0; x < sw; ++x) {
						// be careful! with bpp=3 this could lead to
						// an off by one segfault error, if pitch is
						// not a multiple of four...we just hope the best.
						// could be done quicker with mmx, but this performance
						// is not that critical here
						Uint32 pv = *(Uint32*)linesrc; // fixme: is that right for Big-Endian machines? SDL Docu suggests yes...
						linedst[0] = Uint8(((pv & fmt.Rmask) >> fmt.Rshift) << fmt.Rloss);
						linedst[1] = Uint8(((pv & fmt.Gmask) >> fmt.Gshift) << fmt.Gloss);
						linedst[2] = Uint8(((pv & fmt.Bmask) >> fmt.Bshift) << fmt.Bloss);
						linesrc += bpp;
						linedst += bpp;
					}
				} else {
					// bpp = 4 here
					Uint32* linedst = (Uint32*)ptr;
					Uint32* linesrc = (Uint32*)offset;
					for (unsigned x = 0; x < sw; ++x) {
						Uint32 pv = linesrc[x];
						Uint32 r = (((pv & fmt.Rmask) >> fmt.Rshift) << fmt.Rloss);
						Uint32 g = (((pv & fmt.Gmask) >> fmt.Gshift) << fmt.Gloss);
						Uint32 b = (((pv & fmt.Bmask) >> fmt.Bshift) << fmt.Bloss);
						Uint32 a = (((pv & fmt.Bmask) >> fmt.Bshift) << fmt.Bloss);
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
						pv = a | (b << 8) | (g << 16) | (r << 24);
#else
						pv = r | (g << 8) | (b << 16) | (a << 24);
#endif
						linedst[x] = pv;
					}
				}
#endif
				// old code, that assumes bytes come in R,G,B order:
				memcpy(ptr, offset, sw*bpp);
				offset += teximage->pitch;
				ptr += tw*bpp;
			}
		}
	}
	SDL_UnlockSurface(teximage);
	init(data, makenormalmap, detailh);
}
	


void texture::init(const vector<Uint8>& data, bool makenormalmap, float detailh)
{
	// error checks.
	if (mapping < 0 || mapping >= NR_OF_MAPPING_MODES)
		throw texerror(get_name(), "illegal mapping mode!");
	if (clamping < 0 || clamping >= NR_OF_CLAMPING_MODES)
		throw texerror(get_name(), "illegal clamping mode!");

	// automatic resizing of textures if they're too large
	// fixme: this old code for old cards. should be obsolete. any cards newer
	// than geforce2mx can do 4096x4096 textures. Geforce2mx can do 2048x2048.
	// Far enough for us. So do it only when card is so old, that it can't do
	// non-power-two textures.
	const vector<Uint8>* pdata = &data;
	if (!size_non_power_two()) {
		vector<Uint8> data2;
		unsigned ms = get_max_size();
		if (width > ms || height > ms) {
			unsigned newwidth = width, newheight = height;
			unsigned xf = 1, yf = 1;
			if (width > ms) {
				newwidth = ms;
				xf = width/ms;
			}
			if (height > ms) {
				newheight = ms;
				yf = height/ms;
			}
			unsigned bpp = get_bpp();
			data2.resize(newwidth*newheight*bpp);
			unsigned area = xf*yf;
			for (unsigned y = 0; y < newheight; ++y) {
				for (unsigned x = 0; x < newwidth; ++x) {
					for (unsigned b = 0; b < bpp; ++b) {
						unsigned valsum = 0;
						for (unsigned yy = 0; yy < yf; ++yy) {
							for (unsigned xx = 0; xx < xf; ++xx) {
								unsigned ptr = ((y*yf+yy) * width +
										(x*xf+xx)) * bpp + b;
								valsum += unsigned(data[ptr]);
							}
						}
						valsum /= area;
						data2[(y*newwidth+x)*bpp + b] = Uint8(valsum);
					}
				}
			}
			pdata = &data2;
			gl_width = newwidth;
			gl_height = newheight;
		}
	}

	glGenTextures(1, &opengl_name);
	glBindTexture(GL_TEXTURE_2D, opengl_name);

#ifdef MEMMEASURE
	unsigned add_mem_used = 0;
#endif

	if (makenormalmap && format == GL_LUMINANCE) {
		// make own mipmap building for normal maps here...
		// give increasing levels with decreasing w/h down to 1x1
		// e.g. 64x16 -> 32x8, 16x4, 8x2, 4x1, 2x1, 1x1
		format = GL_RGB;
		vector<Uint8> nmpix = make_normals(*pdata, gl_width, gl_height, detailh);
		int internalformat = format;
#ifdef COMPRESSED_TEXTURES
		format = GL_COMPRESSED_LUMINANCE_ARB;
#endif
		glTexImage2D(GL_TEXTURE_2D, 0, internalformat, gl_width, gl_height, 0, format,
			     GL_UNSIGNED_BYTE, &nmpix[0]);

#ifdef MEMMEASURE
		add_mem_used = gl_width * gl_height * get_bpp();
#endif

		if (do_mipmapping[mapping]) {
#if 1
			// fixme: doesn't work with textures that don't have power of two size...
			gluBuild2DMipmaps(GL_TEXTURE_2D, format, gl_width, gl_height,
					  format, GL_UNSIGNED_BYTE, &nmpix[0]);

#else
			// buggy version. gives white textures. maybe some mipmap levels
			// are missing so that gl complains by make white textures?
			// fixme: test it again after the mapping/clamping bugfix!

			// fixme: if we let GLU do the mipmap calculation, the result is wrong.
			// A filtered version of the normals is not the same as a normal map
			// of the filtered height field!
			// E.g. scaling down the map to 1x1 pixel gives a medium height of 128,
			// that is a flat plane with a normal of (0,0,1)
			// But filtering down the normals to one pixel could give
			// RGB=0.5 -> normal of (0,0,0) (rare...)!
			// fixme: mipmapping for textures with non-power-of-two resolution is
			// untested!
			vector<Uint8> curlvl;
			const vector<Uint8>* gdat = pdata;
			for (unsigned level = 1, w = gl_width/2, h = gl_height/2;
			     w > 0 && h > 0; w /= 2, h /= 2) {
				cout << "level " << level << " w " << w << " h " << h << "\n";
				curlvl = scale_half(*gdat, w, h, 1);
				gdat = &curlvl;
				//fixme: must detailh also get halfed here? yes...
				vector<Uint8> nmpix = make_normals(*gdat, w, h, detailh);
				int internalformat = GL_RGB;
#ifdef COMPRESSED_TEXTURES
				internalformat = GL_COMPRESSED_RGB_ARB;
#endif
				glTexImage2D(GL_TEXTURE_2D, level, internalformat, w, h, 0, GL_RGB,
					     GL_UNSIGNED_BYTE, &nmpix[0]);
				w /= 2;
				h /= 2;
			}
#endif
		}
	} else {
		// make gl texture
		int internalformat = format;
#ifdef COMPRESSED_TEXTURES
		switch (format) {
		case GL_RGB:
			internalformat = GL_COMPRESSED_RGB_ARB;
			break;
		case GL_RGBA:
			internalformat = GL_COMPRESSED_RGBA_ARB;
			break;
		case GL_LUMINANCE:
			internalformat = GL_COMPRESSED_LUMINANCE_ARB;
			break;
		case GL_LUMINANCE_ALPHA:
			internalformat = GL_COMPRESSED_LUMINANCE_ALPHA_ARB;
			break;
		}
#endif
		glTexImage2D(GL_TEXTURE_2D, 0, internalformat, gl_width, gl_height, 0, format,
			     GL_UNSIGNED_BYTE, &((*pdata)[0]));
#ifdef MEMMEASURE
		add_mem_used = gl_width * gl_height * get_bpp();
#endif
		if (do_mipmapping[mapping]) {
			// fixme: does this command set the base level, too?
			// i.e. are the two gl commands redundant?
			// fixme: doesn't work with textures that don't have power of two size...
			gluBuild2DMipmaps(GL_TEXTURE_2D, format, gl_width, gl_height,
					  format, GL_UNSIGNED_BYTE, &((*pdata)[0]));
		}
	}

#ifdef MEMMEASURE
	if (do_mipmapping[mapping])
		add_mem_used = (4*add_mem_used)/3;
	mem_used += add_mem_used;
	ostringstream oss; oss << "Allocated " << add_mem_used << " bytes of video memory for texture '" << texfilename << "', total video mem use " << mem_used/1024 << " kb";
	sys().add_console(oss.str());
	mem_alloced += add_mem_used;
	ostringstream oss2; oss2 << "Video mem usage " << mem_alloced << " vs " << mem_freed;
	sys().add_console(oss2.str());
#endif

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mapmodes[mapping]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magfilter[mapping]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, clampmodes[clamping]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, clampmodes[clamping]);
}




vector<Uint8> texture::scale_half(const vector<Uint8>& src, unsigned w, unsigned h, unsigned bpp)
{
	if (!size_non_power_two()) {
		if (w < 1 || (w & (w-1)) != 0)
			throw texerror("[scale_half]", "texture width is no power of two!");
		if (h < 1 || (h & (h-1)) != 0)
			throw texerror("[scale_half]", "texture height is no power of two!");
	}

	vector<Uint8> dst(w*h*bpp/4);
	unsigned ptr = 0;
	for (unsigned y = 0; y < h; y += 2) {
		for (unsigned x = 0; x < w; x += 2) {
			for (unsigned b = 0; b < bpp; ++b) {
				dst[ptr++] =
					Uint8((unsigned(src[(y*w+x)*bpp+b]) +
					       unsigned(src[(y*w+x+1)*bpp+b]) +
					       unsigned(src[((y+1)*w+x)*bpp+b]) +
					       unsigned(src[((y+1)*w+x+1)*bpp+b])) / 4);
			}
		}
	}
	return dst;
}
	


vector<Uint8> texture::make_normals(const vector<Uint8>& src, unsigned w, unsigned h,
				    float detailh)
{
	// src size must be w*h
	vector<Uint8> dst(3*w*h);
	// Note! zh must be multiplied with 2*sample_distance!
	// sample_distance is real distance between texels, we assume 1 for it...
	// This depends on the size of the face the normal map is mapped onto.
	// but all other code is written to match 255/detailh, especially
	// bump scaling in model.cpp, so don't change this!
	float zh = /* 2.0f* */ 255.0f/detailh;
	unsigned ptr = 0;
	for (unsigned yy = 0; yy < h; ++yy) {
		unsigned y1 = (yy + h - 1) & (h - 1);
		unsigned y2 = (yy +     1) & (h - 1);
		for (unsigned xx = 0; xx < w; ++xx) {
			unsigned x1 = (xx + w - 1) & (w - 1);
			unsigned x2 = (xx +     1) & (w - 1);
			float hr = src[yy*w+x2];
			float hu = src[y1*w+xx];
			float hl = src[yy*w+x1];
			float hd = src[y2*w+xx];
			vector3f nm = vector3f(hl-hr, hd-hu, zh).normal();
			dst[ptr + 0] = Uint8(nm.x*127 + 128);
			dst[ptr + 1] = Uint8(nm.y*127 + 128);
			dst[ptr + 2] = Uint8(nm.z*127 + 128);
			ptr += 3;
		}
	}
	return dst;
}



texture::texture(const string& filename, mapping_mode mapping_, clamping_mode clamp,
		 bool makenormalmap, float detailh, bool rgb2grey)
{
	mapping = mapping_;
	clamping = clamp;
	texfilename = filename;

	sdl_image teximage(filename);
	sdl_init(teximage.get_SDL_Surface(), 0, 0, teximage->w, teximage->h, makenormalmap, detailh, rgb2grey);
}	



texture::texture(SDL_Surface* teximage, unsigned sx, unsigned sy, unsigned sw, unsigned sh,
		 mapping_mode mapping_, clamping_mode clamp, bool makenormalmap, float detailh,
		 bool rgb2grey)
{
	mapping = mapping_;
	clamping = clamp;
	sdl_init(teximage, sx, sy, sw, sh, makenormalmap, detailh, rgb2grey);
}



texture::texture(const sdl_image& teximage, unsigned sx, unsigned sy, unsigned sw, unsigned sh,
		 mapping_mode mapping_, clamping_mode clamp, bool makenormalmap, float detailh,
		 bool rgb2grey)
{
	mapping = mapping_;
	clamping = clamp;
	sdl_init(teximage.get_SDL_Surface(), sx, sy, sw, sh, makenormalmap, detailh, rgb2grey);
}



texture::texture(const vector<Uint8>& pixels, unsigned w, unsigned h, int format_,
		 mapping_mode mapping_, clamping_mode clamp, bool makenormalmap, float detailh)
{
	mapping = mapping_;
	clamping = clamp;
	
	if (!size_non_power_two()) {
		if (w < 1 || (w & (w-1)) != 0)
			throw texerror(get_name(), "texture width is no power of two!");
		if (h < 1 || (h & (h-1)) != 0)
			throw texerror(get_name(), "texture height is no power of two!");
	}

	width = gl_width = w;
	height = gl_height = h;
	format = format_;

	init(pixels, makenormalmap, detailh);
}



texture::texture(unsigned w, unsigned h, int format_,
		 mapping_mode mapping_, clamping_mode clamp)
{
	mapping = mapping_;
	clamping = clamp;
	
	if (!size_non_power_two()) {
		if (w < 1 || (w & (w-1)) != 0)
			throw texerror(get_name(), "texture width is no power of two!");
		if (h < 1 || (h & (h-1)) != 0)
			throw texerror(get_name(), "texture height is no power of two!");
	}

	width = gl_width = w;
	height = gl_height = h;
	format = format_;

	// error checks.
	if (mapping < 0 || mapping >= NR_OF_MAPPING_MODES)
		throw texerror(get_name(), "illegal mapping mode!");
	if (clamping < 0 || clamping >= NR_OF_CLAMPING_MODES)
		throw texerror(get_name(), "illegal clamping mode!");

	glGenTextures(1, &opengl_name);
	glBindTexture(GL_TEXTURE_2D, opengl_name);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mapmodes[mapping]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magfilter[mapping]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, clampmodes[clamping]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, clampmodes[clamping]);
	// initialize texel data with empty pixels
	glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, GL_RGB,
		     GL_UNSIGNED_BYTE, (void*)0);
}



texture::~texture()
{
#ifdef MEMMEASURE
	unsigned sub_mem_used = gl_width * gl_height * get_bpp();
	if (do_mipmapping[mapping])
		sub_mem_used = (4*sub_mem_used)/3;
	mem_used -= sub_mem_used;
	ostringstream oss; oss << "Freed " << sub_mem_used << " bytes of video memory for texture '" << texfilename << "', total video mem use " << mem_used/1024 << " kb";
	sys().add_console(oss.str());
	mem_freed += sub_mem_used;
	ostringstream oss2; oss2 << "Video mem usage " << mem_alloced << " vs " << mem_freed;
	sys().add_console(oss2.str());
#endif
	glDeleteTextures(1, &opengl_name);
}



void texture::sub_image(int xoff, int yoff, unsigned w, unsigned h,
			const std::vector<Uint8>& pixels, int format_)
{
	glBindTexture(GL_TEXTURE_2D, opengl_name);
	glTexSubImage2D(GL_TEXTURE_2D, 0 /* mipmap level */,
			xoff, yoff, w, h, format_, GL_UNSIGNED_BYTE, &pixels[0]);
	glBindTexture(GL_TEXTURE_2D, 0);
}



unsigned texture::get_bpp() const
{
	switch (format) {
		case GL_RGB: return 3;
		case GL_RGBA: return 4;
		case GL_LUMINANCE: return 1;
		case GL_LUMINANCE_ALPHA: return 2;
		default:
			ostringstream oss; oss << "unknown texture format " << format << "\n";
			throw texerror(get_name(), oss.str());
	}
	return 4;
}

void texture::set_gl_texture() const
{
	glBindTexture(GL_TEXTURE_2D, get_opengl_name());
}

// draw_image
void texture::draw(int x, int y) const
{
	draw(x, y, width, height);
}

void texture::draw_hm(int x, int y) const
{
	draw_hm(x, y, width, height);
}

void texture::draw_vm(int x, int y) const
{
	draw_vm(x, y, width, height);
}

void texture::draw(int x, int y, int w, int h) const
{
	float u = float(width)/gl_width;
	float v = float(height)/gl_height;
	glBindTexture(GL_TEXTURE_2D, get_opengl_name());
	glBegin(GL_QUADS);
	glTexCoord2f(0,0);
	glVertex2i(x,y);
	glTexCoord2f(0,v);
	glVertex2i(x,y+h);
	glTexCoord2f(u,v);
	glVertex2i(x+w,y+h);
	glTexCoord2f(u,0);
	glVertex2i(x+w,y);
	glEnd();
}

void texture::draw_hm(int x, int y, int w, int h) const
{
	float u = float(width)/gl_width;
	float v = float(height)/gl_height;
	glBindTexture(GL_TEXTURE_2D, get_opengl_name());
	glBegin(GL_QUADS);
	glTexCoord2f(u,0);
	glVertex2i(x,y);
	glTexCoord2f(u,v);
	glVertex2i(x,y+h);
	glTexCoord2f(0,v);
	glVertex2i(x+w,y+h);
	glTexCoord2f(0,0);
	glVertex2i(x+w,y);
	glEnd();
}

void texture::draw_vm(int x, int y, int w, int h) const
{
	float u = float(width)/gl_width;
	float v = float(height)/gl_height;
	glBindTexture(GL_TEXTURE_2D, get_opengl_name());
	glBegin(GL_QUADS);
	glTexCoord2f(0,v);
	glVertex2i(x,y);
	glTexCoord2f(0,0);
	glVertex2i(x,y+h);
	glTexCoord2f(u,0);
	glVertex2i(x+w,y+h);
	glTexCoord2f(u,v);
	glVertex2i(x+w,y);
	glEnd();
}

void texture::draw_rot(int x, int y, double angle) const
{
	draw_rot(x, y, angle, get_width()/2, get_height()/2);
}

void texture::draw_rot(int x, int y, double angle, int tx, int ty) const
{
	glPushMatrix();
	glTranslatef(x, y, 0);
	glRotatef(angle, 0, 0, 1);
	draw(-tx, -ty);
	glPopMatrix();
}

void texture::draw_tiles(int x, int y, int w, int h) const
{
	float tilesx = float(w)/gl_width;
	float tilesy = float(h)/gl_height;
	glBindTexture(GL_TEXTURE_2D, get_opengl_name());
	glBegin(GL_QUADS);
	glTexCoord2f(0,0);
	glVertex2i(x,y);
	glTexCoord2f(0,tilesy);
	glVertex2i(x,y+h);
	glTexCoord2f(tilesx,tilesy);
	glVertex2i(x+w,y+h);
	glTexCoord2f(tilesx,0);
	glVertex2i(x+w,y);
	glEnd();
}

void texture::draw_subimage(int x, int y, int w, int h, unsigned tx, unsigned ty,
	unsigned tw, unsigned th) const
{
	float x1 = float(tx)/gl_width;
	float y1 = float(ty)/gl_height;
	float x2 = float(tx+tw)/gl_width;
	float y2 = float(ty+th)/gl_height;
	glBindTexture(GL_TEXTURE_2D, get_opengl_name());
	glBegin(GL_QUADS);
	glTexCoord2f(x1,y1);
	glVertex2i(x,y);
	glTexCoord2f(x1,y2);
	glVertex2i(x,y+h);
	glTexCoord2f(x2,y2);
	glVertex2i(x+w,y+h);
	glTexCoord2f(x2,y1);
	glVertex2i(x+w,y);
	glEnd();
}

unsigned texture::get_max_size()
{
	GLint i;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &i);
	return i;
}
