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

// main program
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "oglext/OglExt.h"
#include <glu.h>
#include <SDL.h>
#include <SDL_net.h>

#include "system.h"
#include "vector3.h"
#include "model.h"
#include "texture.h"
#include <iostream>
#include <sstream>
#include "image.h"
#include "faulthandler.h"
#include "datadirs.h"
#include "frustum.h"
#include "shader.h"
#include "font.h"
#include "fpsmeasure.h"
#include "log.h"
#include "mymain.cpp"

#include <time.h>









// ------------------------------------------- code ---------------------------------------

/*
  geoclipmap rendering

  we only need a function that tells us the height for any coordinate in
  the (x,y) plane (with some parameter for the detail). With that
  information we could render any kind of terrain, no matter if the height
  is synthesized or taken from (compressed) stored data.

  We have either one VBO for vertices of all levels or one VBO per
  level. The vertices on the border between two levels aren't shared
  except for the zero area triangles used to make the mesh watertight.
  But we can render the zero area triangles only with vertex data from
  the inner level.
  Having multiple VBOs could be better for updating, we don't need to map
  the whole VBO, which interferes with rendering. Inner levels could be
  updated while the card renders outer levels or similar
  situations.

  However we would need some extra space to render the connecting
  triangles, which could be done by reserving 4*N/2 space at end of each
  VBO and storing there the heights of the next inner level vertices on
  the edge, but this is clumsy as well. But this is not true! We don't
  need that data, as the zero area triangles that connect the levels can
  be rendered with data from inner level only when the transition vertex
  shader is used. For the vertices on that border z=z_c because of
  transition, so the height of every vertex on the border is the same as
  in the next coarser level.

  It is easier to compute all vertex data for a level (thus N*N vertices)
  and to not leave the center gap out. The gap in the center is N*N/4
  vertices in size and is filled with the next finer level. The overhead
  is small and we can reuse the computed height data (see below), plus we
  can easily move the gap region around without reloading more vertex data
  to the GPU.

  The VBO holds vertex data with coordinate wrap around. Define distance
  between two values at innermost level to be d, resolution of a level is
  NxN always, the area is thus 2^l * d * (N-1) in square. l is the level
  number starting at 0 for the innermost level. Thus with real coordinate
  r=(rx,ry) we have r/(2^l * d) = s, so s is level-relative coordinate,
  and s%N is coordinate in the VBO. We define that there is height info at
  (0,0) exactly.

  What is N? We could define N to be a power of two and for every level we
  render N*N triangle pairs, so we have (N+1)*(N+1) vertices per
  level. This fits best to all computations. Its more a must have!
  Because the number of quads on the outside must be even, since the next
  level has half resolution. Thus we have N*N quads per leven and thus
  (N+1)*(N+1) vertices, which makes the mod operation a bit more expensive.

  Updating L-shaped regions of vertex data when the viewer moves is
  problematic, that is for updating columns of vertices. This can't be
  represented well by VBO commands. We would need one glBufferSubData call
  per row, which is costly. The only alternative would be to map the
  buffer, but this could interfere with drawing. And we can't map parts of
  the data only. Mapping seems best alternative though. Some sites state
  that mapping is so costly that it should be used for updates of more
  than 32kb of data only.

  The indices must be recomputed every frame. An obvious optimization is
  to remember the clip area of the last frame, and to not recompute them
  if that area didn't change. We could either draw the triangles directly
  with data from host memory or copy the data to an index VBO and render
  from there. The latter means extra overhead because of mapping and
  copying, but decouples CPU and GPU and may be faster (test showed that
  this can be indeed the case). It would be good also to reuse the data
  between frames when it hasn't changed.

  For each vertex we need to know the height of it in the next coarser
  level. This can be trivially computed by requesting the height function
  with a parameter for the detail one less than that of the current level.
  The only problem is that every second vertex in x and y direction (thus
  3/4 of all vertices) fall between vertices of coarser level. Its height
  can be computed by linear combination of the neighbouring 2 or 4
  vertices. Compute missing heights in rows with odd numbers first by
  mixing vertex height of previous and next line 50%/50%, then compute
  missing heights in all lines by mixing heights of previous/next vertex
  50%/50%. The data can be looked up in the height data of the next
  coarser level, if that is not only stored in the GPU but also the CPU
  (system memory). Storing the data also in system memory just for that
  lookup is wasteful on the other hand. We need the height data only for
  updating vertex heights, which is done at a low rate per frame (most
  vertex data is constant). So calling the terrain height function twice
  per newly created vertex should be acceptable, and this eats less memory
  and doesn't trash the cache.

  We have to test if linear interpolation instead of cosine interpolation
  for perlin noise generation would look good enough, because the height
  values are transformed by the terrace function later anyway.
  Linear interpolation is much faster.

  The resolution ("N") needs to be high enough or level transitions are
  visible. Since height interpolation can be done only for vertices
  they need to be close enough to each other to make the interpolation
  look ok. N = 2^8 looks good, but eats much performance.
  With N=2^7 it looks good in the near, but you can see popping in the
  distance. This may be fixed by correct height interpolation of z_c
  (bilinear)

  fixme todo:
  - move geoclipmap part to standalone test program like portal!
  - viewpos change is too small compared to last rendered viewpos, do nothing, else
    render and update last rendered viewpos
  - check for combining updates to texture/VBO (at least texture possible) PERFORMANCE
  - write good height generator
  - do not render too small detail (start at min_level, but test that this works)
  - compute how many tris per second are rendered as performance measure
  - interpolation between levels is sometimes visible (on sharp high mountain ridges),
    there heights seem to jump. It seems that interpolation area doesnt reach 1.0 on
    outer edge always, but close to. But the jumps are worse - reason unknown.

  done:
  - render triangles from outmost patch to horizon
    problem later when rendering coast, to the sea horizon z-value must be < 0,
    to the land > 0. maybe stretch xy coordinates of last level beyond viewing range,
    that would solve both problems, but the height values must get computed accordingly then
  - store normals with double resolution (request higher level from height gen.)
    for extra detail, or maybe even ask height generator for normals directly.
  - render T-junction triangles
  - don't waste space for index VBO, compute correct max. space
  - vertex shader for height interpolation
  - normal computation (or do it via texture)
    compute 2 vertices more in xy direction when computing an update
    then do simple normal generation.
    normals in textures would be much easier, but the height generator
    needs to compute them then.
  - a idiv takes 40 cycles! even worse, -33 % 34 = 33 and not 1 as we need
    it to be... so "% (N+1)" doesnt work as expected.
    it would be VERY important to make N+1 a power of two!
    just render N-2 triangles then, should work too...
  - efficient update of VBO data
    --> more performance
  - clipping of patches (index-patch) against viewing frustum
    -- patches could become empty
    generate polygon in z=0 plane for the four rectangles of each level to be updated
    clip polygon(s) by camera viewing frustum.
    compute min/max x/y values of resulting polygons (polygons could be empty)
    snap x/y values to next grid positions (min->floor, max->ceil), so that resulting
    area embraces the polygon, but is minimal.
    generate indices for the resulting area.
    DONE - works, gives nearly double frame rate. but the clipping is too restrictive,
    triangles in the near are clipped, we should at max height in all directions
    of the clip area to compensate this, or similar tricks.
    problem: giving extra 100m kills fps (-15), and still isnt enough.
    It depends on viewer height also, and the extra-clip-space depends on level!
    viewerpos-xy should be an additional min/max in the clipping, but then
    clipping is less effective?
  - compute z_c and n_c correctly
    linear interpol. of z of coarser level!
    can be done by requesting patch of 1/2 size of coarser level, then upscaling
    coordinates by hand in the scratchbuffer
    same for normals! costly...
    normals should be placed in textures, then n_c is computed correctly by the
    GPU already (linear interpolation of the values)
    what do we need? one texture RGB per level, with res N*N (again essential that
    N is a power of 2) that would be 256x256x3x7=1300k, no problem, small memory
    if n is just stored to a texture when it is computed, we get n_c without extra
    cost, we only need to store and update one texture per level and need to enhance
    the shader, and need to store n in the texture.
    we can easily give the mix factor alpha from the vshader to the fshader
    so it is all a matter of texture updates.
    to find the up to 4 update areas:
    update area mod N, if tr.xy < bl.xy then horiz/vert there are 2 areas (< not <= )
    the width is then tr.xy + 1 for the second area, N - bl.xy for the first (N=2^n)
    so max coordinates for bl/tr are N-1.
  - texcoord computation is not fully correct in shader
    NOW works much better, but check for exact vertex<->pixel mapping!
  - generate tri-strips with better post-transform-cache-use
    (columns of 16-32 triangles) PERFORMANCE
  - current implementation is CPU limited! profile it DONE, slightly optimized
  - check if map vs. bufferdata/buffersubdata would be faster
  - the terrace-like look is no bug but is caused by the low resolution of height maps
    (8 bit per height)


    triangle count: N*N*2*3/4 per level plus N*N*2*1/4 for inner level
    N*N/2 * (3*level+1)

    height interpolation with subdivision interpolator!

    finer levels with height generation by noise!
    generate height from outer levels...
    that is to generate heights of level x, we could generate heights of level x-1 at half res +1
    scale them up (maybe with subdivision interpolator) and add the level x detail.
    we would need to store current heights of all levels then (scratchbuf per level),
    but it could be easier and save computations.
    7 levels with 2^8 res each give 458752 samples, with 10*4 bytes each: 18.3mb ca, that is ok
    some extra for overlap (N+1 or N+3) but ca. 20mb.




    About terrain synthesis:
    
    final height data is built recursivly from coarsest level to finest level.
    Each value of level x is build of surrounding values of level x+1 using
    smooth surface interpolation with factors -1/16 9/16 9/16 -1/16.
    In 2d this gives 16 values of level x+1 to generate one value of level x
    from. We need to store only the difference between the interpolated
    value and the real value. The values should be quantized on a level-
    adaptive basis, with growing quantizers for finer levels.
    Compacting the resulting quantizers with some entropy coder like zlib
    should give a much smaller result (original paper achieved factor
    1:100 using only 0.02 byte per sample), so 40GB of data could get
    compressed to 400mb.
    Generation is done by partitioning original data to smaller tiles with
    e.g. 256x256 size and downsampling each tile down to 1x1. From that
    coarse level tiles are upsampled with subdivision (using adjacent
    tiles for information across edges to avoid creases), and the differences
    between interpolated tile and orignal tile is stored (avoid to accumulate
    errors, always compare level with original one!)
    For a 256x256 tile with 16bit per height we would have 128k. With levels
    we have 4^0+4^1+...+4^8 coefficients. That makes 4^9-1 / 4-3 = 87381
    total, so 1/3 more than the original 65536. Compression thus must be
    factor 1:133 to reach original 1:100.
    To generate the data:
    downsample original tile T in N steps T1...TN, where T1=T by simple 2:1
    downsampling. Sample back up T'k-1 from T'k where T'N=TN by interpolatory
    subdivision. Compute difference between T'k and Tk and code that
    difference. Only TN and the N-1 difference coefficient maps are stored.
    Several problems remain:
    1- what happens near tile edges, problematic for interpolatory upsampling
    2- what happens near global edges
    3- how to efficiently store difference coefficients to make compression
       as best as possible
    4- what quantizers to use, how large they may be
    ad 1: We could take data from neighbouring tiles for upsampling, this would
    give best results for later, but we would need level data of all 4
    surrounding tiles as well - can be done though.
    ad 2: Just use data of line 0 for virtual line -1 and -2, should give best
    results. To avoid costly checks, we could store global data in a 2+2 larger
    array (both in x/y directions) and replicate the first/last row/column
    twice. Or use dummy neighbour tiles (virtual ones) that have same height
    values on their edge and all rows/cols as the neighbour.
    ad 3: best method would be to store all coefficients of level N, then N-1
    and so on. No special pattern like with jpeg is useful as we have no
    DCT or similar.
    ad 4: remaining height diffs should use less than or equal 8 bits.
    hopefully a good entropy encoder can pack them with high rates.

*/


//#define GEOCLIPMAPTEST
#undef  GEOCLIPMAPTEST

#ifdef GEOCLIPMAPTEST

/// interface class to compute heights
class height_generator
{
public:
	/// destructor
	virtual ~height_generator() {}
	/// compute a rectangle of height information (z and z_c)
	///@note coordinates are relative to detail! so detail=0 coord=2,2 matches detail=1 coord=1,1 etc.
	///@param detail - accuracy of height (level of detail)
	///@param coord - coordinate to compute z/z_c for
	///@param dest - pointer to first z value to write to
	virtual float compute_height(unsigned detail, const vector2i& coord) = 0;
	virtual vector3f compute_normal(unsigned detail, const vector2i& coord, float zh) {
		float hr = compute_height(detail, coord + vector2i(1, 0));
		float hu = compute_height(detail, coord + vector2i(0, 1));
		float hl = compute_height(detail, coord + vector2i(-1, 0));
		float hd = compute_height(detail, coord + vector2i(0, -1));
		return vector3f(hl-hr, hd-hu, zh*2).normal();
	}
	virtual vector3f compute_normal_extra(unsigned detail, const vector2i& coord, float zh) { return vector3(0,0,1); }
	virtual color compute_color(unsigned detail, const vector2i& coord) { return color(128, 128, 128); }
	virtual color compute_color_extra(unsigned detail, const vector2i& coord) { return color(128, 128, 128); }
	// deprecated?
	virtual float compute_height_extra(unsigned detail, const vector2i& coord) { return 0.0; }
	virtual void get_min_max_height(double& minh, double& maxh) const = 0;
};



class height_generator_test : public height_generator
{
public:
	float compute_height(unsigned detail, const vector2i& coord) {
		int xc = coord.x * int(1 << detail);
		int yc = coord.y * int(1 << detail);
		return sin(2*3.14159*xc*0.01) * sin(2*3.14159*yc*0.01) * 40.0;
	}
	void get_min_max_height(double& minh, double& maxh) const { minh = -40.0; maxh = 40.0; }
};




class height_generator_test2 : public height_generator
{
	perlinnoise pn;
	const unsigned s2;
	const unsigned height_segments;
	const float total_height;
	const float terrace_height;
	std::vector<float> extrah;
	perlinnoise pn2;
public:
	height_generator_test2()
		: pn(64, 4, 6, true), s2(256*16), height_segments(10),
		  total_height(256.0), terrace_height(total_height/height_segments),
		  pn2(64, 2, 16)
	{
		/*
		lookup_function<float, 256U> asin_lookup;
		for (unsigned i = 0; i <= 256; ++i)
			asin_lookup.set_value(i, asin(float(i)/256) / M_PI + 0.5);
		*/
		std::vector<Uint8> extrah2 = pn2.generate();
		extrah.resize(64*64);
		for (unsigned y = 0; y < 64; ++y)
			for (unsigned x = 0; x < 64; ++x)
				extrah[64*y+x] = rnd()-0.5;//extrah2[64*y+x]/256.0-0.5;
	}
	float compute_height(unsigned detail, const vector2i& coord) {
		int xc = coord.x * int(1 << detail);
		int yc = coord.y * int(1 << detail);
		if (detail <= 6) {
			float h = pn.value(xc, yc, 6-detail)/255.0f;
			return 0 + h*h*h*0.5 * 256;
		} else
			return 0;
	}
	float compute_height_extra(unsigned detail, const vector2i& coord) {
		int xc = coord.x >> detail;
		int yc = coord.y >> detail;
		float baseh = compute_height(0, vector2i(xc, yc));
		/*
		xc = coord.x & ((1 << detail)-1);
		yc = coord.y & ((1 << detail)-1);
		*/
		baseh += extrah[(coord.y&63)*64+(coord.x&63)];
		return baseh;
	}
	vector3f compute_normal_extra(unsigned detail, const vector2i& coord, float zh) {
		vector3f n = compute_normal(0, vector2i(coord.x >> detail, coord.y >> detail),
					    zh * (1 << detail));
		n.x += extrah[(coord.y&63)*64+(coord.x&63)] * 0.5; // / (1<<detail) ...
		n.y += extrah[(coord.y+32&63)*64+(coord.x&63)] * 0.5; // / (1<<detail) ...
		return n.normal();
	}
	void get_min_max_height(double& minh, double& maxh) const { minh = 0.0; maxh = 128.0; }
};


template<class T, class U>
std::vector<T> scaledown(const std::vector<T>& v, unsigned newres)
{
	std::vector<T> result(newres * newres);
	for (unsigned y = 0; y < newres; ++y) {
		for (unsigned x = 0; x < newres; ++x) {
			result[y*newres+x] = T((U(v[2*y*newres*2+2*x]) +
						U(v[2*y*newres*2+2*x+1]) +
						U(v[(2*y+1)*newres*2+2*x]) +
						U(v[(2*y+1)*newres*2+2*x+1])) / 4);
		}
	}
	return result;
}



class height_generator_test3 : public height_generator
{
//	std::vector<std::vector<Uint8> > heightdata;
	std::vector<std::vector<Uint16> > heightdata;
	const unsigned baseres;
	const float heightmult, heightadd;
	std::vector<float> extrah;
	perlinnoise pn2;
public:
	height_generator_test3(const std::vector<Uint16>& hg, unsigned baseres_log2, float hm = 0.75, float ha = -80.0f)
		: heightdata(baseres_log2+1), baseres(1<<baseres_log2), heightmult(hm), heightadd(ha),
		  pn2(64, 2, 16)
	{
		heightdata[0] = hg;
		for (unsigned i = 1; i + 1 < heightdata.size(); ++i)
			heightdata[i] = scaledown<Uint16, Uint32>(heightdata[i-1], baseres >> i);
		heightdata.back().resize(1, 0);
		std::vector<Uint8> extrah2 = pn2.generate();
		extrah.resize(64*64);
		for (unsigned y = 0; y < 64; ++y)
			for (unsigned x = 0; x < 64; ++x)
				extrah[64*y+x] = //rnd()-0.5;
					extrah2[64*y+x]/256.0-0.5;
	}
	float compute_height(unsigned detail, const vector2i& coord) {
		const unsigned shift = (baseres >> detail);
		const unsigned mask = shift - 1;
		unsigned xc = unsigned(coord.x + shift/2) & mask;
		unsigned yc = unsigned(coord.y + shift/2) & mask;
		return heightdata[detail][yc * shift + xc] * heightmult + heightadd;
	}
	vector3f compute_normal_extra(unsigned detail, const vector2i& coord, float zh) {
		vector3f n = compute_normal(0, vector2i(coord.x >> detail, coord.y >> detail),
					    zh * (1 << detail));
		n.x += extrah[(coord.y&63)*64+(coord.x&63)] * 0.5; // / (1<<detail) ...
		n.y += extrah[(coord.y+32&63)*64+(coord.x&63)] * 0.5; // / (1<<detail) ...
		return n.normal();
	}
	color compute_color(unsigned detail, const vector2i& coord) {
		int xc = coord.x * int(1 << detail);
		int yc = coord.y * int(1 << detail);
		float h = compute_height(detail, coord);
		vector3f n = compute_normal(detail, coord, 1.0*(1<<detail));//fixme hack, give zh here
		float k = extrah[(coord.y&63)*64+(coord.x&63)];
		return (n.z > 0.9) ? (h > 20 ? color(240, 240, 242) : color(20,75+k*50,20))
			: color(64+h*0.5+k*32, 64+h*0.5+k*32, 64+h*0.5+k*32);
	}
	color compute_color_extra(unsigned detail, const vector2i& coord) {
		float h = compute_height(0, vector2i(coord.x>>detail, coord.y>>detail));
		vector3f n = compute_normal_extra(detail, coord, 1.0/(1<<detail));//fixme hack, give zh here
		float k = extrah[(coord.y&63)*64+(coord.x&63)];
		return (n.z > 0.9) ? (h > 20 ? color(240, 240, 242) : color(20,75+k*50,20))
			: color(64+h*0.5+k*32, 64+h*0.5+k*32, 64+h*0.5+k*32);
	}
	void get_min_max_height(double& minh, double& maxh) const { minh = heightadd; maxh = heightadd + 65535*heightmult; }
};



class height_generator_test4 : public height_generator
{
public:
	float compute_height(unsigned detail, const vector2i& coord) {
		return detail * 20.0f;
	}
	void get_min_max_height(double& minh, double& maxh) const { minh = 0.0; maxh = 8*20.0; }
};



class height_generator_test5 : public height_generator
{
public:
	float compute_height(unsigned detail, const vector2i& coord) {
		if (coord.x == 0 || coord.y == 0 || coord.x==coord.y) {
			return 40.0;
		} else {
			return 0.0;
		}
	}
	void get_min_max_height(double& minh, double& maxh) const { minh = 0.0; maxh = 40.0; }
};



/// geoclipmap rendering
class geoclipmap
{
#define geoclipmap_fperv 4
	// "N", must be power of two
	const unsigned resolution;  // resolution of triangles in VBO buffer
	const unsigned resolution_vbo; // resolution of VBO buffer
	const unsigned resolution_vbo_mod; // resolution of VBO buffer - 1
	// distance between each vertex on finest level in real world space
	const double L;

	// scratch buffer for VBO data, for transmission
	std::vector<float> vboscratchbuf;

	// scratch buffer for texture data, for transmission
	std::vector<Uint8> texnormalscratchbuf;

	// scratch buffer for color texture data, for transmission
	std::vector<Uint8> texcolorscratchbuf;

	// scratch buffer for index generation, for transmission
	std::vector<uint32_t> idxscratchbuf;

	struct area
	{
		// bottom left and top right vertex coordinates
		// so empty area is when tr < bl
		vector2i bl, tr;

		area() : bl(0, 0), tr(-1, -1) {}
		area(const vector2i& a, const vector2i& b) : bl(a), tr(b) {}
		/*
		  area clip(const area& other) const {
		  return area(bl.max(other.bl), tr.min(other.tr));
		  }
		*/
		vector2i size() const { return vector2i(tr.x - bl.x + 1, tr.y - bl.y + 1); }
		bool empty() const { vector2i sz = size(); return sz.x*sz.y <= 0; }
	};

	/// per-level data
	class level
	{
		geoclipmap& gcm;
		/// distance between samples of that level
		double L_l;
		/// level index
		const unsigned index;
		/// vertex data
		vertexbufferobject vertices;
		/// store here for reuse ? we have 4 areas for indices...
		mutable vertexbufferobject indices;
		/// which coordinate area is stored in the VBO, in per-level coordinates
		area vboarea;
		/// offset in VBO of bottom left (vboarea.bl) data sample (dataoffset.x/y in [0...N] range)
		vector2i dataoffset;

		mutable area tmp_inner, tmp_outer;
		bool outmost;

		unsigned generate_indices(const frustum& f,
					  uint32_t* buffer, unsigned idxbase,
					  const vector2i& offset,
					  const vector2i& size,
					  const vector2i& vbooff) const;
		unsigned generate_indices2(uint32_t* buffer, unsigned idxbase,
					   const vector2i& size,
					   const vector2i& vbooff) const;
		unsigned generate_indices_T(uint32_t* buffer, unsigned idxbase) const;
		unsigned generate_indices_horizgap(uint32_t* buffer, unsigned idxbase) const;
		void update_region(const geoclipmap::area& upar);
		void update_VBO_and_tex(const vector2i& scratchoff,
					int scratchmod,
					const vector2i& sz,
					const vector2i& vbooff);

		texture::ptr normals;
		texture::ptr colors;
	public:
		level(geoclipmap& gcm_, unsigned idx, bool outmost_level);
		area set_viewerpos(const vector3f& new_viewpos, const geoclipmap::area& inner);
		void display(const frustum& f) const;
		texture& normals_tex() const { return *normals; }
		texture& colors_tex() const { return *colors; }
	};

	ptrvector<level> levels;
	height_generator& height_gen;

	int mod(int n) const {
		return n & resolution_vbo_mod;
	}

	vector2i clamp(const vector2i& v) const {
		return vector2i(mod(v.x), mod(v.y));
	}

	mutable glsl_shader_setup myshader;
	unsigned myshader_vattr_z_c_index;
	unsigned loc_texterrain;
	unsigned loc_texnormal;
	unsigned loc_texnormal_c;
	unsigned loc_texcolor;
	unsigned loc_texcolor_c;
	unsigned loc_w_p1;
	unsigned loc_w_rcp;
	unsigned loc_viewpos;
	unsigned loc_xysize2;
	unsigned loc_L_l_rcp;
	unsigned loc_N_rcp;
	unsigned loc_texcshift;
	texture::ptr horizon_normal;

public:
	/// create geoclipmap data
	///@param nr_levels - number of levels
	///@param resolution_exp - power of two of resolution factor "N"
	///@param L - distance between samples in real world space on finest level
	///@param hg - instance of height generator object
	geoclipmap(unsigned nr_levels, unsigned resolution_exp, double L, height_generator& hg);

	/// d'tor
	~geoclipmap();

	/// set/change viewer position
	void set_viewerpos(const vector3f& viewpos);

	/// render the view (will only fetch the vertex/index data, no texture setup)
	void display(const frustum& f) const;
};



geoclipmap::geoclipmap(unsigned nr_levels, unsigned resolution_exp, double L_, height_generator& hg)
	: resolution((1 << resolution_exp) - 2),
	  resolution_vbo(1 << resolution_exp),
	  resolution_vbo_mod(resolution_vbo-1),
	  L(L_),
	  vboscratchbuf((resolution_vbo+2)*(resolution_vbo+2)*geoclipmap_fperv), // 4 floats per VBO sample (x,y,z,zc)
	  // ^ extra space for normals
	  texnormalscratchbuf(resolution_vbo*2*resolution_vbo*2*3),
	  texcolorscratchbuf(resolution_vbo*2*resolution_vbo*2*3),
	  idxscratchbuf((resolution_vbo+2)*resolution_vbo*2 + 256),
	  levels(nr_levels),
	  height_gen(hg),
	  myshader(get_shader_dir() + "geoclipmap.vshader",
		   get_shader_dir() + "geoclipmap.fshader"),
	  myshader_vattr_z_c_index(0)
{
	// initialize vertex VBO and all level VBOs
	for (unsigned lvl = 0; lvl < levels.size(); ++lvl) {
		levels.reset(lvl, new level(*this, lvl, lvl+1==levels.size()));
	}

	myshader.use();
	myshader_vattr_z_c_index = myshader.get_vertex_attrib_index("z_c");
	loc_texterrain = myshader.get_uniform_location("texterrain");
	loc_texcolor = myshader.get_uniform_location("texcolor");
	loc_texcolor_c = myshader.get_uniform_location("texcolor_c");
	loc_texnormal = myshader.get_uniform_location("texnormal");
	loc_texnormal_c = myshader.get_uniform_location("texnormal_c");
	loc_w_p1 = myshader.get_uniform_location("w_p1");
	loc_w_rcp = myshader.get_uniform_location("w_rcp");
	loc_viewpos = myshader.get_uniform_location("viewpos");
	loc_xysize2 = myshader.get_uniform_location("xysize2");
	loc_L_l_rcp = myshader.get_uniform_location("L_l_rcp");
	loc_N_rcp = myshader.get_uniform_location("N_rcp");
	loc_texcshift = myshader.get_uniform_location("texcshift");
	const float w_fac = 0.2f;//0.1f;
	myshader.set_uniform(loc_w_p1, resolution_vbo * w_fac + 1.0f);
	myshader.set_uniform(loc_w_rcp, 1.0f/(resolution_vbo * w_fac));
	myshader.set_uniform(loc_N_rcp, 1.0f/resolution_vbo);
	myshader.set_uniform(loc_texcshift, 0.5f/resolution_vbo);
	myshader.use_fixed();
	// set a texture for normals outside coarsest level, just 0,0,1
	std::vector<Uint8> pxl(3, 128);
	pxl[2] = 255;
	horizon_normal.reset(new texture(pxl, 1, 1, GL_RGB, texture::LINEAR, texture::REPEAT));
	myshader.use_fixed();
}



geoclipmap::~geoclipmap()
{
}



void geoclipmap::set_viewerpos(const vector3f& new_viewpos)
{
	// for each level compute clip area for that new viewerpos
	// for each level compute area that needs to get updated and do that

	// empty area for innermost level
	area levelborder;

	// compute lowest level number by height
	// draw level if new_viewpos.z <= 0.4 * resolution * L_l
	// L_l = L * 2^level
	// that is if log2(new_viewpos.z / (0.4*resolution*L)) <= level
	// so compute floor(log2(new_viewpos.z/(0.4*resolution*L))) as minimum level
	unsigned min_level = unsigned(std::max(floor(log2(new_viewpos.z/(0.4*resolution*L))), 0.0));
	// fixme: later test to begin drawing at min_level
	//log_debug("min_level=" << min_level);

	for (unsigned lvl = 0; lvl < levels.size(); ++lvl) {
		levelborder = levels[lvl]->set_viewerpos(new_viewpos, levelborder);
		// next level has coordinates with half resolution
		// let outer area of current level be inner area of next level
		levelborder.bl.x /= 2;
		levelborder.bl.y /= 2;
		levelborder.tr.x /= 2;
		levelborder.tr.y /= 2;
	}

	myshader.use();
	myshader.set_uniform(loc_viewpos, new_viewpos);
	myshader.use_fixed();
}



void geoclipmap::display(const frustum& f) const
{
	// display levels from inside to outside
	//unsigned min_level = unsigned(std::max(floor(log2(new_viewpos.z/(0.4*resolution*L))), 0.0));
	glActiveTexture(GL_TEXTURE1);
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE2);
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE3);
	glEnable(GL_TEXTURE_2D);
	myshader.use();
	for (unsigned lvl = 0; lvl < levels.size(); ++lvl) {
		myshader.set_gl_texture(levels[lvl]->colors_tex(), loc_texcolor, 0);
		myshader.set_gl_texture(levels[lvl]->normals_tex(), loc_texnormal, 2);
		if (lvl + 1 < levels.size()) {
			myshader.set_gl_texture(levels[lvl+1]->colors_tex(), loc_texcolor_c, 1);
			myshader.set_gl_texture(levels[lvl+1]->normals_tex(), loc_texnormal_c, 3);
		} else {
			myshader.set_gl_texture(levels[levels.size()-1]->colors_tex(), loc_texcolor_c, 1);
			myshader.set_gl_texture(*horizon_normal, loc_texnormal_c, 3);
		}
		levels[lvl]->display(f);
	}
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);
	myshader.use_fixed();
}



geoclipmap::level::level(geoclipmap& gcm_, unsigned idx, bool outmost_level)
	: gcm(gcm_),
	  L_l(gcm.L * double(1 << idx)),
	  index(idx),
	  vertices(false),
	  indices(true),
	  outmost(outmost_level)
{
	// mostly static...
	vertices.init_data(gcm.resolution_vbo*gcm.resolution_vbo*geoclipmap_fperv*4
			   + (outmost ? 8*geoclipmap_fperv*4 : 0),
			   0, GL_STATIC_DRAW);
	// size of T-junction triangles: 4 indices per triangle (3 + 2 degen. - 1), 4*N/2 triangles
	// max. size for normal triangles + T-junc.: 2*N^2 - 4*N + 8*N - 16 = 2*N^2 + 4*N - 16
	// size for multiple columns: 2 per new column, at most N columns, mostly less.
	indices.init_data(2*gcm.resolution_vbo*gcm.resolution_vbo*4 + 4*gcm.resolution*4
			  + (outmost ? 4*2*gcm.resolution_vbo*4 : 0),
			  0, GL_STATIC_DRAW);
	// create space for normal texture
	std::vector<Uint8> pxl(3*gcm.resolution_vbo*gcm.resolution_vbo*2*2);
	normals.reset(new texture(pxl, gcm.resolution_vbo*2,
				  gcm.resolution_vbo*2, GL_RGB, texture::LINEAR, texture::REPEAT));
	memset(&pxl[0], index*30, gcm.resolution_vbo*gcm.resolution_vbo*2*2*3);
	colors.reset(new texture(pxl, gcm.resolution_vbo*2,
				 gcm.resolution_vbo*2, GL_RGB, texture::LINEAR, texture::REPEAT));
}



geoclipmap::area geoclipmap::level::set_viewerpos(const vector3f& new_viewpos, const geoclipmap::area& inner)
{
	// x_base/y_base tells offset in sample data according to level and
	// viewer position (new_viewpos)
	// this multiply with 0.5 then round then *2 lets the patches map to
	// "even" vertices and must be used to determine which patch number to render.
	// we need to make sure that clip area is at max N vertices!
	// We use a round function that always rounds in same direction, i.e.
	// when 3.5 is rounded up to 4.0, then -3.5 is rounded up to -3.0.
	// this is done by using round(x) := floor(x + 0.5)
	// so when this gives -3.5....3.5 we take floor(-3.0)....floor(4.0) -> -3...4
	area outer(vector2i(int(floor(0.5*new_viewpos.x/L_l - 0.25*gcm.resolution + 0.5))*2,
			    int(floor(0.5*new_viewpos.y/L_l - 0.25*gcm.resolution + 0.5))*2),
		   vector2i(int(floor(0.5*new_viewpos.x/L_l + 0.25*gcm.resolution + 0.5))*2,
			    int(floor(0.5*new_viewpos.y/L_l + 0.25*gcm.resolution + 0.5))*2));
	tmp_inner = inner;
	tmp_outer = outer;
	//log_debug("index="<<index<<" area inner="<<inner.bl<<"|"<<inner.tr<<" outer="<<outer.bl<<"|"<<outer.tr);
	// for vertex updates we only need to know the outer area...
	// compute part of "outer" that is NOT covered by old outer area,
	// this gives a rectangular or L-shaped form, but this can not be expressed
	// as area, only with at least 2 areas...
	if (vboarea.empty()) {
		vboarea = outer;	// set this to make the update work correctly
		dataoffset = gcm.clamp(outer.bl);
		update_region(outer);
	} else {
		area outercmp = outer;
		unsigned nr_updates = 0;
		if (outercmp.bl.y < vboarea.bl.y) {
			update_region(area(outercmp.bl, vector2i(outercmp.tr.x, vboarea.bl.y - 1)));
			outercmp.bl.y = vboarea.bl.y;
			++nr_updates;
		}
		if (vboarea.tr.y < outercmp.tr.y) {
			update_region(area(vector2i(outercmp.bl.x, vboarea.tr.y + 1), outercmp.tr));
			outercmp.tr.y = vboarea.tr.y;
			++nr_updates;
		}
		if (outercmp.bl.x < vboarea.bl.x) {
			update_region(area(outercmp.bl, vector2i(vboarea.bl.x - 1, outercmp.tr.y)));
			outercmp.bl.x = vboarea.bl.x;
			++nr_updates;
		}
		if (vboarea.tr.x < outercmp.tr.x) {
			update_region(area(vector2i(vboarea.tr.x + 1, outercmp.bl.y), outercmp.tr));
			outercmp.tr.x = vboarea.tr.x;
			++nr_updates;
		}
		if (nr_updates > 2)
			throw error("got more than 2 update regions?! BUG!");
	}
	// we updated the vertices, so update area/offset
	dataoffset = gcm.clamp(outer.bl - vboarea.bl + dataoffset);
	vboarea = outer;

	if (outmost) {
		// give 8 vertices to fill horizon gap
		static const int dx[8] = { -1, 0, 1, 1, 1, 0, -1, -1 };
		static const int dy[8] = { -1,-1,-1, 0, 1, 1,  1,  0 };
		for (unsigned i = 0; i < 8; ++i) {
			gcm.vboscratchbuf[4*i+0] = new_viewpos.x + 32000 * dx[i];
			gcm.vboscratchbuf[4*i+1] = new_viewpos.x + 32000 * dy[i];
			gcm.vboscratchbuf[4*i+2] = 0; // fixme: later give +- 10 for land/sea
			gcm.vboscratchbuf[4*i+3] = 0; // same value here
		}
		vertices.init_sub_data(gcm.resolution_vbo*gcm.resolution_vbo*geoclipmap_fperv*4,
				       8*geoclipmap_fperv*4, &gcm.vboscratchbuf[0]);
	}

	return outer;
}



void geoclipmap::level::update_region(const geoclipmap::area& upar)
{
	//log_debug("lvl="<<index<<" i="<<i<<" upar="<<upar.bl<<","<<upar.tr);
	if (upar.empty()) throw error("update area empty?! BUG!");//continue; // can happen on initial update
	vector2i sz = upar.size();
	// height data coordinates upar.bl ... upar.tr need to be updated, but what VBO offset?
	// since data is stored toroidically in VBO, a rectangle can be split into up to
	// 4 rectangles...
	// we need to call get_height function with index as detail and the area as
	// parameter...
	// prepare a N*N height lookup/scratch buffer, fill in w*h samples, then feed
	// the correct samples to the GPU
	// maybe better prepare a VBO scratch buffer, maybe even one per level (doesn't cost
	// too much ram), and store the fixed VBO data there
	// that is x,y,height and height_c. the get_height function must write the correct
	// values then (with stride) or enters x,y,h_c as well.
	// yes, if get_height could give h and h_c at once, it would be better, and this
	// can easily be done.
	// get_height: takes upar and a target buffer and enters realx,realy,realz,realz_c
	// z_c means z of coarser level, we must give some helper values to
	// generate realx/realy or write it ourselfs (maybe better).
	// data per vertex? 4 floats x,y,z,zc plus normal? or normal as texmap?
	// normal computation is not trivial!

	// update VBO toroidically

	// compute the heights first (+1 in every direction to compute normals too)
	vector2i upcrd = upar.bl + vector2i(-1, -1);
	// idea: fill in height here only from current level
	// fill in every 2nd x/y value by height of coarser level (we need m/2+1 values)
	// interpolate missing heights manually here
	// for that scratchbuffer needs to get enlarged possibly
	// it depends on wether upcrd.xy is odd or even where to add lines for z_c computation
	// later we need to compute n_c as well, so we need even more lines.
	// note! do not call "x / 2" for int x, its more efficient to use "x >> 1"!
	// It is even WRONG to call x / 2 sometimes, as -1 / 2 gives 0 and not -1 as the shift method does,
	// when we want to round down...
	unsigned ptr = 0;
	for (int y = 0; y < sz.y + 2; ++y) {
		vector2i upcrd2 = upcrd;
		for (int x = 0; x < sz.x + 2; ++x) {
			gcm.vboscratchbuf[ptr+0] = upcrd2.x * L_l; // fixme: maybe subtract viewerpos here later.
			gcm.vboscratchbuf[ptr+1] = upcrd2.y * L_l;
			gcm.vboscratchbuf[ptr+2] = gcm.height_gen.compute_height(index, upcrd2);
			ptr += geoclipmap_fperv;
			++upcrd2.x;
		}
		++upcrd.y;
	}

	// get heights of coarser level, only for values inside original scratchbuf area
	// (without the +1 perimeter for normals)
	upcrd.x = upar.bl.x >> 1; // need to shift here because values could be negative!
	upcrd.y = upar.bl.y >> 1;
	// sz.xy can be odd, so we must compute size with rounding up tr...
	const vector2 szc(((upar.tr.x+1)>>1) - upcrd.x + 1, ((upar.tr.y+1)>>1) - upcrd.y + 1);
	unsigned ptr3 = ptr = ((sz.x+2)*(1-(upar.bl.y & 1)) + (1-(upar.bl.x & 1)))*geoclipmap_fperv;
	for (int y = 0; y < szc.y; ++y) {
		vector2i upcrd2 = upcrd;
		unsigned ptr2 = ptr;
		for (int x = 0; x < szc.x; ++x) {
			gcm.vboscratchbuf[ptr2+3] = gcm.height_gen.compute_height(index+1, upcrd2);
			ptr2 += 2*geoclipmap_fperv;
			++upcrd2.x;
		}
		++upcrd.y;
		ptr += 2*(sz.x+2)*geoclipmap_fperv;
	}

	// interpolate z_c, first fill in missing columns on even rows
	ptr = ptr3;
	for (int y = 0; y < szc.y; ++y) {
		unsigned ptr2 = ptr;
		for (int x = 0; x < szc.x-1; ++x) {
			float f0 = gcm.vboscratchbuf[ptr2+3];
			float f1 = gcm.vboscratchbuf[ptr2+2*geoclipmap_fperv+3];
			gcm.vboscratchbuf[ptr2+geoclipmap_fperv+3] = (f0+f1)*0.5f;
			ptr2 += 2*geoclipmap_fperv;
		}
		ptr += 2*(sz.x+2)*geoclipmap_fperv;
	}

	// interpolate z_c, first fill in missing rows
	ptr = ptr3 + (sz.x+2)*geoclipmap_fperv;
	for (int y = 0; y < szc.y-1; ++y) {
		unsigned ptr2 = ptr;
		for (int x = 0; x < szc.x*2-1; ++x) {//here we could spare 1 column
			float f0 = gcm.vboscratchbuf[ptr2-(sz.x+2)*geoclipmap_fperv+3];
			float f1 = gcm.vboscratchbuf[ptr2+(sz.x+2)*geoclipmap_fperv+3];
			gcm.vboscratchbuf[ptr2+3] = (f0+f1)*0.5f;
			ptr2 += geoclipmap_fperv;
		}
		ptr += 2*(sz.x+2)*geoclipmap_fperv;
	}

	// compute normals
	unsigned tptr = 0;
	//log_debug("tex scratch sz="<<sz);
	/* fixme: maybe let height generator let deliver normals directly */
	/* fixme: maybe store normals in double res. (from next finer level) in texture
	   with double res. as vertices -> more detail!
	*/
	if(index>0)
	for (int y = 0; y < sz.y*2; ++y) {
		for (int x = 0; x < sz.x*2; ++x) {
			vector3f nm = gcm.height_gen.compute_normal(index-1, upar.bl*2 + vector2i(x, y), L_l*0.5);
			nm = nm * 127;
			gcm.texnormalscratchbuf[tptr+0] = Uint8(nm.x + 128);
			gcm.texnormalscratchbuf[tptr+1] = Uint8(nm.y + 128);
			gcm.texnormalscratchbuf[tptr+2] = Uint8(nm.z + 128);
			tptr += 3;
		}
	}
	else
	for (int y = 0; y < sz.y*2; ++y) {
		for (int x = 0; x < sz.x*2; ++x) {
			vector3f nm = gcm.height_gen.compute_normal_extra(1, upar.bl*2 + vector2i(x, y), L_l*0.5);
			nm = nm * 127;
			gcm.texnormalscratchbuf[tptr+0] = Uint8(nm.x + 128);
			gcm.texnormalscratchbuf[tptr+1] = Uint8(nm.y + 128);
			gcm.texnormalscratchbuf[tptr+2] = Uint8(nm.z + 128);
			tptr += 3;
		}
	}

	// color update
	tptr = 0;
	if(index>0)
	for (int y = 0; y < sz.y*2; ++y) {
		for (int x = 0; x < sz.x*2; ++x) {
			color c = gcm.height_gen.compute_color(index-1, upar.bl*2 + vector2i(x, y));
			gcm.texcolorscratchbuf[tptr+0] = c.r;
			gcm.texcolorscratchbuf[tptr+1] = c.g;
			gcm.texcolorscratchbuf[tptr+2] = c.b;
			tptr += 3;
		}
	}
	else
	for (int y = 0; y < sz.y*2; ++y) {
		for (int x = 0; x < sz.x*2; ++x) {
			color c = gcm.height_gen.compute_color_extra(1, upar.bl*2 + vector2i(x, y));
			gcm.texcolorscratchbuf[tptr+0] = c.r;
			gcm.texcolorscratchbuf[tptr+1] = c.g;
			gcm.texcolorscratchbuf[tptr+2] = c.b;
			tptr += 3;
		}
	}

	ptr = 0;
	geoclipmap::area vboupdate(gcm.clamp(upar.bl - vboarea.bl + dataoffset),
				   gcm.clamp(upar.tr - vboarea.bl + dataoffset));
	// check for continuous update areas
	//log_debug("vboupdate area="<<vboupdate.bl<<" | "<<vboupdate.tr);
	//fixme: since texture/VBO updates are line by line anyway, we don't need to
	//handle the y-wrap here...
	if (vboupdate.tr.x < vboupdate.bl.x) {
		// area crosses VBO border horizontally
		// area 1 width gcm.resolution_vbo - bl.xy
		// area 2 width tr.xy + 1
#if 0
		if (vboupdate.tr.y < vboupdate.bl.y) {
			// area crosses VBO border horizontally and vertically
			int szx = gcm.resolution_vbo - vboupdate.bl.x;
			int szy = gcm.resolution_vbo - vboupdate.bl.y;
			update_VBO_and_tex(vector2i(0, 0), sz.x, vector2i(szx, szy), vboupdate.bl);
			update_VBO_and_tex(vector2i(szx, 0), sz.x, vector2i(vboupdate.tr.x + 1, szy),
					   vector2i(gcm.mod(vboupdate.bl.x + szx), vboupdate.bl.y));
			update_VBO_and_tex(vector2i(0, szy), sz.x, vector2i(szx, vboupdate.tr.y + 1),
					   vector2i(vboupdate.bl.x, gcm.mod(vboupdate.bl.y + szy)));
			update_VBO_and_tex(vector2i(szx, szy), sz.x, vector2i(vboupdate.tr.x + 1, vboupdate.tr.y + 1),
					   vector2i(gcm.mod(vboupdate.bl.x + szx), gcm.mod(vboupdate.bl.y + szy)));
		} else {
#endif
			// area crosses VBO border horizontally
			int szx = gcm.resolution_vbo - vboupdate.bl.x;
			update_VBO_and_tex(vector2i(0, 0), sz.x, vector2i(szx, sz.y), vboupdate.bl);
			update_VBO_and_tex(vector2i(szx, 0), sz.x, vector2i(vboupdate.tr.x + 1, sz.y),
					   vector2i(gcm.mod(vboupdate.bl.x + szx), vboupdate.bl.y));
#if 0
		}
	} else if (vboupdate.tr.y < vboupdate.bl.y) {
		// area crosses VBO border vertically
		int szy = gcm.resolution_vbo - vboupdate.bl.y;
		update_VBO_and_tex(vector2i(0, 0), sz.x, vector2i(sz.x, szy), vboupdate.bl);
		update_VBO_and_tex(vector2i(0, szy), sz.x, vector2i(sz.x, vboupdate.tr.y + 1),
				   vector2i(vboupdate.bl.x, gcm.mod(vboupdate.bl.y + szy)));
#endif
	} else {
		// no border crossed
		update_VBO_and_tex(vector2i(0, 0), sz.x, sz, vboupdate.bl);
	}
}



void geoclipmap::level::update_VBO_and_tex(const vector2i& scratchoff,
					   int scratchmod,
					   const vector2i& sz,
					   const vector2i& vbooff)
{		
	/* fixme: CPU load is high, especially kernel part, could be cause of
	   these calls to GL.
	   We could at least reduce the glTexSubImage2D calls by copying the
	   texture data to a consecutive block. But then we need to do y-clipping
	   in the calling function.
	   And we could try buffer-mapping instead of many glBufferSubData calls
	*/
	//log_debug("update2 off="<<scratchoff<<" sz="<<sz<<" vbooff="<<vbooff);
	// copy data to normals texture
	// we need to do it line by line anyway, as the source is not packed.
	// maybe we can get higher frame rates if it would be packed...
	// test showed that this is negligible
	glActiveTexture(GL_TEXTURE2);
	normals->set_gl_texture();
	for (int y = 0; y < sz.y*2; ++y) {
		//log_debug("update texture xy off ="<<vbooff.x<<"|"<<gcm.mod(vbooff.y+y)<<" idx="<<((scratchoff.y+y)*scratchmod+scratchoff.x)*3);
		glTexSubImage2D(GL_TEXTURE_2D, 0 /* mipmap level */,
				vbooff.x*2, (vbooff.y*2+y) & (gcm.resolution_vbo_mod*2+1), sz.x*2, 1, GL_RGB, GL_UNSIGNED_BYTE,
				&gcm.texnormalscratchbuf[((scratchoff.y*2+y)*scratchmod*2+scratchoff.x*2)*3]);
	}

	glActiveTexture(GL_TEXTURE0);
	colors->set_gl_texture();
	for (int y = 0; y < sz.y*2; ++y) {
		glTexSubImage2D(GL_TEXTURE_2D, 0 /* mipmap level */,
				vbooff.x*2, (vbooff.y*2+y) & (gcm.resolution_vbo_mod*2+1), sz.x*2, 1, GL_RGB, GL_UNSIGNED_BYTE,
				&gcm.texcolorscratchbuf[((scratchoff.y*2+y)*scratchmod*2+scratchoff.x*2)*3]);
	}

	// copy data to real VBO.
	// we need to do it line by line anyway.
	for (int y = 0; y < sz.y; ++y) {
		vertices.init_sub_data((vbooff.x + gcm.mod(vbooff.y+y)*gcm.resolution_vbo)*geoclipmap_fperv*4,
				       sz.x*geoclipmap_fperv*4,
				       &gcm.vboscratchbuf[((scratchoff.y+y+1)*(scratchmod+2)+1+scratchoff.x)*geoclipmap_fperv]);
	}
}



static const int geoidx[2*4*6] = {
	0,0, 1,0, 1,1, 0,1,
	1,0, 2,0, 2,1, 1,1,
	2,0, 3,0, 3,1, 2,1,
	3,0, 0,0, 0,1, 3,1,
	0,1, 1,1, 2,1, 3,1,
	0,0, 3,0, 2,0, 1,0,
};

// give real world offset (per-level coordinates) here as well as frustum-ref
unsigned geoclipmap::level::generate_indices(const frustum& f,
					     uint32_t* buffer, unsigned idxbase,
					     const vector2i& offset, // in per-level coordinates, global
					     const vector2i& size, // size of patch
					     const vector2i& vbooff) const // offset in VBO
{
	//log_debug("genindi="<<index<<" size="<<size);
	if (size.x <= 1 || size.y <= 1) return 0;

	/* how clipping works:
	   each patch forms a rectangle in xy-plane. Together with min. and max. height
	   of the terrain this forms an axis aligned box (bounding box of patch).
	   We create 6 polygons to represent that box and clip them by the viewing
	   frustum. The resulting points are projected to the xy plane and their
	   min. and max. xy coordinates are computed.
	   That area is used to clip the patch area.
	*/
	// box base points in ccw order
	vector2 cv[4] = { vector2(offset.x * L_l, offset.y * L_l),
			  vector2((offset.x + size.x) * L_l, offset.y * L_l),
			  vector2((offset.x + size.x) * L_l, (offset.y + size.y) * L_l),
			  vector2(offset.x * L_l, (offset.y + size.y) * L_l) };
	double minmaxz[2];
	gcm.height_gen.get_min_max_height(minmaxz[0], minmaxz[1]);
	vector2 minv(1e30, 1e30), maxv(-1e30, -1e30);
	// if viewer is inside the bounding box, use its position as additional
	// bound point.
	const double eps = 0.5;
	if (f.viewpos.z > minmaxz[0] - eps && f.viewpos.z < minmaxz[1] + eps) {
		minv = maxv = f.viewpos.xy();
	} 
	bool allempty = true;
	for (unsigned i = 0; i < 6; ++i) {
		polygon p = f.clip(polygon(cv[geoidx[i*8+0]].xyz(minmaxz[geoidx[i*8+1]]),
					   cv[geoidx[i*8+2]].xyz(minmaxz[geoidx[i*8+3]]),
					   cv[geoidx[i*8+4]].xyz(minmaxz[geoidx[i*8+5]]),
					   cv[geoidx[i*8+6]].xyz(minmaxz[geoidx[i*8+7]])));
		if (!p.empty()) {
			allempty = false;
			for (unsigned j = 0; j < p.points.size(); ++j) {
				minv = minv.min(p.points[j].xy());
				maxv = maxv.max(p.points[j].xy());
			}
		}
	}

	if (allempty) return 0;
				
	// convert coordinates back to integer values with rounding down/up
	vector2i minvi(int(floor(minv.x / L_l)), int(floor(minv.y / L_l)));
	vector2i maxvi(int(ceil(maxv.x / L_l)), int(ceil(maxv.y / L_l)));
	vector2i newoffset = minvi, newsize = maxvi - minvi + vector2i(1, 1);
	// avoid size/offset to move out of what was given...
	// size seems to be at most 1 more than old size, offset at most 1 less.
	// this can be explained with float rounding errors.
//	if (newoffset.x<offset.x||newoffset.y<offset.y)
//		log_debug("offset diff, old="<<offset<<" new="<<newoffset);
	newoffset = newoffset.max(offset);
// 	if (newsize.x>offset.x + size.x - newoffset.x||newsize.y>offset.y + size.y - newoffset.y)
// 		log_debug("size diff, old="<<offset+size-newoffset<<" new="<<newsize);
	newsize = newsize.min(offset + size - newoffset);
// 	log_debug("clip area, vbooff="<<vbooff<<" now "<<vbooff + newoffset - offset
// 		  <<" size="<<size<<" now "<<newsize);
// 	log_debug("offset shift: "<<newoffset-offset<<" sizechange: "<<size-newsize);
	
	// modify vbooff accordingly (no clamping needed, is done later anyway)
	vector2i vbooff2 = vbooff + newoffset - offset;
	vector2i size2 = newsize;
	// check again if patch is still valid
	if (size2.x <= 1 || size2.y <= 1) return 0;

#if 1
	const unsigned colw = 17;
	unsigned cols = (size2.x + colw-2) / (colw-1);
	unsigned coloff = 0;
	unsigned result = 0;
	for (unsigned c = 0; c < cols; ++c) {
		unsigned szx = std::min(colw, unsigned(size2.x) - coloff);
		result += generate_indices2(buffer, idxbase+result,
					    vector2i(szx, size2.y),
					    vector2i(vbooff2.x+coloff,vbooff2.y));
		coloff += colw-1;
	}
	return result;
#else
	return generate_indices2(buffer, idxbase, size2, vbooff2);
#endif
}

unsigned geoclipmap::level::generate_indices2(uint32_t* buffer, unsigned idxbase,
					     const vector2i& size2,
					     const vector2i& vbooff2) const
{
	// fixme: according to the profiler, this eats 70% of cpu time!!!
	//gcm.mod costs 3 instructions because of dereferencing...
	//better handle/inc buffer pointer directly instead of maintaining ptr-variable!
	//to get the address of buffer the gcc uses many strange instructions!
	//it seems the writing to the VBO costs most...
	//not really, when writing to a victim buffer, total cpu usage goes from 70%
	//down to 43%. So VBO writing is 1-43/70=39% of the cpu time.
	//It is doubtful that glBufferSubData would be faster...
	//so replace gcm.mod, we can't do much more...
	//done, now cpu load 65%, but still high.
	//it variies with geometric detail or viewer height, so its hard to tell
	//but current version has least number of instructions in inner loop
	//that can be generated with gcc, for lower numbers one needs
	//assembler.

	// always put the first index of a row twice and also the last
	uint32_t* bufptr = buffer + idxbase;
	const unsigned resolution_vbo_mod = gcm.resolution_vbo_mod;
	int vbooffy0 = vbooff2.y & resolution_vbo_mod;
	int vbooffy1 = (vbooffy0 + 1) & resolution_vbo_mod;
	for (int y = 0; y + 1 < size2.y; ++y) {
		int vbooffy0_l = vbooffy0 * gcm.resolution_vbo;
		int vbooffy1_l = vbooffy1 * gcm.resolution_vbo;
		int vbooffx0 = vbooff2.x & resolution_vbo_mod;
		// store first index twice (line or patch transition)
		*bufptr++ = vbooffy1_l + vbooffx0;
#if 0
		for (int x = 0; x < size2.x; ++x) {
			//gcc does strange things with this line!
			//it is indexing it via the loop counter etc., it cant be optimal
 			*bufptr++ = vbooffy1_l + vbooffx0;
 			*bufptr++ = vbooffy0_l + vbooffx0;
			vbooffx0 = (vbooffx0 + 1) & resolution_vbo_mod;
		}
#else
		// try a loop with pointers directly
		for (uint32_t* limit = bufptr + size2.x*2; bufptr != limit; bufptr += 2) {
 			bufptr[0] = vbooffy1_l + vbooffx0;
 			bufptr[1] = vbooffy0_l + vbooffx0;
			vbooffx0 = (vbooffx0 + 1) & resolution_vbo_mod;
		}
#endif
		uint32_t lastidx = vbooffy0_l + ((vbooffx0 - 1) & resolution_vbo_mod);
		// store last index twice (line or patch transition)
		*bufptr++ = lastidx;
		vbooffy0 = vbooffy1;
		vbooffy1 = (vbooffy1 + 1) & resolution_vbo_mod;
	}
	return bufptr - (buffer + idxbase);
}



unsigned geoclipmap::level::generate_indices_T(uint32_t* buffer, unsigned idxbase) const
{
	unsigned ptr = idxbase;
	vector2i v = dataoffset;
	for (unsigned i = 0; i < gcm.resolution/2; ++i) {
		buffer[ptr] = buffer[ptr+1] = v.x + v.y * gcm.resolution_vbo;
		v.y = gcm.mod(v.y + 1);
		buffer[ptr+2] = v.x + v.y * gcm.resolution_vbo;
		v.y = gcm.mod(v.y + 1);
		buffer[ptr+3] = v.x + v.y * gcm.resolution_vbo;
		ptr += 4;
	}
	for (unsigned i = 0; i < gcm.resolution/2; ++i) {
		buffer[ptr] = buffer[ptr+1] = v.x + v.y * gcm.resolution_vbo;
		v.x = gcm.mod(v.x + 1);
		buffer[ptr+2] = v.x + v.y * gcm.resolution_vbo;
		v.x = gcm.mod(v.x + 1);
		buffer[ptr+3] = v.x + v.y * gcm.resolution_vbo;
		ptr += 4;
	}
	for (unsigned i = 0; i < gcm.resolution/2; ++i) {
		buffer[ptr] = buffer[ptr+1] = v.x + v.y * gcm.resolution_vbo;
		v.y = gcm.mod(v.y - 1);
		buffer[ptr+2] = v.x + v.y * gcm.resolution_vbo;
		v.y = gcm.mod(v.y - 1);
		buffer[ptr+3] = v.x + v.y * gcm.resolution_vbo;
		ptr += 4;
	}
	for (unsigned i = 0; i < gcm.resolution/2; ++i) {
		buffer[ptr] = buffer[ptr+1] = v.x + v.y * gcm.resolution_vbo;
		v.x = gcm.mod(v.x - 1);
		buffer[ptr+2] = v.x + v.y * gcm.resolution_vbo;
		v.x = gcm.mod(v.x - 1);
		buffer[ptr+3] = v.x + v.y * gcm.resolution_vbo;
		ptr += 4;
	}
	return ptr - idxbase;
}



unsigned geoclipmap::level::generate_indices_horizgap(uint32_t* buffer, unsigned idxbase) const
{
	// repeat last index for degenerated triangle conjunction. legal, because
	// there are always indices in buffer from former generate_indices_T.
	unsigned ptr = idxbase;
	buffer[ptr++] = buffer[idxbase-1];
	// extra vertices base index
	unsigned evb = gcm.resolution_vbo*gcm.resolution_vbo;
	const vector2i sz = tmp_outer.size();
	vector2i offset  = gcm.clamp(tmp_outer.bl - vboarea.bl + dataoffset);
	vector2i offset2 = gcm.clamp(tmp_outer.tr - vboarea.bl + dataoffset);

	// bottom edge
	buffer[ptr++] = evb+2;
	buffer[ptr++] = offset2.x + gcm.resolution_vbo*offset.y;
	for (int i = 1; i < sz.x; ++i) {
		buffer[ptr++] = evb+1;
		buffer[ptr++] = gcm.mod(offset2.x-i) + gcm.resolution_vbo*offset.y;
	}
	// left edge
	buffer[ptr++] = evb+0;
	buffer[ptr++] = offset.x + gcm.resolution_vbo*offset.y;
	for (int i = 1; i < sz.x; ++i) {
		buffer[ptr++] = evb+7;
		buffer[ptr++] = offset.x + gcm.resolution_vbo*gcm.mod(offset.y+i);
	}
	// top edge
	buffer[ptr++] = evb+6;
	buffer[ptr++] = offset.x + gcm.resolution_vbo*offset2.y;
	for (int i = 1; i < sz.x; ++i) {
		buffer[ptr++] = evb+5;
		buffer[ptr++] = gcm.mod(offset.x+i) + gcm.resolution_vbo*offset2.y;
	}
	// right edge
	buffer[ptr++] = evb+4;
	buffer[ptr++] = offset2.x + gcm.resolution_vbo*offset2.y;
	for (int i = 1; i < sz.x; ++i) {
		buffer[ptr++] = evb+3;
		buffer[ptr++] = offset2.x + gcm.resolution_vbo*gcm.mod(offset2.y-i);
	}
	// final vertex to close it
	buffer[ptr++] = evb+2;

	return ptr - idxbase;
}



void geoclipmap::level::display(const frustum& f) const
{
	//glColor4f(1,index/8.0,0,1);//fixme test
	glColor4f(0,1,0,1);//fixme test

	vector2i outszi = tmp_outer.size();
	vector2f outsz = vector2f(outszi.x - 1, outszi.y - 1) * 0.5f;
	gcm.myshader.set_uniform(gcm.loc_xysize2, outsz);
	gcm.myshader.set_uniform(gcm.loc_L_l_rcp, 1.0f/L_l);

	uint32_t* indexvbo = &gcm.idxscratchbuf[0];

	// we need to compute up to four rectangular areas made up of tri-strips.
	// they need to get clipped to the viewing frustum later.
	// the four areas form the ring or torus, after clipping they are still
	// rectangles. We should build a rectangle of several columns of rectangles,
	// each with a width of 16 or 32 quads, to make best use of the post-transform
	// vertex cache.
	// So we need a function to generate indices for a rectangular patch, input
	// is area size and offset in vertex VBO for wrapping.
	unsigned nridx = 0;
	if (tmp_inner.empty()) {
		nridx = generate_indices(f, indexvbo, 0, tmp_outer.bl, tmp_outer.size(), dataoffset);
	} else {
		// 4 columns: L,U,D,R (left, up, down, right)
		// LUR
		// L R
		// LDR
		// left colum (L)
		vector2i patchsz(tmp_inner.bl.x - tmp_outer.bl.x + 1, tmp_outer.tr.y - tmp_outer.bl.y + 1);
		vector2i off = tmp_outer.bl;
		nridx = generate_indices(f, indexvbo, nridx, off, patchsz, dataoffset);
		// lower/down column (D)
		patchsz.x = tmp_inner.tr.x - tmp_inner.bl.x + 1;
		patchsz.y = tmp_inner.bl.y - tmp_outer.bl.y + 1;
		vector2i patchoff(tmp_inner.bl.x - tmp_outer.bl.x, 0);
		off.x += patchoff.x;
		nridx += generate_indices(f, indexvbo, nridx, off, patchsz, gcm.clamp(dataoffset + patchoff));
		// upper column (U)
		patchsz.y = tmp_outer.tr.y - tmp_inner.tr.y + 1;
		patchoff.y = tmp_inner.tr.y - tmp_outer.bl.y;
		off.y += patchoff.y;
		nridx += generate_indices(f, indexvbo, nridx, off, patchsz, gcm.clamp(dataoffset + patchoff));
		// right column (R)
		patchsz.x = tmp_outer.tr.x - tmp_inner.tr.x + 1;
		patchsz.y = tmp_outer.tr.y - tmp_outer.bl.y + 1;
		off.x -= patchoff.x;
		off.y -= patchoff.y;
		patchoff.x = tmp_inner.tr.x - tmp_outer.bl.x;
		patchoff.y = 0;
		off.x += patchoff.x;
		off.y += patchoff.y;
		nridx += generate_indices(f, indexvbo, nridx, off, patchsz, gcm.clamp(dataoffset + patchoff));
	}
	// add t-junction triangles - they are never clipped against viewing frustum, but there are only few
	nridx += generate_indices_T(indexvbo, nridx);

	// add horizon gap triangles if needed
	if (outmost) {
		nridx += generate_indices_horizgap(indexvbo, nridx);
	}

	indices.init_sub_data(0, nridx*4, &gcm.idxscratchbuf[0]);

	// render the data
	if (nridx < 4) return; // first index is remove always, and we need at least 3 for one triangle
	vertices.bind();
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, geoclipmap_fperv*4, (float*)0 + 0);
	glVertexAttribPointer(gcm.myshader_vattr_z_c_index, 1, GL_FLOAT, GL_FALSE, geoclipmap_fperv*4, (float*)0 + 3);
	glEnableVertexAttribArray(gcm.myshader_vattr_z_c_index);
	vertices.unbind();
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	indices.bind();
	// we always skip the first index here, because it is identical to the second,
	// that is because of the line/patch transition code (it is easier)
	glDrawRangeElements(GL_TRIANGLE_STRIP,
			    0/*min_vertex_index*/,
			    gcm.resolution_vbo*gcm.resolution_vbo-1/*max_vertex_index*/,
			    nridx-1, GL_UNSIGNED_INT, (unsigned*)0 + 1); // skip first index
	indices.unbind();
	glDisableVertexAttribArray(gcm.myshader_vattr_z_c_index);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
}


#endif












#ifdef GEOCLIPMAPTEST
	//height_generator_test hgt;
	//height_generator_test2 hgt;
#if 1
	std::vector<Uint8> heights;
	std::vector<Uint16> heights16;
#if 0
	float hm = 0.75, ha = -80.0f;
	{
		perlinnoise pn(64, 4, 6, true); // max. 8192
		const unsigned s2 = 256*16;
		heights = pn.values(0, 0, s2, s2, 6);
#if 1
		const unsigned height_segments = 10;
		const float total_height = 128.0;
		const float terrace_height = total_height / height_segments;
		lookup_function<float, 256U> asin_lookup;
		for (unsigned i = 0; i <= 256; ++i)
			asin_lookup.set_value(i, asin(float(i)/256) / M_PI + 0.5);
		for (unsigned y = 0; y < s2; ++y) {
			for (unsigned x = 0; x < s2; ++x) {
				float f = heights[y*s2+x];
				unsigned t = unsigned(floor(f / terrace_height));
				float f_frac = f / terrace_height - t;
				float f2 = f_frac * 2.0 - 1.0; // be in -1...1 range
				// skip this for softer hills (x^3 = more steep walls)
				//f2 = f2 * f2 * f2;
				f2 = asin_lookup.value(f2);
				heights[y*s2+x] = Uint8((t + f2) * terrace_height);
			}
		}
#endif		
	}
#else
	float hm = 0.2, ha = -40.0f;
	{
		// set heights from a file
		unsigned s2 = 1024;
#if 0
		sdl_image si("./heights2.png");
		if (si->w != s2 || si->h != s2)
			throw error("invalid pic size");
		si.lock();
		Uint8* px = (Uint8*) si->pixels;
		heights.resize(s2*s2);
		for (unsigned y = 0; y < s2; ++y) {
			for (unsigned x = 0; x < s2; ++x) {
				heights[y*s2+x] = px[y*si->pitch+x];
			}
		}
		si.unlock();
#else
		std::ifstream is("test.tif.raw");
 		heights16.resize(s2*s2);
		is.read((char*)&heights16[0], s2*s2*2);
// 		sdl_image si("./heights2rg.png");
// 		if (si->w != s2 || si->h != s2)
// 			throw error("invalid pic size");
// 		si.lock();
// 		Uint8* px = (Uint8*) si->pixels;
// 		for (unsigned y = 0; y < s2; ++y) {
// 			for (unsigned x = 0; x < s2; ++x) {
// 				heights16[y*s2+x] = Uint16(px[y*si->pitch+3*x])*256+px[y*si->pitch+3*x+1];
// 			}
// 		}
// 		si.unlock();
#endif
	}
#endif
	//height_generator_test3 hgt(heights, 12, hm, ha);
	height_generator_test3 hgt(heights16, 10, 1.0/256, -64);
	heights.clear();
#endif
	//height_generator_test4 hgt;
	//height_generator_test5 hgt;
	// total area covered = 2^(levels-1) * L * N
	// 8, 7, 1.0 gives 2^14m = 16384m
	geoclipmap gcm(7, 8/*8*/ /* 2^x=N */, 1.0, hgt);
	//gcm.set_viewerpos(vector3(0, 0, 30.0));
#endif
#if 0
	{
		// 2^5 * 256 detail is enough = 8192
		// with 1 value per meter -> repeat every 8192m, far enough
		perlinnoise pn(64, 4, 6, true); // max. 8192
		//const unsigned s = 256*256;
		const unsigned s2 = 256*16;
		//const unsigned s3 = 32;
		const unsigned height_segments = 10;
		const float total_height = 256.0;
		const float terrace_height = total_height / height_segments;
		std::vector<Uint8> heights = pn.values(0, 0, s2, s2, 6);
		lookup_function<float, 256U> asin_lookup;
		for (unsigned i = 0; i <= 256; ++i)
			asin_lookup.set_value(i, asin(float(i)/256) / M_PI + 0.5);
#if 1
		for (unsigned y = 0; y < s2; ++y) {
			for (unsigned x = 0; x < s2; ++x) {
				float f = heights[y*s2+x];
				unsigned t = unsigned(floor(f / terrace_height));
				float f_frac = f / terrace_height - t;
				float f2 = f_frac * 2.0 - 1.0; // be in -1...1 range
				// skip this for softer hills (x^3 = more steep walls)
				f2 = f2 * f2 * f2;
				f2 = asin_lookup.value(f2);
				heights[y*s2+x] = Uint8((t + f2) * terrace_height);
			}
		}
#endif
		save_pgm("pntest.pgm", s2, s2, &heights[0]);

#if 0
		heights = pn.values(0, 0, s2/2, s2/2, 5);
		for (unsigned y = 0; y < s2/2; ++y) {
			for (unsigned x = 0; x < s2/2; ++x) {
				float f = heights[y*s2/2+x];
				unsigned t = unsigned(floor(f / terrace_height));
				float f_frac = f / terrace_height - t;
				float f2 = f_frac * 2.0 - 1.0; // be in -1...1 range
				// skip this for softer hills (x^3 = more steep walls)
				f2 = f2 * f2 * f2;
				f2 = asin_lookup.value(f2);
				heights[y*s2/2+x] = Uint8((t + f2) * terrace_height);
			}
		}
		save_pgm("pntest2.pgm", s2/2, s2/2, &heights[0]);
#endif
	}
#endif








#ifdef GEOCLIPMAPTEST
		gcm.set_viewerpos(campos);
		glColor4f(1,0,0,1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);

		matrix4 mv = matrix4::get_gl(GL_MODELVIEW_MATRIX);
		matrix4 prj = matrix4::get_gl(GL_PROJECTION_MATRIX);
		matrix4 mvp = prj * mv;
		matrix4 invmvp = mvp.inverse();
		vector3 wbln = invmvp * vector3(-1,-1,-1);
		vector3 wbrn = invmvp * vector3(+1,-1,-1);
		vector3 wtln = invmvp * vector3(-1,+1,-1);
		vector3 wtrn = invmvp * vector3(+1,+1,-1);
		polygon viewwindow(wbln, wbrn, wtrn, wtln);
		// hmm wouldn't be znear the distance of point (0,0,0) to the viewindow?
		// that is the distance of the plane through that window to 0?
		// if we do a scalar product of any point with the normal, we get the direction
		// (fabs it, or negate?)
		vector3 wn = (wbrn - wbln).cross(wtln - wbln).normal();
		double neardist = fabs((wbln - campos) * wn);
		//log_debug("neardist="<<neardist);//gives 1.0, seems correct?
		frustum viewfrustum(viewwindow, campos, cm.view_dir(), 0.1 /* fixme: read from matrix! */);

		gcm.display(viewfrustum);
#endif









class system* mysys;
int res_x, res_y;

void run();

font* font_arial;

texture* metalbackgr;
texture* woodbackgr;
texture* terraintex;

texture* reliefwall_diffuse;
texture* reliefwall_bump;
texture* stonewall_diffuse;
texture* stonewall_bump;
glsl_shader_setup* glsl_reliefmapping;
int loc_tex_color;
int loc_tex_normal;
int vertex_attrib_index;
int loc_depth_factor;

#define LVL_X 13
#define LVL_Y 13
#define LVL_Z 3
const char* level[3][13] = {
	{
		"xxxxxxxxxxxxx",
		"x x     x   x",
		"x x xxx x x x",
		"x   x x x x x",
		"xxx x   x x x",
		"x x x x x x x",
		"x         x x",
		"x   x x xxx x",
		"xxxxx x x   x",
		"x     x x xxx",
		"x  x  xxx   x",
		"x           x",
		"xxxxxxxxxxxxx"
	},
	{
		"xxxxxxxxxxxxx",
		"x xxxxxxxxxxx",
		"xxxxxxxxxxxxx",
		"xxxxxxxxxxx x",
		"xxxxxxxxxxxxx",
		"xxx xxxxxxxxx",
		"xxxxxxxxxxxxx",
		"xxxxxxxxxxxxx",
		"xxxxxxxxxxxxx",
		"xxx xxxx xxxx",
		"xxxxxxxxxxxxx",
		"xxxxxxxxxxx x",
		"xxxxxxxxxxxxx"
	},
	{
		"xxxxxxxxxxxxx",
		"x       x   x",
		"xxxxxxx x x x",
		"x     x x x x",
		"x xxx x x   x",
		"x   x   x x x",
		"xxx xxxxx   x",
		"x   x x   x x",
		"x xxx x x   x",
		"x x   x x x x",
		"x xxx xxx x x",
		"x           x",
		"xxxxxxxxxxxxx"
	},
};
unsigned level_at(int x, int y, int z)
{
	if (x < 0 || x >= LVL_X) return 1;
	if (y < 0 || y >= LVL_Y) return 1;
	if (z < 0 || z >= LVL_Z) return 1;
	return level[z][LVL_Y-1-y][x] == 'x' ? 1 : 0;
}


int mymain(list<string>& args)
{
	// report critical errors (on Unix/Posix systems)
	install_segfault_handler();

	// randomize
	srand(time(0));

	// command line argument parsing
	res_x = 1024;
	bool fullscreen = true;

	// parse commandline
	for (list<string>::iterator it = args.begin(); it != args.end(); ++it) {
		if (*it == "--help") {
			cout << "*** Danger from the Deep ***\nusage:\n--help\t\tshow this\n"
			<< "--res n\t\tuse resolution n horizontal,\n\t\tn is 512,640,800,1024 (recommended) or 1280\n"
			<< "--nofullscreen\tdon't use fullscreen\n"
			<< "--debug\t\tdebug mode: no fullscreen, resolution 800\n"
			<< "--editor\trun mission editor directly\n"
			<< "--mission fn\trun mission from file fn (just the filename in the mission directory)\n"
			<< "--nosound\tdon't use sound\n";
			return 0;
		} else if (*it == "--nofullscreen") {
			fullscreen = false;
		} else if (*it == "--debug") {
			fullscreen = false;
			res_x = 800;
		} else if (*it == "--res") {
			list<string>::iterator it2 = it; ++it2;
			if (it2 != args.end()) {
				int r = atoi(it2->c_str());
				if (r==512||r==640||r==800||r==1024||r==1280)
					res_x = r;
				++it;
			}
		}
	}

	// fixme: also allow 1280x1024, set up gl viewport for 4:3 display
	// with black borders at top/bottom (height 2*32pixels)
	res_y = res_x*3/4;
	// weather conditions and earth curvature allow 30km sight at maximum.
	mysys = new class system(1.0, 30000.0+500.0, res_x, res_y, fullscreen);
	mysys->set_res_2d(1024, 768);
//	mysys->set_max_fps(60);
	
	log_info("Danger from the Deep");

	GLfloat lambient[4] = {0.3,0.3,0.3,1};
	GLfloat ldiffuse[4] = {1,1,1,1};
	GLfloat lposition[4] = {0,0,1,0};
	glLightfv(GL_LIGHT0, GL_AMBIENT, lambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, ldiffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, lposition);
	glEnable(GL_LIGHT0);

 	font_arial = new font(get_font_dir() + "font_arial");
 	mysys->draw_console_with(font_arial, 0);

	run();

 	delete font_arial;
	delete mysys;

	return 0;
}



class portal
{
public:
	polygon shape;
	class sector* adj_sector;
	// bool mirror;
	portal(polygon shp, class sector* adsc)	: shape(shp), adj_sector(adsc) {}
};



class sector
{
public:
	//model geometry;
	vector3 basepos;
	unsigned walls;
	std::vector<portal> portals;
	mutable bool displayed;
	mutable bool visited;	// needed during rendering.
	sector() : walls(0), displayed(false), visited(false) {}
	void display(const frustum& f) const;
	bool check_movement(const vector3& currpos, const vector3& nextpos, sector*& nextseg) const;
};



void sector::display(const frustum& f) const
{
	visited = true;
	if (!displayed) {
		glColor3f(1,1,1);
		glsl_reliefmapping->use();
		glsl_reliefmapping->set_gl_texture(*stonewall_diffuse, loc_tex_color, 0);
		glsl_reliefmapping->set_gl_texture(*stonewall_bump, loc_tex_normal, 1);
		glsl_reliefmapping->set_uniform(loc_depth_factor, float(0.05));
		if (walls & 1) {
			glBegin(GL_QUADS);
			glTexCoord2f(0,0);
			glNormal3f(0,-1,0);
			glVertexAttrib3f(vertex_attrib_index, 1,0,0);
			glVertex3f(basepos.x, basepos.y+1, basepos.z);
			glTexCoord2f(1,0);
			glNormal3f(0,-1,0);
			glVertexAttrib3f(vertex_attrib_index, 1,0,0);
			glVertex3f(basepos.x+1, basepos.y+1, basepos.z);
			glTexCoord2f(1,1);
			glNormal3f(0,-1,0);
			glVertexAttrib3f(vertex_attrib_index, 1,0,0);
			glVertex3f(basepos.x+1, basepos.y+1, basepos.z+1);
			glTexCoord2f(0,1);
			glNormal3f(0,-1,0);
			glVertexAttrib3f(vertex_attrib_index, 1,0,0);
			glVertex3f(basepos.x, basepos.y+1, basepos.z+1);
			glEnd();
		}
		glsl_reliefmapping->set_gl_texture(*reliefwall_diffuse, loc_tex_color, 0);
		glsl_reliefmapping->set_gl_texture(*reliefwall_bump, loc_tex_normal, 1);
		if (walls & 2) {
			glBegin(GL_QUADS);
			glTexCoord2f(0,0);
			glNormal3f(0,1,0);
			glVertexAttrib3f(vertex_attrib_index, -1,0,0);
			glVertex3f(basepos.x+1, basepos.y, basepos.z);
			glTexCoord2f(1,0);
			glNormal3f(0,1,0);
			glVertexAttrib3f(vertex_attrib_index, -1,0,0);
			glVertex3f(basepos.x, basepos.y, basepos.z);
			glTexCoord2f(1,1);
			glNormal3f(0,1,0);
			glVertexAttrib3f(vertex_attrib_index, -1,0,0);
			glVertex3f(basepos.x, basepos.y, basepos.z+1);
			glTexCoord2f(0,1);
			glNormal3f(0,1,0);
			glVertexAttrib3f(vertex_attrib_index, -1,0,0);
			glVertex3f(basepos.x+1, basepos.y, basepos.z+1);
			glEnd();
		}
		if (walls & 4) {
			glBegin(GL_QUADS);
			glTexCoord2f(0,0);
			glNormal3f(1,0,0);
			glVertexAttrib3f(vertex_attrib_index, 0,1,0);
			glVertex3f(basepos.x, basepos.y, basepos.z);
			glTexCoord2f(1,0);
			glNormal3f(1,0,0);
			glVertexAttrib3f(vertex_attrib_index, 0,1,0);
			glVertex3f(basepos.x, basepos.y+1, basepos.z);
			glTexCoord2f(1,1);
			glNormal3f(1,0,0);
			glVertexAttrib3f(vertex_attrib_index, 0,1,0);
			glVertex3f(basepos.x, basepos.y+1, basepos.z+1);
			glTexCoord2f(0,1);
			glNormal3f(1,0,0);
			glVertexAttrib3f(vertex_attrib_index, 0,1,0);
			glVertex3f(basepos.x, basepos.y, basepos.z+1);
			glEnd();
		}
		if (walls & 8) {
			glBegin(GL_QUADS);
			glTexCoord2f(0,0);
			glNormal3f(-1,0,0);
			glVertexAttrib3f(vertex_attrib_index, 0,-1,0);
			glVertex3f(basepos.x+1, basepos.y+1, basepos.z);
			glTexCoord2f(1,0);
			glNormal3f(-1,0,0);
			glVertexAttrib3f(vertex_attrib_index, 0,-1,0);
			glVertex3f(basepos.x+1, basepos.y, basepos.z);
			glTexCoord2f(1,1);
			glNormal3f(-1,0,0);
			glVertexAttrib3f(vertex_attrib_index, 0,-1,0);
			glVertex3f(basepos.x+1, basepos.y, basepos.z+1);
			glTexCoord2f(0,1);
			glNormal3f(-1,0,0);
			glVertexAttrib3f(vertex_attrib_index, 0,-1,0);
			glVertex3f(basepos.x+1, basepos.y+1, basepos.z+1);
			glEnd();
		}
		if (walls & 16) {
			//metalbackgr->set_gl_texture();
			glBegin(GL_QUADS);
			glTexCoord2f(0,0);
			glNormal3f(0,0,1);
			glVertexAttrib3f(vertex_attrib_index, 1,0,0);
			glVertex3f(basepos.x, basepos.y, basepos.z);
			glTexCoord2f(1,0);
			glNormal3f(0,0,1);
			glVertexAttrib3f(vertex_attrib_index, 1,0,0);
			glVertex3f(basepos.x+1, basepos.y, basepos.z);
			glTexCoord2f(1,1);
			glNormal3f(0,0,1);
			glVertexAttrib3f(vertex_attrib_index, 1,0,0);
			glVertex3f(basepos.x+1, basepos.y+1, basepos.z);
			glTexCoord2f(0,1);
			glNormal3f(0,0,1);
			glVertexAttrib3f(vertex_attrib_index, 1,0,0);
			glVertex3f(basepos.x, basepos.y+1, basepos.z);
			glEnd();
		}
		if (walls & 32) {
			//woodbackgr->set_gl_texture();
			glBegin(GL_QUADS);
			glTexCoord2f(0,0);
			glNormal3f(0,0,-1);
			glVertexAttrib3f(vertex_attrib_index, 1,0,0);
			glVertex3f(basepos.x+1, basepos.y, basepos.z+1);
			glTexCoord2f(1,0);
			glNormal3f(0,0,-1);
			glVertexAttrib3f(vertex_attrib_index, 1,0,0);
			glVertex3f(basepos.x, basepos.y, basepos.z+1);
			glTexCoord2f(1,1);
			glNormal3f(0,0,-1);
			glVertexAttrib3f(vertex_attrib_index, 1,0,0);
			glVertex3f(basepos.x, basepos.y+1, basepos.z+1);
			glTexCoord2f(0,1);
			glNormal3f(0,0,-1);
			glVertexAttrib3f(vertex_attrib_index, 1,0,0);
			glVertex3f(basepos.x+1, basepos.y+1, basepos.z+1);
			glEnd();
		}
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
		displayed = true;
	}

	// check for other portals
//	f.draw();
	for (unsigned i = 0; i < portals.size(); ++i) {
		const portal& p = portals[i];
		// avoid portals facing away.
		// this means viewpos must be on the inner side of the portal plane.
		// fixme: if we are too close to a portal this is a problem and leads to bugs.
		// compare distance to portal and znear.
		// if we're beyond the portal, ignore it.
		// the problem still appears when we have four sectors with portals between each other, so they form one room.
		// if we step exactly on the corner between the four sectors (or maybe the problem exists already with three),
		// sometimes nothing is drawn...
		// if we're too close to a portal, render both sectors, but avoid re-visit (or infinite loops will result).
		plane portal_plane = p.shape.get_plane();
		double dist_to_portal = portal_plane.distance(f.viewpos);
		//cout << "viewpos dist: " << portal_plane.distance(f.viewpos) << "\n";
		if (fabs(dist_to_portal) < f.znear) {
			// we're to close to the portal.
			// render both adjacent sectors with frustum f, but only if they haven't been visited.
			// we already rendered this sector, so render other sector
			if (!p.adj_sector->visited) {
				p.adj_sector->display(f);
			}
		} else if (portal_plane.is_left(f.viewpos)) {
			polygon newfpoly = f.clip(p.shape);
			if (!newfpoly.empty()) {
				frustum fnew(newfpoly, f.viewpos, f.viewdir, f.znear);
				p.adj_sector->display(fnew);
			}
		}
	}
	visited = false;
}



bool sector::check_movement(const vector3& currpos, const vector3& nextpos, sector*& nextseg) const
{
	// we assume that curros is inside this sector.
	// check for crossed portal.
	for (unsigned i = 0; i < portals.size(); ++i) {
		plane pl = portals[i].shape.get_plane();
		if (pl.test_side(nextpos) <= 0) {
			// we crossed the plane of that portal, switch sector.
			nextseg = portals[i].adj_sector;
			return false;
		}
	}
	// the value needs to be greater than the largest distance between the viewer's position and one of the
	// corner points of the near rectangle of the viewing frustum. assume sqrt(3*znear^2) or so...
	// znear=0.1 -> dist2wall = 0.173...
	const double DIST2WALL = 0.175;
	nextseg = 0;
	if (nextpos.x < basepos.x + DIST2WALL && (walls & 4)) return false;
	if (nextpos.x > basepos.x + 1 - DIST2WALL && (walls & 8)) return false;
	if (nextpos.y < basepos.y + DIST2WALL && (walls & 2)) return false;
	if (nextpos.y > basepos.y + 1 - DIST2WALL && (walls & 1)) return false;
	if (nextpos.z < basepos.z + DIST2WALL && (walls & 16)) return false;
	if (nextpos.z > basepos.z + 1 - DIST2WALL && (walls & 32)) return false;
	return true;
}



void line(const vector3& a, const vector3& b)
{
	glVertex3dv(&a.x);
	glVertex3dv(&b.x);
}



void run()
{
	/* 3d portal rendering:
	   define frustum as list of planes (4 at begin, depends on FOV etc, maybe get plane
	   equations from projection matrix etc.
	   clip portal against frustum by clipping the portal polygon against all frustum planes
	   sequentially.
	   Avoid portals that are facing away.
	   Resulting polygon is either empty or valid.
	   Construct new frustum by making planes from points of polygon and camera position.
	   Continue...
	   mark each sector as rendered when you render it (could be visible through > 1 portals).
	   avoid rerender. clear all tags before rendering.
	*/

	metalbackgr = new texture(get_texture_dir() + "foam.png", texture::LINEAR_MIPMAP_LINEAR);
	woodbackgr = new texture(get_texture_dir() + "wooden_desk.png", texture::LINEAR_MIPMAP_LINEAR);
	terraintex = new texture(get_texture_dir() + "terrain.jpg", texture::LINEAR_MIPMAP_LINEAR);

	stonewall_diffuse = new texture(get_texture_dir() + "stonewall_diffuse.jpg", texture::LINEAR_MIPMAP_LINEAR);
	stonewall_bump = new texture(get_texture_dir() + "stonewall_bump.png", texture::LINEAR_MIPMAP_LINEAR);
	reliefwall_diffuse = new texture(get_texture_dir() + "reliefwall_diffuse.jpg", texture::LINEAR_MIPMAP_LINEAR);
	reliefwall_bump = new texture(get_texture_dir() + "reliefwall_bump.png", texture::LINEAR_MIPMAP_LINEAR);
	glsl_reliefmapping = new glsl_shader_setup(get_shader_dir() + "reliefmapping.vshader",
						   get_shader_dir() + "reliefmapping.fshader");
	glsl_reliefmapping->use();
	loc_tex_normal = glsl_reliefmapping->get_uniform_location("tex_normal");
	loc_tex_color = glsl_reliefmapping->get_uniform_location("tex_color");
	vertex_attrib_index = glsl_reliefmapping->get_vertex_attrib_index("tangentx");
	loc_depth_factor = glsl_reliefmapping->get_uniform_location("depth_factor");
	glsl_shader_setup::use_fixed();

	vector<sector> sectors(LVL_X * LVL_Y * LVL_Z);
	for (int z = 0; z < LVL_Z; ++z) {
		for (int y = 0; y < LVL_Y; ++y) {
			for (int x = 0; x < LVL_X; ++x) {
				if (level_at(x, y, z) == 0) {
					// create sector
					sector& s = sectors[x+LVL_X*(y+LVL_Y*z)];
					s.basepos = vector3(x, y, z);
					s.walls = 0x00;
					polygon pup   (s.basepos+vector3(0,1,0), s.basepos+vector3(1,1,0), s.basepos+vector3(1,1,1), s.basepos+vector3(0,1,1));
					polygon pleft (s.basepos+vector3(0,0,0), s.basepos+vector3(0,1,0), s.basepos+vector3(0,1,1), s.basepos+vector3(0,0,1));
					polygon pright(s.basepos+vector3(1,1,0), s.basepos+vector3(1,0,0), s.basepos+vector3(1,0,1), s.basepos+vector3(1,1,1));
					polygon pdown (s.basepos+vector3(1,0,0), s.basepos+vector3(0,0,0), s.basepos+vector3(0,0,1), s.basepos+vector3(1,0,1));
					polygon ptop  (s.basepos+vector3(1,0,1), s.basepos+vector3(0,0,1), s.basepos+vector3(0,1,1), s.basepos+vector3(1,1,1));
					polygon pbott (s.basepos+vector3(0,0,0), s.basepos+vector3(1,0,0), s.basepos+vector3(1,1,0), s.basepos+vector3(0,1,0));
					// look for adjacent sectors, create portals to them
					if (level_at(x, y+1, z) == 0)
						s.portals.push_back(portal(pup, &sectors[x+LVL_X*(y+1+LVL_Y*z)]));
					else
						s.walls |= 1;
					if (level_at(x, y-1, z) == 0)
						s.portals.push_back(portal(pdown, &sectors[x+LVL_X*(y-1+LVL_Y*z)]));
					else
						s.walls |= 2;
					if (level_at(x-1, y, z) == 0)
						s.portals.push_back(portal(pleft, &sectors[x-1+LVL_X*(y+LVL_Y*z)]));
					else
						s.walls |= 4;
					if (level_at(x+1, y, z) == 0)
						s.portals.push_back(portal(pright, &sectors[x+1+LVL_X*(y+LVL_Y*z)]));
					else
						s.walls |= 8;
					if (level_at(x, y, z-1) == 0)
						s.portals.push_back(portal(pbott, &sectors[x+LVL_X*(y+LVL_Y*(z-1))]));
					else
						s.walls |= 16;
					if (level_at(x, y, z+1) == 0)
						s.portals.push_back(portal(ptop, &sectors[x+LVL_X*(y+LVL_Y*(z+1))]));
					else
						s.walls |= 32;
				}
			}
		}
	}

	sector* currsector = &sectors[1+LVL_X*(1+LVL_Y*0)];
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	sys().gl_perspective_fovx(70, 4.0/3.0, 0.1, 1000);
	glMatrixMode(GL_MODELVIEW);

	glDisable(GL_LIGHTING);

	vector3 viewangles(0, 0, 0);
	vector3 pos(1.5, 1.5, 0.3);

	double tm0 = sys().millisec();
	int mv_forward = 0, mv_upward = 0, mv_sideward = 0;

	fpsmeasure fpsm(1.0f);

	while (true) {
		double tm1 = sys().millisec();
		double delta_t = tm1 - tm0;
		tm0 = tm1;

		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		glBindTexture(GL_TEXTURE_2D, 0);

		// compute mvp etc. for user
		glLoadIdentity();
		// make camera look to pos. y-axis.
		glRotatef(-90, 1, 0, 0);

		glRotatef(-viewangles.x, 1, 0, 0);
		glRotatef(-viewangles.y, 0, 1, 0);
		glRotatef(-viewangles.z, 0, 0, 1);
		matrix4 mvr = matrix4::get_gl(GL_MODELVIEW_MATRIX);
		glTranslated(-pos.x, -pos.y, -pos.z);
		matrix4 mv = matrix4::get_gl(GL_MODELVIEW_MATRIX);
		matrix4 prj = matrix4::get_gl(GL_PROJECTION_MATRIX);
		matrix4 mvp = prj * mv;
		matrix4 invmv = mv.inverse();
		matrix4 invmvr = mvr.inverse();
		matrix4 invmvp = mvp.inverse();
		vector3 wbln = invmvp * vector3(-1,-1,-1);
		vector3 wbrn = invmvp * vector3(+1,-1,-1);
		vector3 wtln = invmvp * vector3(-1,+1,-1);
		vector3 wtrn = invmvp * vector3(+1,+1,-1);
		vector3 vd = invmvr * vector3(0,0,-1);
		polygon viewwindow(wbln, wbrn, wtrn, wtln);
		frustum viewfrustum(viewwindow, pos, vd, 0.1 /* fixme: read from matrix! */);

		// set light
		vector3 ld(cos((sys().millisec()%10000)*2*3.14159/10000), sin((sys().millisec()%10000)*2*3.14159/10000), 1.0);
		ld.normalize();
		GLfloat lposition[4] = {ld.x,ld.y,ld.z,0};
		glLightfv(GL_LIGHT0, GL_POSITION, lposition);

		// render sectors.
		for (unsigned i = 0; i < sectors.size(); ++i)
			sectors[i].displayed = false;
		currsector->display(viewfrustum);
		glsl_shader_setup::use_fixed();

		vector3 oldpos = pos;
		const double movesc = 0.25;
		list<SDL_Event> events = mysys->poll_event_queue();
		vector3 forward = -invmvr.column3(2) * movesc;
		vector3 upward = invmvr.column3(1) * movesc;
		vector3 sideward = invmvr.column3(0) * movesc;
		for (list<SDL_Event>::iterator it = events.begin(); it != events.end(); ++it) {
			SDL_Event& event = *it;
			if (event.type == SDL_KEYDOWN) {
				switch (event.key.keysym.sym) {
				case SDLK_ESCAPE:
					return;
				case SDLK_KP4: mv_sideward = -1; break;
				case SDLK_KP6: mv_sideward = 1; break;
				case SDLK_KP8: mv_upward = 1; break;
				case SDLK_KP2: mv_upward = -1; break;
				case SDLK_KP1: mv_forward = 1; break;
				case SDLK_KP3: mv_forward = -1; break;
				default: break;
				}
			} else if (event.type == SDL_KEYUP) {
				switch (event.key.keysym.sym) {
				case SDLK_KP4: mv_sideward = 0; break;
				case SDLK_KP6: mv_sideward = 0; break;
				case SDLK_KP8: mv_upward = 0; break;
				case SDLK_KP2: mv_upward = 0; break;
				case SDLK_KP1: mv_forward = 0; break;
				case SDLK_KP3: mv_forward = 0; break;
				default: break;
				}
			} else if (event.type == SDL_MOUSEMOTION) {
				if (event.motion.state & SDL_BUTTON_LMASK) {
					viewangles.z -= event.motion.xrel * 0.5;
					viewangles.x -= event.motion.yrel * 0.5;
				} else if (event.motion.state & SDL_BUTTON_RMASK) {
					viewangles.y += event.motion.xrel * 0.5;
// 				} else if (event.motion.state & SDL_BUTTON_MMASK) {
// 					pos.x += event.motion.xrel * 0.05;
// 					pos.y += event.motion.yrel * 0.05;
				}
			} else if (event.type == SDL_MOUSEBUTTONDOWN) {
// 				if (event.button.button == SDL_BUTTON_WHEELUP) {
// 					pos.z -= movesc;
// 				} else if (event.button.button == SDL_BUTTON_WHEELDOWN) {
// 					pos.z += movesc;
// 				}
			}
		}
		const double move_speed = 0.003;
		pos += forward * mv_forward * delta_t * move_speed
			+ sideward * mv_sideward * delta_t * move_speed
			+ upward * mv_upward * delta_t * move_speed;

		// check for sector switch by movement
		sector* seg = 0;
		bool movementok = currsector->check_movement(oldpos, pos, seg);
		if (!movementok) {
			if (seg) {
				// switch sector
				currsector = seg;
			} else {
				// avoid movement
				pos = oldpos;
			}
		}

		// record fps
		float fps = fpsm.account_frame();

		sys().prepare_2d_drawing();
		std::ostringstream oss; oss << "FPS: " << fps << "\n(all time total " << fpsm.get_total_fps() << ")";
		font_arial->print(0, 0, oss.str());
		sys().unprepare_2d_drawing();
		
		mysys->swap_buffers();
	}

	delete glsl_reliefmapping;
	delete reliefwall_bump;
	delete reliefwall_diffuse;
	delete stonewall_bump;
	delete stonewall_diffuse;

	delete metalbackgr;
	delete woodbackgr;
	delete terraintex;
}
