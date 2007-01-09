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
#include "color.h"
#include "angle.h"
#include "vector3.h"
#include "texture.h"
#include "ship.h"
#include "ocean_wave_generator.h"
#include "perlinnoise.h"
#include "vertexbufferobject.h"
#include "framebufferobject.h"
#include "shader.h"

///\brief Rendering of ocean water surfaces.
class water
{
protected:
	double mytime;			// store global time in seconds
	unsigned xres, yres;		// resolution of grid

	const unsigned wave_phases;	// >= 256 is a must
	const float wavetile_length;	// >= 512m makes wave look MUCH more realistic
	const float wavetile_length_rcp;// reciprocal of former value
	const double wave_tidecycle_time;	// depends on fps. with 25fps and 256 phases, use ~10seconds.

	std::vector<unsigned> gridindices;
	std::vector<unsigned> gridindices2;//only used for test grid drawing, could be ifdef'ed away

	std::auto_ptr<texture> reflectiontex;
	std::auto_ptr<texture> foamtex;
	std::auto_ptr<texture> foamamounttex;
	std::auto_ptr<texture> foamamounttrail;
	std::auto_ptr<texture> foamperimetertex;
	std::auto_ptr<texture> fresnelcolortex;	// texture for fresnel values and water color

	std::auto_ptr<framebufferobject> reflectiontex_fbo;
	std::auto_ptr<framebufferobject> foamamounttex_fbo;

	std::auto_ptr<texture> waterspecularlookup;	// lookup 1d texture map for water specular term

	std::vector<Uint8> fresnelcolortexd;	// stored for updates of water color

	colorf last_light_color;	// used to determine when new refraction color tex must get computed

	const unsigned wave_resolution;	// fft resolution for water tile (use 64+, better 128+)
	const unsigned wave_resolution_shift;	// the log2 of wave_resolution

	const bool wave_subdetail;	// use subdetail for height or not
	const unsigned subdetail_size;	// sub detail resolution
	const unsigned subdetail_size_shift;	// the log2 of it

	// about wavetile data:
	// 32 bit per coordinate, quantified integer storage. 12 bit for height, 10 for x/y displacement.
	// max. height amplitude ~32meters, so multiply with 32/4096=128, resolution ~8cm, enough.
	// max. displ. amplitude ~16meters, so multiply with 16/1024=64, resolution ~12cm etc.
	// fixme: measure quantifier values or make them dynamic (more time/ram needed while generation!)
	// We can then use 256x256 sized tiles with 256 phases. They take 64mb of ram (2^(3*8)*4=2^18=64m)
	// currently displacement +- 5m, height +- 3.5m. currently used +-16m for height,+-8m for displ.

	struct wavetile_phase
	{
		std::vector<vector3f> data;
		float minh, maxh;
		wavetile_phase() : minh(0), maxh(0) {}
		float get_height(unsigned idx) const
		{
			return data[idx].z;
		}
		const vector3f& get_height_and_displacement(unsigned idx) const
		{
			return data[idx];
		}
		void set_values(unsigned idx, float dx, float dy, float h)
		{
			data[idx] = vector3f(dx, dy, h);
		}
	};

	// wave tile data
	std::vector<wavetile_phase> wavetile_data;
	const wavetile_phase* curr_wtp;	// pointer to current phase

	// Arrays used while drawing a tile. To avoid repeated re-alloc, they're here
	mutable std::vector<vector3f> coords;
	mutable std::vector<vector3f> uv1;
	mutable std::vector<vector3f> normals;
	mutable std::vector<vector2f> uv0;

	// test
	ocean_wave_generator<float> owg;

	// sub detail
	std::vector<Uint8> waveheight_subdetail;	// same as water bump map data, recomputed periodically

	// testing: with fragment programs we need some sub-noise
	std::auto_ptr<texture> water_bumpmap;

#if 0		// old code, kept for reference, especially for foam
	// waves are stored in display lists to speed up drawing.
	// this increases fps > 100% compared to vertex arrays / glDrawElements
	// the display lists can take MUCH ram!
	unsigned wavedisplaylists;		// # of first display list
	std::vector<std::vector<float> > wavetileh;	// wave tile heights (generated)
	std::vector<std::vector<vector3f> > wavetilen;	// wave tile normals (generated)
	std::vector<float> wavefoam;			// 2d array with foam values (0-1), maybe use fixed point integer here
	// display lists are const, but we need dynamic data for foam. So we store foam in a texture and update this
	// texture each frame or each 1/10th second. The texture has a texel for each wave vertex and each tile,
	// thus it is #tiles*#vertices_per_tile wide and high. We use an color table indexed texture, so
	// we have to transfer (32*8)^2=64k or (64*16)^2=256k per frame, far less memory than updating geometry data
	// each frame (like with vertex arrays).
	// alternative foam generation: clear texture every frame (or 1/10th second), draw lines from ship trails
	// into the texture (if they're inside) and use this texture
	std::vector<Uint8> wavefoamtexdata;
	unsigned wavefoamtex;
#endif

	// Config options (only used when supported)
	bool use_shaders;

	// Shader program
	std::auto_ptr<glsl_shader_setup> glsl_water;

	// for subdetail
	perlinnoise png;

	// times for generation
	double last_subdetail_gen_time;

	// store index data as vertex buffer object
	vertexbufferobject vbo_indices;

	// store texture data in vertex buffer objects as well
	// we can't store coords in VBOs, as we read them for normal generation, so
	// read would be inefficient. Same for normals.
	// We must use at most one vertex buffer for all data... we can bind at most
	// one vertex buffer and one index buffer
	mutable vertexbufferobject vbo_uv01;

	water& operator= (const water& other);
	water(const water& other);

	void setup_textures(const matrix4& reflection_projmvmat, const vector2f& transl) const;
	void cleanup_textures() const;

	vector3f compute_coord(vector2f& xyfrac) const;
	vector3f get_wave_normal_at(unsigned x, unsigned y) const;

#ifdef USE_SSE
	bool usex86sse;
	void compute_coord_1line_sse(const vector2f& v, const vector2f& vadd, const vector2f& transl) const;
#endif

	void generate_wavetile(double tiletime, wavetile_phase& wtp);
	void generate_subdetail_and_bumpmap();

public:
	water(unsigned xres_, unsigned yres_, double tm = 0.0);	// give day time in seconds
	void set_time(double tm);
	~water();

	void draw_foam_for_ship(const game& gm, const ship* shp, const vector3& viewpos) const;
	void compute_amount_of_foam_texture(const game& gm, const vector3& viewpos,
					    const std::vector<ship*>& allships) const;

	// give absolute position of viewer as viewpos, but modelview matrix without translational component!
	void display(const vector3& viewpos, angle dir, double max_view_dist) const;
	float get_height(const vector2& pos) const;
	// give f as multiplier for difference to (0,0,1)
	vector3f get_normal(const vector2& pos, double f = 1.0) const;
	static float exact_fresnel(float x);
	void set_refraction_color(const colorf& light_color);

	/// prepare reflection texture for mirror drawing
	void refltex_render_bind() const;

	/// finish mirror drawing
	void refltex_render_unbind() const;
};

#endif
