// depth charges
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "gun_shell.h"
#include "global_data.h"
#include "model.h"
#include "system.h"
#include "game.h"

gun_shell::gun_shell(const sea_object& parent, angle direction, angle elevation,
	double initial_velocity) : sea_object()
{
	position = parent.get_pos();	// fixme: calc correct position
	oldpos = position;
	vector2 d = direction.direction() * elevation.cos() * initial_velocity;
	velocity = vector3(d.x, d.y, elevation.sin() * initial_velocity);
	//fixme: should be done in sea_object!
	size3d = vector3f(gun_shell_mdl->get_width(), gun_shell_mdl->get_length(), gun_shell_mdl->get_height());

	system::sys().add_console("shell created");
}

void gun_shell::load(istream& in, class game& g)
{
	sea_object::load(in, g);
	oldpos = read_vector3(in);
}

void gun_shell::save(ostream& out, const class game& g) const
{
	sea_object::save(out, g);
	write_vector3(out, oldpos);
}

void gun_shell::simulate(game& gm, double delta_time)
{
	oldpos = position;
	sea_object::simulate(gm, delta_time);

	if (position.z <= 0) {
		bool impact = gm.gs_impact(position);
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
