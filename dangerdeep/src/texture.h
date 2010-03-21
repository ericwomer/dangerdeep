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

// SDL/OpenGL based textures
// (C)+(W) by Thorsten Jordan. See LICENSE

#ifndef TEXTURE_H
#define TEXTURE_H

#ifdef WIN32
// 2006-12-01 doc1972 added check to prevent double definition. 
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif /* WIN32_LEAN_AND_MEAN */

// taken from DevIL library

#include <windows.h>
#endif

#include <SDL.h>
#include <SDL_image.h>
#include "oglext/OglExt.h"

#include <vector>
#include <list>
#include <string>
#include <memory>

#include "error.h"
#include "vector3.h"
#include "color.h"
#include "cfg.h"

/// wrapper for SDL_Surface/SDL_images to make memory management automatic
class sdl_image
{
 public:
	/// create image from file
	///@note can combine RGB(jpg) and A(png) into one surface
	sdl_image(const std::string& filename);

	/// destroy image
	~sdl_image();

	/// lock surface
	void lock();

	/// unlock surface
	void unlock();

	/// get pointer to surface
	SDL_Surface* get_SDL_Surface() const { return img; }

	/// access elements of surface
	SDL_Surface* operator->() const throw() { return img; }

	/// transform values to plain vector
	std::vector<uint8_t> get_plain_data(unsigned& w, unsigned& h, unsigned& byte_per_pixel);

 protected:
	SDL_Surface* img;

 private:
	sdl_image();	// no copy
	sdl_image& operator= (const sdl_image& other);
	sdl_image(const sdl_image& other);
};



///\brief Handles an OpenGL texture with loading and display.
class texture
{
public:
	typedef std::auto_ptr<texture> ptr;

	enum mapping_mode {
		NEAREST,
		LINEAR,
		NEAREST_MIPMAP_NEAREST,
		NEAREST_MIPMAP_LINEAR,
		LINEAR_MIPMAP_NEAREST,
		LINEAR_MIPMAP_LINEAR,
		NR_OF_MAPPING_MODES
	};

	enum clamping_mode {
		REPEAT,
		CLAMP,
		NR_OF_CLAMPING_MODES
	};

private:
	texture& operator=(const texture& other);
	texture(const texture& other);

protected:
	GLuint opengl_name;
	unsigned width;
	unsigned height;
	unsigned gl_width;
	unsigned gl_height;
	std::string texfilename;
	int format;	// GL_RGB, GL_RGBA, etc.
	GLenum dimension;
	mapping_mode mapping; // how GL draws the texture (GL_NEAREST, GL_LINEAR, etc.)
	clamping_mode clamping; // how GL handles the border (GL_REPEAT, GL_CLAMP_TO_EDGE)
	
	void sdl_init(SDL_Surface* teximage, unsigned sx, unsigned sy, unsigned sw, unsigned sh,
		      bool makenormalmap = false, float detailh = 1.0f, bool rgb2grey = false);

	void sdl_rgba_init(SDL_Surface* teximagergb, SDL_Surface* teximagea);

	// copy data to OpenGL, set parameters
	void init(const std::vector<Uint8>& data, bool makenormalmap = false,
		  float detailh = 1.0f);

	static int size_non_power_2;

	// statistics.
	static unsigned mem_used;
	static unsigned mem_alloced;
	static unsigned mem_freed;

	texture() {}
	
	struct dds_data {
	    GLsizei  width;
	    GLsizei  height;
		GLint    components;
	    GLenum   format;
		int      numMipMaps;
	    std::vector<GLubyte> pixels;
	};


	// taken from DevIL library
#ifdef _MSC_VER
__declspec( align( 1 ) )
#endif
	typedef struct DDSHEAD
	{
		int8_t	Signature[4];

		uint32_t	Size1;				// size of the structure (minus MagicNum)
		uint32_t	Flags1; 			// determines what fields are valid
		uint32_t	Height; 			// height of surface to be created
		uint32_t	Width;				// width of input surface
		uint32_t	LinearSize; 		// Formless late-allocated optimized surface size
		uint32_t	Depth;				// Depth if a volume texture
		uint32_t	MipMapCount;		// number of mip-map levels requested
		uint32_t	AlphaBitDepth;		// depth of alpha buffer requested

		uint32_t	NotUsed[10];

		uint32_t	Size2;				// size of structure
		uint32_t	Flags2;				// pixel format flags
		uint32_t	FourCC;				// (FOURCC code)
		uint32_t	RGBBitCount;		// how many bits per pixel
		uint32_t	RBitMask;			// mask for red bit
		uint32_t	GBitMask;			// mask for green bits
		uint32_t	BBitMask;			// mask for blue bits
		uint32_t	RGBAlphaBitMask;	// mask for alpha channel

		uint32_t	ddsCaps1, ddsCaps2, ddsCaps3, ddsCaps4; // direct draw surface capabilities
		uint32_t	TextureStage;
	}
#ifdef __GNUC__
	__attribute__ ((packed))
#endif
	DDSHEAD;
	
	void load_dds(const std::string& filename, dds_data& target);


public:
	class texerror : public error
	{
	public:
		texerror(const std::string& name, const std::string& s)
			: error(std::string("texture \"") + name +
				std::string("\" error: ") + s) {}
		~texerror() throw() {}
	};

	// if "makenormalmap" is true and format is GL_LUMINANCE,
	// a normal map (RGB) is computed from the texture.
	// give height of detail (scale factor) for normal mapping, mostly much larger than 1.0
	texture(const std::string& filename, mapping_mode mapping_ = NEAREST, clamping_mode clamp = REPEAT,
		bool makenormalmap = false, float detailh = 1.0f, bool rgb2grey = false, GLenum _dimension = GL_TEXTURE_2D);

	// create texture from subimage of SDL surface.
	// sw,sh need not to be powers of two.
	texture(SDL_Surface* teximage, unsigned sx, unsigned sy, unsigned sw, unsigned sh,
		mapping_mode mapping_ = NEAREST, clamping_mode clamp = REPEAT,
		bool makenormalmap = false, float detailh = 1.0f, bool rgb2grey = false, GLenum _dimension = GL_TEXTURE_2D);

	// create texture from subimage of SDL surface, based on sdl_image
	// sw,sh need not to be powers of two.
	texture(const sdl_image& teximage, unsigned sx, unsigned sy, unsigned sw, unsigned sh,
		mapping_mode mapping_ = NEAREST, clamping_mode clamp = REPEAT,
		bool makenormalmap = false, float detailh = 1.0f, bool rgb2grey = false, GLenum _dimension = GL_TEXTURE_2D);

	// create texture from memory values (use openGl constants for format,etc.
	// w,h must be powers of two.
	texture(const std::vector<Uint8>& pixels, unsigned w, unsigned h,
		int format_, mapping_mode mapping_,
		clamping_mode clamp, bool makenormalmap = false, float detailh = 1.0f, GLenum _dimension = GL_TEXTURE_2D);

	// allocate a GL texture, set some values, but do not fill texel values
	// w,h are more for information purposes.
	texture(unsigned w, unsigned h,
		int format_, mapping_mode mapping_,
		clamping_mode clamp, bool force_no_compression = false);
	
	// load a compressed DDS file (DXT1, DXT3, DXT5)
	texture(const std::string& filename, bool dummy, mapping_mode mapping_ = NEAREST, clamping_mode clamp = REPEAT);

	~texture();

	/// change sub-area of texture from memory values (use openGL constants for format,etc.
	void sub_image(int xoff, int yoff, unsigned w, unsigned h,
		       const std::vector<Uint8>& pixels, int format);

	int get_format() const { return format; }
	unsigned get_bpp() const;
	unsigned get_opengl_name() const { return opengl_name; }
	void set_gl_texture() const;
	std::string get_name() const { return texfilename; }
	unsigned get_width() const { return width; }
	unsigned get_height() const { return height; }
	unsigned get_gl_width() const { return gl_width; }
	unsigned get_gl_height() const { return gl_height; }

	// 2d drawing must be turned on for this functions
	void draw(int x, int y, const colorf& col = colorf(1,1,1,1)) const;
	void draw_hm(int x, int y, const colorf& col = colorf(1,1,1,1)) const;	// horizontally mirrored
	void draw_vm(int x, int y, const colorf& col = colorf(1,1,1,1)) const;	// vertically mirrored
	void draw(int x, int y, int w, int h, const colorf& col = colorf(1,1,1,1)) const;
	void draw_hm(int x, int y, int w, int h, const colorf& col = colorf(1,1,1,1)) const;	// horizontally mirrored
	void draw_vm(int x, int y, int w, int h, const colorf& col = colorf(1,1,1,1)) const;	// vertically mirrored
	// draw rotated image around x,y (global)
	void draw_rot(int x, int y, double angle, const colorf& col = colorf(1,1,1,1)) const;
	// draw rotated image around x,y (global), rotate around local tx,ty
	void draw_rot(int x, int y, double angle, int tx, int ty, const colorf& col = colorf(1,1,1,1)) const;
	// repeat texture in tiles in the given screen rectangle
	void draw_tiles(int x, int y, int w, int h, const colorf& col = colorf(1,1,1,1)) const;
	// draw a sub image given by the tx,ty,tw,th values to a screen rectangle x,y,w,h
	void draw_subimage(int x, int y, int w, int h, unsigned tx, unsigned ty,
		unsigned tw, unsigned th, const colorf& col = colorf(1,1,1,1)) const;

	static unsigned get_max_size();

	///> returns if texture sizes other than powers of two are allowed. call after GL init.
	static bool size_non_power_two();

	// will scale down the image data to half size in each direction (at least w/h=1)
	// w,h must be > 1
	static std::vector<Uint8> scale_half(const std::vector<Uint8>& src,
					     unsigned w, unsigned h, unsigned bpp);

	// give powers of two for w,h
	static std::vector<Uint8> make_normals(const std::vector<Uint8>& src,
					       unsigned w, unsigned h, float detailh);

	// give powers of two for w,h
	static std::vector<Uint8> make_normals_with_alpha(const std::vector<Uint8>& src,
							  unsigned w, unsigned h, float detailh);
};



///\brief Handles an OpenGL 3d texture
class texture3d : public texture
{
 protected:
	unsigned depth;
	unsigned gl_depth;

 public:
	// create texture from memory values (use openGl constants for format,etc.
	texture3d(const std::vector<Uint8>& pixels, unsigned w, unsigned h, unsigned d,
		int format_, mapping_mode mapping_, clamping_mode clamp);

	// allocate a GL texture, set some values, but do not fill texel values
	// w,h are more for information purposes.
	texture3d(unsigned w, unsigned h, unsigned d,
		int format_, mapping_mode mapping_,
		clamping_mode clamp);

	/// change sub-area of texture from memory values (use openGL constants for format,etc.
	void sub_image(int xoff, int yoff, int zoff, unsigned w, unsigned h, unsigned d,
		       const std::vector<Uint8>& pixels, int format);

	unsigned get_depth() const { return depth; }
	unsigned get_gl_depth() const { return gl_depth; }

	/// render a quad with the texture applied
	void draw(int x, int y, int w, int h,
		  const vector3f& tc0, const vector3f& tcdx, const vector3f& tcdy) const;

 private:
	// not available for 3d textures
	void sub_image(int xoff, int yoff, unsigned w, unsigned h,
		       const std::vector<Uint8>& pixels, int format);
	void draw(int x, int y) const;
	void draw_hm(int x, int y) const;
	void draw_vm(int x, int y) const;
	void draw(int x, int y, int w, int h) const;
	void draw_hm(int x, int y, int w, int h) const;
	void draw_vm(int x, int y, int w, int h) const;
	void draw_rot(int x, int y, double angle) const;
	void draw_rot(int x, int y, double angle, int tx, int ty) const;
	void draw_tiles(int x, int y, int w, int h) const;
	void draw_subimage(int x, int y, int w, int h, unsigned tx, unsigned ty,
		unsigned tw, unsigned th) const;
};

#endif
