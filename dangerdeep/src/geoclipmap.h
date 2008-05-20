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

#ifndef GEOCLIPMAP_H
#define GEOCLIPMAP_H

#define geoclipmap_fperv 4

#include "texture.h"
#include "shader.h"
#include "vertexbufferobject.h"
#include "ptrvector.h"
#include "frustum.h"
#include "datadirs.h"
#include "height_generator.h"
#include "color.h"

class geoclipmap
{
 public:
	/// create geoclipmap data
	///@param nr_levels - number of levels
	///@param resolution_exp - power of two of resolution factor "N"
	///@param hg - instance of height generator object
	geoclipmap(unsigned nr_levels, unsigned resolution_exp, height_generator& hg);

	/// d'tor
	~geoclipmap();

	/// set/change viewer position
	void set_viewerpos(const vector3f& viewpos);

	/// render the view (will only fetch the vertex/index data, no texture setup)
	void display(const frustum& f) const;

 protected:
	// "N", must be power of two
	const unsigned resolution;  // resolution of triangles in VBO buffer
	const unsigned resolution_vbo; // resolution of VBO buffer
	const unsigned resolution_vbo_mod; // resolution of VBO buffer - 1
	// distance between each vertex on finest level in real world space
	const double L;

	// resolution factor vertex to color
	const unsigned color_res_fac;
	const unsigned log2_color_res_fac;

	// scratch buffer for VBO data, for transmission
	std::vector<float> vboscratchbuf;

	// scratch buffer for texture data, for transmission
	std::vector<vector3f> texnormalscratchbuf_3f;
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
		const double L_l;
		// resolution factor vertex to color
		const unsigned color_res_fac;
		const unsigned log2_color_res_fac;
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
	bool wireframe;	// for testing purposes only
};

#endif
