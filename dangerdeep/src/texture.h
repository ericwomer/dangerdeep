// SDL/OpenGL based textures
// (C)+(W) by Thorsten Jordan. See LICENSE

#ifndef TEXTURE_H
#define TEXTURE_H

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <SDL.h>
#ifdef USE_NATIVE_GL
#include <gl.h>
#else
#include "oglext/OglExt.h"
#endif

#include <vector>
#include <list>
#include <string>
#include <memory>

#include "error.h"

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
		CLAMP_TO_EDGE,
		NR_OF_CLAMPING_MODES
	};

private:
	texture();
	texture& operator=(const texture& other);
	texture(const texture& other);

protected:
	// fixme: with automatic size adjustment width/height could change...
	// when user retrieves w/h later he could get strange results.
	// maybe store real and gl width, with two get_... functions.
	unsigned opengl_name;
	unsigned width;
	unsigned height;
	unsigned gl_width;
	unsigned gl_height;
	std::string texfilename;
	int format;	// GL_RGB, GL_RGBA, etc.
	mapping_mode mapping; // how GL draws the texture (GL_NEAREST, GL_LINEAR, etc.)
	clamping_mode clamping; // how GL handles the border (GL_REPEAT, GL_CLAMP, GL_CLAMP_TO_EDGE)
	
	void sdl_init(SDL_Surface* teximage, unsigned sx, unsigned sy, unsigned sw, unsigned sh,
		      bool makenormalmap = false, float detailh = 1.0f, bool rgb2grey = false);

	void sdl_rgba_init(SDL_Surface* teximagergb, SDL_Surface* teximagea);

	// copy data to OpenGL, set parameters
	void init(const std::vector<Uint8>& data, bool makenormalmap = false,
		  float detailh = 1.0f);

	// will scale down the image data to half size in each direction (at least w/h=1)
	// w,h must be > 1
	std::vector<Uint8> scale_half(const std::vector<Uint8>& src,
				      unsigned w, unsigned h, unsigned bpp) const;
	
	// give powers of two for w,h, bpp must be 1!
	std::vector<Uint8> make_normals(const std::vector<Uint8>& src,
					unsigned w, unsigned h, float detailh) const;

	// statistics.
	static unsigned mem_used;
	static unsigned mem_alloced;
	static unsigned mem_freed;

	class texerror : public error
	{
	public:
		texerror(const std::string& name, const std::string& s)
			: error(std::string("texture \"") + name +
				std::string("\" error: ") + s) {}
		~texerror() throw() {}
	};

public:
	// if "makenormalmap" is true and format is GL_LUMINANCE,
	// a normal map (RGB) is computed from the texture.
	// give height of detail (scale factor) for normal mapping, mostly much larger than 1.0
	texture(const std::string& filename, mapping_mode mapping_ = NEAREST, clamping_mode clamp = REPEAT,
		bool makenormalmap = false, float detailh = 1.0f, bool rgb2grey = false);

	// create texture from subimage of SDL surface.
	// sw,sh need not to be powers of two.
	texture(SDL_Surface* teximage, unsigned sx, unsigned sy, unsigned sw, unsigned sh,
		mapping_mode mapping_ = NEAREST, clamping_mode clamp = REPEAT,
		bool makenormalmap = false, float detailh = 1.0f, bool rgb2grey = false);

	// create texture from memory values (use openGl constants for format,etc.
	// w,h must be powers of two.
	texture(const std::vector<Uint8>& pixels, unsigned w, unsigned h,
		int format_, mapping_mode mapping_,
		clamping_mode clamp, bool makenormalmap = false, float detailh = 1.0f);

	// allocate a GL texture, set some values, but do not fill texel values
	// w,h are more for information purposes.
	texture(unsigned w, unsigned h,
		int format_, mapping_mode mapping_,
		clamping_mode clamp);

	~texture();

	// reads an image file into a surface,
	// can combine RGB(jpg) and A(png) into one surface
	static SDL_Surface* read_from_file(const std::string& filename);
	
	int get_format() const { return format; }
	unsigned get_bpp() const;
	unsigned get_opengl_name() const { return opengl_name; };
	void set_gl_texture() const;
	std::string get_name() const { return texfilename; };
	unsigned get_width() const { return width; };
	unsigned get_height() const { return height; };
	unsigned get_gl_width() const { return gl_width; };
	unsigned get_gl_height() const { return gl_height; };

	// 2d drawing must be turned on for this functions
	void draw(int x, int y) const;
	void draw_hm(int x, int y) const;	// horizontally mirrored
	void draw_vm(int x, int y) const;	// vertically mirrored
	void draw(int x, int y, int w, int h) const;
	void draw_hm(int x, int y, int w, int h) const;	// horizontally mirrored
	void draw_vm(int x, int y, int w, int h) const;	// vertically mirrored
	// draw rotated image around x,y (global)
	void draw_rot(int x, int y, double angle) const;
	// draw rotated image around x,y (global), rotate around local tx,ty
	void draw_rot(int x, int y, double angle, int tx, int ty) const;
	// repeat texture in tiles in the given screen rectangle
	void draw_tiles(int x, int y, int w, int h) const;
	// draw a sub image given by the tx,ty,tw,th values to a screen rectangle x,y,w,h
	void draw_subimage(int x, int y, int w, int h, unsigned tx, unsigned ty,
		unsigned tw, unsigned th) const;

	static unsigned get_max_size();

	// shader handling.
	// give GL_FRAGMENT_PROGRAM_ARB or GL_VERTEX_PROGRAM_ARB as type
	// fixme: a bit misplaced here!
	static GLuint create_shader(GLenum type, const std::string& filename,
				    const std::list<std::string>& defines = std::list<std::string>());
	static void delete_shader(GLuint nr);

	// subclasses needed for shader parsing and compilation
	struct shader_node
	{
		enum state {
			eof,
			ifdef,
			elsedef,
			endifdef,
			code
		};
		virtual ~shader_node() {}
		virtual void compile(std::vector<std::string>& dstprg, const std::list<std::string>& defines,
				     bool disabled) = 0;
		static state get_state(const std::vector<std::string>& lines, unsigned& current_line);
	};

	struct shader_node_code : public shader_node
	{
		std::vector<std::string> lines;
		void compile(std::vector<std::string>& dstprg, const std::list<std::string>& defines,
			     bool disabled);
		shader_node_code(const std::vector<std::string>& lines, unsigned& current_line);
	};

	struct shader_node_program : public shader_node
	{
		std::list<shader_node*> subnodes;
		~shader_node_program();
		void compile(std::vector<std::string>& dstprg, const std::list<std::string>& defines,
			     bool disabled);
		shader_node_program(const std::vector<std::string>& lines, unsigned& current_line, bool endonelse = false, bool endonendif = false);
	};

	struct shader_node_ifelse : public shader_node
	{
		shader_node_program* ifnode;
		shader_node_program* elsenode;
		std::string define;
		shader_node_ifelse(const std::vector<std::string>& lines, unsigned& current_line);
		~shader_node_ifelse() { delete ifnode; delete elsenode; }
		void compile(std::vector<std::string>& dstprg, const std::list<std::string>& defines,
			     bool disabled);
	};
};

#endif
