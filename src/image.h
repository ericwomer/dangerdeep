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
#include "ptrvector.h"
#include "texture.h"	// needed at least for correct "delete" usage in ptrvector


///\brief Handles an image for OpenGL based rendering.
class image
{
protected:
	std::string name;	// filename
	unsigned width, height;
	unsigned gltx, glty;	// no. of textures in x and y direction
	ptrvector<texture> textures;

private:
	image();
	image& operator= (const image& other);
	image(const image& other);

public:
	/// create image
	image(const std::string& s);

	/// draw image at position
	void draw(int x, int y, const colorf& col = colorf(1,1,1,1)) const;

	unsigned get_width() const { return width; };
	unsigned get_height() const { return height; };
};

#endif
