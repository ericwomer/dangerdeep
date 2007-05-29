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

#include "depth_charge.h"
#include "global_data.h"
#include "model.h"
#include "system.h"
#include "game.h"
#include "environment.h"



depth_charge::depth_charge(game& gm_)
	: sea_object(gm_, "depth_charge.3ds"), explosion_depth(0)
{
	// for loading
}



depth_charge::depth_charge(game& gm_, double expl_depth, const vector3& pos)
	: sea_object(gm_, "depth_charge.3ds"), explosion_depth(expl_depth)
{
	// fixme depends on parent! and parent's size, dc's can be thrown, etc.!
	position = pos;
	system::sys().add_console("depth charge created");
}



void depth_charge::load(const xml_elem& parent)
{
	sea_object::load(parent);
	explosion_depth = parent.child("explosion_depth").attrf();
}



void depth_charge::save(xml_elem& parent) const
{
	sea_object::save(parent);
	parent.add_child("explosion_depth").set_attr(explosion_depth);
}



void depth_charge::simulate(double delta_time)
{
	sea_object::simulate(delta_time);

	if (position.z < -explosion_depth) {
		gm.dc_explosion(*this);
		kill();	// dc is "dead"
	}
}



void depth_charge::compute_force_and_torque(vector3& F, vector3& T) const
{
	// force is in world space!
	if (position.z > 0) {	// DC's can be thrown, so they can be above water.
		F.z = -GRAVITY * mass;
	} else {
		double vm = velocity.z/DEPTH_CHARGE_SINK_SPEED;
		F.z = (-GRAVITY + GRAVITY*vm*vm) * mass;
	}
}
