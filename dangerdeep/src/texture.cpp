// SDL/OpenGL based textures
// (C)+(W) by Thorsten Jordan. See LICENSE

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "oglext/OglExt.h"
#include <GL/glu.h>
#include <SDL.h>
#include <SDL_image.h>

#include "texture.h"
#include "system.h"
#include <vector>
using namespace std;


void texture::init(SDL_Surface* teximage, unsigned sx, unsigned sy, unsigned sw, unsigned sh,
	int mapping, int clamp)
{
	GLuint texname;
	
	// compute texture width and height
	unsigned tw = 1, th = 1;
	while (tw < sw) tw *= 2;
	while (th < sh) th *= 2;
	
	system::sys().myassert(tw <= get_max_size(), "texture: texture width too big");
	system::sys().myassert(th <= get_max_size(), "texture: texture height too big");

	glGenTextures(1, &texname);
	glBindTexture(GL_TEXTURE_2D, texname);
	SDL_LockSurface(teximage);

	unsigned bpp = teximage->format->BytesPerPixel;

	GLenum internalformat;
	GLenum externalformat;

	unsigned char* tmpimage = 0;
	
	if (teximage->format->palette != 0) {
		glEnable(GL_COLOR_TABLE);
		system::sys().myassert(bpp == 1, "texture: only 8bit palette files supported");
		int ncol = teximage->format->palette->ncolors;
		system::sys().myassert(ncol <= 256, "texture: max. 256 colors in palette supported");
		bool usealpha = false;
		vector<unsigned> palette(256);
		for (int i = 0; i < ncol; i++)
			palette[i] = ((unsigned*)(teximage->format->palette->colors))[i] | 0xff000000;
		if (teximage->flags & SDL_SRCCOLORKEY) {
			palette[teximage->format->colorkey & 0xff] &= 0xffffff;
			usealpha = true;
		}
		cerr << "creating paletted texture, ncol " << ncol << " usealpha " << usealpha << "\n";

		internalformat = usealpha ? GL_RGBA : GL_RGB;

		// fixme: with big endian machines the colors could be stored reversed, check this
		glColorTable(GL_TEXTURE_2D, internalformat, 256, GL_RGBA, GL_UNSIGNED_BYTE, &(palette[0]));
		internalformat = GL_COLOR_INDEX8_EXT;
		externalformat = GL_COLOR_INDEX;
		tmpimage = new unsigned char [tw*th];
		memset(tmpimage, 0, tw*th);
		unsigned char* ptr = tmpimage;
		unsigned char* offset = ((unsigned char*)(teximage->pixels)) + sy*teximage->pitch + sx;
		for (unsigned y = 0; y < sh; y++) {
			memcpy(ptr, offset, sw);
			offset += teximage->pitch;
			ptr += tw;
		}
	} else {
		bool usealpha = teximage->format->Amask != 0;
		internalformat = usealpha ? GL_RGBA : GL_RGB;
		externalformat = usealpha ? GL_RGBA : GL_RGB;
		tmpimage = new unsigned char [tw*th*bpp];
		memset(tmpimage, 0, tw*th*bpp);
		unsigned char* ptr = tmpimage;
		unsigned char* offset = ((unsigned char*)(teximage->pixels))
			+ sy*teximage->pitch + sx*bpp;
		for (unsigned y = 0; y < sh; y++) {
			memcpy(ptr, offset, sw*bpp);
			offset += teximage->pitch;
			ptr += tw*bpp;
		}
	}
	SDL_UnlockSurface(teximage);
	
	// create texture
	glTexImage2D(GL_TEXTURE_2D, 0, internalformat, tw, th, 0,
		externalformat, GL_UNSIGNED_BYTE, tmpimage);
	if (	mapping == GL_NEAREST_MIPMAP_NEAREST
		|| mapping == GL_NEAREST_MIPMAP_LINEAR
		|| mapping == GL_LINEAR_MIPMAP_NEAREST
		|| mapping == GL_LINEAR_MIPMAP_LINEAR ) {

		gluBuild2DMipmaps(GL_TEXTURE_2D, internalformat, tw, th, externalformat,
			GL_UNSIGNED_BYTE, tmpimage);
	}

	glDisable(GL_COLOR_TABLE);	
	delete [] tmpimage;
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mapping);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mapping);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, clamp);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, clamp);

	opengl_name = texname;
	width = tw;
	height = th;
}
	
texture::texture(const string& filename, int mapping, int clamp)
{
	texfilename = filename;
	SDL_Surface* teximage = IMG_Load(filename.c_str());
	system::sys().myassert(teximage != 0, string("texture: failed to load")+filename);
	init(teximage, 0, 0, teximage->w, teximage->h, mapping, clamp);
	SDL_FreeSurface(teximage);
}	

texture::texture(void* pixels, unsigned w, unsigned h, int extformat, int intformat, int type,
	int mapping, int clamp)
{
	width = w;
	height = h;
	glGenTextures(1, &opengl_name);
	glBindTexture(GL_TEXTURE_2D, opengl_name);

	GLenum internalformat = intformat;
	GLenum externalformat = extformat;

	glTexImage2D(GL_TEXTURE_2D, 0, internalformat, w, h, 0,
		externalformat, type, pixels);
	if (	mapping == GL_NEAREST_MIPMAP_NEAREST
		|| mapping == GL_NEAREST_MIPMAP_LINEAR
		|| mapping == GL_LINEAR_MIPMAP_NEAREST
		|| mapping == GL_LINEAR_MIPMAP_LINEAR ) {

		gluBuild2DMipmaps(GL_TEXTURE_2D, internalformat, w, h, externalformat,
			type, pixels);
	}
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mapping);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mapping);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, clamp);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, clamp);
}

texture::~texture()
{
	GLuint tex_name = opengl_name;
	glDeleteTextures(1, &tex_name);
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

void texture::draw_tiles(int x, int y, int w, int h, unsigned tilesx, unsigned tilesy) const
{
	glBindTexture(GL_TEXTURE_2D, get_opengl_name());
	glBegin(GL_QUADS);
	glTexCoord2i(0,0);
	glVertex2i(0,0);
	glTexCoord2i(0,tilesy);
	glVertex2i(0,h);
	glTexCoord2i(tilesx,tilesy);
	glVertex2i(w,h);
	glTexCoord2i(tilesx,0);
	glVertex2i(w,0);
	glEnd();
}

unsigned texture::get_max_size(void)
{
	GLint i;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &i);
	return i;
}
