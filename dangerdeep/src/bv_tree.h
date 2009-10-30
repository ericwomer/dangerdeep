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

//
//  A bounding volume tree (spheres) (C)+(W) 2009 Thorsten Jordan
//

#ifndef BV_TREE_H
#define BV_TREE_H

#include <list>
#include <memory>
#include <vector>
#include "sphere.h"
#include "matrix4.h"
#include <stdint.h>

/// a binary tree representing a bounding volume hierarchy
class bv_tree
{
 public:
	/// data representing a triangle for tree construction
	struct leaf_data
	{
		uint32_t tri_idx[3];
		leaf_data() { tri_idx[0] = tri_idx[1] = tri_idx[2] = uint32_t(-1); }
		const vector3f& get_pos(const std::vector<vector3f>& vertices, unsigned corner) const {
			return vertices[tri_idx[corner]];
		}
		vector3f get_center(const std::vector<vector3f>& vertices) const { return (get_pos(vertices, 0) + get_pos(vertices, 1) + get_pos(vertices, 2)) * (1.f/3); }
	};

	bv_tree(const spheref& sph, const leaf_data& ld)
		: volume(sph), leafdata(ld) {}
	bv_tree(const spheref& sph, std::auto_ptr<bv_tree> left_tree, std::auto_ptr<bv_tree> right_tree);
	static std::auto_ptr<bv_tree> create(const std::vector<vector3f>& vertices, std::list<leaf_data>& nodes);
	bool is_inside(const vector3f& v) const;
	bool collides(const std::vector<vector3f>& vertices,
		      const bv_tree& other, const std::vector<vector3f>& other_vertices,
		      std::list<vector3f>& contact_points) const;
	bool collides(const std::vector<vector3f>& vertices,
		      const bv_tree& other, const std::vector<vector3f>& other_vertices,
		      std::list<vector3f>& contact_points,
		      const matrix4f& transform, const matrix4f& other_transform) const;
	void transform(const matrix4f& mat);
	void compute_min_max(vector3f& minv, vector3f& maxv) const;
	void debug_dump(unsigned level = 0) const;
	const spheref& get_sphere() const { return volume; }

 protected:
	spheref volume;
	leaf_data leafdata;
	std::auto_ptr<bv_tree> children[2];
	bool is_leaf() const { return children[0].get() == 0; }

 private:
	bv_tree();
	bv_tree(const bv_tree& );
	bv_tree& operator= (const bv_tree& );
};

#endif
