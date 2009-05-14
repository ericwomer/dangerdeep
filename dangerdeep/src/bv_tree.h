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
#include "sphere.h"
#include "matrix4.h"
#include "ptrlist.h"
#include <memory>

class bv_tree
{
 public:
	bv_tree(const spheref& sph, unsigned triangle_)
		: volume(sph), triangle(triangle_) {}
	bv_tree(std::auto_ptr<bv_tree> left_tree, std::auto_ptr<bv_tree> right_tree);
	static std::auto_ptr<bv_tree> create(ptrlist<bv_tree>& nodes);
	bool is_inside(const vector3f& v) const;
	bool collides(const bv_tree& other, std::list<vector3f>& contact_points) const;
	bool collides(const bv_tree& other, const matrix4f& other_transform, std::list<vector3f>& contact_points) const;
	void transform(const matrix4f& mat);
	void compute_min_max(vector3f& minv, vector3f& maxv) const;
	void debug_dump(unsigned level = 0) const;
	const spheref& get_sphere() const { return volume; }

 protected:
	spheref volume;
	unsigned triangle; // -1 except for leafs
	std::auto_ptr<bv_tree> children[2];

 private:
	bv_tree();
	bv_tree(const bv_tree& );
	bv_tree& operator= (const bv_tree& );
};

#endif
