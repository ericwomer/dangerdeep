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
#include <vector>
#include <string>
using namespace std;

class texture
{
protected:
	unsigned opengl_name, width, height;
	string texfilename;
	int format;		// GL_RGB or GL_RGBA
	int mapping;		// how GL draws the texture (GL_NEAREST, GL_LINEAR, etc.)
	vector<Uint8> data;	// texture data (only stored if user wishes that)

	texture() {};
	texture& operator=(const texture& other) { return *this; };
	texture(const texture& other) {};
	
	// share common constructor code
	void init(SDL_Surface* teximage, unsigned sx, unsigned sy, unsigned sw, unsigned sh,
		int clamp, bool keep);

public:
	texture(const string& filename, int mapping_ = GL_NEAREST, int clamp = GL_REPEAT, bool keep = false);

	// create texture from subimage of SDL surface.
	// sw,sh need not to be powers of two.
	texture(SDL_Surface* teximage, unsigned sx, unsigned sy, unsigned sw, unsigned sh,
		int mapping_ = GL_NEAREST, int clamp = GL_REPEAT, bool keep = true) {
			mapping = mapping_;
			init(teximage, sx, sy, sw, sh, clamp, keep);
		};

	// create texture from memory values (use openGl constants for format,type,etc.
	// w,h must be powers of two.
	texture(void* pixels, unsigned w, unsigned h, int format_, int type,
		int mapping_, int clamp, bool keep = true);
	
	~texture();
	
	// (re)creates OpenGL texture from stored data
	void update(void) const;
	int get_format(void) const { return format; }
	vector<Uint8>& get_data(void) { return data; }
	
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
	// repeat texture in tiles in the given screen rectangle
	void draw_tiles(int x, int y, int w, int h) const;
	// draw a sub image given by the tx,ty,tw,th values to a screen rectangle x,y,w,h
	void draw_subimage(int x, int y, int w, int h, unsigned tx, unsigned ty,
		unsigned tw, unsigned th) const;

	static unsigned get_max_size(void);
};

#endif
