// SDL/OpenGL based textures
// (C)+(W) by Thorsten Jordan. See LICENSE

#ifndef TEXTURE_H
#define TEXTURE_H

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <SDL.h>
#ifdef USE_NATIVE_GL
#include <gl.h>
#else
#include "oglext/OglExt.h"
#endif

#include <vector>
#include <string>
#include <memory>

class texture
{
public:
	typedef std::auto_ptr<texture> ptr;

private:
	texture();
	texture& operator=(const texture& other);
	texture(const texture& other);

protected:
	// fixme: with automatic size adjustment width/height could change...
	// when user retrieves w/h later he could get strange results.
	// maybe store real and gl width, with two get_... functions.
	unsigned opengl_name;
	unsigned width;
	unsigned height;
	unsigned gl_width;
	unsigned gl_height;
	std::string texfilename;
	int format;	// GL_RGB, GL_RGBA, etc.
	int mapping;	// how GL draws the texture (GL_NEAREST, GL_LINEAR, etc.)
	int clamping;	// how GL handles the border (GL_REPEAT, GL_CLAMP, GL_CLAMP_TO_EDGE)
	
	void sdl_init(SDL_Surface* teximage, unsigned sx, unsigned sy, unsigned sw, unsigned sh,
		      bool makenormalmap = false, float detailh = 1.0f);

	// copy data to OpenGL, set parameters
	void init(const std::vector<Uint8>& data, bool makenormalmap = false,
		  float detailh = 1.0f);

	// will scale down the image data to half size in each direction (at least w/h=1)
	// w,h must be > 1
	static std::vector<Uint8> scale_half(const std::vector<Uint8>& src,
					     unsigned w, unsigned h, unsigned bpp);
	
	// give powers of two for w,h, bpp must be 1!
	static std::vector<Uint8> make_normals(const std::vector<Uint8>& src,
					       unsigned w, unsigned h, float detailh);

	// statistics.
	static unsigned mem_used;
	static unsigned mem_alloced;
	static unsigned mem_freed;

public:
	// if "makenormalmap" is true and format is GL_LUMINANCE. a normal map (RGB) is
	// computed from the texture.
	// give height of detail (scale factor) for normal mapping, mostly much larger than 1.0
	texture(const std::string& filename, int mapping_ = GL_NEAREST, int clamp = GL_REPEAT,
		bool makenormalmap = false, float detailh = 1.0f);

	// create texture from subimage of SDL surface.
	// sw,sh need not to be powers of two.
	texture(SDL_Surface* teximage, unsigned sx, unsigned sy, unsigned sw, unsigned sh,
		int mapping_ = GL_NEAREST, int clamp = GL_REPEAT,
		bool makenormalmap = false, float detailh = 1.0f);

	// create texture from memory values (use openGl constants for format,etc.
	// w,h must be powers of two.
	texture(const std::vector<Uint8>& pixels, unsigned w, unsigned h,
		int format_, int mapping_,
		int clamp, bool makenormalmap = false, float detailh = 1.0f);
	~texture();
	
	int get_format(void) const { return format; }
	unsigned get_bpp(void) const;
	unsigned get_opengl_name(void) const { return opengl_name; };
	void set_gl_texture(void) const;
	std::string get_name(void) const { return texfilename; };
	unsigned get_width(void) const { return width; };
	unsigned get_height(void) const { return height; };
	unsigned get_gl_width(void) const { return gl_width; };
	unsigned get_gl_height(void) const { return gl_height; };

	// 2d drawing must be turned on for this functions
	void draw(int x, int y) const;
	void draw_hm(int x, int y) const;	// horizontally mirrored
	void draw_vm(int x, int y) const;	// vertically mirrored
	void draw(int x, int y, int w, int h) const;
	void draw_hm(int x, int y, int w, int h) const;	// horizontally mirrored
	void draw_vm(int x, int y, int w, int h) const;	// vertically mirrored
	// draw rotated image around x,y (global)
	void draw_rot(int x, int y, double angle) const;
	// draw rotated image around x,y (global), rotate around local tx,ty
	void draw_rot(int x, int y, double angle, int tx, int ty) const;
	// repeat texture in tiles in the given screen rectangle
	void draw_tiles(int x, int y, int w, int h) const;
	// draw a sub image given by the tx,ty,tw,th values to a screen rectangle x,y,w,h
	void draw_subimage(int x, int y, int w, int h, unsigned tx, unsigned ty,
		unsigned tw, unsigned th) const;

	static unsigned get_max_size(void);

	// shader handling.
	// give GL_FRAGMENT_PROGRAM_ARB or GL_VERTEX_PROGRAM_ARB as type
	// fixme: a bit misplaced here!
	static GLuint create_shader(GLenum type, const std::string& filename);
	static void delete_shader(GLuint nr);
};

#endif
