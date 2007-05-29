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

gun_shell::gun_shell(game& gm_)
	: sea_object(gm_, "gun_shell.3ds"), damage_amount(0)
{
	// for loading
}



gun_shell::gun_shell(game& gm_, const vector3& pos, angle direction, angle elevation,
	double initial_velocity, double damage)
	: sea_object(gm_, "gun_shell.3ds")
{
	position = pos;
	orientation = quaternion::rot(direction.value(), 0, 0, 1);
	impulse = mass * vector3(0, 0, elevation.sin() * initial_velocity);
	angular_momentum = vector3();
	compute_helper_values();
	oldpos = position;
	damage_amount = damage;

	sys().add_console("shell created");
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



void gun_shell::simulate(double delta_time)
{
	oldpos = position;
	sea_object::simulate(delta_time);

	// very crude, fixme. compute intersection of line oldpos->position with objects.
	if (position.z <= 0) {
		// 2006-11-30 we need to check impact yes or no
		/*bool impact = */gm.gs_impact(this);
		kill();
	}
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
