// SDL/OpenGL based textures
// (C)+(W) by Thorsten Jordan. See LICENSE

#ifndef TEXTURE_H
#define TEXTURE_H

#include <string>
using namespace std;
#ifdef WIN32
#include <SDL.h>
#include <gl.h>
#else
#include <SDL/SDL.h>
#include <GL/gl.h>
#endif

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
	
	~texture();
	unsigned get_opengl_name(void) const { return opengl_name; };
	void set_gl_texture(void) const;
	string get_name(void) const { return texfilename; };
	unsigned get_width(void) const { return width; };
	unsigned get_height(void) const { return height; };

	// 2d drawing must be turned on for this functions
	void draw_image(int x, int y) const;
	void draw_hm_image(int x, int y) const;	// horizontally mirrored
	void draw_vm_image(int x, int y) const;	// vertically mirrored
	void draw_image(int x, int y, int w, int h) const;
	void draw_hm_image(int x, int y, int w, int h) const;	// horizontally mirrored
	void draw_vm_image(int x, int y, int w, int h) const;	// vertically mirrored
	void draw_rot_image(int x, int y, double angle) const;	// draw rotated image
	void draw_tiles(int x, int y, int w, int h, unsigned tiles, unsigned tilesy) const;	

	static int paletted_textures;	// 0 no 1 yes -1 untested
	static bool check_for_paletted_textures(void);
	static unsigned get_max_width(void) { return 256; } // fixme ask opengl
	static unsigned get_max_height(void) { return 256; } // fixme ask opengl
};

#endif
