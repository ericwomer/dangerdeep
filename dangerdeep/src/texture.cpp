// SDL/OpenGL based textures
// (C)+(W) by Thorsten Jordan. See LICENSE

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "oglext/OglExt.h"
#include <glu.h>

#include "system.h"

#include <SDL.h>
#include <SDL_image.h>
#include "vector3.h"
#include "texture.h"
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
using namespace std;

#undef MEMMEASURE

#ifdef MEMMEASURE
unsigned texture::mem_used = 0;
unsigned texture::mem_alloced = 0;
unsigned texture::mem_freed = 0;
#endif

// ------------------------------- GL mode tables -------------------
static GLuint mapmodes[texture::NR_OF_MAPPING_MODES] = {
	GL_NEAREST,
	GL_LINEAR,
	GL_NEAREST_MIPMAP_NEAREST,
	GL_NEAREST_MIPMAP_LINEAR,
	GL_LINEAR_MIPMAP_NEAREST,
	GL_LINEAR_MIPMAP_LINEAR
};

static bool do_mipmapping[texture::NR_OF_MAPPING_MODES] = {
	false,
	false,
	true,
	true,
	true,
	true
};

static GLuint magfilter[texture::NR_OF_MAPPING_MODES] = {
	GL_NEAREST,
	GL_LINEAR,
	GL_NEAREST,
	GL_NEAREST,
	GL_LINEAR,
	GL_LINEAR
};

static GLuint clampmodes[texture::NR_OF_CLAMPING_MODES] = {
	GL_REPEAT,
	GL_CLAMP,
	GL_CLAMP_TO_EDGE
};
// --------------------------------------------------



void texture::sdl_init(SDL_Surface* teximage, unsigned sx, unsigned sy, unsigned sw, unsigned sh,
		       bool makenormalmap, float detailh, bool rgb2grey)
{
	// compute texture width and height
	unsigned tw = 1, th = 1;
	while (tw < sw) tw *= 2;
	while (th < sh) th *= 2;
	width = sw;
	height = sh;
	gl_width = tw;
	gl_height = th;

//	not longer necessary because of automatic texture resize in update()	
//	sys().myassert(tw <= get_max_size(), "texture: texture width too big");
//	sys().myassert(th <= get_max_size(), "texture: texture height too big");

	SDL_LockSurface(teximage);

	unsigned bpp = teximage->format->BytesPerPixel;

/*
	cout << "texture: " << texfilename
	     << " palette: " << teximage->format->palette
	     << " bpp " << unsigned(teximage->format->BitsPerPixel)
	     << " bytepp " << unsigned(teximage->format->BytesPerPixel)
	     << " Rmask " << teximage->format->Rmask
	     << " Gmask " << teximage->format->Gmask
	     << " Bmask " << teximage->format->Bmask
	     << " Amask " << teximage->format->Amask
	     << "\n";
*/

	vector<Uint8> data;
	if (teximage->format->palette != 0) {
		//old color table code, does not work
		//glEnable(GL_COLOR_TABLE);
		sys().myassert(bpp == 1, "texture: only 8bit palette files supported");
		int ncol = teximage->format->palette->ncolors;
		sys().myassert(ncol <= 256, "texture: max. 256 colors in palette supported");
		bool usealpha = (teximage->flags & SDL_SRCCOLORKEY);

		// check for greyscale images (GL_LUMINANCE), fixme: add also LUMINANCE_ALPHA!
		bool lumi = false;
		if (ncol == 256 && !usealpha) {
			unsigned i = 0;
			for ( ; i < 256; ++i) {
				if (unsigned(teximage->format->palette->colors[i].r) != i) break;
				if (unsigned(teximage->format->palette->colors[i].g) != i) break;
				if (unsigned(teximage->format->palette->colors[i].b) != i) break;
			}
			if (i == 256) lumi = true;
		}

		if (lumi) {
			// grey value images
			format = GL_LUMINANCE;
			bpp = 1;

			data.resize(tw*th*bpp);
			unsigned char* ptr = &data[0];
			unsigned char* offset = ((unsigned char*)(teximage->pixels)) + sy*teximage->pitch + sx;
			for (unsigned y = 0; y < sh; y++) {
				memcpy(ptr, offset, sw/* * bpp */);
				offset += teximage->pitch;
				ptr += tw*bpp;
			}
		} else {
			// color images
			format = usealpha ? GL_RGBA : GL_RGB;
			bpp = usealpha ? 4 : 3;

			//old color table code, does not work		
			//glColorTable(GL_TEXTURE_2D, internalformat, 256, GL_RGBA, GL_UNSIGNED_BYTE, &(palette[0]));
			//internalformat = GL_COLOR_INDEX8_EXT;
			//externalformat = GL_COLOR_INDEX;
			data.resize(tw*th*bpp);
			unsigned char* ptr = &data[0];
			unsigned char* offset = ((unsigned char*)(teximage->pixels)) + sy*teximage->pitch + sx;
			for (unsigned y = 0; y < sh; y++) {
				unsigned char* ptr2 = ptr;
				for (unsigned x = 0; x < sw; ++x) {
					Uint8 pixindex = *(offset+x);
					const SDL_Color& pixcolor = teximage->format->palette->colors[pixindex];
					*ptr2++ = pixcolor.r;
					*ptr2++ = pixcolor.g;
					*ptr2++ = pixcolor.b;
					if (usealpha)
						*ptr2++ = (pixindex == (teximage->format->colorkey & 0xff)) ? 0x00 : 0xff;
				}
				//old color table code, does not work
				//memcpy(ptr, offset, sw);
				offset += teximage->pitch;
				ptr += tw*bpp;
			}
		}
	} else {
		bool usealpha = teximage->format->Amask != 0;
		if (rgb2grey) {
			if (usealpha) {
				format = GL_LUMINANCE_ALPHA;
				bpp = 2;
			} else {
				format = GL_LUMINANCE;
				bpp = 1;
			}
		} else {
			if (usealpha) {
				format = GL_RGBA;
				bpp = 4;
			} else {
				format = GL_RGB;
				bpp = 3;
			}
		}
		data.resize(tw*th*bpp);
		unsigned char* ptr = &data[0];
		unsigned char* offset = ((unsigned char*)(teximage->pixels))
			+ sy*teximage->pitch + sx*bpp;
		if (rgb2grey) {
			for (unsigned y = 0; y < sh; y++) {
				for (unsigned x = 0; x < sw; ++x)
					ptr[x*bpp] = offset[x*(bpp+2)+1]; // take green value, it doesn't matter
				if (bpp == 2)
					// with alpha
					for (unsigned x = 0; x < sw; ++x)
						ptr[x*2+1] = offset[x*4+3];
				offset += teximage->pitch;
				ptr += tw*bpp;
			}
		} else {
			for (unsigned y = 0; y < sh; y++) {
				memcpy(ptr, offset, sw*bpp);
				offset += teximage->pitch;
				ptr += tw*bpp;
			}
		}
	}
	SDL_UnlockSurface(teximage);
	init(data, makenormalmap, detailh);
}
	


void texture::init(const vector<Uint8>& data, bool makenormalmap, float detailh)
{
	// error checks.
	sys().myassert(mapping >= 0 && mapping < NR_OF_MAPPING_MODES, "illegal mapping mode!");
	sys().myassert(clamping >= 0 && clamping < NR_OF_CLAMPING_MODES, "illegal clamping mode!");

	// automatic resizing of textures if they're too large
	vector<Uint8> data2;
	const vector<Uint8>* pdata = &data;
	unsigned ms = get_max_size();
	if (width > ms || height > ms) {
		unsigned newwidth = width, newheight = height;
		unsigned xf = 1, yf = 1;
		if (width > ms) {
			newwidth = ms;
			xf = width/ms;
		}
		if (height > ms) {
			newheight = ms;
			yf = height/ms;
		}
		unsigned bpp = get_bpp();
		data2.resize(newwidth*newheight*bpp);
		unsigned area = xf*yf;
		for (unsigned y = 0; y < newheight; ++y) {
			for (unsigned x = 0; x < newwidth; ++x) {
				for (unsigned b = 0; b < bpp; ++b) {
					unsigned valsum = 0;
					for (unsigned yy = 0; yy < yf; ++yy) {
						for (unsigned xx = 0; xx < xf; ++xx) {
							unsigned ptr = ((y*yf+yy) * width +
									(x*xf+xx)) * bpp + b;
							valsum += unsigned(data[ptr]);
						}
					}
					valsum /= area;
					data2[(y*newwidth+x)*bpp + b] = Uint8(valsum);
				}
			}
		}
		pdata = &data2;
		gl_width = newwidth;
		gl_height = newheight;
	}

	glGenTextures(1, &opengl_name);
	glBindTexture(GL_TEXTURE_2D, opengl_name);

#ifdef MEMMEASURE
	unsigned add_mem_used = 0;
#endif

	if (makenormalmap && format == GL_LUMINANCE) {
		// make own mipmap building for normal maps here...
		// give increasing levels with decreasing w/h down to 1x1
		// e.g. 64x16 -> 32x8, 16x4, 8x2, 4x1, 2x1, 1x1
		format = GL_RGB;
		vector<Uint8> nmpix = make_normals(*pdata, gl_width, gl_height, detailh);
		glTexImage2D(GL_TEXTURE_2D, 0, format, gl_width, gl_height, 0, format,
			     GL_UNSIGNED_BYTE, &nmpix[0]);

#ifdef MEMMEASURE
		add_mem_used = gl_width * gl_height * get_bpp();
#endif

		if (do_mipmapping[mapping]) {
#if 1
			gluBuild2DMipmaps(GL_TEXTURE_2D, format, gl_width, gl_height,
					  format, GL_UNSIGNED_BYTE, &nmpix[0]);

#else
			// buggy version. gives white textures. maybe some mipmap levels
			// are missing so that gl complains by make white textures?
			// fixme: test it again after the mapping/clamping bugfix!

			// fixme: if we let GLU do the mipmap calculation, the result is wrong.
			// A filtered version of the normals is not the same as a normal map
			// of the filtered height field!
			// E.g. scaling down the map to 1x1 pixel gives a medium height of 128,
			// that is a flat plane with a normal of (0,0,1)
			// But filtering down the normals to one pixel could give
			// RGB=0.5 -> normal of (0,0,0) (rare...)!
			vector<Uint8> curlvl;
			const vector<Uint8>* gdat = pdata;
			for (unsigned level = 1, w = gl_width/2, h = gl_height/2;
			     w > 0 && h > 0; w /= 2, h /= 2) {
				cout << "level " << level << " w " << w << " h " << h << "\n";
				curlvl = scale_half(*gdat, w, h, 1);
				gdat = &curlvl;
				//fixme: must detailh also get halfed here? yes...
				vector<Uint8> nmpix = make_normals(*gdat, w, h, detailh);
				glTexImage2D(GL_TEXTURE_2D, level, GL_RGB, w, h, 0, GL_RGB,
					     GL_UNSIGNED_BYTE, &nmpix[0]);
				w /= 2;
				h /= 2;
			}
#endif
		}
	} else {
		// make gl texture
		glTexImage2D(GL_TEXTURE_2D, 0, format, gl_width, gl_height, 0, format,
			     GL_UNSIGNED_BYTE, &((*pdata)[0]));
#ifdef MEMMEASURE
		add_mem_used = gl_width * gl_height * get_bpp();
#endif
		if (do_mipmapping[mapping]) {
			// fixme: does this command set the base level, too?
			// i.e. are the two gl commands redundant?
			gluBuild2DMipmaps(GL_TEXTURE_2D, format, gl_width, gl_height,
					  format, GL_UNSIGNED_BYTE, &((*pdata)[0]));
		}
	}

#ifdef MEMMEASURE
	if (do_mipmapping[mapping])
		add_mem_used = (4*add_mem_used)/3;
	mem_used += add_mem_used;
	ostringstream oss; oss << "Allocated " << add_mem_used << " bytes of video memory for texture '" << texfilename << "', total video mem use " << mem_used/1024 << " kb";
	sys().add_console(oss.str());
	mem_alloced += add_mem_used;
	ostringstream oss2; oss2 << "Video mem usage " << mem_alloced << " vs " << mem_freed;
	sys().add_console(oss2.str());
#endif

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mapmodes[mapping]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magfilter[mapping]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, clampmodes[clamping]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, clampmodes[clamping]);
}




vector<Uint8> texture::scale_half(const vector<Uint8>& src, unsigned w, unsigned h, unsigned bpp)
{
	// fixme
	sys().myassert(w > 1 && (w & (w-1)) == 0, "texture width is no power of two!");
	sys().myassert(h > 1 && (h & (h-1)) == 0, "texture height is no power of two!");

	vector<Uint8> dst(w*h*bpp/4);
	unsigned ptr = 0;
	for (unsigned y = 0; y < h; y += 2) {
		for (unsigned x = 0; x < w; x += 2) {
			for (unsigned b = 0; b < bpp; ++b) {
				dst[ptr++] =
					Uint8((unsigned(src[(y*w+x)*bpp+b]) +
					       unsigned(src[(y*w+x+1)*bpp+b]) +
					       unsigned(src[((y+1)*w+x)*bpp+b]) +
					       unsigned(src[((y+1)*w+x+1)*bpp+b])) / 4);
			}
		}
	}
	return dst;
}
	


vector<Uint8> texture::make_normals(const vector<Uint8>& src, unsigned w, unsigned h,
				    float detailh)
{
	// src size must be w*h
	vector<Uint8> dst(3*w*h);
	float zh = 255.0f/detailh;
	unsigned ptr = 0;
	for (unsigned yy = 0; yy < h; ++yy) {
		unsigned y1 = (yy + h - 1) & (h - 1);
		unsigned y2 = (yy +     1) & (h - 1);
		for (unsigned xx = 0; xx < w; ++xx) {
			unsigned x1 = (xx + w - 1) & (w - 1);
			unsigned x2 = (xx +     1) & (w - 1);
			float hr = src[yy*w+x2];
			float hu = src[y1*w+xx];
			float hl = src[yy*w+x1];
			float hd = src[y2*w+xx];
			vector3f nm = vector3f(hl-hr, hd-hu, zh).normal();
			dst[ptr + 0] = Uint8(nm.x*127 + 128);
			dst[ptr + 1] = Uint8(nm.y*127 + 128);
			dst[ptr + 2] = Uint8(nm.z*127 + 128);
			ptr += 3;
		}
	}
	return dst;
}



texture::texture(const string& filename, mapping_mode mapping_, clamping_mode clamp,
		 bool makenormalmap, float detailh, bool rgb2grey)
{
	mapping = mapping_;
	clamping = clamp;
	texfilename = filename;
	SDL_Surface* teximage = IMG_Load(filename.c_str());
	sys().myassert(teximage != 0, string("texture: failed to load ")+filename);
	sdl_init(teximage, 0, 0, teximage->w, teximage->h, makenormalmap, detailh, rgb2grey);
	SDL_FreeSurface(teximage);
}	



texture::texture(SDL_Surface* teximage, unsigned sx, unsigned sy, unsigned sw, unsigned sh,
		 mapping_mode mapping_, clamping_mode clamp, bool makenormalmap, float detailh,
		 bool rgb2grey)
{
	mapping = mapping_;
	clamping = clamp;
	sdl_init(teximage, sx, sy, sw, sh, makenormalmap, detailh, rgb2grey);
}



texture::texture(const vector<Uint8>& pixels, unsigned w, unsigned h, int format_,
		 mapping_mode mapping_, clamping_mode clamp, bool makenormalmap, float detailh)
{
	mapping = mapping_;
	clamping = clamp;
	
	sys().myassert(w > 0 && (w & (w-1)) == 0, "texture width is no power of two!");
	sys().myassert(h > 0 && (h & (h-1)) == 0, "texture height is no power of two!");

	width = gl_width = w;
	height = gl_height = h;
	format = format_;

	init(pixels, makenormalmap, detailh);
}



texture::~texture()
{
#ifdef MEMMEASURE
	unsigned sub_mem_used = gl_width * gl_height * get_bpp();
	if (do_mipmapping[mapping])
		sub_mem_used = (4*sub_mem_used)/3;
	mem_used -= sub_mem_used;
	ostringstream oss; oss << "Freed " << sub_mem_used << " bytes of video memory for texture '" << texfilename << "', total video mem use " << mem_used/1024 << " kb";
	sys().add_console(oss.str());
	mem_freed += sub_mem_used;
	ostringstream oss2; oss2 << "Video mem usage " << mem_alloced << " vs " << mem_freed;
	sys().add_console(oss2.str());
#endif
	glDeleteTextures(1, &opengl_name);
}



unsigned texture::get_bpp() const
{
	switch (format) {
		case GL_RGB: return 3;
		case GL_RGBA: return 4;
		case GL_LUMINANCE: return 1;
		case GL_LUMINANCE_ALPHA: return 2;
		default:
			ostringstream oss; oss << "unknown texture format " << format << "\n";
			sys().myassert(false, oss.str());
	}
	return 4;
}

void texture::set_gl_texture() const
{
	glBindTexture(GL_TEXTURE_2D, get_opengl_name());
}

// draw_image
void texture::draw(int x, int y) const
{
	draw(x, y, width, height);
}

void texture::draw_hm(int x, int y) const
{
	draw_hm(x, y, width, height);
}

void texture::draw_vm(int x, int y) const
{
	draw_vm(x, y, width, height);
}

void texture::draw(int x, int y, int w, int h) const
{
	float u = float(width)/gl_width;
	float v = float(height)/gl_height;
	glBindTexture(GL_TEXTURE_2D, get_opengl_name());
	glBegin(GL_QUADS);
	glTexCoord2f(0,0);
	glVertex2i(x,y);
	glTexCoord2f(0,v);
	glVertex2i(x,y+h);
	glTexCoord2f(u,v);
	glVertex2i(x+w,y+h);
	glTexCoord2f(u,0);
	glVertex2i(x+w,y);
	glEnd();
}

void texture::draw_hm(int x, int y, int w, int h) const
{
	float u = float(width)/gl_width;
	float v = float(height)/gl_height;
	glBindTexture(GL_TEXTURE_2D, get_opengl_name());
	glBegin(GL_QUADS);
	glTexCoord2f(u,0);
	glVertex2i(x,y);
	glTexCoord2f(u,v);
	glVertex2i(x,y+h);
	glTexCoord2f(0,v);
	glVertex2i(x+w,y+h);
	glTexCoord2f(0,0);
	glVertex2i(x+w,y);
	glEnd();
}

void texture::draw_vm(int x, int y, int w, int h) const
{
	float u = float(width)/gl_width;
	float v = float(height)/gl_height;
	glBindTexture(GL_TEXTURE_2D, get_opengl_name());
	glBegin(GL_QUADS);
	glTexCoord2f(0,v);
	glVertex2i(x,y);
	glTexCoord2f(0,0);
	glVertex2i(x,y+h);
	glTexCoord2f(u,0);
	glVertex2i(x+w,y+h);
	glTexCoord2f(u,v);
	glVertex2i(x+w,y);
	glEnd();
}

void texture::draw_rot(int x, int y, double angle) const
{
	draw_rot(x, y, angle, get_width()/2, get_height()/2);
}

void texture::draw_rot(int x, int y, double angle, int tx, int ty) const
{
	glPushMatrix();
	glTranslatef(x, y, 0);
	glRotatef(angle, 0, 0, 1);
	draw(-tx, -ty);
	glPopMatrix();
}

void texture::draw_tiles(int x, int y, int w, int h) const
{
	float tilesx = float(w)/gl_width;
	float tilesy = float(h)/gl_height;
	glBindTexture(GL_TEXTURE_2D, get_opengl_name());
	glBegin(GL_QUADS);
	glTexCoord2f(0,0);
	glVertex2i(x,y);
	glTexCoord2f(0,tilesy);
	glVertex2i(x,y+h);
	glTexCoord2f(tilesx,tilesy);
	glVertex2i(x+w,y+h);
	glTexCoord2f(tilesx,0);
	glVertex2i(x+w,y);
	glEnd();
}

void texture::draw_subimage(int x, int y, int w, int h, unsigned tx, unsigned ty,
	unsigned tw, unsigned th) const
{
	float x1 = float(tx)/gl_width;
	float y1 = float(ty)/gl_height;
	float x2 = float(tx+tw)/gl_width;
	float y2 = float(ty+th)/gl_height;
	glBindTexture(GL_TEXTURE_2D, get_opengl_name());
	glBegin(GL_QUADS);
	glTexCoord2f(x1,y1);
	glVertex2i(x,y);
	glTexCoord2f(x1,y2);
	glVertex2i(x,y+h);
	glTexCoord2f(x2,y2);
	glVertex2i(x+w,y+h);
	glTexCoord2f(x2,y1);
	glVertex2i(x+w,y);
	glEnd();
}

unsigned texture::get_max_size()
{
	GLint i;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &i);
	return i;
}



GLuint texture::create_shader(GLenum type, const string& filename, const list<string>& defines)
{
	GLuint nr;

	glGenProgramsARB(1, &nr);
	glBindProgramARB(type, nr);
	ifstream ifprg(filename.c_str(), ios::in);
	sys().myassert(!ifprg.fail(), string("failed to open ")+filename);

	// read lines.
	vector<string> lines;
	while (!ifprg.eof()) {
		string s;
		getline(ifprg, s);
		lines.push_back(s);
	}

	// build shader node tree by parsing.
	unsigned current_line = 0;
	shader_node_program* snp = new shader_node_program(lines, current_line);

	// compile to destination program
	vector<string> finalprogram;
	snp->compile(finalprogram, defines, false);
	delete snp;

	// debug output
//	for (unsigned i = 0; i < finalprogram.size(); ++i)
//		cout << "line " << i << ": '" << finalprogram[i] << "'\n";

	string prg;
	for (unsigned i = 0; i < finalprogram.size(); ++i)
		prg += finalprogram[i] + "\n";
	glProgramStringARB(type, GL_PROGRAM_FORMAT_ASCII_ARB,
			   prg.size(), prg.c_str());

	// error handling
	int errorpos;

	glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &errorpos);
	if (errorpos != -1) {
		// get error string
		string errstr;
		const char* errstr2 = (const char*)glGetString(GL_PROGRAM_ERROR_STRING_ARB);
		if (errstr2 != 0)
			errstr = errstr2;
		ostringstream oss;
		oss << "GL shader program error (program type " << type
		    << ") at position " << errorpos << " in file "
		    << filename << ", error string is '" << errstr << "'\n";
		sys().myassert(false, oss.str());
	}

	GLint undernativelimits;
	glGetProgramivARB(type, GL_PROGRAM_UNDER_NATIVE_LIMITS_ARB, &undernativelimits);
	if (undernativelimits == 0) {
		ostringstream oss;
		oss << "GL shader program exceeds native limits (program type "
		    << type << "), filename "
		    << filename << "\n";
		sys().myassert(false, oss.str());
	}
	
	return nr;
}



void texture::delete_shader(GLuint nr)
{
	glDeleteProgramsARB(1, &nr);
}



texture::shader_node::state texture::shader_node::get_state(const vector<string>& lines, unsigned& current_line)
{
	if (current_line >= lines.size()) return eof;
	string ln = lines[current_line].substr(0, 6);
	if (ln == "#ifdef") return ifdef;
	if (ln == "#else ") return elsedef;
	if (ln == "#endif") return endifdef;
	return code;
}

void texture::shader_node_code::compile(vector<string>& dstprg, const list<string>& /*defines*/,
					bool disabled)
{
	for (vector<string>::iterator it = lines.begin(); it != lines.end(); ++it)
		dstprg.push_back(disabled ? string("#") + *it : *it);
}

texture::shader_node_code::shader_node_code(const vector<string>& lines, unsigned& current_line)
{
	while (true) {
		state st = get_state(lines, current_line);
		if (st == code) {
			this->lines.push_back(lines[current_line++]);
		} else {
			return;
		}
	}
}

texture::shader_node_program::~shader_node_program()
{
	for (list<shader_node*>::iterator it = subnodes.begin(); it != subnodes.end(); ++it) delete *it;
}

void texture::shader_node_program::compile(vector<string>& dstprg, const list<string>& defines,
					   bool disabled)
{
	for (list<shader_node*>::iterator it = subnodes.begin(); it != subnodes.end(); ++it)
		(*it)->compile(dstprg, defines, disabled);
}

texture::shader_node_program::shader_node_program(const vector<string>& lines, unsigned& current_line, bool endonelse, bool endonendif)
{
	while (current_line < lines.size()) {
		subnodes.push_back(new shader_node_code(lines, current_line));
		switch (shader_node::get_state(lines, current_line)) {
		case shader_node::eof:
			return;
		case shader_node::ifdef:
			subnodes.push_back(new shader_node_ifelse(lines, current_line));
			break;
		case shader_node::elsedef:
			sys().myassert(endonelse, "parsed #else without #if in shader!");
			return;
		case shader_node::endifdef:
			sys().myassert(endonendif, "parsed #endif without #if in shader!");
			return;
		case shader_node::code:
			sys().myassert(false, "internal error. state is 'code' after end of code parsing");
		}
	}
}

texture::shader_node_ifelse::shader_node_ifelse(const vector<string>& lines, unsigned& current_line) : ifnode(0), elsenode(0)
{
	define = lines[current_line].substr(7);
	++current_line;	// skip #ifdef
	ifnode = new shader_node_program(lines, current_line, true, true);
	state st = get_state(lines, current_line);
	if (st == shader_node::elsedef) {
		++current_line; // skip #else
		elsenode = new shader_node_program(lines, current_line, false, true);
		st = get_state(lines, current_line);
	}
	sys().myassert(st == shader_node::endifdef, "found no #endif after #else or #ifdef in shader program");
	++current_line;	// skip #endif
}

void texture::shader_node_ifelse::compile(vector<string>& dstprg, const list<string>& defines,
					  bool disabled)
{
	dstprg.push_back(string("#ifdef ")+define);
	bool doif = false;
	for (list<string>::const_iterator it = defines.begin(); it != defines.end(); ++it) {
		if (*it == define) {
			doif = true;
			break;
		}
	}
	ifnode->compile(dstprg, defines, disabled || !doif);
	if (elsenode) {
		dstprg.push_back(string("#else"));
		elsenode->compile(dstprg, defines, disabled || doif);
	}
	dstprg.push_back(string("#endif"));
}
