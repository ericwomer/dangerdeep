// SDL/OpenGL based textures
// (C)+(W) by Thorsten Jordan. See LICENSE

#define GL_GLEXT_LEGACY
#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include "texture.h"
#include "system.h"
#include <vector>
using namespace std;

int texture::paletted_textures = -1;

void texture::init(SDL_Surface* teximage, unsigned sx, unsigned sy, unsigned sw, unsigned sh,
	unsigned mapping, bool clamp, bool morealpha)
{
	GLuint texname;
	
	// compute texture width and height
	unsigned tw = 1, th = 1;
	while (tw < sw) tw *= 2;
	while (th < sh) th *= 2;
	
	system::sys()->myassert(tw <= 256, "texture: textures wider than 256 texels are not supported");
	system::sys()->myassert(th <= 256, "texture: textures heigher than 256 texels are not supported");

	glGenTextures(1, &texname);
	glBindTexture(GL_TEXTURE_2D, texname);
	SDL_LockSurface(teximage);

	unsigned bpp = teximage->format->BytesPerPixel;

	GLenum internalformat;
	GLenum externalformat;

	unsigned char* tmpimage = 0;
	
	if (teximage->format->palette != 0) {
		system::sys()->myassert(bpp == 1, "texture: only 8bit palette files supported");
		int ncol = teximage->format->palette->ncolors;
		bool usealpha = false;
		vector<unsigned> palette(ncol);
		for (int i = 0; i < ncol; i++)
			palette[i] = ((unsigned*)(teximage->format->palette->colors))[i] | 0xff000000;
		if (teximage->flags & SDL_SRCCOLORKEY) {
			palette[teximage->format->colorkey & 0xff] &= 0xffffff;
			usealpha = true;
		}

		internalformat = usealpha ? (morealpha ? GL_RGBA : GL_RGB5_A1) : GL_RGB;

#ifdef GL_EXT_paletted_texture
		if (check_for_paletted_textures()) {
			glColorTableEXT(GL_TEXTURE_2D, internalformat, 256, GL_RGBA, GL_UNSIGNED_BYTE, &(palette[0]));
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
#endif
			tmpimage = new unsigned char [tw*th*4];
			memset(tmpimage, 0, tw*th*4);
			unsigned* ptr = (unsigned*)tmpimage;
			unsigned char* offset = ((unsigned char*)(teximage->pixels)) + sy*teximage->pitch + sx;
			for (unsigned y = 0; y < sh; y++) {
				for (unsigned x = 0; x < sw; x++) {
					*ptr++ = palette[*offset++];
				}
				offset += teximage->pitch - sw;
				ptr += tw - sw;
			}
			externalformat = GL_RGBA;
#ifdef GL_EXT_paletted_texture
		}
#endif
	} else {
		bool usealpha = teximage->format->Amask != 0;
		internalformat = usealpha ? (morealpha ? GL_RGBA : GL_RGB5_A1) : GL_RGB;
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
	if (mapping >= 2) {
		gluBuild2DMipmaps(GL_TEXTURE_2D, internalformat, tw, th, externalformat,
			GL_UNSIGNED_BYTE, tmpimage);
	}
	
	delete [] tmpimage;
	
	switch (mapping) {
		case 1:
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			break;
		case 2:
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_NEAREST);
			break;
		case 3:
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			break;
		default:
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			break;
	}
	if (clamp) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	} else {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	opengl_name = texname;
	width = tw;
	height = th;
}
	
texture::texture(const string& filename, unsigned mapping, bool clamp, bool morealpha)
{
	texfilename = filename;
	SDL_Surface* teximage = IMG_Load(filename.c_str());
	system::sys()->myassert(teximage, string("texture: failed to load")+filename);
	init(teximage, 0, 0, teximage->w, teximage->h, mapping, clamp, morealpha);
	SDL_FreeSurface(teximage);
}	
	
texture::~texture()
{
	GLuint tex_name = opengl_name;
	glDeleteTextures(1, &tex_name);
}

bool texture::check_for_paletted_textures(void)
{
	if (paletted_textures == -1) {
		const char* c = (const char*)(glGetString(GL_EXTENSIONS));
		if (c) {
			string glexts(c);
			return (glexts.find("GL_EXT_paletted_texture") != string::npos);
		}
	}
	return false;
}
