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

#include "texture.h"
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
using namespace std;



void texture::init(SDL_Surface* teximage, unsigned sx, unsigned sy, unsigned sw, unsigned sh,
	int clamp, bool keep)
{
	// compute texture width and height
	unsigned tw = 1, th = 1;
	while (tw < sw) tw *= 2;
	while (th < sh) th *= 2;
	width = tw;
	height = th;

//	not longer necessary because of automatic texture resize in update()	
//	sys().myassert(tw <= get_max_size(), "texture: texture width too big");
//	sys().myassert(th <= get_max_size(), "texture: texture height too big");

	glGenTextures(1, &opengl_name);
	SDL_LockSurface(teximage);

	unsigned bpp = teximage->format->BytesPerPixel;

	if (teximage->format->palette != 0) {
		//old color table code, does not work
		//glEnable(GL_COLOR_TABLE);
		sys().myassert(bpp == 1, "texture: only 8bit palette files supported");
		int ncol = teximage->format->palette->ncolors;
		sys().myassert(ncol <= 256, "texture: max. 256 colors in palette supported");
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
	
	update();
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mapping);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mapping);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, clamp);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, clamp);

	if (!keep) data.clear();
}
	
texture::texture(const string& filename, int mapping_, int clamp, bool keep)
{
	mapping = mapping_;
	texfilename = filename;
	SDL_Surface* teximage = IMG_Load(filename.c_str());
	sys().myassert(teximage != 0, string("texture: failed to load ")+filename);
	init(teximage, 0, 0, teximage->w, teximage->h, clamp, keep);
	SDL_FreeSurface(teximage);
}	

texture::texture(Uint8* pixels, unsigned w, unsigned h, int format_,
	int mapping_, int clamp, bool keep)
{
	width = w;
	height = h;
	format = format_;
	mapping = mapping_;
	glGenTextures(1, &opengl_name);
	data.resize(get_bpp()*w*h);
	if (pixels)
		memcpy(&data[0], pixels, data.size());
	
	update();

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mapping);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mapping);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, clamp);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, clamp);

	if (!keep) data.clear();
}


texture* texture::make_normal_map(Uint8* heights, unsigned w, unsigned h, float detailh,
				  int mapping, int clamp)
{
	vector<Uint8> nmpix(3*w*h);
	float zh = 255.0f/detailh;
	unsigned ptr = 0;
	for (unsigned yy = 0; yy < h; ++yy) {
		unsigned y1 = (yy + h - 1) & (h - 1);
		unsigned y2 = (yy +     1) & (h - 1);
		for (unsigned xx = 0; xx < w; ++xx) {
			unsigned x1 = (xx + w - 1) & (w - 1);
			unsigned x2 = (xx +     1) & (w - 1);
			float h = heights[yy*w+xx];
			float hr = heights[yy*w+x2];
			float ho = heights[y1*w+xx];
			float hl = heights[yy*w+x1];
			float hu = heights[y2*w+xx];
			vector3f nm = (
				vector3f(h-hr, h-ho, zh).normal() +
				vector3f(hl-h, h-ho, zh).normal() +
				vector3f(hl-h, hu-h, zh).normal() +
				vector3f(h-hr, hu-h, zh).normal() ).normal();
			nmpix[ptr + 0] = Uint8(nm.x*127 + 128);
			nmpix[ptr + 1] = Uint8(-nm.y*127 + 128);
			nmpix[ptr + 2] = Uint8(nm.z*127 + 128);
			ptr += 3;
		}
	}
	return new texture(&nmpix[0], w, h, GL_RGB, mapping, clamp, false);
}


texture::~texture()
{
	glDeleteTextures(1, &opengl_name);
}

void texture::update(void)
{
	if (data.size() == 0) return;

	// automatic resizing of textures if they're too large
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
		vector<Uint8> data2(newwidth*newheight*bpp);
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
		data2.swap(data);
		width = newwidth;
		height = newheight;
	}
	
	glBindTexture(GL_TEXTURE_2D, opengl_name);
	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, &data[0]);
	if (	mapping == GL_NEAREST_MIPMAP_NEAREST
		|| mapping == GL_NEAREST_MIPMAP_LINEAR
		|| mapping == GL_LINEAR_MIPMAP_NEAREST
		|| mapping == GL_LINEAR_MIPMAP_LINEAR ) {

		gluBuild2DMipmaps(GL_TEXTURE_2D, format, width, height, format, GL_UNSIGNED_BYTE, &data[0]);
	}
}

unsigned texture::get_bpp(void) const
{
	switch (format) {
		case GL_RGB: return 3;
		case GL_RGBA: return 4;
		case GL_LUMINANCE: return 1;
		case GL_LUMINANCE_ALPHA: return 2;
		default:
			ostringstream oss; oss << "unknown texture format " << format << "\n";
			sys().myassert(false, oss.str());
	}
	return 4;
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



GLuint texture::create_shader(GLenum type, const string& filename)
{
	GLuint nr;

	glGenProgramsARB(1, &nr);
	glBindProgramARB(type, nr);
	ifstream ifprg(filename.c_str(), ios::in);
	sys().myassert(!ifprg.fail(), string("failed to open ")+filename);
	ifprg.seekg(0, ios::end);
	unsigned prglen = ifprg.tellg();
	ifprg.seekg(0, ios::beg);
	string prg(prglen, ' ');
	ifprg.read(&prg[0], prglen);

	glProgramStringARB(type, GL_PROGRAM_FORMAT_ASCII_ARB,
			   prg.size(), prg.c_str());

	// error handling
	int errorpos;

	glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &errorpos);
	if (errorpos != -1) {
		// get error string
		string errstr;
		const char* errstr2 = (const char*)glGetString(GL_PROGRAM_ERROR_STRING_ARB);
		if (errstr2 != 0)
			errstr = errstr2;
		ostringstream oss;
		oss << "GL shader program error (program type " << type
		    << ") at position " << errorpos << " in file "
		    << filename << ", error string is '" << errstr << "'\n";
		sys().myassert(false, oss.str());
	}

	GLint undernativelimits;
	glGetProgramivARB(type, GL_PROGRAM_UNDER_NATIVE_LIMITS_ARB, &undernativelimits);
	if (undernativelimits == 0) {
		ostringstream oss;
		oss << "GL shader program exceeds native limits (program type "
		    << type << "), filename "
		    << filename << "\n";
		sys().myassert(false, oss.str());
	}
	
	return nr;
}



void texture::delete_shader(GLuint nr)
{
	glDeleteProgramsARB(1, &nr);
}
