// OpenGL texture drawing based on SDL Surfaces
// (C)+(W) by Thorsten Jordan. See LICENSE

#ifndef IMAGE_H
#define IMAGE_H

#include "texture.h"
#include "oglext/OglExt.h"
#include <vector>

class image
{
public:
	typedef std::auto_ptr<image> ptr;

protected:
	SDL_Surface* img;
	std::string name;	// filename
	unsigned width, height;

	// cache exactly ONE image as textures.
	static const image* cached_object;
	static unsigned gltx, glty;	// no. of textures in x and y direction
	static std::vector<texture*> textures;

	// create texture(s) from image for faster drawing
	static void clear_cache(void);
	static void check_cache(const image* obj);	// store in cache if not already cached

	// statistics.
	static unsigned mem_used;
	static unsigned mem_alloced;
	static unsigned mem_freed;
private:
	image();
	image& operator= (const image& other);	// later fixme
	image(const image& other);		// later fixme
					// operator=: copy image or assign? fixme

public:
	// images are mostly used for background drawing.
	// creating them as textures leads to fast display and large video memory consumption.
	// So they should kept in system memory only and drawn via glDrawPixels
	// or at least a 1-slot cache could be realized (size: 1 screen, only last drawn
	// image is cached there)
	image(const std::string& s);
	~image();
	void draw(int x, int y) const;
	// returns 0 if image is stored in texture
	SDL_Surface* get_SDL_Surface(void) const { return img; }
	unsigned get_width(void) const { return width; };
	unsigned get_height(void) const { return height; };
};

#endif
