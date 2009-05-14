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

#include "bv_tree.h"

std::auto_ptr<bv_tree> bv_tree::create(ptrlist<bv_tree>& nodes)
{
	std::auto_ptr<bv_tree> result;
	// if list has zero entries, return empty pointer
	if (nodes.empty())
		return result;
	// if list has one entry, return that
	if (nodes.size() == 1)
		return nodes.release_front();
	// if node list has two entries, partition intro tree node
	if (nodes.size() == 2) {
		const bv_tree* ta = nodes.front();
		const bv_tree* tb = nodes.back();
		result.reset(new bv_tree(spheref(ta->volume.compute_bound(tb->volume)), unsigned(-1)));
		result->children[0] = nodes.release_front();
		result->children[1] = nodes.release_back();
		return result;
	}
	// compute bounding sphere for all nodes
	vector3f minv(1e30, 1e30, 1e30);
	vector3f maxv = -minv;
	ptrlist<bv_tree>::const_iterator itc = nodes.begin();
	spheref union_sphere = itc->get_sphere();
	itc->compute_min_max(minv, maxv);
	++itc;
	for ( ; itc != nodes.end(); ++itc) {
		union_sphere = union_sphere.compute_bound(itc->get_sphere());
		itc->compute_min_max(minv, maxv);
	}
	vector3f deltav = maxv - minv;
	// chose axis with longest value range, sort along that axis,
	// split in center of union sphere.
	std::cout << "nodes " << nodes.size() << " unionsph=" << union_sphere.center << "|" << union_sphere.radius << "\n";
	unsigned split_axis = 0; // x - default
	if (deltav.y > deltav.x) {
		if (deltav.z > deltav.y) {
			split_axis = 2; // z
		} else {
			split_axis = 1; // y
		}
	} else if (deltav.z > deltav.x) {
		split_axis = 2; // z
	}
	std::cout << "deltav " << deltav << " split axis " << split_axis << "\n";
	ptrlist<bv_tree> left_nodes, right_nodes;
	float vcenter[3];
	union_sphere.center.to_mem(vcenter);
	for (ptrlist<bv_tree>::iterator it = nodes.nc_begin(); it != nodes.nc_end(); ++it) {
		const vector3f& c = it->volume.center;
		float vc[3];
		c.to_mem(vc);
		if (vc[split_axis] < vcenter[split_axis])
			left_nodes.push_back(it.release());
		else
			right_nodes.push_back(it.release());
	}
	if (left_nodes.empty() || right_nodes.empty()) {
		std::cout << "special case\n";
		// special case: force division
		ptrlist<bv_tree>& empty_list = left_nodes.empty() ? left_nodes : right_nodes;
		ptrlist<bv_tree>& full_list = left_nodes.empty() ? right_nodes : left_nodes;
		for (unsigned i = 0; i < full_list.size() / 2; ++i) {
			empty_list.push_back(full_list.release_front());
		}
	}
	std::cout << "left " << left_nodes.size() << " right " << right_nodes.size() << "\n";
	result.reset(new bv_tree(create(left_nodes), create(right_nodes)));
	std::cout << "final volume " << result->volume.center << "|" << result->volume.radius << "\n";
	return result;
}



bv_tree::bv_tree(std::auto_ptr<bv_tree> left_tree, std::auto_ptr<bv_tree> right_tree)
	: volume(left_tree->volume.compute_bound(right_tree->volume)), triangle(unsigned(-1))
{
	children[0] = left_tree;
	children[1] = right_tree;
}



bool bv_tree::is_inside(const vector3f& v) const
{
	if (!volume.is_inside(v))
		return false;
	for (int i = 0; i < 2; ++i)
		if (children[i].get())
			if (children[i]->is_inside(v))
				return true;
	return false;
}



bool bv_tree::collides(const bv_tree& other, std::list<vector3f>& contact_points) const
{
	return false; // fixme
}



bool bv_tree::collides(const bv_tree& other, const matrix4f& other_transform, std::list<vector3f>& contact_points) const
{
	return false; // fixme
}



void bv_tree::transform(const matrix4f& mat)
{
	volume.center = mat.mul4vec3xlat(volume.center);
	for (int i = 0; i < 2; ++i)
		if (children[i].get())
			children[i]->transform(mat);
}



void bv_tree::compute_min_max(vector3f& minv, vector3f& maxv) const
{
	volume.compute_min_max(minv, maxv);
	for (int i = 0; i < 2; ++i)
		if (children[i].get())
			children[i]->compute_min_max(minv, maxv);
}



void bv_tree::debug_dump(unsigned level) const
{
	for (unsigned i = 0; i < level; ++i)
		std::cout << "\t";
	std::cout << "Level " << level << " Sphere " << volume.center << " | " << volume.radius << "\n";
	for (int i = 0; i < 2; ++i)
		if (children[i].get())
			children[i]->debug_dump(level + 1);
}
