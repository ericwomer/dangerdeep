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

#include "texture.h"
#include "font.h"
#include "system.h"
#include "shader.h"
#include "datadirs.h"
#include "model.h"
#include "make_mesh.h"
#include "global_data.h"
#include "perlinnoise.h"
#include "sky.h"
#include "angle.h"
#include "bspline.h"
#include "log.h"
#include "ptrvector.h"
#include "frustum.h"

using namespace std;


const char* credits[] = {
	"$80ffc0Project idea and initial code",
	"$ffffffThorsten Jordan",
	"", "", "", "", "",
	"$80ffc0Program",
	"$ffffffThorsten Jordan",
	"Markus Petermann",
	"Viktor Radnai",
	"Andrew Rice",
	"Alexandre Paes",
	"Matt Lawrence",
	"Michael Kieser",
	"Renato Golin",
	"Hiten Parmar",
	"", "", "", "", "",
	"$80ffc0Graphics",
	"$ffffffLuis Barrancos",
	"Markus Petermann",
	"Christian Kolaß",
	"Thorsten Jordan",
	"", "", "", "", "",
	"$80ffc0Models",
	"$ffffffLuis Barrancos",
	"Christian Kolaß",
	"Thorsten Jordan",
	"", "", "", "", "",
	"$80ffc0Music and sound effects",
	"$ffffffMartin Alberstadt",
	"Marco Sarolo",
	"", "", "", "", "",
	"$80ffc0Hardcore Beta Testing",
	"$ffffffAlexander W. Janssen",
	"", "", "", "", "",
	"$80ffc0Operating system adaption",
	"$ffffffNico Sakschewski (Win32)",
	"Andrew Rice (MacOSX)",
	"Jose Alonso Cardenas Marquez (acm) (FreeBSD)",
	"", "", "", "", "",
	"$80ffc0Web site administrator",
	"$ffffffMatt Lawrence",
	"$ffffffAlexandre Paes",
	"$ffffffViktor Radnai",
	"", "", "", "", "",
	"$80ffc0Packaging",
	"$ffffffMarkus Petermann (SuSE rpm)",
	"Viktor Radnai (Debian)",
	"Giuseppe Borzi (Mandrake rpm)",
	"Michael Kieser (WIN32-Installer)",
	"", "", "", "", "",
	"$80ffc0Bug reporting and thanks",
	"$ffffffRick McDaniel",
	"Markus Petermann",
	"Viktor Radnai",
	"Christian Kolaß",
	"Nico Sakschewski",
	"Martin Butterweck",
	"Bernhard Kaindl",
	"Robert Obryk",
	"Giuseppe Lipari",
	"John Hopkin",
	"Michael Wilkinson",
	"Lee Close",
	"Christopher Dean (Naval Warfare Simulations, Sponsoring)",
	"Arthur Anker",
	"Stefan Vilijoen",
	"Luis Barrancos",
	"Tony Becker",
	"Frank Kaune",
	"Paul Marks",
	"Aaron Kulkis",
	"Giuseppe Borzi",
	"Andrew Rice",
	"Alexandre Paes",
	"Alexander W. Janssen",
	"vonhalenbach",
	"...",
	"...and all i may have forgotten here (write me!)",
	"(no bockwursts were harmed in the making of this game).",
	0
};



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
  - render triangles from outmost patch to horizon
    problem later when rendering coast, to the sea horizon z-value must be < 0,
    to the land > 0. maybe stretch xy coordinates of last level beyond viewing range,
    that would solve both problems, but the height values must get computed accordingly then
  - maybe store normals with double resolution (request higher level from height gen.)
    for extra detail, or maybe even ask height generator for normals directly.
    Give then doubled N to v-shader... DONE but a bit crude
  - viewpos change is too small compared to last rendered viewpos, do nothing, else
    render and update last rendered viewpos
  - check for combining updates to texture/VBO (at least texture possible) PERFORMANCE
  - write good height generator
  - do not render too small detail (start at min_level, but test that this works)
  - compute how many tris per second are rendered as performance measure
  - current implementation is CPU limited! profile it DONE, slightly optimized
  - check if map vs. bufferdata/buffersubdata would be faster
  - the terrace-like look is no bug but is caused by the low resolution of height maps
    (8 bit per height)

  done:
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
		vector3f n = compute_normal(detail, coord, 1.0*(1<<detail));//fixme hack
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



static Uint8 terrcol[32*3] = {
239, 237, 237, 202, 196, 195, 180, 171, 170, 178, 159, 158, 162, 148,
147, 163, 147, 144, 191, 183, 180, 185, 176, 172, 173, 163, 158, 166,
154, 146, 163, 146, 135, 170, 139, 122, 205, 138, 106, 217, 139, 101,
228, 131, 88, 245, 156, 96, 255, 194, 106, 255, 215, 108, 233, 231, 92,
207, 232, 70, 197, 234, 57, 199, 244, 35, 192, 249, 20, 152, 255, 0,
109, 255, 0, 53, 236, 21, 50, 230, 24, 43, 235, 30, 16, 251, 75, 12,
221, 180, 17, 200, 203, 28, 159, 227,
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

		unsigned generate_indices(const frustum& f,
					  uint32_t* buffer, unsigned idxbase,
					  const vector2i& offset,
					  const vector2i& size,
					  const vector2i& vbooff) const;
		unsigned generate_indices2(uint32_t* buffer, unsigned idxbase,
					   const vector2i& size,
					   const vector2i& vbooff) const;
		unsigned generate_indices_T(uint32_t* buffer, unsigned idxbase) const;
		void update_region(const geoclipmap::area& upar);
		void update_VBO_and_tex(const vector2i& scratchoff,
					int scratchmod,
					const vector2i& sz,
					const vector2i& vbooff);

		texture::ptr normals;
		texture::ptr colors;
	public:
		level(geoclipmap& gcm_, unsigned idx);
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
	texture::ptr terrain_tex;
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
	  texcolorscratchbuf(resolution_vbo*resolution_vbo*3),
	  levels(nr_levels),
	  height_gen(hg),
	  myshader(get_shader_dir() + "geoclipmap.vshader",
		   get_shader_dir() + "geoclipmap.fshader"),
	  myshader_vattr_z_c_index(0)
{
	// initialize vertex VBO and all level VBOs
	for (unsigned lvl = 0; lvl < levels.size(); ++lvl) {
		levels.reset(lvl, new level(*this, lvl));
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
	std::vector<Uint8> terrcol2(&terrcol[0], &terrcol[3*32]);
	terrain_tex.reset(new texture(terrcol2, 1, 32, GL_RGB, texture::LINEAR, texture::CLAMP_TO_EDGE));
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
	glActiveTexture(GL_TEXTURE2);
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE3);
	glEnable(GL_TEXTURE_2D);
	myshader.use();
	myshader.set_gl_texture(*terrain_tex, loc_texterrain, 2);
	for (unsigned lvl = 0; lvl < levels.size(); ++lvl) {
		myshader.set_gl_texture(levels[lvl]->colors_tex(), loc_texcolor, 0);
		myshader.set_gl_texture(levels[lvl]->normals_tex(), loc_texnormal, 2);
		if (lvl + 1 < levels.size()) {
			myshader.set_gl_texture(levels[lvl+1]->colors_tex(), loc_texnormal_c, 1);
			myshader.set_gl_texture(levels[lvl+1]->normals_tex(), loc_texnormal_c, 3);
		} else {
			myshader.set_gl_texture(*horizon_normal, loc_texcolor_c, 1);
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



geoclipmap::level::level(geoclipmap& gcm_, unsigned idx)
	: gcm(gcm_),
	  L_l(gcm.L * double(1 << idx)),
	  index(idx),
	  vertices(false),
	  indices(true)
{
	// mostly static...
	vertices.init_data(gcm.resolution_vbo*gcm.resolution_vbo*geoclipmap_fperv*4, 0, GL_STATIC_DRAW);
	// fixme: init space for indices, give correct access mode or experiment
	// fixme: set correct max. size, seems enough and not too much atm.
	// size of T-junction triangles: 4 indices per triangle (3 + 2 degen. - 1), 4*N/2 triangles
	// max. size for normal triangles + T-junc.: 2*N^2 - 4*N + 8*N - 16 = 2*N^2 + 4*N - 16
	// size for multiple columns: 2 per new column, at most N columns, mostly less.
	// fixme: we have here: 2*N^2 + 4*N + 8*N = 2*N^2 + 12*N, so enough but a bit too much.
	indices.init_data((gcm.resolution_vbo+2)*gcm.resolution_vbo*2*4 + (4*gcm.resolution/2*4)*4,
			  0, GL_STATIC_DRAW);
	// create space for normal texture
	std::vector<Uint8> pxl(3*gcm.resolution_vbo*gcm.resolution_vbo*2*2);
	normals.reset(new texture(pxl, gcm.resolution_vbo*2,
				  gcm.resolution_vbo*2, GL_RGB, texture::LINEAR, texture::REPEAT));
	memset(&pxl[0], index*30, gcm.resolution_vbo*gcm.resolution_vbo*3);
	colors.reset(new texture(pxl, gcm.resolution_vbo,
				 gcm.resolution_vbo, GL_RGB, texture::LINEAR, texture::REPEAT));
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
	//fime: transform to int first, then *2 ? doesnt help
	area outer(vector2i(int(floor(0.5*new_viewpos.x/L_l - 0.25*gcm.resolution + 0.5))*2,
			    int(floor(0.5*new_viewpos.y/L_l - 0.25*gcm.resolution + 0.5))*2),
		   vector2i(int(floor(0.5*new_viewpos.x/L_l + 0.25*gcm.resolution + 0.5))*2,
			    int(floor(0.5*new_viewpos.y/L_l + 0.25*gcm.resolution + 0.5))*2));
	tmp_inner = inner;
	tmp_outer = outer;
	//log_debug("index="<<index<<" area inner="<<inner.bl<<"|"<<inner.tr<<" outer="<<outer.bl<<"|"<<outer.tr);
	// set active texture for update
	glActiveTexture(GL_TEXTURE0);
	colors->set_gl_texture();
	glActiveTexture(GL_TEXTURE2);
	normals->set_gl_texture();
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
	ptr = (sz.x+2 + 1)*geoclipmap_fperv;
	unsigned tptr = 0;//(sz.y-1)*3*sz.x;
	//log_debug("tex scratch sz="<<sz);
	/* fixme: maybe let height generator let deliver normals directly */
	/* fixme: maybe store normals in double res. (from next finer level) in texture
	   with double res. as vertices -> more detail!
	*/
#if 1
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
#else
	for (int y = 0; y < sz.y; ++y) {
		for (int x = 0; x < sz.x; ++x) {
			float hr = gcm.vboscratchbuf[ptr+geoclipmap_fperv+2];
			float hu = gcm.vboscratchbuf[ptr+geoclipmap_fperv*(sz.x+2)+2];
			float hl = gcm.vboscratchbuf[ptr-geoclipmap_fperv+2];
			float hd = gcm.vboscratchbuf[ptr-geoclipmap_fperv*(sz.x+2)+2];
			vector3f nm = vector3f(hl-hr, hd-hu, L_l * 2).normal();
			nm = nm * 127;
			gcm.texnormalscratchbuf[tptr+0] = Uint8(nm.x + 128);
			gcm.texnormalscratchbuf[tptr+1] = Uint8(nm.y + 128);
			gcm.texnormalscratchbuf[tptr+2] = Uint8(nm.z + 128);
			tptr += 3;
			ptr += geoclipmap_fperv;
		}
		ptr += 2*geoclipmap_fperv;
	}
#endif

	// color update
	ptr = (sz.x + 1)*geoclipmap_fperv;
	tptr = 0;
	for (int y = 0; y < sz.y; ++y) {
		for (int x = 0; x < sz.x; ++x) {
			color c = gcm.height_gen.compute_color(index, upar.bl + vector2i(x, y));
			gcm.texcolorscratchbuf[tptr+0] = c.r;
			gcm.texcolorscratchbuf[tptr+1] = c.g;
			gcm.texcolorscratchbuf[tptr+2] = c.b;
			tptr += 3;
			ptr += geoclipmap_fperv;
		}
		ptr += 2*geoclipmap_fperv;
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
	normals->set_gl_texture();
	for (int y = 0; y < sz.y*2; ++y) {
		//log_debug("update texture xy off ="<<vbooff.x<<"|"<<gcm.mod(vbooff.y+y)<<" idx="<<((scratchoff.y+y)*scratchmod+scratchoff.x)*3);
		glTexSubImage2D(GL_TEXTURE_2D, 0 /* mipmap level */,
				vbooff.x*2, (vbooff.y*2+y) & (gcm.resolution_vbo_mod*2+1), sz.x*2, 1, GL_RGB, GL_UNSIGNED_BYTE,
				&gcm.texnormalscratchbuf[((scratchoff.y*2+y)*scratchmod*2+scratchoff.x*2)*3]);
	}

	colors->set_gl_texture();
	for (int y = 0; y < sz.y; ++y) {
		glTexSubImage2D(GL_TEXTURE_2D, 0 /* mipmap level */,
				vbooff.x, (vbooff.y+y) & gcm.resolution_vbo_mod, sz.x, 1, GL_RGB, GL_UNSIGNED_BYTE,
				&gcm.texcolorscratchbuf[((scratchoff.y+y)*scratchmod+scratchoff.x)*3]);
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


uint32_t idxtmp[2*256*256+6*256];
void geoclipmap::level::display(const frustum& f) const
{
	//glColor4f(1,index/8.0,0,1);//fixme test
	glColor4f(0,1,0,1);//fixme test

	vector2i outszi = tmp_outer.size();
	vector2f outsz = vector2f(outszi.x - 1, outszi.y - 1) * 0.5f;
	gcm.myshader.set_uniform(gcm.loc_xysize2, outsz);
	gcm.myshader.set_uniform(gcm.loc_L_l_rcp, 1.0f/L_l);

#if 0
	// this could be done to clear the VBO because we refill it completely.
	// it doesn't increase frame rate though.
 	indices.init_data((gcm.resolution_vbo+2)*gcm.resolution_vbo*2*4 + (4*gcm.resolution/2*4)*4,
 			  0, GL_STREAM_DRAW);
	// compute indices and store them in the VBO.
	// mapping of VBO should be sensible.
	uint32_t* indexvbo = (uint32_t*)indices.map(GL_WRITE_ONLY);
#else
	uint32_t* indexvbo = (uint32_t*)idxtmp;
#endif

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
#if 0
	indices.unmap();
#else
	indices.init_sub_data(0, nridx*4, idxtmp);
#endif

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







#if 0
void render_bobs(const model::mesh& sph, unsigned tm, unsigned dtm)
{
	glPushMatrix();
	glLoadIdentity();
	glTranslatef(0, 0, -100);
	glRotatef(-60, 1, 0, 0);
	glRotatef(360.0/20 * tm/1000, 0, 0, 1);
	float a = sin(2*3.14159*0.25*tm/1000);
	float b = cos(2*3.14159*0.5*tm/1000);
	float c = 1.5*sin(2*3.14159*0.25*tm/1000+3);
	float morphfac = myfmod(0.1*tm/1000, 3.0);
	for (int y = 0; y <= 50; ++y) {
		float yf = (y-25)/5.0;
		for (int x = 0; x <= 50; ++x) {
			float xf = (x-25)/5.0;
			glPushMatrix();
			float h = a*(xf*yf*yf)+b*sin(2*3.14159*yf*(1.5+a)*0.25+a)*10.0+c*sin(2*3.14159*xf+a)*10.0;
			vector3f p0((x-25)*2, (y-25)*2, h);
			float r = 50.0*sin(3.14159*y/51);
			float beta = 2*3.14159*x/51;
			vector3f p1(r*cos(beta), r*sin(beta), (y-25)*2);
			float gamma = 2*3.14159*1*y/51;
			vector3f p2((x-25)*2*cos(gamma), (x-25)*2*sin(gamma), (y-25)*2);
			vector3f p;
			if (morphfac < 1.0f)
				p = p0*(1.0-morphfac) + p1*morphfac;
			else if (morphfac < 2.0f)
				p = p1*(2.0-morphfac) + p2*(morphfac-1.0);
			else
				p = p2*(3.0-morphfac) + p0*(morphfac-2.0);
			glTranslatef(p.x, p.y, p.z);
			sph.display();
			glPopMatrix();
		}
	}
	glPopMatrix();
}
#endif


class heightmap
{
	std::vector<float> data;
	unsigned xres, yres;
	//fixme: dimension 3 rather!
	vector2f scal, trans;
	vector2f min_coord, max_coord, area;

public:
	heightmap(const std::vector<float>& data2, unsigned rx, unsigned ry, const vector2f& s, const vector2f& t) :
		data(data2),//(rx*ry),
		xres(rx),
		yres(ry),
		scal(s),
		trans(t),
		min_coord(t),
		max_coord(vector2f(rx*s.x, ry*s.y) + t),
		area(max_coord - min_coord - vector2f(1e-3, 1e-3))
	{
	}

	/// get height with coordinate clamping and bilinear height interpolation
	float compute_height(const vector2f& coord) const;

	const std::vector<float>& heights() { return data; }
	unsigned get_xres() const { return xres; }
	unsigned get_yres() const { return yres; }
};



float heightmap::compute_height(const vector2f& coord) const
{
	// clamp
	vector2f c = coord.max(min_coord).min(max_coord) - min_coord;
	c.x = xres * c.x / area.x;
	c.y = yres * c.y / area.y;
	unsigned x = unsigned(floor(c.x));
	unsigned y = unsigned(floor(c.y));
	c.x -= x;
	c.y -= y;
	unsigned x2 = x + 1, y2 = y + 1;
	if (x2 + 1 >= xres) x2 = xres - 1;
	if (y2 + 1 >= yres) y2 = yres - 1;
	return (data[y * xres + x] * (1.0f - c.x) + data[y * xres + x2] * c.x) * (1.0f - c.y) +
		(data[y2* xres + x] * (1.0f - c.x) + data[y2* xres + x2] * c.x) * c.y;
}



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



class canyon
{
	std::auto_ptr<model::mesh> mymesh;
	std::vector<float> heightdata;
	glsl_shader_setup myshader;
	unsigned loc_texsandrock;
	unsigned loc_texnoise;
	unsigned loc_texgrass;
	std::auto_ptr<texture> sandrocktex;
	std::auto_ptr<texture> noisetex;
	std::auto_ptr<texture> grasstex;

	struct canyon_material : public model::material
	{
		canyon& cyn;
		canyon_material(canyon& c) : cyn(c) {}
		void set_gl_values(const texture* /* unused */) const;
	};
public:
	canyon(unsigned w = 256, unsigned h = 256);
	void display() const;
	/*const*/ std::vector<float>& get_heightdata() /*const*/ { return heightdata; }
};



canyon::canyon(unsigned w, unsigned h)
	: heightdata(w * h),
	  myshader(get_shader_dir() + "sandrock.vshader",
		   get_shader_dir() + "sandrock.fshader")
{
	vector<Uint8> pn = perlinnoise(w, 4, w/2).generate(); // generate_sqr(); // also looks good
	//save_pgm("canyon.pgm", w, w, &pn[0]);
	//heightdata = heightmap(w, h, vector2f(2.0f, 2.0f), vector2f(-128, -128));
	heightdata.resize(w * h);
	for (unsigned y = 0; y < h; ++y) {
		for (unsigned x = 0; x < w; ++x) {
			heightdata[y * w + x] = pn[y*w+x];
		}
	}

	// make terraces
	const unsigned height_segments = 6;
	const float total_height = 256.0;
	const float terrace_height = total_height / height_segments;
	for (unsigned y = 0; y < h; ++y) {
		for (unsigned x = 0; x < w; ++x) {
			float f = heightdata[y * w + x];
			unsigned t = unsigned(floor(f / terrace_height));
			float f_frac = f / terrace_height - t;
			float f2 = f_frac * 2.0 - 1.0; // be in -1...1 range
			// skip this for softer hills (x^3 = more steep walls)
			f2 = f2 * f2 * f2;
			f2 = asin(f2) / M_PI + 0.5; // result in 0...1 range
			heightdata[y * w + x] = (t + f2) * terrace_height;
		}
	}

	mymesh.reset(new model::mesh(w, h, heightdata, vector3f(2.0f, 2.0f, 0.5f),
				     vector3f(0.0f, 0.0f, 0.0f)));
	//fixme: only color here!
	sandrocktex.reset(new texture(get_texture_dir() + "sandrock.png", texture::LINEAR_MIPMAP_LINEAR, texture::REPEAT));
	vector<Uint8> noisevalues = perlinnoise(256, 2, 128).generate();
	noisetex.reset(new texture(noisevalues, 256, 256, GL_LUMINANCE, texture::LINEAR_MIPMAP_LINEAR, texture::REPEAT));
	grasstex.reset(new texture(get_texture_dir() + "grass.png", texture::LINEAR_MIPMAP_LINEAR, texture::REPEAT));
	mymesh->mymaterial = new canyon_material(*this);
#if 0
	mymesh->mymaterial = new model::material();
	mymesh->mymaterial->diffuse = color(32, 32, 255);
	mymesh->mymaterial->colormap.reset(new model::material::map());
	mymesh->mymaterial->colormap->set_texture(new texture(get_texture_dir() + "sandrock.jpg", texture::LINEAR_MIPMAP_LINEAR, texture::REPEAT));
	mymesh->mymaterial->specular = color(0, 0, 0);
#endif
	for (unsigned y = 0; y < h; ++y) {
		float fy = float(y)/(h-1);
		for (unsigned x = 0; x < w; ++x) {
			float fx = float(x)/(w-1);
			mymesh->texcoords[y*w+x] = vector2f((fx+sin(fy*8*2*M_PI)/32)*32 /*+fy*fx*64*/, heightdata[y*w+x]/256);
		}
	}
#if 0
	for (unsigned y = 0; y < h; ++y) {
		for (unsigned x = 0; x < w; ++x) {
			unsigned i = y * w + x;
			if (x == 0 || (x+1) == w) {
				mymesh->vertices[i].x *= 200;
			} else if (y == 0 || (y+1) == h) {
				mymesh->vertices[i].y *= 200;
			}
		}
	}
#endif
	mymesh->compile();

	myshader.use();
	loc_texsandrock = myshader.get_uniform_location("texsandrock");
	loc_texnoise = myshader.get_uniform_location("texnoise");
	loc_texgrass = myshader.get_uniform_location("texgrass");
	myshader.use_fixed();
}



void canyon::canyon_material::set_gl_values(const texture* /* unused */) const
{
	cyn.myshader.use();
	cyn.myshader.set_gl_texture(*cyn.sandrocktex.get(), cyn.loc_texsandrock, 0);
	cyn.myshader.set_gl_texture(*cyn.noisetex.get(), cyn.loc_texnoise, 1);
	cyn.myshader.set_gl_texture(*cyn.grasstex.get(), cyn.loc_texgrass, 2);
}



void canyon::display() const
{
	mymesh->display();
	myshader.use_fixed();
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
}



class plant
{
public:
	static const unsigned nr_plant_types = 8;

	vector3f pos;
	vector2f size;
	unsigned type;

	plant(const vector3f& p, const vector2f& s, unsigned t)
		: pos(p), size(s), type(t)
	{
	}
};



struct plant_alpha_sortidx
{
	float sqd;
	unsigned idx;
	plant_alpha_sortidx() {}
	plant_alpha_sortidx(const std::vector<plant>& plants, unsigned i, const vector2f& viewpos)
		: sqd(plants[i].pos.xy().square_distance(viewpos)), idx(i) {}
	bool operator< (const plant_alpha_sortidx& other) const {
		return sqd > other.sqd;
	}
};


class plant_set
{
	// with the VBO we don't really need to store the plant vertex data...
	std::vector<plant> plants;
 	mutable vertexbufferobject plantvertexdata;
 	mutable vertexbufferobject plantindexdata;
	std::auto_ptr<texture> planttex;
	glsl_shader_setup myshader;
	unsigned loc_textrees;
	unsigned loc_viewpos;
	unsigned loc_windmovement;
	unsigned vattr_treesize_idx;
	mutable std::vector<plant_alpha_sortidx> sortindices;
public:
	plant_set(vector<float>& heightdata, unsigned nr = 40000, unsigned w = 256, unsigned h = 256, const vector2f& scal = vector2f(2.0f, 2.0f));
	void display(const vector3& viewpos, float zang) const;
};


plant_set::plant_set(vector<float>& heightdata, unsigned nr, unsigned w, unsigned h, const vector2f& scal)
	: plantvertexdata(false), plantindexdata(true),
	  myshader(get_shader_dir() + "billboardtrees.vshader",
		   get_shader_dir() + "billboardtrees.fshader"),
	  vattr_treesize_idx(0)
{
	float areaw = w * scal.x, areah = h * scal.y;
	plants.reserve(nr);
	const float treeheight = 4.0;
	const float treewidth = 2.0;
	for (unsigned t = 0; t < nr; ) {
		float x = (rnd() - 0.5) * areaw;
		float y = (rnd() - 0.5) * areah;
		unsigned idxy = myclamp(unsigned((y + areah*0.5) / scal.y), 0U, h-1);
		unsigned idxx = myclamp(unsigned((x + areaw*0.5) / scal.x), 0U, w-1);
		float nz = 0.0;
		if (idxx > 0 && idxx < w-1 && idxy > 0 && idxy < h-1) {
			float hl = heightdata[idxy * w + idxx - 1];
			float hr = heightdata[idxy * w + idxx + 1];
			float hd = heightdata[idxy * w + idxx - w];
			float hu = heightdata[idxy * w + idxx + w];
			nz = vector3f(hl-hr, hd-hu, scal.x * scal.y).normal().z;
		}
		if (nz < 0.95) {
			continue;
		} else {
			++t;
		}
		float th = treeheight * rnd() * 0.25;
		float tw = treewidth * rnd() * 0.25;
		float h = heightdata[idxy * w + idxx] * 0.5;
		plants.push_back(plant(vector3f(x, y, h),
				       vector2f(treewidth + tw, treeheight + th),
				       rnd(plant::nr_plant_types)));
	}
	planttex.reset(new texture(get_texture_dir() + "plants.png", texture::LINEAR_MIPMAP_LINEAR, texture::CLAMP_TO_EDGE));

	// set up sorting indices
	sortindices.resize(plants.size());
	for (unsigned i = 0; i < plants.size(); ++i) {
		sortindices[i].idx = i;
	}

	myshader.use();
	vattr_treesize_idx = myshader.get_vertex_attrib_index("treesize");
	loc_textrees = myshader.get_uniform_location("textrees");
	loc_viewpos = myshader.get_uniform_location("viewpos");
	loc_windmovement = myshader.get_uniform_location("windmovement");
	// this is done only once... hmm are uniforms stored per shader and never changed?! fixme
	myshader.set_gl_texture(*planttex, loc_textrees, 0);
	// vertex data per plant are 4 * (3+2+1) floats (3x pos, 2x texc, 1x attr)
	plantvertexdata.init_data(4 * (3 + 2 + 1) * 4 * plants.size(), 0, GL_STATIC_DRAW);
	float* vertexdata = (float*) plantvertexdata.map(GL_WRITE_ONLY);
	for (unsigned i = 0; i < plants.size(); ++i) {
		// render each plant
		const plant& p = plants[i];
		// vertex 0
		vertexdata[6*(4*i + 0) + 0] = p.pos.x;
		vertexdata[6*(4*i + 0) + 1] = p.pos.y;
		vertexdata[6*(4*i + 0) + 2] = p.pos.z;
		vertexdata[6*(4*i + 0) + 3] = float(p.type)/plant::nr_plant_types;
		vertexdata[6*(4*i + 0) + 4] = 1.0f;
		vertexdata[6*(4*i + 0) + 5] = -p.size.x * 0.5;
		// vertex 1
		vertexdata[6*(4*i + 1) + 0] = p.pos.x;
		vertexdata[6*(4*i + 1) + 1] = p.pos.y;
		vertexdata[6*(4*i + 1) + 2] = p.pos.z;
		vertexdata[6*(4*i + 1) + 3] = float(p.type+1)/plant::nr_plant_types;
		vertexdata[6*(4*i + 1) + 4] = 1.0f;
		vertexdata[6*(4*i + 1) + 5] = p.size.x * 0.5;
		// vertex 2
		vertexdata[6*(4*i + 2) + 0] = p.pos.x;
		vertexdata[6*(4*i + 2) + 1] = p.pos.y;
		vertexdata[6*(4*i + 2) + 2] = p.pos.z + p.size.y;
		vertexdata[6*(4*i + 2) + 3] = float(p.type+1)/plant::nr_plant_types;
		vertexdata[6*(4*i + 2) + 4] = 0.0f;
		vertexdata[6*(4*i + 2) + 5] = p.size.x * 0.5;
		// vertex 3
		vertexdata[6*(4*i + 3) + 0] = p.pos.x;
		vertexdata[6*(4*i + 3) + 1] = p.pos.y;
		vertexdata[6*(4*i + 3) + 2] = p.pos.z + p.size.y;
		vertexdata[6*(4*i + 3) + 3] = float(p.type)/plant::nr_plant_types;
		vertexdata[6*(4*i + 3) + 4] = 0.0f;
		vertexdata[6*(4*i + 3) + 5] = -p.size.x * 0.5;
	}
	plantvertexdata.unmap();
	myshader.use_fixed();
}



void plant_set::display(const vector3& viewpos, float zang) const
{
	vector3f vp(viewpos);

	//unsigned tm0 = sys().millisec();
	for (unsigned i = 0; i < plants.size(); ++i) {
		sortindices[i].sqd = plants[sortindices[i].idx].pos.xy().square_distance(vp.xy());
	}
	//unsigned tm1 = sys().millisec();
	// this sucks up to 16ms, so this limits fps at 60.
	// this can't be shadowed by gpu time.
	std::sort(sortindices.begin(), sortindices.end());
	//unsigned tm2 = sys().millisec();
	//DBGOUT2(tm1-tm0,tm2-tm1);

#if 1 // indices in VBO (3fps faster)
	// index data per plant are 4 indices = 16 byte
	//fixme: why transfer this to a VBO? why not drawing these indices
	//directly from the array?!
	plantindexdata.init_data(4 * 4 * plants.size(), 0, GL_STREAM_DRAW);
	uint32_t* indexdata = (uint32_t*) plantindexdata.map(GL_WRITE_ONLY);
	for (unsigned i = 0; i < plants.size(); ++i) {
		// 4 vertices per plant
		unsigned bi = sortindices[i].idx * 4; // base index for plant i
		indexdata[4*i + 0] = bi;
		indexdata[4*i + 1] = bi+1;
		indexdata[4*i + 2] = bi+2;
		indexdata[4*i + 3] = bi+3;
	}
	plantindexdata.unmap();
#else
	vector<unsigned> pidat;
	pidat.reserve(plants.size()*4);
	for (unsigned i = 0; i < plants.size(); ++i) {
		unsigned bi = sortindices[i].idx * 4;
		pidat.push_back(bi+0);
		pidat.push_back(bi+1);
		pidat.push_back(bi+2);
		pidat.push_back(bi+3);
	}
#endif

	glActiveTexture(GL_TEXTURE0);
	planttex->set_gl_texture();
	glColor4f(1,1,1,1);
	glNormal3f(0, 0, 1);

	// fixme: cull invisible plants

	glDepthMask(GL_FALSE);
	myshader.use();
	myshader.set_uniform(loc_viewpos, vp.xy());
	myshader.set_uniform(loc_windmovement, myfrac(sys().millisec()/4000.0));

	plantvertexdata.bind();
	glEnableClientState(GL_VERTEX_ARRAY);
	glClientActiveTexture(GL_TEXTURE0);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glVertexPointer(3, GL_FLOAT, 6*4, (float*)0 + 0);
	glTexCoordPointer(2, GL_FLOAT, 6*4, (float*)0 + 3);
	glVertexAttribPointer(vattr_treesize_idx, 1, GL_FLOAT, GL_FALSE, 6*4, (float*)0 + 5);
	glEnableVertexAttribArray(vattr_treesize_idx);
	plantvertexdata.unbind();
#if 1 // indices in VBO (3fps faster)
	plantindexdata.bind();
	glDrawRangeElements(GL_QUADS, 0, plants.size()*4 - 1, plants.size() * 4, GL_UNSIGNED_INT, 0);
	plantindexdata.unbind();
#else
	glDrawRangeElements(GL_QUADS, 0, plants.size()*4 - 1, plants.size() * 4, GL_UNSIGNED_INT, &pidat[0]);
#endif
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableVertexAttribArray(vattr_treesize_idx);
	glDepthMask(GL_TRUE);
	myshader.use_fixed();
}



void add_tree(const vector3f& pos, float ang,
	      vector<vector3f>& vertices,
	      vector<vector2f>& texcoords,
	      vector<vector3f>& normals,
	      vector<Uint32>& indices)
{
	const float treeheight = 4.0;
	const float treewidth = 2.0;
	unsigned bi = vertices.size();
	// 10 vertices per tree
	// 48 indices per tree (16 triangles, 4 per direction, 2-sided quads)
	// form pine tree with cones? cyliner with 3 quads at bottom, 8 tris for cone
	// makes 14 tris. (verts: 8+1+1 + 2*3 at least = 16)
	// normally we should use billboarding anyway (check pbs)
	float th = treeheight * rnd() * 0.25;
	float tw = treewidth * rnd() * 0.25;
	vector3f postop = pos;
	postop.z += treeheight + th;
	vertices.push_back(postop);
	normals.push_back(vector3f(0, 0, 1));
	texcoords.push_back(vector2f(0.5, 0.0));
	for (unsigned i = 0; i <= 8; ++i) {
		angle a(ang - i * 360/8);
		vector3f pos2 = pos + (a.direction() * (treewidth + tw) * 0.5).xyz((postop.z - pos.z)*0.25);
		vertices.push_back(pos2);
		normals.push_back(a.direction().xyz(2.0f).normal());
		texcoords.push_back(vector2f(float(i)/8, 0.75));
	}
	for (unsigned i = 0; i < 8; ++i) {
		indices.push_back(bi);
		indices.push_back(bi+1+i);
		indices.push_back(bi+1+i+1);
	}
	bi = vertices.size();
	for (unsigned i = 0; i < 3; ++i) {
		angle a(ang - i * 360/3);
		vector3f pos2 = pos + (a.direction() * (treewidth + tw) * 0.1).xyz((postop.z - pos.z)*0.25);
		vertices.push_back(pos2);
		pos2.z = pos.z;
		vertices.push_back(pos2);
		normals.push_back(a.direction().xyz(2.0f).normal());
		normals.push_back(a.direction().xyz(2.0f).normal());
		texcoords.push_back(vector2f(float(i)/3, 0.75));
		texcoords.push_back(vector2f(float(i)/3, 1.0));
	}
	for (unsigned i = 0; i < 3; ++i) {
		indices.push_back(bi+2*i);
		indices.push_back(bi+2*i+1);
		indices.push_back(bi+2*((i+1)%3));
		indices.push_back(bi+2*((i+1)%3));
		indices.push_back(bi+2*i+1);
		indices.push_back(bi+2*((i+1)%3)+1);
	}

#if 0
	vector3f postop = pos;
	postop.z += treeheight + th;
	vertices.push_back(pos);
	vertices.push_back(postop);
	normals.push_back(vector3f(0, 0, 1));
	normals.push_back(vector3f(0, 0, 1));
	texcoords.push_back(vector2f(1.0, 1.0));
	texcoords.push_back(vector2f(1.0, 0.0));
	for (unsigned i = 0; i < 4; ++i) {
		angle a(ang + 90 * i);
		vector3f pos2 = pos + (a.direction() * (treewidth + tw) * 0.5).xy0();
		vector3f postop2 = pos2;
		postop2.z += treeheight + th;
		vertices.push_back(pos2);
		vertices.push_back(postop2);
		normals.push_back(vector3f(0, 0, 1));
		normals.push_back(vector3f(0, 0, 1));
		texcoords.push_back(vector2f(0.0, 1.0));
		texcoords.push_back(vector2f(0.0, 0.0));
		// side one
		indices.push_back(bi+2+i*2);
		indices.push_back(bi      );
		indices.push_back(bi+3+i*2);
		indices.push_back(bi+3+i*2);
		indices.push_back(bi      );
		indices.push_back(bi+1    );
		// side two
		indices.push_back(bi      );
		indices.push_back(bi+2+i*2);
		indices.push_back(bi+1    );
		indices.push_back(bi+1    );
		indices.push_back(bi+2+i*2);
		indices.push_back(bi+3+i*2);
	}
#endif
}



auto_ptr<model::mesh> generate_trees(vector<float>& heightdata, unsigned nr = 20000, unsigned w = 256, unsigned h = 256, const vector2f& scal = vector2f(2.0f, 2.0f))
{
	float areaw = w * scal.x, areah = h * scal.y;
	auto_ptr<model::mesh> m(new model::mesh("trees"));
	for (unsigned t = 0; t < nr; ) {
		float x = (rnd() - 0.5) * areaw;
		float y = (rnd() - 0.5) * areah;
		unsigned idxy = myclamp(unsigned((y + areah*0.5) / scal.y), 0U, h-1);
		unsigned idxx = myclamp(unsigned((x + areaw*0.5) / scal.x), 0U, w-1);
		float nz = 0.0;
		if (idxx > 0 && idxx < w-1 && idxy > 0 && idxy < h-1) {
			float hl = heightdata[idxy * w + idxx - 1];
			float hr = heightdata[idxy * w + idxx + 1];
			float hd = heightdata[idxy * w + idxx - w];
			float hu = heightdata[idxy * w + idxx + w];
			nz = vector3f(hl-hr, hd-hu, scal.x * scal.y).normal().z;
		}
		if (nz < 0.95) {
			continue;
		} else {
			++t;
		}
		float h = heightdata[idxy * w + idxx] * 0.5;
		add_tree(vector3f(x, y, h), rnd()*90, m->vertices, m->texcoords, m->normals, m->indices);
	}
	m->mymaterial = new model::material();
	m->mymaterial->colormap.reset(new model::material::map());
//	m->mymaterial->colormap->set_texture(new texture(get_texture_dir() + "tree1.png", texture::LINEAR_MIPMAP_LINEAR, texture::CLAMP_TO_EDGE));
	m->mymaterial->specular = color(0, 0, 0);
	m->compile();
	return m;
}



void generate_fadein_pixels(vector<Uint8>& pix, unsigned ctr, unsigned s)
{
	// draw spiral
	unsigned x = 0, y = 0;
	const int dx[4] = { 1, 0,-1, 0 };
	const int dy[4] = { 0, 1, 0,-1 };
	unsigned i = 0;
	for (unsigned m = s; m > 0; m -= 2) {
		for (unsigned k = 0; k < 4; ++k) {
			for (unsigned j = 0; j < m - 1; ++j) {
				pix[2*(y*s+x)+1] = (i < ctr) ? 0x00 : 0xff;
				x += dx[k];
				y += dy[k];
				++i;
			}
		}
		// we need to go down and right one cell
		x += 1;
		y += 1;
	}
}


template <class T, unsigned size>
class lookup_function
{
	typename std::vector<T> values;
	float dmin, dmax, drange_rcp;
public:
	lookup_function(float dmin_ = 0.0f, float dmax_ = 1.0f)
		: values(size + 2), dmin(dmin_), dmax(dmax_), drange_rcp(1.0f/(dmax_ - dmin_)) {}
	void set_value(unsigned idx, T v) {
		values.at(idx) = v;
		// duplicate last value (avoid the if (idx == size), its faster to
		// just do it.
		values[size+1] = values[size];
	}
	T value(float f) const {
		if (f < dmin) f = dmin;
		if (f > dmax) f = dmax;
		// note: if drange_rcp is a bit too large (float is unprecise)
		// the result could be a bit larger than 1.0f * size
		// which is no problem when its smaller than 1.0 + 1/(size+1)
		// which is normally the case. to avoid segfaults we just
		// make "values" one entry bigger and duplicate the last value.
		return values[unsigned(size * ((f - dmin) * drange_rcp))];
	}
	unsigned get_value_range() const { return size + 1; }
};

void show_credits()
{
	//glClearColor(0.1,0.25,0.4,0);
	glClearColor(0.175,0.25,0.125,0.0);

	/* geoclipmap test*/
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

	//srand(0);

#if 0
	// create a sphere
	auto_ptr<model::mesh> sph;
	sph.reset(make_mesh::sphere(1.0f, 2.0f, 8, 8, 1, 1, true, "sun"));
	sph->mymaterial = new model::material();
	sph->mymaterial->diffuse = color(32, 32, 255);
	sph->mymaterial->specular = color(255, 255, 255);
	sph->compile();
#endif

	vector3 viewpos(0, 0, 64);

	vector<float> heightdata;
	canyon cyn(256, 256);
	heightmap chm(cyn.get_heightdata(), 256, 256, vector2f(2.0f, 2.0f), vector2f(-256, -256));
	auto_ptr<model::mesh> trees = generate_trees(cyn.get_heightdata());
	plant_set ps(cyn.get_heightdata());
	auto_ptr<sky> mysky(new sky(8*3600.0)); // 10 o'clock
	vector3 sunpos(0, 3000, 4000);
	mysky->rebuild_colors(sunpos, vector3(-500, -3000, 1000), viewpos);

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
		bsppts.push_back(a.xyz(chm.compute_height(a) * 0.5 + 20.0));
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
#if 0
	bool use_shaders = glsl_program::supported();
	if (use_shaders) {
		const unsigned sz = 16;
		vector<Uint8> data(sz*sz);
		for (unsigned y = 0; y < sz; ++y)
			for (unsigned x = 0; x < sz; ++x)
				data[y*sz+x] = rand() & 0xff;
		bkg.reset(new texture(data, sz, sz, GL_LUMINANCE, texture::LINEAR, texture::REPEAT));
		glss.reset(new glsl_shader_setup(get_shader_dir() + "credits.vshader",
						 get_shader_dir() + "credits.fshader"));
	}
#endif

	int lineheight = font_arial->get_height();
	int lines_per_page = (768+lineheight-1)/lineheight;
	int textpos = -lines_per_page;
	int textlines = 0;
	for ( ; credits[textlines] != 0; ++textlines);
	int textendpos = textlines;
	float lineoffset = 0.0f;

	//float lposition[4] = {200, 0, 0, 1};

	std::auto_ptr<texture> fadein_tex;
	std::vector<Uint8> fadein_pixels(8*8*2);
	generate_fadein_pixels(fadein_pixels, 0, 8);
	fadein_tex.reset(new texture(fadein_pixels, 8, 8, GL_LUMINANCE_ALPHA,
				     texture::NEAREST, texture::REPEAT));
	unsigned fadein_ctr = 0;

	bool quit = false;
	float lines_per_sec = 2;
	float ctr = 0.0, ctradd = 1.0/32.0;
	unsigned tm = sys().millisec();
	unsigned tm0 = tm;
	unsigned frames = 1;
	unsigned lastframes = 1;
	double fpstime = sys().millisec() / 1000.0;
	double totaltime = sys().millisec() / 1000.0;
	double measuretime = 5;	// seconds
	while (!quit) {
		list<SDL_Event> events = sys().poll_event_queue();
		for (list<SDL_Event>::iterator it = events.begin(); it != events.end(); ++it) {
			if (it->type == SDL_KEYDOWN) {
				quit = true;
			} else if (it->type == SDL_MOUSEBUTTONUP) {
				quit = true;
			}
		}

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

#if 0
		// render sphere bobs
		render_bobs(*sph, sys().millisec() - tm0, sys().millisec() - tm);
#endif

		glColor4f(1,1,1,1);
		glPushMatrix();
		glLoadIdentity();
		float zang = 360.0/40 * (sys().millisec() - tm0)/1000;
		float zang2 = 360.0/200 * (sys().millisec() - tm0)/1000;
		vector3 viewpos2 = viewpos + (angle(-zang2).direction() * 192).xy0();
		float terrainh = chm.compute_height(vector2f(viewpos2.x, viewpos2.y));
		viewpos2.z = terrainh * 0.5 + 20.0; // fixme, heightmap must take care of z scale

		float path_fac = myfrac((1.0/120) * (sys().millisec() - tm0)/1000);
		vector3f campos = cam_path.value(path_fac);
		vector3f camlookat = cam_path.value(myfrac(path_fac + 0.01));
#ifdef GEOCLIPMAPTEST
		camlookat.z -= 10;
#endif
		//camera cm(viewpos2, viewpos2 + angle(zang).direction().xyz(-0.25));
		camera cm(campos, camlookat);
		zang = cm.look_direction().value();
		cm.set_gl_trans();

		// sky also sets light source position
		mysky->display(colorf(1.0f, 1.0f, 1.0f), viewpos2, 30000.0, false);
		//glDisable(GL_FOG);
		glFogi(GL_FOG_MODE, GL_EXP);
		float fog_color[4] = { 0.6, 0.6, 0.6, 1.0 };
		glFogfv(GL_FOG_COLOR, fog_color);
		glFogf(GL_FOG_DENSITY, 0.0008);

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
#else
		// render canyon
		cyn.display();
		//trees->display();
		ps.display(campos, zang);//viewpos2 here, but it flickers
#endif
		glPopMatrix();

		sys().prepare_2d_drawing();
		if (fadein_tex.get()) {
			fadein_ctr = (sys().millisec() - tm0) * 64 / 3200;
			// generate fadein tex
			generate_fadein_pixels(fadein_pixels, fadein_ctr, 8);
			fadein_tex.reset(new texture(fadein_pixels, 8, 8, GL_LUMINANCE_ALPHA,
						     texture::NEAREST, texture::REPEAT));
#ifndef GEOCLIPMAPTEST
			glPushMatrix();
			glScalef(4.0, 4.0, 4.0);
			fadein_tex->draw_tiles(0, 0, sys().get_res_x_2d()/4, sys().get_res_y_2d()/4);
			glPopMatrix();
#endif
			if (fadein_ctr >= 64)
				fadein_tex.reset();
		}

#if 0
		if (use_shaders) {
			glss->use();
			glColor4f(0.2,0.8,1,1);
			bkg->set_gl_texture();
			glBegin(GL_QUADS);
			glTexCoord2f(0+ctr,0);
			glMultiTexCoord2f(GL_TEXTURE1_ARB, ctr, ctr*0.5);
			glVertex2i(0,768);
			glTexCoord2f(1.33333+ctr,0);
			glMultiTexCoord2f(GL_TEXTURE1_ARB, ctr, ctr*0.5);
			glVertex2i(1024,768);
			glTexCoord2f(1.33333+ctr,1);
			glMultiTexCoord2f(GL_TEXTURE1_ARB, ctr, ctr*0.5);
			glVertex2i(1024,0);
			glTexCoord2f(0+ctr,1);
			glMultiTexCoord2f(GL_TEXTURE1_ARB, ctr, ctr*0.5);
			glVertex2i(0,0);
			glEnd();
			glBindTexture(GL_TEXTURE_2D, 0);
			glss->use_fixed();
		}
#endif

#ifndef GEOCLIPMAPTEST
		for (int i = textpos; i <= textpos+lines_per_page; ++i) {
			if (i >= 0 && i < textlines) {
				int y = (i-textpos)*lineheight+int(-lineoffset*lineheight);
				font_arial->print_hc(512+int(64*sin(y*2*M_PI/640)), y, credits[i], color::white(), true);
			}
		}
#endif
		sys().unprepare_2d_drawing();

		unsigned tm2 = sys().millisec();
		lineoffset += lines_per_sec*(tm2-tm)/1000.0f;
		int tmp = int(lineoffset);
		lineoffset -= tmp;
		textpos += tmp;
		if (textpos >= textendpos) textpos = -lines_per_page;
		ctr += ctradd * (tm2-tm)/1000.0f;
		tm = tm2;

		// record fps
		++frames;
		totaltime = tm2 / 1000.0;
		if (totaltime - fpstime >= measuretime) {
			fpstime = totaltime;
			log_info("fps " << (frames - lastframes)/measuretime);
			lastframes = frames;
		}

		sys().swap_buffers();
	}
	
	glClearColor(0, 0, 1, 0);
}
