// SDL/OpenGL based textures
// (C)+(W) by Thorsten Jordan. See LICENSE

#ifndef TEXTURE_H
#define TEXTURE_H

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <SDL.h>
#include <GL/gl.h>
#include <string>
using namespace std;

class texture
{
protected:
	unsigned opengl_name, width, height;
	string texfilename;
private:
	texture() {};
	texture& operator=(const texture& other) { return *this; };
	texture(const texture& other) {};
	
	// share common constructor code
	void init(SDL_Surface* teximage, unsigned sx, unsigned sy, unsigned sw, unsigned sh,
		int mapping, int clamp);
public:
	texture(const string& filename, int mapping = GL_NEAREST, int clamp = GL_REPEAT);

	// create texture from subimage of SDL surface.
	// sw,sh need not to be powers of two.
	texture(SDL_Surface* teximage, unsigned sx, unsigned sy, unsigned sw, unsigned sh,
		int mapping = GL_NEAREST, int clamp = GL_REPEAT) {
			init(teximage, sx, sy, sw, sh, mapping, clamp);
		};

	// create texture from memory values (use openGl constants for format,type,etc.
	// w,h must be powers of two.
	texture(void* pixels, unsigned w, unsigned h, int extformat, int intformat, int type,
		int mapping, int clamp);
	
	~texture();
	unsigned get_opengl_name(void) const { return opengl_name; };
	void set_gl_texture(void) const;
	string get_name(void) const { return texfilename; };
	unsigned get_width(void) const { return width; };
	unsigned get_height(void) const { return height; };

	// 2d drawing must be turned on for this functions
	void draw(int x, int y) const;
	void draw_hm(int x, int y) const;	// horizontally mirrored
	void draw_vm(int x, int y) const;	// vertically mirrored
	void draw(int x, int y, int w, int h) const;
	void draw_hm(int x, int y, int w, int h) const;	// horizontally mirrored
	void draw_vm(int x, int y, int w, int h) const;	// vertically mirrored
	void draw_rot(int x, int y, double angle) const;	// draw rotated image
	void draw_tiles(int x, int y, int w, int h, unsigned tiles, unsigned tilesy) const;	

	static unsigned get_max_size(void);
};

#endif
