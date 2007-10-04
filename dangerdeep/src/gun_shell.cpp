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

// depth charges
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "gun_shell.h"
#include "global_data.h"
#include "model.h"
#include "system.h"
#include "game.h"
#include "ship.h"
#include "log.h"

gun_shell::gun_shell(game& gm_)
	: sea_object(gm_, "gun_shell.3ds"), damage_amount(0)
{
	// for loading
	mass = 20;
	mass_inv = 1.0/mass;
}



gun_shell::gun_shell(game& gm_, const vector3& pos, angle direction, angle elevation,
	double initial_velocity, double damage)
	: sea_object(gm_, "gun_shell.3ds")
{
	orientation = quaternion::rot(-direction.value(), 0, 0, 1);
	mass = 20;
	mass_inv = 1.0/mass;
	linear_momentum = mass * vector3(0,
					 elevation.cos() * initial_velocity,
					 elevation.sin() * initial_velocity);
	// set off initial pos. like 0.1 seconds after firing, to avoid
	// collision with parent
	position = pos + orientation.rotate(linear_momentum * (mass_inv * 0.1));
	angular_momentum = vector3();
	compute_helper_values();
	oldpos = position;
	damage_amount = damage;

	log_info("shell created");
}



void gun_shell::load(const xml_elem& parent)
{
	sea_object::load(parent);
	oldpos = parent.child("oldpos").attrv3();
	damage_amount = parent.child("damage_amount").attrf();
}



void gun_shell::save(xml_elem& parent) const
{
	sea_object::save(parent);
	parent.add_child("oldpos").set_attr(oldpos);
	parent.add_child("damage_amount").set_attr(damage_amount);
}



void gun_shell::check_collision()
{
	/* For gun shells we need to check for intersection of a line to all ships.
	   The line is determined by the movement of the shell between two simulation
	   steps. Let it be b + t * d, where b, d are vectors and d has length 1.
	   The bounding sphere is given by point p and radius r.
	   Thus the line hits the sphere when ||b + t*d - p|| = r and t in [0...q]
	   Which can be computed so: k := b - p
	   t1,2 = -<k,d> +- sqrt( <k,d>^2 - <k,k> + r^2)
	   Either it has no solution or t is not in [0...q] then the line does not
	   intersect. Where q = length of the line, i.e. length of d before
	   normalizing it. Additionally if t1,2 have a different sign, the whole
	   part of the line is inside the sphere.
	   To check for line<->bbox intersection we cut the line by the six planes
	   defining the bbox, plane a*x+b*y+c*z+d=0 and line bv + t * dv
	   then solve t = ((a,b,c)*bv-d) / ((a,b,c)*dv).
	   If we can't solve it, the line does not intersect the plane.
	   Depending on wether bv is left or right of the plane, t gives new upper
	   or lower bound. Keep two values u0,u1 defining total upper and lower bound,
	   and compute min/max of them and t. If the resulting lower bound is >= upper
	   bound, the line does not intersect the box.
	   For finer check (voxel<->line) we can again compute line<->sphere
	   intersections. With that we can compute shell hits very precisely.
	   On the other hand we can compute which voxels intersect the line because we
	   know the position and order of the voxels, without checking every voxel...
	   We know where the ray enters the object and thus the entry voxel, from that
	   we can follow by raycasting through the voxels without the need to check
	   for intersection between every voxel and the ray. This is maybe more code
	   but it is much more efficient.
	   We need to check for intersection of shell with water surface too.
	   It is sufficient to compute wether the new position is below water surface.
	   That is, get the water height at its xy pos and compare to its z pos.
	   The shells only fall down and start above the water.
	   It may happen then that a shell explodes below the water and not exactly at
	   the surface, but this doesn't matter and is in fact realistic.
	*/
	vector3 dv = position - oldpos;
	// avoid NaN on first round
	double dvl = dv.square_length();
	if (dvl < 1e-8)
		return;
	dvl = sqrt(dvl);
	dv = dv * (1.0/dvl);
	std::vector<const ship*> allships = gm.get_all_ships();
	for (unsigned i = 0; i < allships.size(); ++i) {
		const ship* s = allships[i];
		vector3 k = s->get_pos() - oldpos;
		double kd = k * dv;
		double r = s->get_bounding_radius();
		double tmp = kd*kd - k*k + r*r;
		if (tmp <= 0.0)
			continue;
		tmp = sqrt(tmp);
		double t0 = -kd + tmp, t1 = -kd - tmp;
		if (t0*t1 < 0.0 || t0 >= 0.0 && t0 <= dvl || t1 >= 0.0 && t1 <= dvl) {
			log_debug("gun_shell "<<this<<" intersects bsphere of "<<s);
		}
	}

}



void gun_shell::simulate(double delta_time)
{
	check_collision();
	oldpos = position;
	log_debug("GS: position="<<position);
	sea_object::simulate(delta_time);

#if 0
	// very crude, fixme. compute intersection of line oldpos->position with objects.
	//fixme: with new physics this leads to bugs, because a shell may be z<0 instantly
	//when sub is low in water!
	if (position.z <= -10) {
		// 2006-11-30 we need to check impact yes or no
		/*bool impact = */gm.gs_impact(this);
		kill();
	}
#endif
}



void gun_shell::display() const
{
	// direction of shell is equal to normalized velocity vector.
	// so compute a rotation matrix from velocity and multiply it
	// onto the current modelview matrix.
	//fixme: using orientation should do the trick!
	vector3 vn = velocity.normal();
	vector3 up = vector3(0, 0, 1);
	vector3 side = vn.orthogonal(up);
	up = side.orthogonal(vn);
	float m[16] = { side.x, side.y, side.z, 0,
			vn.x, vn.y, vn.z, 0,
			up.x, up.y, up.z, 0,
			0, 0, 0, 1 };
	glPushMatrix();
	glMultMatrixf(m);
	sea_object::display();
	glPopMatrix();
}



float gun_shell::surface_visibility(const vector2& watcher) const
{
	return 100.0f;	// square meters... test hack
}
