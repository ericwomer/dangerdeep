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
#include "ship.h"
#include "ocean_wave_generator.h"
#include "perlinnoise.h"

class water
{
protected:
	double mytime;			// store global time in seconds
	unsigned xres, yres;		// resolution of grid

	const unsigned wave_phases;	// >= 256 is a must
	const float wavetile_length;	// >= 512m makes wave look MUCH more realistic
	const float wavetile_length_rcp;// reciprocal of former value
	const double wave_tidecycle_time;	// depends on fps. with 25fps and 256 phases, use ~10seconds.

	vector<unsigned> gridindices;
	vector<unsigned> gridindices2;//only used for test grid drawing, could be ifdef'ed away

	auto_ptr<texture> reflectiontex;
	auto_ptr<texture> foamtex;
	auto_ptr<texture> foamamounttex;
	auto_ptr<texture> foamamounttrail;
	auto_ptr<texture> foamperimetertex;
	auto_ptr<texture> fresnelcolortex;	// texture for fresnel values and water color

	auto_ptr<texture> waterspecularlookup;	// lookup 1d texture map for water specular term

	vector<Uint8> fresnelcolortexd;	// stored for updates of water color

	float last_light_brightness;	// used to determine when new refraction color tex must get computed

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
		// store displacements and height in one 32bit-word:
		// 10bits y, 10bits x, 12bits height.
		// stored as unsigned values with offset 512/512/2048,
		// represent -8...+8m or -16...+16m.
		vector<Uint32> data;
		float minh, maxh;
		wavetile_phase() : minh(0), maxh(0) {}
		float get_height(unsigned idx) const
		{
			return (int(data[idx] & 0xfff) - 2048) * (16.0f/2048);
		}
		// be careful... when conversion to uint rounds up, result of 2048 can occour...
		// we mask unused bits out, although it is not really necessary (rather paranoia)
		vector3f get_height_and_displacement(unsigned idx) const
		{
			Uint32 v = data[idx];
			return vector3f((int((v >> 22) & 0x3ff) - 512) * (8.0f/512),
					(int((v >> 12) & 0x3ff) - 512) * (8.0f/512),
					(int(v & 0xfff) - 2048) * (16.0f/2048));
		}
		void set_values(unsigned idx, float dx, float dy, float h)
		{
			data[idx] =
				(Uint32(Sint32(h * (2048/16)) + 2048) & 0xfff) |
				((Uint32(Sint32(dx * (512/8)) + 512) & 0x3ff) << 12) |
				((Uint32(Sint32(dy * (512/8)) + 512) & 0x3ff) << 22);
		}
	};

	// wave tile data
	vector<wavetile_phase> wavetile_data;
	const wavetile_phase* curr_wtp;	// pointer to current phase

	// Arrays used while drawing a tile. To avoid repeated re-alloc, they're here
	mutable vector<vector3f> coords;
	mutable vector<vector3f> uv1;
	mutable vector<vector3f> normals;
	mutable vector<vector2f> uv0;

	// test
	ocean_wave_generator<float> owg;

	// sub detail
	vector<Uint8> waveheight_subdetail;	// same as water bump map data, recomputed periodically

	// testing: with fragment programs we need some sub-noise
	auto_ptr<texture> water_bumpmap;

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

	// for subdetail
	perlinnoise png;

	// times for generation
	double last_subdetail_gen_time;

	water& operator= (const water& other);
	water(const water& other);

	void setup_textures(const matrix4& reflection_projmvmat, const vector2f& transl) const;
	void cleanup_textures(void) const;

	vector3f compute_coord(const vector3f& xyzpos, const vector2f& transl) const;
	vector3f get_wave_normal_at(unsigned x, unsigned y) const;

#ifdef USE_SSE
	void compute_coord_1line_sse(const vector2f& v, const vector2f& vadd, const vector2f& transl) const;
#endif

	void generate_wavetile(double tiletime, wavetile_phase& wtp);
	void generate_subdetail_and_bumpmap();

public:
	water(unsigned xres_, unsigned yres_, double tm = 0.0);	// give day time in seconds
	void set_time(double tm);
	~water();
	void update_foam(double deltat);		// needed for dynamic foam
	void spawn_foam(const vector2& pos);		// dito

	void draw_foam_for_ship(const game& gm, const ship* shp, const vector3& viewpos) const;
	void compute_amount_of_foam_texture(const game& gm, const vector3& viewpos,
					    const vector<ship*>& allships) const;

	// give absolute position of viewer as viewpos, but modelview matrix without translational component!
	void display(const vector3& viewpos, angle dir, double max_view_dist) const;
	float get_height(const vector2& pos) const;
	// give f as multiplier for difference to (0,0,1)
	vector3f get_normal(const vector2& pos, double f = 1.0) const;
	const texture* get_reflectiontex(void) const { return reflectiontex.get(); }
	static float exact_fresnel(float x);
	void set_refraction_color(float light_brightness);
};

#endif
