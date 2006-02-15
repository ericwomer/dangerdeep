/*
Danger from the Deep - Open source submarine simulation
Copyright (C) 2003-2006  Thorsten Jordan, Luis Barrancos and others.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

// OpenGL texture drawing based on SDL Surfaces
// (C)+(W) by Thorsten Jordan. See LICENSE

#ifndef IMAGE_H
#define IMAGE_H

#include <vector>
#include <list>
#include <string>
#include <SDL.h>

class texture;

///\brief Handles an image for OpenGL based rendering.
class image
{
protected:
	SDL_Surface* img;
	std::string name;	// filename
	unsigned width, height;

	// caching.
	struct cache_entry 
	{
		const image* object;		// for comparison
		unsigned time_stamp;	// for cache handling
		unsigned gltx, glty;	// no. of textures in x and y direction
		std::vector<texture*> textures;
		void generate(const image* obj);
		cache_entry();
		~cache_entry();
	};

	// the cache
	static std::list<cache_entry> cache;

	// create texture(s) from image for faster drawing
	// store in cache if not already cached
	static cache_entry& check_cache(const image* obj);

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
	// draw with caching image in texture memory
	void draw(int x, int y) const;
	// draw image by directly copying pixels to the frame buffer
	void draw_direct(int x, int y) const;
	// returns 0 if image is stored in texture
	SDL_Surface* get_SDL_Surface() const { return img; }
	unsigned get_width() const { return width; };
	unsigned get_height() const { return height; };
};

#endif
