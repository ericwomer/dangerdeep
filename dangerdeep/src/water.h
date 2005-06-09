// (ocean) water simulation and display (OpenGL)
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef WATER_H
#define WATER_H

/*
	This class simulates and displays the water.
	Wave with animation, Fresnel etc.
*/

#include <vector>
#include <memory>
using namespace std;
#include "color.h"
#include "angle.h"
#include "vector3.h"
#include "texture.h"

class water
{
protected:
	double mytime;			// store global time in seconds
	unsigned xres, yres;		// resolution of grid

	// precomputed data. Outer array for phases
	//projgrid: per vertex: height,displacement,normal
	//values must be stored in mipmap fashion, linear interpolation of heights/displacements
	//is ok, interpolation of normals requires one sqrt per vertex. alternatively we could
	//recompute the normals per vertex (also one sqrt), this needs less memory and more
	//computations.
	//mipmap: we need 4/3 of space, 1+4+16+64+256+...n = ~ 4/3 n
	// another option: don't store fft values but recompute them every
	// frame (slower, but more dynamically, and using less memory)
	// for a 64x64 grid in 256 phases with mipmaps and normals we have
	// 256*64*64*(6*4)*4/3 = 1M*24*4/3 = 32M
	vector<vector<vector2f> > wavetiledisplacements;
	vector<vector<float> > wavetileheights;	//fixme store heights as uint8 (resolution is high enough, 75% space save)

	vector<unsigned> gridindices;
	vector<unsigned> gridindices2;//only used for test grid drawing, could be ifdef'ed away

	//projgrid: mipmaps too
	vector<vector<float> > wavetilefoam;

	// minimum and maximum heights of water (over all values)
	float minh, maxh;
	
	unsigned reflectiontexsize;
	unsigned reflectiontex;//fixme better handle that with class texture
	auto_ptr<texture> foamtex;
	auto_ptr<texture> fresnelcolortex;	// texture for fresnel values and water color

	auto_ptr<texture> waterspecularlookup;	// lookup 1d texture map for water specular term

	vector<Uint8> fresnelcolortexd;	// stored for updates of water color

	float last_light_brightness;	// used to determine when new refraction color tex must get computed

	// Arrays used while drawing a tile. To avoid repeated re-alloc, they're here
	mutable vector<vector3f> coords;
	mutable vector<vector3f> uv1;
	mutable vector<vector3f> normals;
	mutable vector<vector2f> uv0;

	// sub detail
	vector<vector<Uint8> > waveheight_subdetail;	// same as water bump map data

	// testing: with fragment programs we need some sub-noise
	vector<texture*> water_bumpmaps;

#if 0		// old code, kept for reference, especially for foam
	// waves are stored in display lists to speed up drawing.
	// this increases fps > 100% compared to vertex arrays / glDrawElements
	// the display lists can take MUCH ram!
	unsigned wavedisplaylists;		// # of first display list
	vector<vector<float> > wavetileh;	// wave tile heights (generated)
	vector<vector<vector3f> > wavetilen;	// wave tile normals (generated)
	vector<float> wavefoam;			// 2d array with foam values (0-1), maybe use fixed point integer here
	// display lists are const, but we need dynamic data for foam. So we store foam in a texture and update this
	// texture each frame or each 1/10th second. The texture has a texel for each wave vertex and each tile,
	// thus it is #tiles*#vertices_per_tile wide and high. We use an color table indexed texture, so
	// we have to transfer (32*8)^2=64k or (64*16)^2=256k per frame, far less memory than updating geometry data
	// each frame (like with vertex arrays).
	// alternative foam generation: clear texture every frame (or 1/10th second), draw lines from ship trails
	// into the texture (if they're inside) and use this texture
	vector<Uint8> wavefoamtexdata;
	unsigned wavefoamtex;
#endif

	// Booleans for supported OpenGL extensions
	bool vertex_program_supported;
	bool fragment_program_supported;
	bool compiled_vertex_arrays_supported;

	// Config options (only used when supported)
	bool use_shaders;

	// Shader programs
	GLuint water_vertex_program;
	GLuint water_fragment_program;

	water& operator= (const water& other);
	water(const water& other);

	void setup_textures(const matrix4& reflection_projmvmat, const vector2f& transl) const;
	void cleanup_textures(void) const;

	vector3f compute_coord(int phase, const vector3f& xyzpos, const vector2f& transl) const;
	vector3f get_wave_normal_at(unsigned wavephase, unsigned x, unsigned y) const;

public:
	water(unsigned xres_, unsigned yres_, double tm = 0.0);	// give day time in seconds
	void set_time(double tm);
	~water();
	void update_foam(double deltat);		// needed for dynamic foam
	void spawn_foam(const vector2& pos);		// dito
	// give absolute position of viewer as viewpos, translation in modelview matrix included!
	void display(const vector3& viewpos, angle dir, double max_view_dist, const matrix4& reflection_projmvmat) const;
	float get_height(const vector2& pos) const;
	// give f as multiplier for difference to (0,0,1)
	vector3f get_normal(const vector2& pos, double f = 1.0) const;
	unsigned get_reflectiontex(void) const { return reflectiontex; }
	unsigned get_reflectiontex_size(void) const { return reflectiontexsize; }
	static float exact_fresnel(float x);
	void set_refraction_color(float light_brightness);
};

#endif
