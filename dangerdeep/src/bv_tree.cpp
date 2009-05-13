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
		result.reset(new bv_tree(spheref(ta->sphere.compute_bound(tb->sphere)), unsigned(-1)));
		result->children[0] = nodes.release_front();
		result->children[1] = nodes.release_back();
		return result;
	}
	// compute bounds in x,y,z axis directions
	vector3f minv(1e30, 1e30, 1e30);
	vector3f maxv = -minv;
	vector3f center;
	for (ptrlist<bv_tree>::const_iterator it = nodes.begin(); it != nodes.end(); ++it) {
		it->compute_min_max(minv, maxv);
		center += it->sphere.center;
	}
	center *= 1.0/nodes.size();
	vector3f deltav = maxv - minv;
	// chose axis with longest value range, sort along that axis,
	// split in gravity center of nodes.
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
	ptrlist<bv_tree> left_nodes, right_nodes;
	float vcenter[3] = { center.x, center.y, center.z };
	for (ptrlist<bv_tree>::iterator it = nodes.nc_begin(); it != nodes.nc_end(); ++it) {
		const vector3f& c = it->sphere.center;
		float vc[3] = { c.x, c.y, c.z };
		if (vc[split_axis] < vcenter[split_axis])
			left_nodes.push_back(it.release());
		else
			right_nodes.push_back(it.release());
	}
	result.reset(new bv_tree(create(left_nodes), create(right_nodes)));
	return result;
}



bv_tree::bv_tree(std::auto_ptr<bv_tree> left_tree, std::auto_ptr<bv_tree> right_tree)
	: sphere(left_tree->sphere.compute_bound(right_tree->sphere)), triangle(unsigned(-1))
{
	children[0] = left_tree;
	children[1] = right_tree;
}



bool bv_tree::is_inside(const vector3f& v) const
{
	if (!sphere.is_inside(v))
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
	sphere.center = mat.mul4vec3xlat(sphere.center);
	for (int i = 0; i < 2; ++i)
		if (children[i].get())
			children[i]->transform(mat);
}



void bv_tree::compute_min_max(vector3f& minv, vector3f& maxv) const
{
	sphere.compute_min_max(minv, maxv);
	for (int i = 0; i < 2; ++i)
		if (children[i].get())
			children[i]->compute_min_max(minv, maxv);
}
