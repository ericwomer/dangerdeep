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

#include "rastered_map.h"
#include "system.h"
#include "vector3.h"
#include "model.h"
#include "texture.h"
#include "image.h"
#include "faulthandler.h"
#include "datadirs.h"
#include "frustum.h"
#include "shader.h"
#include "font.h"
#include "fpsmeasure.h"
#include "log.h"
#include "mymain.cpp"
#include "height_generator.h"
#include "perlinnoise.h"
#include "global_data.h"
//#include "sky.h"
#include "geoclipmap.h"
#include "fpsmeasure.h"
#include "bspline.h"
#include "angle.h"
#include <time.h>
#include <iostream>
#include <sstream>


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


class height_generator_test1 : public height_generator
{
public:
	height_generator_test1() : height_generator(1.0) {}
	void compute_heights(int detail, const vector2i& coord_bl,
			     const vector2i& coord_sz, float* dest, unsigned stride = 0,
			     unsigned line_stride = 0) {
		if (!stride) stride = 1;
		if (!line_stride) line_stride = coord_sz.x * stride;
		for (int y = 0; y < coord_sz.y; ++y) {
			float* dest2 = dest;
			for (int x = 0; x < coord_sz.x; ++x) {
				vector2i coord = coord_bl + vector2i(x, y);
				if (detail >= 0) {
					int xc = coord.x * int(1 << detail);
					int yc = coord.y * int(1 << detail);
					*dest2 = sin(2*3.14159*xc*0.01) * sin(2*3.14159*yc*0.01) * 20.0 - 20;
				} else {
					float xc = coord.x / float(1 << -detail);
					float yc = coord.y / float(1 << -detail);
					*dest2 = sin(2*3.14159*xc*0.01) * sin(2*3.14159*yc*0.01) * 20.0 - 20;
				}
				dest2 += stride;
			}
			dest += line_stride;
		}
	}

	void get_min_max_height(double& minh, double& maxh) const { minh = -40.0; maxh = 0.0; }
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

	std::vector<uint8_t> ct[8];
	unsigned cw, ch;
public:
	height_generator_test2()
		: height_generator(1.0, 2),
		  pn(64, 4, 6, true), s2(256*16), height_segments(10),
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
		const char* texnames[8] = {
			"tex_grass.jpg",
			"tex_grass2.jpg",
			"tex_grass3.jpg",
			"tex_grass4.jpg",
			"tex_grass5.jpg",
			"tex_mud.jpg",
			"tex_stone.jpg",
			"tex_sand.jpg"
		};
		for (unsigned i = 0; i < 8; ++i) {
			sdl_image tmp(get_texture_dir() + texnames[i]);
			unsigned bpp = 0;
			ct[i] = tmp.get_plain_data(cw, ch, bpp);
			if (bpp != 3) throw error("color bpp != 3");
		}
	}
	void compute_heights(int detail, const vector2i& coord_bl,
			     const vector2i& coord_sz, float* dest, unsigned stride = 0,
			     unsigned line_stride = 0) {
		if (!stride) stride = 1;
		if (!line_stride) line_stride = coord_sz.x * stride;
		for (int y = 0; y < coord_sz.y; ++y) {
			float* dest2 = dest;
			for (int x = 0; x < coord_sz.x; ++x) {
				vector2i coord = coord_bl + vector2i(x, y);
				if (detail >= 0) {
					int xc = coord.x * int(1 << detail);
					int yc = coord.y * int(1 << detail);
					if (detail <= 6) {
						float h = pn.value(xc, yc, 6-detail)/255.0f;
						*dest2 = -130 + h*h*h*0.5 * 256;
					} else
						*dest2 = -130;
				} else {
					int xc = coord.x >> -detail;
					int yc = coord.y >> -detail;
					//fixme: too crude, need to bilinear sample height from 4 base values
					//fixme: replace compute_height!
					//generate result for detail=0, and upscale it
					//-detail times with bilinear sampling.
					//probably needs more values in base level to
					//upsample correctly.
					float baseh = compute_height(0, vector2i(xc, yc));
					baseh += extrah[(coord.y&63)*64+(coord.x&63)] * 0.25;
					*dest2 = baseh;
				}
				dest2 += stride;
			}
			dest += line_stride;
		}
	}

	float compute_height(int detail, const vector2i& coord) {
		if (detail >= 0) {
			int xc = coord.x * int(1 << detail);
			int yc = coord.y * int(1 << detail);
			if (detail <= 6) {
				float h = pn.value(xc, yc, 6-detail)/255.0f;
				return -130 + h*h*h*0.5 * 256;
			} else
				return -130;
		} else {
			int xc = coord.x >> -detail;
			int yc = coord.y >> -detail;
			//fixme: too crude, need to bilinear sample height from 4 base values
			float baseh = compute_height(0, vector2i(xc, yc));
			baseh += extrah[(coord.y&63)*64+(coord.x&63)] * 0.25;
			return baseh;
		}
	}
	void compute_colors(int detail, const vector2i& coord_bl,
			    const vector2i& coord_sz, Uint8* dest) {
		for (int y = 0; y < coord_sz.y; ++y) {
			for (int x = 0; x < coord_sz.x; ++x) {
				vector2i coord = coord_bl + vector2i(x, y);
				color c;
				if (detail >= -2) {
					unsigned xc = coord.x << (detail+2);
					unsigned yc = coord.y << (detail+2);
					unsigned xc2,yc2;
					if (detail >= 0) {
						xc2 = coord.x << detail;
						yc2 = coord.y << detail;
					} else {
						xc2 = coord.x >> -detail;
						yc2 = coord.y >> -detail;
					}
					//fixme: replace compute_height!
					float z = compute_height(0/*detail*/, vector2i(xc2,yc2));//coord);
					float zif = (z + 130) * 4 * 8 / 256;
					if (zif < 0.0) zif = 0.0;
					if (zif >= 7.0) zif = 6.999;
					unsigned zi = unsigned(zif);
					zif = myfrac(zif);
					//if (zi <= 4) zi = ((xc/256)*(xc/256)*3+(yc/256)*(yc/256)*(yc/256)*2+(xc/256)*(yc/256)*7)%5;
					unsigned i = ((yc&(ch-1))*cw+(xc&(cw-1)));
					float zif2 = 1.0-zif;
					c = color(uint8_t(ct[zi][3*i]*zif2 + ct[zi+1][3*i]*zif),
						  uint8_t(ct[zi][3*i+1]*zif2 + ct[zi+1][3*i+1]*zif),
						  uint8_t(ct[zi][3*i+2]*zif2 + ct[zi+1][3*i+2]*zif));
				} else {
					unsigned xc = coord.x >> (-(detail+2));
					unsigned yc = coord.y >> (-(detail+2));
					//fixme: replace compute_height!
					float z = compute_height(0, vector2i(coord.x>>-detail,coord.y>>-detail));
					float zif = (z + 130) * 4 * 8 / 256;
					if (zif < 0.0) zif = 0.0;
					if (zif >= 7.0) zif = 6.999;
					unsigned zi = unsigned(zif);
					zif = myfrac(zif);
					//if (zi <= 4) zi = ((xc/256)*(xc/256)*3+(yc/256)*(yc/256)*(yc/256)*2+(xc/256)*(yc/256)*7)%5;
					unsigned i = ((yc&(ch-1))*cw+(xc&(cw-1)));
					float zif2 = 1.0-zif;
					c = color(uint8_t(ct[zi][3*i]*zif2 + ct[zi+1][3*i]*zif),
						  uint8_t(ct[zi][3*i+1]*zif2 + ct[zi+1][3*i+1]*zif),
						  uint8_t(ct[zi][3*i+2]*zif2 + ct[zi+1][3*i+2]*zif));
				}
				dest[0] = c.r;
				dest[1] = c.g;
				dest[2] = c.b;
				dest += 3;
			}
		}
	}
	void get_min_max_height(double& minh, double& maxh) const { minh = -130; maxh = 128.0-130; }
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



////////////////////////////////////////////////////////////////////////////////////////////////////
// copied from credits.cpp, later move to own file(s)!

class camera
{
	vector3 position;
	vector3 look_at;
public:
	camera(const vector3& p = vector3(), const vector3& la = vector3(0, 1, 0))
		: position(p), look_at(la) {}
	const vector3& get_pos() const { return position; }
	vector3 view_dir() const { return (look_at - position).normal(); }
	angle look_direction() const { return angle((look_at - position).xy()); }
	void set(const vector3& pos, const vector3& lookat) { position = pos; look_at = lookat; }
	matrix4 get_transformation() const;
	void set_gl_trans() const;
};



matrix4 camera::get_transformation() const
{
	// compute transformation matrix from camera
	// orientation
	// camera points to -z axis with OpenGL
	vector3 zdir = -(look_at - position).normal();
	vector3 ydir(0, 0, 1);
	vector3 xdir = ydir.cross(zdir);
	ydir = zdir.cross(xdir);
	vector3 p(xdir * position, ydir * position, zdir * position);
	return matrix4(xdir.x, xdir.y, xdir.z, -p.x,
		       ydir.x, ydir.y, ydir.z, -p.y,
		       zdir.x, zdir.y, zdir.z, -p.z,
		       0, 0, 0, 1);
}



void camera::set_gl_trans() const
{
	get_transformation().multiply_gl();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

font* font_arial = 0;

void run()
{
	//height_generator_test1 hgt;
	height_generator_test2 hgt;
	// total area covered = 2^(levels-1) * L * N
	// 8, 7, 1.0 gives 2^14m = 16384m
	geoclipmap gcm(7, 8/*8*/ /* 2^x=N */, hgt);
	
#if 0
	rastered_map terrain(get_map_dir()+"/ETOPO2v2c_i2_LSB.xml", get_map_dir()+"/ETOPO2v2c_i2_LSB.bin", vector2l(-16*60, 16*60), 32*60 , (unsigned)9);
	geoclipmap gcm(8, 8, terrain);
#endif
	
	//gcm.set_viewerpos(vector3(0, 0, 30.0));

	glClearColor(0.3,0.4,1.0,0.0);

	vector3 viewpos(0, 0, 64);

//	auto_ptr<sky> mysky(new sky(8*3600.0)); // 10 o'clock
//	vector3 sunpos(0, 3000, 4000);
//	mysky->rebuild_colors(sunpos, vector3(-500, -3000, 1000), viewpos);

	// compute bspline for camera path
	std::vector<vector3f> bsppts;
	vector2f bsp[13] = {
		vector2f( 0.00,  0.75),
		vector2f( 0.75,  0.75),
		vector2f( 0.75,  0.00),
		vector2f( 0.00,  0.00),
		vector2f(-0.75,  0.00),
		vector2f(-0.75, -0.75),
		vector2f( 0.00, -0.75),
		vector2f( 0.75, -0.75),
		vector2f( 0.75,  0.00),
		vector2f( 0.00,  0.00),
		vector2f(-0.75,  0.00),
		vector2f(-0.75,  0.75),
		vector2f( 0.00,  0.75)
	};
	for (unsigned j = 0; j < 13; ++j) {
		vector2f a = bsp[j] * 256;
		bsppts.push_back(a.xyz(0.0/*terrain height*/ * 0.5 + 20.0));
	}
	bsplinet<vector3f> cam_path(2, bsppts);

// we need a function to get height at x,y coord
// store xy res of height map too
// 	const float flight_wh(128, 128);
// 	vector<vector3f> flight_coords;
// 	flight_coords.push_back(vector3f(0, -flight_wh*0.75));
// fly an 8 like figure

	auto_ptr<texture> bkg;
	auto_ptr<glsl_shader_setup> glss;

	bool quit = false;
	unsigned tm = sys().millisec();
	unsigned tm0 = tm;
	fpsmeasure fpsm(1.0f);
	while (!quit) {
		list<SDL_Event> events = sys().poll_event_queue();
		for (list<SDL_Event>::iterator it = events.begin(); it != events.end(); ++it) {
			if (it->type == SDL_KEYDOWN) {
				quit = true;
		} else if (it->type == SDL_MOUSEBUTTONUP) {
				gcm.wireframe = !gcm.wireframe;

			}
		}

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glColor4f(1,1,1,1);
		glPushMatrix();
		glLoadIdentity();
		float zang = 360.0/40 * (sys().millisec() - tm0)/1000;
		float zang2 = 360.0/200 * (sys().millisec() - tm0)/1000;
		vector3 viewpos2 = viewpos + (angle(-zang2).direction() * 192).xy0();

		float path_fac = myfrac((1.0/120) * (sys().millisec() - tm0)/1000);
		vector3f campos = cam_path.value(path_fac);
		vector3f camlookat = cam_path.value(myfrac(path_fac + 0.01)) - vector3f(0, 0, 20);
		//camera cm(viewpos2, viewpos2 + angle(zang).direction().xyz(-0.25));
		camera cm(campos, camlookat);
		zang = cm.look_direction().value();
		cm.set_gl_trans();

		// sky also sets light source position
//		mysky->display(colorf(1.0f, 1.0f, 1.0f), viewpos2, 30000.0, false);
		//glDisable(GL_FOG);
		glFogi(GL_FOG_MODE, GL_EXP);
		float fog_color[4] = { 0.6, 0.6, 0.6, 1.0 };
		glFogfv(GL_FOG_COLOR, fog_color);
		glFogf(GL_FOG_DENSITY, 0.0008);

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
		//double neardist = fabs((wbln - campos) * wn);
		//log_debug("neardist="<<neardist);//gives 1.0, seems correct?
		frustum viewfrustum(viewwindow, campos, cm.view_dir(), 0.1 /* fixme: read from matrix! */);

		glColor4f(1,1,1,1);
		gcm.display(viewfrustum);

		// record fps
		float fps = fpsm.account_frame();

		sys().prepare_2d_drawing();
		std::ostringstream oss; oss << "FPS: " << fps << "\n(all time total " << fpsm.get_total_fps() << ")";
		font_arial->print(0, 0, oss.str());
		sys().unprepare_2d_drawing();

		sys().swap_buffers();
	}
	
	glClearColor(0, 0, 1, 0);
}



int mymain(list<string>& args)
{
	// report critical errors (on Unix/Posix systems)
	install_segfault_handler();

	// randomize
	srand(time(0));

	// command line argument parsing
	int res_x = 1024, res_y;
	bool fullscreen = true;

	// parse commandline
	for (list<string>::iterator it = args.begin(); it != args.end(); ++it) {
		if (*it == "--help") {
			cout << "*** Danger from the Deep ***\nusage:\n--help\t\tshow this\n"
			<< "--res n\t\tuse resolution n horizontal,\n\t\tn is 512,640,800,1024 (recommended) or 1280\n"
			<< "--nofullscreen\tdon't use fullscreen\n"
			<< "--debug\t\tdebug mode: no fullscreen, resolution 800\n"
#if !(defined (WIN32) || (defined (__APPLE__) && defined (__MACH__)))
			     << "--vsync\tsync to vertical retrace signal (for nvidia cards)\n"
#endif
			<< "--nosound\tdon't use sound\n";
			return 0;
		} else if (*it == "--nofullscreen") {
			fullscreen = false;
		} else if (*it == "--debug") {
			fullscreen = false;
			res_x = 800;
#if !(defined (WIN32) || (defined (__APPLE__) && defined (__MACH__)))
		} else if (*it == "--vsync") {
			if (putenv((char*)"__GL_SYNC_TO_VBLANK=1") < 0)
				cout << "ERROR: vsync setting failed.\n";
			//maxfps = 0;
#endif
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
	std::auto_ptr<class system> mysys(new class system(1.0, 30000.0+500.0, res_x, res_y, fullscreen));
	mysys->set_res_2d(1024, 768);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	sys().gl_perspective_fovx(70, 4.0/3.0, 1.0, 30000);
	glMatrixMode(GL_MODELVIEW);
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
	auto_ptr<font> fa(font_arial);
 	mysys->draw_console_with(font_arial, 0);

	run();

	return 0;
}
