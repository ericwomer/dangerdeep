// SDL/OpenGL based textures
// (C)+(W) by Thorsten Jordan. See LICENSE

#ifndef TEXTURE_H
#define TEXTURE_H

#include <string>
using namespace std;
#include <SDL/SDL.h>

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
		unsigned mapping, bool clamp, bool morealpha);
public:
	// mapping: 0 nearest, 1 bilinear, 2 mipmap bilinear, 3 trilinear
	texture(const string& filename, unsigned mapping = 0,
		bool clamp = true, bool morealpha = false);

	// create texture from subimage of SDL surface.
	// sw,sh need not to be powers of two.
	texture(SDL_Surface* teximage, unsigned sx, unsigned sy, unsigned sw, unsigned sh,
		unsigned mapping = 0, bool clamp = true, bool morealpha = false) {
			init(teximage, sx, sy, sw, sh, mapping, clamp, morealpha);
		};
	
	~texture();
	unsigned get_opengl_name(void) const { return opengl_name; };
	string get_name(void) const { return texfilename; };
	unsigned get_width(void) const { return width; };
	unsigned get_height(void) const { return height; };

	static int paletted_textures;	// 0 no 1 yes -1 untested
	static bool check_for_paletted_textures(void);
};

#endif
