// SDL/OpenGL based textures
// (C)+(W) by Thorsten Jordan. See LICENSE

#ifndef MODEL_JUST_LOAD

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "system.h"
#include "oglext/OglExt.h"
#include <GL/glu.h>

#endif /*MODEL_JUST_LOAD*/

#include <SDL.h>
#include <SDL_image.h>

#include "texture.h"
#include <vector>
#include <iostream>
#include <sstream>
using namespace std;


int texture::get_bpp(void) const
{
	switch (format) {
		case GL_RGB: return 3;
		case GL_RGBA: return 4;
		case GL_LUMINANCE: return 1;
		case GL_LUMINANCE_ALPHA: return 2;
		default:
#ifdef MODEL_JUST_LOAD
			cerr << "unknown texture format " << format << "\n";
			exit(-1);
#else
			ostringstream oss; oss << "unknown texture format " << format << "\n";
			system::sys().myassert(false, oss.str());
#endif
	}
	return 4;
}

void texture::init(SDL_Surface* teximage, unsigned sx, unsigned sy, unsigned sw, unsigned sh,
	int clamp, bool keep)
{
	// compute texture width and height
	unsigned tw = 1, th = 1;
	while (tw < sw) tw *= 2;
	while (th < sh) th *= 2;
	width = tw;
	height = th;
	
#ifndef MODEL_JUST_LOAD
	system::sys().myassert(tw <= get_max_size(), "texture: texture width too big");
	system::sys().myassert(th <= get_max_size(), "texture: texture height too big");
	glGenTextures(1, &opengl_name);
#endif
	SDL_LockSurface(teximage);

	unsigned bpp = teximage->format->BytesPerPixel;

	if (teximage->format->palette != 0) {
		//old color table code, does not work
		//glEnable(GL_COLOR_TABLE);
#ifdef MODEL_JUST_LOAD
		if (bpp != 1) {
			cerr << "texture: only 8bit palette files supported\n";
			exit(-1);
		}
#else
		system::sys().myassert(bpp == 1, "texture: only 8bit palette files supported");
#endif
		int ncol = teximage->format->palette->ncolors;
#ifdef MODEL_JUST_LOAD
		if (ncol > 256) {
			cerr << "texture: max. 256 colors in palette supported\n";
			exit(-1);
		}
#else
		system::sys().myassert(ncol <= 256, "texture: max. 256 colors in palette supported");
#endif
		bool usealpha = (teximage->flags & SDL_SRCCOLORKEY);

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
				const SDL_Color& pixcolor = teximage->format->palette->colors[pixindex];
				*ptr2++ = pixcolor.r;
				*ptr2++ = pixcolor.g;
				*ptr2++ = pixcolor.b;
				if (usealpha)
					*ptr2++ = (pixindex == (teximage->format->colorkey & 0xff)) ? 0x00 : 0xff;
			}
			//old color table code, does not work
			//memcpy(ptr, offset, sw);
			offset += teximage->pitch;
			ptr += tw*bpp;
		}
	} else {
		bool usealpha = teximage->format->Amask != 0;
		format = usealpha ? GL_RGBA : GL_RGB;
		data.resize(tw*th*bpp);
		unsigned char* ptr = &data[0];
		unsigned char* offset = ((unsigned char*)(teximage->pixels))
			+ sy*teximage->pitch + sx*bpp;
		for (unsigned y = 0; y < sh; y++) {
			memcpy(ptr, offset, sw*bpp);
			offset += teximage->pitch;
			ptr += tw*bpp;
		}
	}
	SDL_UnlockSurface(teximage);
	
#ifndef MODEL_JUST_LOAD
	update();
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mapping);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mapping);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, clamp);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, clamp);
#endif

	if (!keep) data.clear();
}
	
texture::texture(const string& filename, int mapping_, int clamp, bool keep)
{
	mapping = mapping_;
	texfilename = filename;
	SDL_Surface* teximage = IMG_Load(filename.c_str());
#ifdef MODEL_JUST_LOAD
	if (teximage == 0) {
		cerr << "texture: failed to load " << filename << "\n";
		exit(-1);
	}
#else
	system::sys().myassert(teximage != 0, string("texture: failed to load ")+filename);
#endif
	init(teximage, 0, 0, teximage->w, teximage->h, clamp, keep);
	SDL_FreeSurface(teximage);
}	

texture::texture(void* pixels, unsigned w, unsigned h, int format_,
	int mapping_, int clamp, bool keep)
{
	width = w;
	height = h;
	format = format_;
	mapping = mapping_;
#ifndef MODEL_JUST_LOAD
	glGenTextures(1, &opengl_name);
#endif
	data.resize(get_bpp()*w*h);
	if (pixels)
		memcpy(&data[0], pixels, data.size());
	
#ifndef MODEL_JUST_LOAD
	update();

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mapping);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mapping);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, clamp);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, clamp);
#endif

	if (!keep) data.clear();
}

texture::~texture()
{
#ifndef MODEL_JUST_LOAD
	glDeleteTextures(1, &opengl_name);
#endif
}

#ifndef MODEL_JUST_LOAD
void texture::update(void) const
{
	if (data.size() == 0) return;
	
	glBindTexture(GL_TEXTURE_2D, opengl_name);
	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, &data[0]);
	if (	mapping == GL_NEAREST_MIPMAP_NEAREST
		|| mapping == GL_NEAREST_MIPMAP_LINEAR
		|| mapping == GL_LINEAR_MIPMAP_NEAREST
		|| mapping == GL_LINEAR_MIPMAP_LINEAR ) {

		gluBuild2DMipmaps(GL_TEXTURE_2D, format, width, height, format, GL_UNSIGNED_BYTE, &data[0]);
	}
}

void texture::set_gl_texture(void) const
{
	glBindTexture(GL_TEXTURE_2D, get_opengl_name());
}

// draw_image
void texture::draw(int x, int y) const
{
	draw(x, y, get_width(), get_height());
}

void texture::draw_hm(int x, int y) const
{
	draw_hm(x, y, get_width(), get_height());
}

void texture::draw_vm(int x, int y) const
{
	draw_vm(x, y, get_width(), get_height());
}

void texture::draw(int x, int y, int w, int h) const
{
	glBindTexture(GL_TEXTURE_2D, get_opengl_name());
	glBegin(GL_QUADS);
	glTexCoord2i(0,0);
	glVertex2i(x,y);
	glTexCoord2i(0,1);
	glVertex2i(x,y+h);
	glTexCoord2i(1,1);
	glVertex2i(x+w,y+h);
	glTexCoord2i(1,0);
	glVertex2i(x+w,y);
	glEnd();
}

void texture::draw_hm(int x, int y, int w, int h) const
{
	glBindTexture(GL_TEXTURE_2D, get_opengl_name());
	glBegin(GL_QUADS);
	glTexCoord2i(1,0);
	glVertex2i(x,y);
	glTexCoord2i(1,1);
	glVertex2i(x,y+h);
	glTexCoord2i(0,1);
	glVertex2i(x+w,y+h);
	glTexCoord2i(0,0);
	glVertex2i(x+w,y);
	glEnd();
}

void texture::draw_vm(int x, int y, int w, int h) const
{
	glBindTexture(GL_TEXTURE_2D, get_opengl_name());
	glBegin(GL_QUADS);
	glTexCoord2i(0,1);
	glVertex2i(x,y);
	glTexCoord2i(0,0);
	glVertex2i(x,y+h);
	glTexCoord2i(1,0);
	glVertex2i(x+w,y+h);
	glTexCoord2i(1,1);
	glVertex2i(x+w,y);
	glEnd();
}

void texture::draw_rot(int x, int y, double angle) const
{
	glPushMatrix();
	glTranslatef(x, y, 0);
	glRotatef(angle, 0, 0, 1);
	draw(-int(get_width())/2, -int(get_height())/2);
	glPopMatrix();
}

void texture::draw_tiles(int x, int y, int w, int h) const
{
	float tilesx = float(w)/width;
	float tilesy = float(h)/height;
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
	float x1 = float(tx)/width;
	float y1 = float(ty)/height;
	float x2 = float(tx+tw)/width;
	float y2 = float(ty+th)/height;
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

unsigned texture::get_max_size(void)
{
	GLint i;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &i);
	return i;
}
#endif /*MODEL_JUST_LOAD*/
