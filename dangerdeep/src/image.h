// OpenGL texture drawing based on SDL Surfaces
// (C)+(W) by Thorsten Jordan. See LICENSE

#ifndef IMAGE_H
#define IMAGE_H

#include "texture.h"
#include "oglext/OglExt.h"
#include <vector>
using namespace std;



class image
{
	SDL_Surface* img;
	bool maketexture;
	string name;	// filename
	unsigned width, height;

	unsigned gltx, glty;	// no. of textures in x and y direction
	vector<texture*> textures;
	
private:
	image();
	image& operator= (const image& other);	// later fixme
	image(const image& other);		// later fixme
					// operator=: copy image or assign? fixme

public:
	image(const string& s, bool maketex = false, int mapping = GL_NEAREST, int clamp = GL_CLAMP_TO_EDGE);
	~image();
	void draw(int x, int y) const;
	// returns 0 if image is stored in texture
	SDL_Surface* get_SDL_Surface(void) const { return img; }
	unsigned get_width(void) const { return width; };
	unsigned get_height(void) const { return height; };
};

#endif
