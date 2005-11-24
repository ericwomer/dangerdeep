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
	oldpos = position;
	damage_amount = damage;
	vector2 d = direction.direction() * elevation.cos() * initial_velocity;
	velocity = vector3(d.x, d.y, elevation.sin() * initial_velocity);

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
	if (is_defunct()) return;

	// very crude, fixme. compute intersection of line oldpos->position with objects.
	if (position.z <= 0) {
		bool impact = gm.gs_impact(this);
		kill();
	}
}

void gun_shell::display(void) const
{
	// direction of shell is equal to normalized velocity vector.
	// so compute a rotation matrix from velocity and multiply it
	// onto the current modelview matrix.
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
	gun_shell_mdl->display();
	glPopMatrix();
}

float gun_shell::surface_visibility(const vector2& watcher) const
{
	return 100.0f;	// square meters... test hack
}
