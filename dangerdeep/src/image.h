// OpenGL texture drawing based on SDL Surfaces
// (C)+(W) by Thorsten Jordan. See LICENSE

#ifndef IMAGE_H
#define IMAGE_H

#include "texture.h"
#include <vector>
using namespace std;

class image
{
	SDL_Surface* img;
	bool dynamic;
	string name;	// filename
	unsigned width, height;

	bool texturized;
	unsigned gltx, glty;	// no. of textures in x and y direction
	vector<texture*> textures;
	unsigned mapping;
	bool clamp;
	bool morealpha;
private:
	image();
	image& operator= (const image& other);	// later fixme
	image(const image& other);		// later fixme
					// operator=: copy image or assign? fixme

public:
	image(const string& s, unsigned mapping = 0, bool clamp = true,
		bool morealpha = false, bool loaddynamically = false);
	~image();
	void texturize(void);		// called automatically by draw
	void draw(int x, int y);	// can't be const, may call texturize()
	void clear_textures(void);
	unsigned get_width(void) const { return width; };
	unsigned get_height(void) const { return height; };
};

#endif
