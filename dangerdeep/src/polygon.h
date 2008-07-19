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
//  A generic polygon (C)+(W) 2005 Thorsten Jordan
//

#ifndef POLYGON_H
#define POLYGON_H

#include "plane.h"
#include <vector>
#include <iostream>

#ifdef USE_NATIVE_GL
#include <gl.h>
#else
#include "oglext/OglExt.h"
#endif

template<class D>
class polygon_t
{
public:
	std::vector<vector3t<D> > points;
	/// empty polygon
	polygon_t() {}
	/// polygon with prepared space
	polygon_t(unsigned capacity_) { points.reserve(capacity_); }
	/// make from three points.
	polygon_t(const vector3t<D>& a, const vector3t<D>& b, const vector3t<D>& c) {
		points.reserve(3);
		points.push_back(a);
		points.push_back(b);
		points.push_back(c);
	}
	/// make from four points
	polygon_t(const vector3t<D>& a, const vector3t<D>& b, const vector3t<D>& c, const vector3t<D>& d) {
		points.reserve(4);
		points.push_back(a);
		points.push_back(b);
		points.push_back(c);
		points.push_back(d);
	}
	/// check if polygon is empty. Each polygon must have at least three vertices or it is empty
	bool empty() const { return points.size() < 3; }
	/// compute normal of polygon. Only valid if all points are in a plane.
	vector3t<D> normal() const {
		if (empty()) return vector3t<D>();
		return (points[1] - points[0]).cross(points[2] - points[0]).normal();
	}
	/// clip polygon against plane.
	polygon_t clip(const plane_t<D>& plan) const {
		// do not clip empty polygons.
		if (empty()) return polygon_t();
		// test for each point if it is left of the plane.
		std::vector<int> pside(points.size());
		unsigned pside_ctr[3] = { 0, 0, 0 };	// count #points on right/plane/left side.
		for (unsigned i = 0; i < points.size(); ++i) {
			int pli = plan.test_side(points[i]);
			pside[i] = pli;
			++pside_ctr[pli + 1];
		}
		// we can have at most two clip points.
		// this means all points are splitted in at most two sets (left / right).
		// if all points are left, return *this, if all are right, return an empty polygon.
		if (pside_ctr[0] == 0)
			return *this;	// no points are right.
		if (pside_ctr[2] == 0)
			return polygon_t();	// no points are left.
		// now we have the case that this polygon really intersects the plane (or touches it...)
		// This means there must be at least one point left of the plane and one point right
		// of the plane, both points are NOT ON THE PLANE, BUT REALLY OUTSIDE.
		// this means the result has at most #left + #onplane + 2 points.
		polygon_t result(pside_ctr[1] + pside_ctr[2] + 2);
		// copy all points that are left of the plane to the result
		// insert the two intersection points.
		// consider points on the plane to be "left" of it.
		for (unsigned i = 0; i < points.size(); ++i) {
			unsigned j = (i+1) % points.size();
			if (pside[i] >= 0) {
				if (pside[j] >= 0) {
					// both points are left or on plane, just copy this point
					result.points.push_back(points[i]);
				} else {
					// next point is right, insert this point and intersection point
					result.points.push_back(points[i]);
					if (pside[i] > 0) {
						vector3t<D> interp = plan.intersection(points[i], points[j]);
						result.points.push_back(interp);
					}
				}
			} else {
				if (pside[j] > 0) {
					// next point is left, insert intersection point
					vector3t<D> interp = plan.intersection(points[i], points[j]);
					result.points.push_back(interp);
				} else if (pside[j] == 0) {
					// next point is on the plane, do nothing.
				} else {
					// both points are right, do nothing
				}
			}
		}
		// avoid double points! we do that now... so the check is obsolete.
		/*
		for (unsigned i = 0; i < result.points.size(); ++i) {
			unsigned j = (i+1) % result.points.size();
			if (result.points[i].distance(result.points[j]) < 0.001) {
			     std::cout << "Warning: double points? " << result.points[i] << " | " << result.points[j] << "\n";
			}
		}
		*/
		return result;
	}
	/// print polygon
	void print() const {
		std::cout << "Poly, pts=" << points.size() << "\n";
		for (unsigned i = 0; i < points.size(); ++i)
			std::cout << "P[" << i << "] = " << points[i] << "\n";
	}
	/// render polygon
	void draw() const
	{
		glBegin(GL_LINE_LOOP);
		for (unsigned i = 0; i < points.size(); ++i)
			glVertex3d(points[i].x, points[i].y, points[i].z);
		glEnd();
	}
	/// compute plane that poly lies in
	plane_t<D> get_plane() const {
		if (empty())
			return plane_t<D>();
		return plane_t<D>(points[0], points[1], points[2]);
	}
	/// translate
	void translate(const vector3t<D>& delta) {
		for (unsigned i = 0; i < points.size(); ++i)
			points[i] += delta;
	}
};

typedef polygon_t<double> polygon;
typedef polygon_t<float> polygonf;

#endif
