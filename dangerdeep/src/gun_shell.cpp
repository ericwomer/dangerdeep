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
	length = gun_shell_mdl->get_length();
	width = gun_shell_mdl->get_width();

	system::sys()->add_console("shell created");
	vis_cross_section_factor = CROSS_SECTION_VIS_NULL;
}

void gun_shell::load(istream& in, class game& g)
{
	sea_object::load(in, g);
	velocity.x = read_double(in);
	velocity.y = read_double(in);
	velocity.z = read_double(in);
	oldpos.x = read_double(in);
	oldpos.y = read_double(in);
	oldpos.z = read_double(in);
}

void gun_shell::save(ostream& out, const class game& g) const
{
	sea_object::save(out, g);
	write_double(out, velocity.x);
	write_double(out, velocity.y);
	write_double(out, velocity.z);
	write_double(out, oldpos.x);
	write_double(out, oldpos.y);
	write_double(out, oldpos.z);
}

void gun_shell::simulate(game& gm, double delta_time)
{
	oldpos = position;
	position += velocity * delta_time;
//	velocity *= (1.0 - AIR_RESISTANCE) * delta_time;	// fixme: other formula?
	velocity += vector3(0, 0, -GRAVITY) * delta_time;

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
